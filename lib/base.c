#include "../log.c/src/log.h"
#include "../Object.h"
#include "../Interpreter.h"

Value* fn_print(Scope* scope, ArgumentList* arguments) {
	if(arguments == NULL) {
		putchar('\n');
		return NULL;
	}
	
	// TODO multiple print arguments
	// TODO fix newline printing
	Argument* argument = ArgumentList_get_by_index(arguments, 0);
	Value* first = argument->value;
	if (first->type == OBJ_STRING_TYPE) {
		puts(first->value.string);
	} else if (first->type == OBJ_INTEGER_TYPE) {
		printf("%i", first->value.integer);
	} else if (first->type == OBJ_NUMBER_TYPE) {
		printf("%f", first->value.number);
	} else {
		log_error("Unsupported printing type %i", first->type);
	}
	fflush(stdout);
	
	return NULL;
}

Value* fn_input(Scope* scope, ArgumentList* arguments) {
	char* line = (char*)malloc(1);
	size_t i = 0;
	char c;
	while(c != '\n') {
		c = getchar();
		line = (char*)realloc(line, i + 2);
		line[i] = c;
		i++;
	}
	line[i] = '\0';
	return String_create(line);
}

void lib_base_init(Program* pr) {
	Scope* scope = pr->block->scope;
	Scope_set(scope, "print", Value_fn_create(NativeFunction_create(fn_print, NULL)));
	Scope_set(scope, "input", Value_fn_create(NativeFunction_create(fn_input, NULL)));
}