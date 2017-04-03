#include "Interpreter.h"

Interpreter* Interpreter_create(Program* pr) {
	Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
	interpreter->program = pr;
	return interpreter;
}
void Interpreter_interpret(Interpreter* interpreter) {
	Block_evaluate(interpreter->program->block);
}
void Interpreter_destroy(Interpreter* interpreter) {
	Program_destroy(interpreter->program);
	free(interpreter);
}

Block* Block_create(ASTBlock* ast, Scope* scope) {
	Block* block = (ASTBlock*)malloc(sizeof(ASTBlock));
	block->ast = ast;
	block->scope = scope;
	block->parent = NULL;
	return block;
}
void Block_destroy(Block* block) {
	Scope_destroy(block->scope);
	free(block);
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
	Scope* sc = Scope_create();
	ScopeVariable* current = scope->first;
	while(current != NULL) {
		Scope_set(sc, current->key, current->value);
		current = current->next;
	}
	return sc;
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
			scope->last->next = value;
			scope->last = value;
		} else {
			Value_destroy(found->value);
			found->value = value;
		}
	}
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
		Value_destroy(result);
		current = current->next;
	}
}
Value* BlockStatement_evaluate(ASTBlockStatement* statement, Block* block) {
	return Statement_evaluate(statement->statement, block);
}
Value* Statement_evaluate(ASTStatement* statement, Block* block) {
	// TODO free values malloc'd in this (done)
	if(statement->type == AST_DECL_STATEMENT) {
		printf("Declaration Stmt\n");
		ASTDeclarationStatement* stmt = (ASTDeclarationStatement*)ASTStatement_get(statement);
		
		char* identifier = ASTValue_get(stmt->left);
		printf("DECL IDENTIFIER %s\n", ASTValue_get(stmt->left));
		
		Value* right = Value_evaluate(stmt->right, block);
		Scope_set(block->scope, identifier, right);
		
		return right;
	} else if(statement->type == AST_EXPR_STATEMENT) {
		printf("Expression Stmt\n");
		Value* value = Value_evaluate(
			((ASTExpressionStatement*)ASTStatement_get(statement))->expression,
			block
		);
		if(value->type == AST_STRING_VALUE) {
			printf("STRING %s\n", Value_get(value));
		} else if (value->type == AST_NUMBER_VALUE) {
			printf("NUMBER %f\n", *(double*)Value_get(value));
		}
		return value;
	}
}
Value* Value_evaluate(ASTValue* value, Block* block) {
	if(value == NULL) {
		printf("NULL VALUE\n");
		return NULL;
	} else if(value->type == AST_STRING_VALUE) {
		printf("STRING VALUE\n");
		return String_create(value->value.string);
	} else if (value->type == AST_NUMBER_VALUE) {
		printf("NUMBER VALUE %f\n", *(double*)ASTValue_get(value));
		return Number_create(*value->value.number);
	} else if (value->type == AST_OBJECT_VALUE) {
		// TODO
	} else if (value->type == AST_ARRAY_VALUE) {
		// TODO
	} else if (value->type == AST_IDENTIFIER_VALUE) {
		return Scope_get(block->scope, value->value.string)->value;
	} else if (value->type == AST_BINEXPR_VALUE) {
		ASTBinaryExpression* bin_expr = (ASTBinaryExpression*)ASTValue_get(value);
		Value* left = Value_evaluate(bin_expr->left, block);
		Value* right = Value_evaluate(bin_expr->right, block);
		int op = bin_expr->op;
		printf("OPERATOR %i\n", op);
		if(op == AST_PLUS_OP) {
			if(left->type == OBJ_STRING_TYPE) {
				if(right->type != OBJ_STRING_TYPE) {
					printf("Cannot add non-string to string!\n");
					return NULL;
				}
				char string[strlen(left->value.string) + strlen(right->value.string)];
				strcpy(string, left->value.string);
				strcat(string, right->value.string);
				Value* val = String_create(string);
				
				free(string);
				free(left);
				free(right);
				return val;
			} else if (left->type == OBJ_NUMBER_TYPE) {
				double l = *(double*)Value_get(left);
				double r = *(double*)Value_get(right);
				printf("%f + %f = %f\n", l, r, l + r);
				
				free(left);
				free(right);
				return Number_create(l + r);
			} else {
				printf("Error on binary evaluation, left: %i, right: %i\n", left->type, right->type);
				
				free(left);
				free(right);
				return NULL;
			}
		} else {
			if(left->type != OBJ_NUMBER_TYPE || right->type != OBJ_NUMBER_TYPE) {
				printf("Binary expression requires number!\n");
				
				free(left);
				free(right);
				return NULL;
			}
			
			double l = *(double*)Value_get(left);
			double r = *(double*)Value_get(right);
			
			free(left);
			free(right);
			
			if(op == AST_MINUS_OP)
				return Number_create(l - r);
			else if(op == AST_MULT_OP)
				return Number_create(l * r);
			else if(op == AST_DIV_OP) {
				if(r == 0) {
					printf("Division by zero not allowed!\n");
					return NULL;
				}
				return Number_create(l / r);
			}
		}
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