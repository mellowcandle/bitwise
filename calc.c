#include "bitwise.h"

enum {
	ASSOC_NONE=0,
	ASSOC_LEFT,
	ASSOC_RIGHT
};

static uint64_t eval_uminus(uint64_t a1, uint64_t a2)
{
	return -a1;
}

static uint64_t eval_exp(uint64_t a1, uint64_t a2)
{
	return a2<0 ? 0 : (a2==0?1:a1*eval_exp(a1, a2-1));
}
static uint64_t eval_mul(uint64_t a1, uint64_t a2)
{
	return a1*a2;
}
static uint64_t eval_div(uint64_t a1, uint64_t a2)
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
		exit(EXIT_FAILURE);
	}
	return a1/a2;
}

static uint64_t eval_mod(uint64_t a1, uint64_t a2)
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
		exit(EXIT_FAILURE);
	}
	return a1%a2;
}

static uint64_t eval_add(uint64_t a1, uint64_t a2)
{
	return a1+a2;
}

static uint64_t eval_sub(uint64_t a1, uint64_t a2)
{
	return a1-a2;
}

struct op_s {
	char op;
	int prec;
	int assoc;
	int unary;
	uint64_t (*eval)(uint64_t a1, uint64_t a2);
} ops[]={
	{'_', 10, ASSOC_RIGHT, 1, eval_uminus},
	{'^', 9, ASSOC_RIGHT, 0, eval_exp},
	{'*', 8, ASSOC_LEFT, 0, eval_mul},
	{'/', 8, ASSOC_LEFT, 0, eval_div},
	{'%', 8, ASSOC_LEFT, 0, eval_mod},
	{'+', 5, ASSOC_LEFT, 0, eval_add},
	{'-', 5, ASSOC_LEFT, 0, eval_sub},
	{'(', 0, ASSOC_NONE, 0, NULL},
	{')', 0, ASSOC_NONE, 0, NULL}
};

#define MAXOPSTACK 10
static struct op_s *opstack[MAXOPSTACK];
static int nopstack=0;
#define MAXNUMSTACK 10
static uint64_t numstack[MAXNUMSTACK];
static int nnumstack=0;

static struct op_s *getop(char ch)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(ops); i++) {
		if(ops[i].op == ch) return ops + i;
	}

	return NULL;
}

static int push_opstack(struct op_s *op)
{
	if(nopstack > MAXOPSTACK-1) {
		LOG( "ERROR: Operator stack overflow\n");
		return -1;
	}
	opstack[nopstack++]=op;

	return 0;
}

static struct op_s *pop_opstack()
{
	if(!nopstack) {
		LOG("ERROR: Operator stack empty\n");
		return NULL;
	}
	return opstack[--nopstack];
}

static int push_numstack(uint64_t num)
{
	if(nnumstack > MAXNUMSTACK - 1) {
		LOG("ERROR: Number stack overflow\n");
		return -1;
	}
	numstack[nnumstack++] = num;

	return 0;
}

static uint64_t pop_numstack()
{
	if(!nnumstack) {
		LOG("ERROR: Number stack empty\n");
		exit(EXIT_FAILURE);
	}
	return numstack[--nnumstack];
}

void shunt_op(struct op_s *op)
{
	struct op_s *pop;
	int n1, n2;
	if(op->op=='(') {
		push_opstack(op);
		return;
	} else if(op->op==')') {
		while(nopstack>0 && opstack[nopstack-1]->op!='(') {
			pop=pop_opstack();
			n1=pop_numstack();
			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
		if(!(pop=pop_opstack()) || pop->op!='(') {
			fprintf(stderr, "ERROR: Stack error. No matching \'(\'\n");
			exit(EXIT_FAILURE);
		}
		return;
	}

	if(op->assoc==ASSOC_RIGHT) {
		while(nopstack && op->prec<opstack[nopstack-1]->prec) {
			pop=pop_opstack();
			n1=pop_numstack();
			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	} else {
		while(nopstack && op->prec<=opstack[nopstack-1]->prec) {
			pop=pop_opstack();
			n1=pop_numstack();
			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	}
	push_opstack(op);
}

int calc(int argc, char *argv[])
{
	char *expr;
	int i = 0;
	char *tstart=NULL;
	struct op_s startop={'X', 0, ASSOC_NONE, 0, NULL};	/* Dummy operator to mark start */
	struct op_s *op=NULL;
	int n1, n2;
	struct op_s *lastop=&startop;

	for (i = 0; i < argc; i++)
		if(!tstart) {
			if((op=getop(*argv[i]))) {
				if(lastop && (lastop==&startop || lastop->op!=')')) {
					if(op->op=='-') op=getop('_');
					else if(op->op!='(') {
						LOG( "ERROR: Illegal use of binary operator (%c)\n", op->op);
						exit(EXIT_FAILURE);
					}
				}
				shunt_op(op);
				lastop=op;
			} else if(isdigit(*expr)) tstart=expr;
			else if(!isspace(*expr)) {
				LOG("ERROR: Syntax error\n");
				return EXIT_FAILURE;
			}
		} else {
			if(isspace(*expr)) {
				push_numstack(atoi(tstart));
				tstart=NULL;
				lastop=NULL;
			} else if((op=getop(*expr))) {
				push_numstack(atoi(tstart));
				tstart=NULL;
				shunt_op(op);
				lastop=op;
			} else if(!isdigit(*expr)) {
				fprintf(stderr, "ERROR: Syntax error\n");
				return EXIT_FAILURE;
			}
		}
	}
	if(tstart) push_numstack(atoi(tstart));

	while(nopstack) {
		op=pop_opstack();
		n1=pop_numstack();
		if(op->unary) push_numstack(op->eval(n1, 0));
		else {
			n2=pop_numstack();
			push_numstack(op->eval(n2, n1));
		}
	}
	if(nnumstack!=1) {
		LOG("ERROR: Number stack has %d elements after evaluation. Should be 1.\n", nnumstack);
		return EXIT_FAILURE;
	}
	LOG("success: %d\n", numstack[0]);

	return EXIT_SUCCESS;
}
