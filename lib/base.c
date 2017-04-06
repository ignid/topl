#include "../log.c/src/log.h"
#include "../Object.h"
#include "../Interpreter.h"

Value* fn_print(Scope* scope, ArgumentList* arguments) {
	if(arguments == NULL) {
		printf("\n");
		return NULL;
	}
	
	Argument* argument = ArgumentList_get_by_index(arguments, 0);
	Value* first = argument->value;
	if (first->type == OBJ_STRING_TYPE) {
		printf("%s\n", first->value.string);
	} else if (first->type == OBJ_NUMBER_TYPE) {
		printf("%f\n", *(first->value.number));
	} else { // TODO ERROR
		log_error("ERROR UNSUPPORTED TYPE %i!", first->type);
	}
	
	return NULL;
}

void fn_print_init(Scope* scope) {
	Scope_set(scope, "print", Value_fn_create(NativeFunction_create(fn_print, NULL)));
}

void lib_base_init(Program* pr) {
	Scope* scope = pr->block->scope;
	fn_print_init(scope);
}