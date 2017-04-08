#include <stdlib.h>
#include <stdarg.h>
#include "../log.c/src/log.h"
#include "../Object.h"
#include "../Interpreter.h"
#include "../Conversion.h"

Value* fn_print(Scope* scope, ArgumentList* arguments) {
	if(arguments == NULL) {
		putchar('\n');
		return NULL;
	}
	
	Argument* current = arguments->first;
	Value* value;
	while(current != NULL) {
		value = current->value;
		if (value->type == OBJ_STRING_TYPE) {
			printf("%s", value->value.string);
		} else if (value->type == OBJ_INTEGER_TYPE) {
			printf("%s", integer_to_string(value->value.integer));
		} else if (value->type == OBJ_NUMBER_TYPE) {
			printf("%s", double_to_string(value->value.number));
		} else {
			log_error("Unsupported printing type %i", value->type);
		}
		current = current->next;
	}
	
	return NULL;
}

Value* fn_printf(Scope* scope, ArgumentList* arguments) {
	// TODO: work on this
	Value* first = arguments->first->value;
	if(first->type != OBJ_STRING_TYPE) {
		log_error("printf expects a string type for first argument");
		return NULL;
	}
	
	int i = 0;
	char* string = first->value.string;
	size_t length = strlen(string);
	Argument* current = arguments->first->next;
	while(i <= length) {
		log_debug("%i", i);
		if(string[i] == '%') {
			i++;
			if(i > length) {
				log_error("printf: expected specifier after %");
				Error_throw();
			}
			
			if(string[i] == '%') {
				i++;
				putchar('%');
				continue;
			} else if(current == NULL) {
				log_error("printf: not enough arguments");
				Error_throw();
			}
			
			if(string[i] == '.') {
				i++;
				
				int digits = 0;
				int base = 0;
				while(isdigit(string[i])) {
					digits += 10 * base + (string[i] - '0');
					base++;
					i++;
				}
				log_debug("%i", digits);
				
				if(string[i] == 'd') {
					i++;
					if(current->value->type != OBJ_NUMBER_TYPE) {
						log_error("printf: %d expects a floatin point number");
						continue;
					}
					printf("%s", double_to_string_base(current->value->value.number, digits));
				} else if (string[i] == 'i') {
					i++;
					printf("%s", integer_to_string(current->value->value.integer));
					if(current->value->type != OBJ_INTEGER_TYPE) {
						log_error("printf: %d expects an integer");
						continue;
					}
					while(base--) {
						putchar('0');
					}
				} else {
					log_error("printf: expected specifier, got character %c", string[i]);
				}
			} else if(string[i] == 'd') {
				i++;
				log_debug("dec %i", i);
				printf("%f", current->value->value.number);
			} else if(string[i] == 'i') {
				i++;
				log_debug("int");
				printf("%i", current->value->value.integer);
			} else if(string[i] == 's') {
				i++;
				log_debug("string");
				printf("%s", current->value->value.string);
			} else {
				log_error("printf: expected specifier, got character %c", string[i]);
			}
			current = current->next;
		} else {
			putchar(string[i++]);
		}
	}
	
	return NULL;
}

Value* fn_input(Scope* scope, ArgumentList* arguments) {
	if(arguments != NULL) {
		Argument* current = arguments->first;
		Value* value = current->value;
		if (value->type == OBJ_STRING_TYPE) {
			printf("%s", value->value.string);
		} else if (value->type == OBJ_INTEGER_TYPE) {
			printf("%i", value->value.integer);
		} else if (value->type == OBJ_NUMBER_TYPE) {
			printf("%f", value->value.number);
		} else {
			log_error("Unsupported printing type %i", value->type);
		}
		current = current->next;
	}
	
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

Value* fn_str(Scope* scope, ArgumentList* arguments) {
	if(arguments == NULL) {
		log_error("str function requires at least 1 argument");
		Error_throw();
		return NULL;
	} else {
		Argument* current = arguments->first;
		Value* value = current->value;
		if (value->type == OBJ_STRING_TYPE) {
			return String_create(value->value.string);
		} else if (value->type == OBJ_INTEGER_TYPE) {
			return String_create(integer_to_string(value->value.integer));
		} else if (value->type == OBJ_NUMBER_TYPE) {
			return String_create(double_to_string(value->value.number));
		} else {
			log_error("Unsupported printing type %i", value->type);
			return NULL;
		}
	}
}

void lib_base_init(Program* pr) {
	Scope* scope = pr->block->scope;
	Scope_set(scope, "print", Value_fn_create(NativeFunction_create(fn_print, NULL)));
	Scope_set(scope, "printf", Value_fn_create(NativeFunction_create(fn_printf, NULL)));
	Scope_set(scope, "input", Value_fn_create(NativeFunction_create(fn_input, NULL)));
}