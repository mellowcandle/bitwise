// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bitwise.h"
#include "shunting-yard.h"
#include "stack.h"

typedef enum {
	TOKEN_NONE,
	TOKEN_UNKNOWN,
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_OPERATOR,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER
} TokenType;

typedef struct {
	TokenType type;
	char *value;
} Token;

typedef enum {
	OPERATOR_OTHER,
	OPERATOR_UNARY,
	OPERATOR_BINARY
} OperatorArity;

typedef enum {
	OPERATOR_NONE,
	OPERATOR_LEFT,
	OPERATOR_RIGHT
} OperatorAssociativity;

typedef struct {
	char *symbol;
	int op_len;
	int precedence;
	OperatorArity arity;
	OperatorAssociativity associativity;
} Operator;

/* Width, in bits, of the type every expression is evaluated in. */
#define VALUE_BITS (sizeof(uint64_t) * CHAR_BIT)

static const Token NO_TOKEN = {TOKEN_NONE, NULL};

/*
 * get_operator() matches on the first op_len characters of the token, so
 * the single-character entries also claim their "&=", "^=" and "|=" forms:
 * those tokens find "&", "^" and "|" here and evaluate identically, which
 * is what this calculator means by them anyway -- there is nothing to
 * assign to. Do not add compound-assignment rows; they would sit after the
 * single-character ones and never be reached.
 */
static const Operator OPERATORS[] = {
	{"!", 1, 1, OPERATOR_UNARY,  OPERATOR_RIGHT},
	{"~", 1, 1, OPERATOR_UNARY,  OPERATOR_RIGHT},
	/*
	 * "+" and "-" appear twice: get_arity() decides from the preceding
	 * token which form a given occurrence is, and get_operator() then
	 * matches on the symbol *and* that arity. Without the unary rows the
	 * lookup returned NULL and "-5", "5 * -2" and "10 - -2" were all
	 * rejected as syntax errors, even though apply_unary_operator()
	 * has always known how to evaluate them.
	 */
	{"+", 1, 1, OPERATOR_UNARY,  OPERATOR_RIGHT},
	{"-", 1, 1, OPERATOR_UNARY,  OPERATOR_RIGHT},
	{"*", 1, 2, OPERATOR_BINARY, OPERATOR_LEFT},
	{"/", 1, 2, OPERATOR_BINARY, OPERATOR_LEFT},
	{"%", 1, 2, OPERATOR_BINARY, OPERATOR_LEFT},
	{"+", 1, 3, OPERATOR_BINARY, OPERATOR_LEFT},
	{"-", 1, 3, OPERATOR_BINARY, OPERATOR_LEFT},
	{">>", 2, 4, OPERATOR_BINARY, OPERATOR_LEFT},
	{"<<", 2, 4, OPERATOR_BINARY, OPERATOR_LEFT},
	{"&", 1, 5, OPERATOR_BINARY, OPERATOR_LEFT},
	{"^", 1, 6, OPERATOR_BINARY, OPERATOR_LEFT},
	{"|", 1, 7, OPERATOR_BINARY, OPERATOR_LEFT},
	/*
	 * "(" must stay strictly looser than every real operator. It is a
	 * sentinel, not something apply_operator() can evaluate:
	 * push_operator() stops popping once it compares as tighter than the
	 * incoming operator, and if it ever ties instead, the open
	 * parenthesis is popped and applied, which fails with
	 * ERROR_UNRECOGNIZED. Bump this when adding a looser operator.
	 */
	{"(", 1, 8, OPERATOR_OTHER,  OPERATOR_NONE}
};

// Returns an array of tokens extracted from the expression. The array is
// terminated by a token with type `TOKEN_NONE`.
static Token *tokenize(const char *expression);

// Parses a tokenized expression.
static Status parse(const Token *tokens, Stack **operands, Stack **operators,
                    Stack **functions);

// Pushes an operator to the stack after applying operators with a higher
// precedence.
static Status push_operator(const Operator *operator, Stack **operands,
                            Stack **operators);

