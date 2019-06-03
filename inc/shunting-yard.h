// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <stdint.h>

#ifndef SHUNTING_YARD_H
#define SHUNTING_YARD_H

typedef enum {
    STATUS_OK,
    ERROR_SYNTAX,
    ERROR_OPEN_PARENTHESIS,
    ERROR_CLOSE_PARENTHESIS,
    ERROR_UNRECOGNIZED,
    ERROR_NO_INPUT,
    ERROR_UNDEFINED_FUNCTION,
    ERROR_FUNCTION_ARGUMENTS,
    ERROR_UNDEFINED_CONSTANT
} Status;

// Calculates the result of a mathematical expression.
Status shunting_yard(const char *expression, uint64_t *result);

#endif  // SHUNTING_YARD_H
