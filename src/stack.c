// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "stack.h"

#include <stdlib.h>

struct Stack {
	const void *value;
	Stack *next;
};

void stack_push(Stack **stack, const void *value)
{
	Stack *item = malloc(sizeof * item);
	item->value = value;
	item->next = *stack;
	*stack = item;
}

const void *stack_pop(Stack **stack)
{
	Stack *item = *stack;
	const void *value = item->value;
	*stack = item->next;
	free(item);
	return value;
}

const void *stack_top(const Stack *stack)
{
	return stack->value;
}
