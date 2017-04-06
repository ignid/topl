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
		} else {
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
		log_info("Declaration Stmt");
		ASTDeclarationStatement* stmt = statement->statement.declaration;
		
		char* identifier = ASTValue_get(stmt->left);
		log_info("DECL IDENTIFIER %s", ASTValue_get(stmt->left));
		
		Value* right = Value_evaluate(stmt->right, block);
		Scope_set(block->scope, identifier, right);
		
		return NULL;
	} else if(statement->type == AST_EXPR_STATEMENT) {
		log_info("Expression Stmt");
		return Value_evaluate(statement->statement.expression->expression, block);
	} else if(statement->type == AST_IF_STATEMENT) {
		log_info("If stmt");
		Value* expression = Value_evaluate(statement->statement.ifelse->expression, block);
		if(Value_is_true(expression)) {
			Block* b = Block_create(statement->statement.ifelse->block, block->scope);
			Block_evaluate(b);
			free(b);
		} else { // TODO
			return NULL;
		}
		return NULL;
	} else {
		log_error("Unsupported type %i", statement->type);
	}
}
Value* Value_evaluate(ASTValue* value, Block* block) {
	log_info("TYPE %i", value->type);
	if(value->type == AST_STRING_VALUE) {
		log_info("STRING VALUE %s", value->value.string);
		return String_create(value->value.string);
	} else if (value->type == AST_NUMBER_VALUE) {
		log_info("NUMBER VALUE %f", *(double*)(value->value.number));
		return Number_create(*value->value.number);
	} else if (value->type == AST_OBJECT_VALUE) {
		// TODO
	} else if (value->type == AST_ARRAY_VALUE) {
		// TODO
	} else if (value->type == AST_IDENTIFIER_VALUE) {
		log_info("IDENTIFIER VALUE %s", value->value.string);
		return Scope_get(block->scope, value->value.string)->value;
	} else if (value->type == AST_FN_VALUE) {
		log_info("FUNCTION VALUE");
		ASTFunctionExpression* fn_expr = value->value.fn_expr;
		if(fn_expr->name->type != AST_IDENTIFIER_VALUE) {
			log_error("Expected identifier value for fn_expr");
			Error_throw(1);
		}
		
		ScopeVariable* fn_var = Scope_get(block->scope, fn_expr->name->value.string);
		if(fn_var == NULL || fn_var->value->type != OBJ_FN_TYPE) {
			log_error("Can't call value of type %i.", fn_var->value->type);
			Error_throw(1);
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
				Error_throw(1);
			}
		} else {
			log_error("TODO: IMPLEMENT NON-NATIVE FUNCTION!");
			Error_throw(1);
			//return Block_evaluate();
		}
		
		return NULL;
		
	} else if (value->type == AST_BINEXPR_VALUE) {
		ASTBinaryExpression* bin_expr = (ASTBinaryExpression*)ASTValue_get(value);
		Value* left = Value_evaluate(bin_expr->left, block);
		Value* right = Value_evaluate(bin_expr->right, block);
		int op = bin_expr->op;
		log_info("OPERATOR %i", op);
		if(op == AST_PLUS_OP) {
			if(left->type == OBJ_STRING_TYPE) {
				if(right->type != OBJ_STRING_TYPE) {
					log_error("Cannot add non-string to string!");
					Error_throw(1);
				}
				char string[strlen(left->value.string) + strlen(right->value.string)];
				strcpy(string, left->value.string);
				strcat(string, right->value.string);
				Value* val = String_create(string);
				
				Value_destroy(left);
				Value_destroy(right);
				return val;
			} else if (left->type == OBJ_NUMBER_TYPE) {
				double l = *(double*)Value_get(left);
				double r = *(double*)Value_get(right);
				log_info("%f + %f = %f", l, r, l + r);
				
				Value_destroy(left);
				Value_destroy(right);
				return Number_create(l + r);
			} else {
				log_error("Error on binary evaluation, left: %i, right: %i", left->type, right->type);
				
				Value_destroy(left);
				Value_destroy(right);
				Error_throw(1);
			}
		} else {
			if(left->type != OBJ_NUMBER_TYPE || right->type != OBJ_NUMBER_TYPE) {
				log_error("Binary expression requires number!");
				
				Value_destroy(left);
				Value_destroy(right);
				Error_throw(1);
			}
			
			double l = *(double*)Value_get(left);
			double r = *(double*)Value_get(right);
			
			Value_destroy(left);
			Value_destroy(right);
			
			if(op == AST_MINUS_OP)
				return Number_create(l - r);
			else if(op == AST_MULT_OP)
				return Number_create(l * r);
			else if(op == AST_DIV_OP) {
				if(r == 0) {
					log_info("Division by zero not allowed!");
					Error_throw(1);
				}
				return Number_create(l / r);
			}
		}
	}
}
int Value_is_true(Value* value) {
	if(value->type == OBJ_STRING_TYPE) {
		return strlen(value->value.string);
	} else if (value->type == OBJ_NUMBER_TYPE) {
		return *(value->value.number);
	} else if(value->type == OBJ_OBJECT_TYPE) {
		return 1;
	} else if(value->type == OBJ_ARRAY_TYPE) {
		return 1; // TODO ARRAY
	} else if(value->type == OBJ_FN_TYPE) {
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