// Pushes the multiplication operator to the stack.
static Status push_multiplication(Stack **operands, Stack **operators);

// Allocates memory for a double and pushes it to the stack.
static void push_num(uint64_t x, Stack **operands);

// Pops a double from the stack, frees its memory and returns its value.
static uint64_t pop_num(Stack **operands);

// Converts a string into a number and pushes it to the stack.
static Status push_number(const char *value, Stack **operands);

// Converts a constant identifier into its value and pushes it to the stack.
static Status push_constant(const char *value, Stack **operands);

// Applies an operator to the top one or two operands, depending on if the
// operator is unary or binary.
static Status apply_operator(const Operator *operator, Stack **operands);

// Applies a unary operator to the top operand.
static Status apply_unary_operator(const Operator *operator, Stack **operands);

// Applies a function to the top operand.
static Status apply_function(const char *function, Stack **operands);

// Returns the arity of an operator, using the previous token for context.
static OperatorArity get_arity(char *symbol, const Token *previous);

// Returns a matching operator.
static const Operator *get_operator(char *symbol, OperatorArity arity);

Status shunting_yard(const char *expression, uint64_t *result)
{
	Token *tokens = tokenize(expression);
	Stack *operands = NULL, *operators = NULL, *functions = NULL;
	Status status = parse(tokens, &operands, &operators, &functions);
	if (operands)
		*result = pop_num(&operands);
	else if (status == STATUS_OK)
		status = ERROR_NO_INPUT;

	for (Token *token = tokens; token->type != TOKEN_NONE; token++)
		free(token->value);
	free(tokens);
	while (operands)
		pop_num(&operands);
	while (operators)
		stack_pop(&operators);
	while (functions)
		stack_pop(&functions);
	return status;
}

/*
 * Character sets for the number and identifier tokens. Spans are measured
 * with strspn() rather than scanned into a fixed-size buffer, so an
 * arbitrarily long token can neither overflow nor be silently truncated.
 */
