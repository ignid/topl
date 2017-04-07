#include <setjmp.h>
#include "Error.h"

void Error_throw() {
	longjmp(env, 1);
}