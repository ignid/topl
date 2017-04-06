#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>

static jmp_buf env;
void Error_throw(int err_code);

#endif