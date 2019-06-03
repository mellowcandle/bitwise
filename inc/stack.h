// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#ifndef SHUNTING_YARD_STACK_H
#define SHUNTING_YARD_STACK_H

typedef struct Stack Stack;

// Inserts an item at the top of the stack.
void stack_push(Stack **stack, const void *value);

// Removes an item from the top of the stack.
const void *stack_pop(Stack **stack);

// Returns the item at the top of the stack without removing it.
const void *stack_top(const Stack *stack);

#endif  // SHUNTING_YARD_STACK_H
