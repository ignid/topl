#include <setjmp.h>
#include "Error.h"

void Error_throw(int err_code) {
	longjmp(env, err_code);
}