static const char NUMBER_CHARS[] = "xX0123456789abcdefABCDEF.";
static const char IDENTIFIER_CHARS[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz$";

Token *tokenize(const char *expression)
{
	int length = 0;
	Token *tokens = malloc(sizeof * tokens);
	const char *c = expression;
	while (*c) {
		size_t span;
		Token token = {TOKEN_UNKNOWN, NULL};

		if (*c == '(')
			token.type = TOKEN_OPEN_PARENTHESIS;
		else if (*c == ')')
			token.type = TOKEN_CLOSE_PARENTHESIS;
		else if (!strncmp("<<", c, 2) || !strncmp(">>", c, 2)) {
			token.type = TOKEN_OPERATOR;
			token.value = strndup(c, 2);
		} else if (strchr("^|&", *c) && *(c + 1) == '=') {
			token.type = TOKEN_OPERATOR;
			token.value = strndup(c, 2);
		} else if (strchr("~!^*/%+-|&", *c)) {
			token.type = TOKEN_OPERATOR;
			token.value = strndup(c, 1);
		} else if (!strncmp("bit", c, 3) || !strncmp("BIT", c, 3)) {
			token.value = strndup(c, 3);
			token.type = TOKEN_IDENTIFIER;
		} else if ((span = strspn(c, NUMBER_CHARS)) != 0) {
			token.type = TOKEN_NUMBER;
			token.value = strndup(c, span);
		} else if ((span = strspn(c, IDENTIFIER_CHARS)) != 0) {
			token.type = TOKEN_IDENTIFIER;
			token.value = strndup(c, span);
		}

		if (!isspace((unsigned char)*c)) {
			tokens = realloc(tokens, sizeof * tokens *
					 (++length + 1));
			tokens[length - 1] = token;
		}
		c += token.value ? strlen(token.value) : 1;
	}
	tokens[length] = NO_TOKEN;

	return tokens;
}

Status parse(const Token *tokens, Stack **operands, Stack **operators,
             Stack **functions)
{
	Status status = STATUS_OK;

	for (const Token *token = tokens, *previous = &NO_TOKEN, *next = token + 1;
	     token->type != TOKEN_NONE; previous = token, token = next++) {
		switch (token->type) {
		case TOKEN_OPEN_PARENTHESIS:
			// Implicit multiplication: "(2)(2)".
			if (previous->type == TOKEN_CLOSE_PARENTHESIS)
				status = push_multiplication(operands, operators);

			stack_push(operators, get_operator("(", OPERATOR_OTHER));
			break;

		case TOKEN_CLOSE_PARENTHESIS: {
			// Apply operators until the previous open parenthesis is found.
			bool found_parenthesis = false;
			while (*operators && status == STATUS_OK && !found_parenthesis) {
				const Operator *operator = stack_pop(operators);
				if (!strncmp(operator->symbol, "(", 1))
					found_parenthesis = true;
				else
					status = apply_operator(operator, operands);
			}
			if (!found_parenthesis)
				status = ERROR_CLOSE_PARENTHESIS;
			else if (*functions)
				status = apply_function(stack_pop(functions), operands);
			break;
		}

		case TOKEN_OPERATOR:
			status = push_operator(
			             get_operator(token->value,
			                          get_arity(token->value, previous)),
			             operands, operators);
			break;

		case TOKEN_NUMBER:
			if (previous->type == TOKEN_CLOSE_PARENTHESIS ||
			    previous->type == TOKEN_NUMBER ||
			    previous->type == TOKEN_IDENTIFIER)
				status = ERROR_SYNTAX;
			else {
				status = push_number(token->value, operands);

				// Implicit multiplication: "2(2)" or "2a".
				if (next->type == TOKEN_OPEN_PARENTHESIS ||
				    next->type == TOKEN_IDENTIFIER)
					status = push_multiplication(operands, operators);
			}
			break;

		case TOKEN_IDENTIFIER:
			// The identifier could be either a constant or function.
			status = push_constant(token->value, operands);
			if ((status == ERROR_UNDEFINED_CONSTANT) &&
			     (next->type == TOKEN_OPEN_PARENTHESIS)) {
				stack_push(functions, token->value);
				status = STATUS_OK;
			} else if (next->type == TOKEN_OPEN_PARENTHESIS ||
			           next->type == TOKEN_IDENTIFIER) {
				// Implicit multiplication: "a(2)" or "a b".
				status = push_multiplication(operands, operators);
			}
			break;

		default:
			status = ERROR_UNRECOGNIZED;
		}
		if (status != STATUS_OK)
			return status;
	}

	// Apply all remaining operators.
	while (*operators && status == STATUS_OK) {
		const Operator *operator = stack_pop(operators);
		if (!strncmp(operator->symbol, "(", 1))
			status = ERROR_OPEN_PARENTHESIS;
		else
			status = apply_operator(operator, operands);
	}
	return status;
}

Status push_operator(const Operator *operator, Stack **operands,
                     Stack **operators)
{
	if (!operator)
		return ERROR_SYNTAX;

	Status status = STATUS_OK;
	while (*operators && status == STATUS_OK) {
		const Operator *stack_operator = stack_top(*operators);
		if (operator->arity == OPERATOR_UNARY ||
		    operator->precedence < stack_operator->precedence ||
		    (operator->associativity == OPERATOR_RIGHT &&
		     operator->precedence == stack_operator->precedence))
			break;

		status = apply_operator(stack_pop(operators), operands);
	}
	stack_push(operators, operator);
	return status;
}

Status push_multiplication(Stack **operands, Stack **operators)
{
	return push_operator(get_operator("*", OPERATOR_BINARY), operands,
	                     operators);
}

void push_num(uint64_t x, Stack **operands)
{
	uint64_t *pointer = malloc(sizeof * pointer);
	*pointer = x;
	stack_push(operands, pointer);
}

uint64_t pop_num(Stack **operands)
{
	const uint64_t *pointer = stack_pop(operands);
	uint64_t x = *pointer;
	free((void *)pointer);
	return x;
}

Status push_number(const char *value, Stack **operands)
{
	uint64_t x;

	if (parse_input(value, &x))
		return ERROR_SYNTAX;

	push_num(x, operands);
	return STATUS_OK;
}

Status push_constant(const char *value, Stack **operands)
{
	uint64_t x;

	if (strncmp(value, "$", 1) == 0)
		x = g_val;
	else
		return ERROR_UNDEFINED_CONSTANT;

	push_num(x, operands);
	return STATUS_OK;
}

Status apply_operator(const Operator *operator, Stack **operands)
{
	if (!operator || !*operands)
		return ERROR_SYNTAX;
	if (operator->arity == OPERATOR_UNARY)
		return apply_unary_operator(operator, operands);

	uint64_t y = pop_num(operands);
	if (!*operands)
		return ERROR_SYNTAX;
	uint64_t x = pop_num(operands);
	Status status = STATUS_OK;
	switch (*operator->symbol) {
	case '*':
		x = x * y;
		break;
	case '/':
		if (y == 0)
			return ERROR_DIVIDE_BY_ZERO;
		x = x / y;
		break;
	case '%':
		if (y == 0)
			return ERROR_DIVIDE_BY_ZERO;
		x = x % y;
		break;
	case '+':
		x = x + y;
		break;
	case '-':
		x = x - y;
		break;
	/*
	 * Shifting by more than the operand width is undefined behaviour in
	 * C; every bit is shifted out, so yield the expected 0 instead.
	 * Shifts smaller than that are left alone: the caller masks the
	 * result down to g_width and reports the overflow itself.
	 */
	case '>':
		x = (y >= VALUE_BITS) ? 0 : x >> y;
		break;
	case '<':
		x = (y >= VALUE_BITS) ? 0 : x << y;
		break;
	case '|':
		x = x | y;
		break;
	case '&':
		x = x & y;
		break;
	case '^':
		x = x ^ y;
		break;
	default:
		return ERROR_UNRECOGNIZED;
	}
	push_num(x, operands);
	return status;
}

Status apply_unary_operator(const Operator *operator, Stack **operands)
{
	uint64_t x = pop_num(operands);
	switch (*operator->symbol) {
	case '+':
		break;
	case '-':
		x = -x;
		break;
	case '~':
		x = ~x;
		if (g_width)
			x &= MASK(g_width);
		break;
	case '!':
		x = !x;
		break;
	default:
		return ERROR_UNRECOGNIZED;
	}
	push_num(x, operands);
	return STATUS_OK;
}

Status apply_function(const char *function, Stack **operands)
{
	if (!*operands)
		return ERROR_FUNCTION_ARGUMENTS;

	uint64_t x = pop_num(operands);

	if (strcasecmp(function, "bit") == 0)
		/*
		 * Same guard as the shift operators: BIT() is a shift, and a
		 * bit index at or past the value width shifts every bit out.
		 */
		x = (x >= VALUE_BITS) ? 0 : BIT(x);
	else
		return ERROR_UNDEFINED_FUNCTION;

	push_num(x, operands);
	return STATUS_OK;
}

OperatorArity get_arity(char *symbol, const Token *previous)
{
	if (*symbol == '!' || previous->type == TOKEN_NONE ||
	    previous->type == TOKEN_OPEN_PARENTHESIS ||
	    (previous->type == TOKEN_OPERATOR && *previous->value != '!'))
		return OPERATOR_UNARY;
	return OPERATOR_BINARY;
}

const Operator *get_operator(char *symbol, OperatorArity arity)
{
	for (size_t i = 0; i < sizeof OPERATORS / sizeof OPERATORS[0]; i++) {
		if (!strncmp(OPERATORS[i].symbol, symbol, OPERATORS[i].op_len) &&
		    OPERATORS[i].arity == arity)
			return &OPERATORS[i];
	}

	LOG("couldn't find %s operator\n", symbol);
	return NULL;
}
