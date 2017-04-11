#include <math.h>
#include "log.c/src/log.h"
#include "Error.h"
#include "AST.h"
#include "Interpreter.h"
#include "lib/base.c"

Interpreter* Interpreter_create(Program* pr) {
	Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
	interpreter->program = pr;
	Program_init_lib(pr);
	return interpreter;
}
void Program_init_lib(Program* pr) {
	// place all libs files here
	lib_base_init(pr);
}
void Interpreter_interpret(Interpreter* interpreter) {
	Block_evaluate(interpreter->program->block);
}
void Interpreter_destroy(Interpreter* interpreter) {
	Program_destroy(interpreter->program);
	free(interpreter);
}

Block* Block_create(ASTBlock* ast, Scope* scope) {
	Block* block = (Block*)malloc(sizeof(Block));
	block->ast = ast;
	block->scope = scope;
	block->parent = NULL;
	return block;
}
void Block_destroy(Block* block) {
	Scope_destroy(block->scope);
	free(block);
}
Block* Block_inherit(Block* block) {
	return Block_create(block->ast, Scope_inherit(block->scope));
}

Scope* Scope_create() {
	Scope* scope = (Scope*)malloc(sizeof(Scope));
	scope->first = NULL;
	scope->last = NULL;
	return scope;
}
void Scope_destroy(Scope* scope) {
	ScopeVariable* current = scope->first;
	ScopeVariable* next;
	while(current != NULL) {
		next = current->next;
		ScopeVariable_destroy(current);
		current = next;
	}
	free(scope);
}
Scope* Scope_inherit(Scope* scope) {
	Scope* scope1 = Scope_create();
	// vvvv TODO replace shitty constructor with clone method vvvv
	scope1->first = scope1->last = ScopeVariable_create(scope1, scope->first->key, scope->first->value);
	
	ScopeVariable* current = scope->first->next;
	while(current != NULL) {
		Scope_push(scope1, current->key, current->value);
		current = current->next;
	}
	return scope1;
}
void Scope_copy(Scope* scope1, Scope* scope2) {
	ScopeVariable* current = scope2->first->next;
	while(current != NULL) {
		Scope_push(scope1, current->key, current->value);
		current = current->next;
	}
}
ScopeVariable* Scope_get(Scope* scope, char* key) {
	ScopeVariable* current = scope->first;
	while(current != NULL) {
		if(strcmp(current->key, key) == 0) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
void Scope_set(Scope* scope, char* key, Value* value) {
	ScopeVariable* sc_var = ScopeVariable_create(scope, key, value);
	if(scope->first == NULL) {
		scope->first = scope->last = sc_var;
	} else {
		ScopeVariable* found = Scope_get(scope, key);
		if(found == NULL) {
			Scope_push(scope, key, value);
		} else { // TODO Value_mutate?
			Value_destroy(found->value);
			found->value = value;
		}
	}
}
void Scope_push(Scope* scope, char* key, Value* value) {
	ScopeVariable* sc_var = ScopeVariable_create(scope, key, value);
	scope->last->next = sc_var;
	scope->last = sc_var;
}

ScopeVariable* ScopeVariable_create(Scope* scope, char* key, Value* value) {
	ScopeVariable* sc_var = (ScopeVariable*)malloc(sizeof(ScopeVariable));
	sc_var->key = key;
	sc_var->value = value;
	sc_var->scope = scope;
	sc_var->next = NULL;
	return sc_var;
}
void ScopeVariable_destroy(ScopeVariable* sc_var) {
	Value_destroy(sc_var->value);
	free(sc_var);
}

void Block_evaluate(Block* block) {
	ASTBlock* ast = block->ast;
	ASTBlockStatement* current = ast->first;
	while(current != NULL) {
		Value* result = BlockStatement_evaluate(current, block);
		if(result != NULL) Value_destroy(result);
		current = current->next;
	}
}
Value* BlockStatement_evaluate(ASTBlockStatement* statement, Block* block) {
	return Statement_evaluate(statement->statement, block);
}
Value* Statement_evaluate(ASTStatement* statement, Block* block) {
	if(statement->type == AST_DECL_STATEMENT) {
		log_debug("Declaration Stmt");
		ASTDeclarationStatement* stmt = statement->statement.declaration;
		
		if(stmt->left->type == AST_IDENTIFIER_VALUE) {
			char* identifier = stmt->left->value.string;
			log_debug("DECL IDENTIFIER %s", identifier);
			
			Value* right = Value_evaluate(stmt->right, block);
			Scope_set(block->scope, identifier, right);
		} else if (stmt->left->type == AST_OBJECT_EXPR_VALUE) {
			log_debug("DECL OBJ EXPR");
			
			ASTObjectExpression* object_expr = stmt->left->value.object_expr;
			Value* head = Value_evaluate(object_expr->head, block);
			if(head == NULL || head->type != OBJ_OBJECT_TYPE) {
				log_error("object accessor expression: head type must be object type, got %i", head->type);
				Error_throw();
			}
			
			ASTValue* tail = object_expr->tail;
			if(tail == NULL || tail->type != AST_IDENTIFIER_VALUE) {
				log_error("object accessor expression: tail has be of type identifier value");
				Error_throw();
			}
			log_debug("%s", tail->value.string);
			
			Object_set(head->value.object, ObjectPair_create(tail->value.string, Value_evaluate(stmt->right, block)));
		} else {
			log_error("declaration statement: left must be of type identifier or object expression");
		}
		return NULL;
	} else if(statement->type == AST_EXPR_STATEMENT) {
		log_debug("Expression Stmt");
		return Value_evaluate(statement->statement.expression->expression, block);
	} else if(statement->type == AST_IF_STATEMENT) {
		log_debug("If stmt");
		Value* expression = Value_evaluate(statement->statement.ifelse->expression, block);
		Block* b;
		if(Value_is_true(expression)) {
			b = Block_create(statement->statement.ifelse->block, block->scope);
			Block_evaluate(b);
			free(b);
		} else if (statement->statement.ifelse->block_else != NULL) {
			b = Block_create(statement->statement.ifelse->block_else, block->scope);
			Block_evaluate(b);
			free(b);
		}
		return NULL;
	} else if(statement->type == AST_WHILE_STATEMENT) {
		log_debug("While stmt");
		Value* expression;
		Block* b = Block_create(statement->statement.whiles->block, block->scope);
		while( (expression = Value_evaluate(statement->statement.whiles->expression, block)) &&
			Value_is_true(expression) ) {
			Block_evaluate(b);
		}
		free(b);
		return NULL;
	} else {
		log_error("Unsupported type %i", statement->type);
		Error_throw();
	}
}
Value* Value_evaluate(ASTValue* value, Block* block) {
	log_debug("TYPE %i", value->type);
	if(value->type == AST_STRING_VALUE) {
		log_debug("STRING VALUE %s", value->value.string);
		return String_create(value->value.string);
	} else if (value->type == AST_INTEGER_VALUE) {
		log_debug("INTEGER VALUE %i", value->value.integer);
		return Integer_create(value->value.integer);
	} else if (value->type == AST_NUMBER_VALUE) {
		log_debug("NUMBER VALUE %f", value->value.number);
		return Number_create(value->value.number);
	} else if (value->type == AST_OBJECT_VALUE) {
		log_debug("OBJECT VALUE");
		Object* object = Object_create();
		ASTObjectPair* current = value->value.object->first;
		while(current != NULL) {
			Object_set(object, ObjectPair_create(current->key, Value_evaluate(current->value, block)));
			current = current->next;
		}
		return Value_object_create(object);
	} else if (value->type == AST_ARRAY_VALUE) {
		// TODO
		return NULL;
	} else if (value->type == AST_IDENTIFIER_VALUE) {
		log_debug("IDENTIFIER VALUE %s", value->value.string);
		Value* val = Scope_get(block->scope, value->value.string)->value;
		return Value_clone(val);
	} else if (value->type == AST_FN_VALUE) {
		log_debug("FUNCTION VALUE");
		ASTFunctionExpression* fn_expr = value->value.fn_expr;
		if(fn_expr->name->type != AST_IDENTIFIER_VALUE) {
			log_error("Expected identifier value for fn_expr");
			Error_throw();
		}
		
		ScopeVariable* fn_var = Scope_get(block->scope, fn_expr->name->value.string);
		if(fn_var == NULL || fn_var->value->type != OBJ_FN_TYPE) {
			log_error("Can't call value of type %i.", fn_var->value->type);
			Error_throw();
		}
		
		Function* function = fn_var->value->value.function;
		
		if(function->type == OBJ_FN_NATIVE) {
			// TODO COPY SCOPE VARIABLES INSTEAD OF PASSING DIRECT BLOCK
			ASTArgumentList* ast_argument_list = fn_expr->argument_list;
			if(ast_argument_list->first == NULL) {
				return function->code.native(block->scope, NULL);
			}
			
			ArgumentList* argument_list = ArgumentList_create();
			ASTArgument* current = ast_argument_list->first->next;
			
			if(function->arguments == NULL) {
				argument_list->first = argument_list->last = Argument_create(NULL);
				Value* evaluated = Value_evaluate(ast_argument_list->first->value, block);
				argument_list->first->value = evaluated;
				
				while(current != NULL) { // TODO CHECK FOR NONPOSITIONAL ARGUMENTS 
					argument_list->last = argument_list->last->next = Argument_create(NULL);
					argument_list->last->value = Value_evaluate(current->value, block);
					current = current->next;
				}
				
				return function->code.native(block->scope, argument_list);
			} else { // TODO MAPPING
				Error_throw();
			}
		} else {
			log_error("TODO: IMPLEMENT NON-NATIVE FUNCTION!");
			Error_throw();
			//return Block_evaluate();
		}
		
		return NULL;
		
	} else if (value->type == AST_BINEXPR_VALUE) {
		ASTBinaryExpression* bin_expr = value->value.bin_expr;
		Value* left = Value_evaluate(bin_expr->left, block);
		Value* right = Value_evaluate(bin_expr->right, block);
		int op = bin_expr->op;
		log_debug("OPERATOR %i", op);
		if(op == AST_PLUS_OP) {
			if(left->type == OBJ_STRING_TYPE) {
				if(right->type != OBJ_STRING_TYPE) {
					log_error("Cannot add non-string to string!");
					Error_throw();
				}
				char string[strlen(left->value.string) + strlen(right->value.string)];
				strcpy(string, left->value.string);
				strcat(string, right->value.string);
				Value* val = String_create(string);
				
				Value_destroy(left);
				Value_destroy(right);
				return val;
			} else if (left->type == OBJ_NUMBER_TYPE) {
				double l = left->value.number;
				double r = right->value.number;
				log_debug("%f + %f = %f", l, r, l + r);
				
				Value_destroy(left);
				Value_destroy(right);
				return Number_create(l + r);
			} else if (left->type == OBJ_INTEGER_TYPE) {
				int l = left->value.integer;
				int r = right->value.integer;
				log_debug("%i + %i = %i", l, r, l + r);
				
				Value_destroy(left);
				Value_destroy(right);
				return Integer_create(l + r);
			} else {
				log_error("Incompatible type for binary expression, left: %i, right: %i", left->type, right->type);
				
				Value_destroy(left);
				Value_destroy(right);
				Error_throw();
			}
		} else if(op == AST_EQUAL_OP) {
			int res = Value_compare(left, right);
			log_debug("compare result %i", res);
			
			Value_destroy(left);
			Value_destroy(right);
			return Integer_create(res);
		} else if(op == AST_NOT_EQUAL_OP) {
			int res = Value_compare(left, right);
			log_debug("compare result %i", res);
			
			Value_destroy(left);
			Value_destroy(right);
			return Integer_create(!res);
		} else if (
			op == AST_LESS_OP ||
			op == AST_MORE_OP ||
			op == AST_LESS_EQ_OP ||
			op == AST_MORE_EQ_OP
		) {
			if( !((left->type == OBJ_INTEGER_TYPE || left->type == OBJ_NUMBER_TYPE) ||
				(right->type == OBJ_INTEGER_TYPE || right->type == OBJ_NUMBER_TYPE)) ) {
				log_error("Comparison expression requires number! Left: %i, right: %i", left->type, right->type);
				
				Value_destroy(left);
				Value_destroy(right);
				Error_throw();
			}
			
			// TODO define?
			if(left->type != right->type) {
				int l;
				double r;
				
				if(left->type == OBJ_INTEGER_TYPE) {
					l = left->value.integer;
					r = right->value.number;
				} else {
					l = right->value.integer;
					r = left->value.number;
				}
				
				Value_destroy(left);
				Value_destroy(right);
				if(op == AST_LESS_OP) {
					return Integer_create(l < r);
				} else if (op == AST_MORE_OP) {
					return Integer_create(l > r);
				} else if (op == AST_LESS_EQ_OP) {
					return Integer_create(l >= r);
				} else {
					return Integer_create(l <= r);
				}
			} if(left->type == OBJ_NUMBER_TYPE) {
				double l = left->value.number;
				double r = right->value.number;
				
				Value_destroy(left);
				Value_destroy(right);
				if(op == AST_LESS_OP) {
					return Integer_create(l < r);
				} else if (op == AST_MORE_OP) {
					return Integer_create(l > r);
				} else if (op == AST_LESS_EQ_OP) {
					return Integer_create(l >= r);
				} else {
					return Integer_create(l <= r);
				}
			} else {
				int l = left->value.integer;
				int r = right->value.integer;
				
				Value_destroy(left);
				Value_destroy(right);
				if(op == AST_LESS_OP) {
					return Integer_create(l < r);
				} else if (op == AST_MORE_OP) {
					return Integer_create(l > r);
				} else if (op == AST_LESS_EQ_OP) {
					return Integer_create(l >= r);
				} else {
					return Integer_create(l <= r);
				}
			}
		} else if (op == AST_AND_OP) {
			int res = Value_is_true(left) && Value_is_true(right);
			
			Value_destroy(left);
			Value_destroy(right);
			return Integer_create(res);
		} else if (op == AST_OR_OP) {
			int res = Value_is_true(left) || Value_is_true(right);
			
			Value_destroy(left);
			Value_destroy(right);
			return Integer_create(res);
		} else {
			if( !((left->type == OBJ_INTEGER_TYPE || left->type == OBJ_NUMBER_TYPE) ||
				(right->type == OBJ_INTEGER_TYPE || right->type == OBJ_NUMBER_TYPE)) ) {
				log_error("Binary expression requires number!");
				
				Value_destroy(left);
				Value_destroy(right);
				Error_throw();
			}
			
			
			if(left->type == OBJ_NUMBER_TYPE) {
				double l = left->value.number;
				double r = right->value.number;
				
				Value_destroy(left);
				Value_destroy(right);
				
				if(op == AST_MINUS_OP)
					return Number_create(l - r);
				else if(op == AST_MULT_OP)
					return Number_create(l * r);
				else if(op == AST_MODULO_OP)
					return Number_create(fmod(l, r));
				else if(op == AST_DIV_OP) {
					if(r == 0) {
						log_error("Division by zero not allowed!");
						Error_throw();
					}
					return Number_create(l / r);
				} else {
					log_error("Operator type %i not supported!", op);
					Error_throw();
				}
			} else {
				int l = left->value.integer;
				int r = right->value.integer;
				
				Value_destroy(left);
				Value_destroy(right);
				
				if(op == AST_MINUS_OP)
					return Integer_create(l - r);
				else if(op == AST_MULT_OP)
					return Integer_create(l * r);
				else if(op == AST_MODULO_OP)
					return Integer_create(l % r);
				else if(op == AST_DIV_OP) {
					if(r == 0) {
						log_debug("Division by zero not allowed!");
						Error_throw();
					}
					return Number_create(l / r);
				} else {
					log_error("Operator type %i not supported!", op);
					Error_throw();
				}
			}
		}
	} else if(value->type == AST_OBJECT_EXPR_VALUE) {
		ASTObjectExpression* object_expr = value->value.object_expr;
		Value* head = Value_evaluate(object_expr->head, block);
		if(head == NULL || head->type != OBJ_OBJECT_TYPE) {
			log_error("object accessor expression: head type must be object type, got %i", head->type);
			Error_throw();
		}
		
		ASTValue* tail = object_expr->tail;
		log_debug("%i", tail->type);
		if(tail == NULL || tail->type != AST_IDENTIFIER_VALUE) {
			log_error("object accessor expression: tail has be of type identifier value");
			Error_throw();
		}
		
		Value* val = Object_get(head->value.object, tail->value.string);
		return Value_clone(val);
	} else {
		log_error("Type %i not supported!", value->type);
		Error_throw();
	}
}
int Value_is_true(Value* value) {
	if(value->type == OBJ_STRING_TYPE) {
		return strlen(value->value.string) > 0;
	} else if (value->type == OBJ_INTEGER_TYPE) {
		return value->value.integer > 0;
	} else if (value->type == OBJ_NUMBER_TYPE) {
		return value->value.number > 0;
	} else if(value->type == OBJ_OBJECT_TYPE) {
		return 1;
	} else if(value->type == OBJ_ARRAY_TYPE) {
		return 1; // TODO array is true when length > 0
	} else if(value->type == OBJ_FN_TYPE) {
		return 0;
	} else {
		return 0;
	}
}
int Value_compare(Value* value1, Value* value2) {
	if(value1->type != value2->type) {
		return 0;
	}
	
	if(value1->type == OBJ_STRING_TYPE) {
		return strcmp(value1->value.string, value2->value.string) == 0;
	} else if (value1->type == OBJ_INTEGER_TYPE) {
		return value1->value.integer == value2->value.integer;
	} else if (value1->type == OBJ_NUMBER_TYPE) {
		return value1->value.number == value2->value.number;
	} else if(value1->type == OBJ_OBJECT_TYPE) {
		return 0;
	} else if(value1->type == OBJ_ARRAY_TYPE) {
		return 0;
	} else if(value1->type == OBJ_FN_TYPE) {
		return 0;
	}
}

Program* Program_create(ASTProgram* ast, Scope* scope) {
	Program* program = (Program*)malloc(sizeof(Program));
	program->ast = ast;
	program->block = Block_create(ast->block, scope);
	return program;
}
void Program_destroy(Program* program) {
	Block_destroy(program->block);
	free(program);
}