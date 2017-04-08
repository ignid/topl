#include <stdlib.h>
#include <string.h>
#include "log.c/src/log.h"
#include "AST.h"

ASTValue* ASTIdentifier_create(char* string) {
	ASTValue* value = ASTValue_create();
	value->value.string = strdup(string);
	value->type = AST_IDENTIFIER_VALUE;
	return value;
}
ASTValue* ASTString_create(char* string) {
	ASTValue* value = ASTValue_create();
	value->value.string = strdup(string);
	value->type = AST_STRING_VALUE;
	return value;
}
ASTValue* ASTNumber_create(double number) {
	ASTValue* value = ASTValue_create();
	value->value.number = number;
	value->type = AST_NUMBER_VALUE;
	return value;
}
ASTValue* ASTValue_object_create(ASTObject* object) {
	ASTValue* value = ASTValue_create();
	value->value.object = object;
	value->type = AST_OBJECT_VALUE;
	return value;
}
ASTValue* ASTValue_bin_expr_create(ASTBinaryExpression* bin_expr) {
	ASTValue* value = ASTValue_create();
	value->value.bin_expr = bin_expr;
	value->type = AST_BINEXPR_VALUE;
	return value;
}
ASTValue* ASTValue_fn_expr_create(ASTFunctionExpression* fn_expr) {
	ASTValue* value = ASTValue_create();
	value->value.fn_expr = fn_expr;
	value->type = AST_FN_VALUE;
	return value;
}
ASTValue* ASTInteger_create(int integer) {
	ASTValue* value = ASTValue_create();
	value->value.integer = integer;
	value->type = AST_INTEGER_VALUE;
	return value;
}
ASTValue* ASTValue_object_expr_create(ASTObjectExpression* object_expr) {
	ASTValue* value = ASTValue_create();
	value->value.object_expr = object_expr;
	value->type = AST_OBJECT_EXPR_VALUE;
	return value;
}
ASTValue* ASTValue_create() {
	ASTValue* value = (ASTValue*)malloc(sizeof(ASTValue));
	value->type = AST_UNDEFINED_VALUE;
	return value;
}
void ASTValue_destroy(ASTValue* value) {
	if(value->type == AST_STRING_VALUE || value->type == AST_IDENTIFIER_VALUE) {
		free(value->value.string);
	} else if(value->type == AST_INTEGER_VALUE || value->type == AST_NUMBER_VALUE) {
		// numbers aren't malloc'd
	} else if(value->type == AST_OBJECT_VALUE) {
		ASTObject_destroy(value->value.object);
	} else if(value->type == AST_ARRAY_VALUE) {
		ASTArray_destroy(value->value.array);
	} else if (value->type == AST_BINEXPR_VALUE) {
		ASTBinaryExpression_destroy(value->value.bin_expr);
	}
	free(value);
}

ASTObjectPair* ASTObjectPair_create(char* key, ASTValue* value) {
	ASTObjectPair* objectPair = (ASTObjectPair*)malloc(sizeof(ASTObjectPair));
	objectPair->key = key;
	objectPair->value = value;
	objectPair->next = NULL;
	return objectPair;
}
void ASTObjectPair_destroy(ASTObjectPair* objectPair) {
	ASTValue_destroy(objectPair->value);
	free(objectPair);
}

ASTObject* ASTObject_create() {
	ASTObject* object = (ASTObject*)malloc(sizeof(ASTObject));
	object->first = NULL;
	object->last = NULL;
	return object;
}
void ASTObject_destroy(ASTObject* object) {
	ASTObjectPair* current = object->first;
	ASTObjectPair* next;
	while(current != NULL) {
		next = current->next;
		ASTObjectPair_destroy(current);
		current = next;
	}
	free(object);
}
void ASTObject_set(ASTObject* object, ASTObjectPair* objectPair) {
	if(object->first == NULL) {
		object->first = object->last = objectPair;
	} else {
		ASTObjectPair* found = ASTObject_get(object, objectPair->key);
		if(found == NULL) {
			object->last->next = objectPair;
			object->last = objectPair;
		} else {// TODO Value_mutate
			Value_destroy(objectPair->value);
			found->value = objectPair->value;
		}
	}
}
ASTObjectPair* ASTObject_get(ASTObject* object, char* key) {
	ASTObjectPair* current = object->first;
	while(current != NULL) {
		if(strcmp(current->key, key) == 0) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
ASTObjectExpression* ASTObjectExpression_create(ASTValue* head, ASTValue* tail) {
	ASTObjectExpression* object_expr = (ASTObjectExpression*)malloc(sizeof(ASTObjectExpression));
	object_expr->head = head;
	object_expr->tail = tail;
	return object_expr;
}
void ASTObjectExpression_destroy(ASTObjectExpression* object_expr) {
	Value_destroy(object_expr->head);
	Value_destroy(object_expr->tail);
	free(object_expr);
}

ASTArrayElement* ASTArrayElement_create(ASTValue* value) {
	ASTArrayElement* arrayElement = (ASTArrayElement*)malloc(sizeof(ASTArrayElement));
	arrayElement->value = value;
	arrayElement->next = NULL;
	return arrayElement;
}
ASTArray* ASTArray_create() {
	ASTArray* array = (ASTArray*)malloc(sizeof(ASTArray));
	array->first = NULL;
	array->last = NULL;
	return array;
}
void ASTArray_push(ASTArray* array, ASTArrayElement* arrayElement) {
	if(array->first == NULL) {
		array->first = array->last = arrayElement;
	} else {
		array->last->next = arrayElement;
		array->last = arrayElement;
	}
}
void ASTArray_destroy(ASTArray* array) {
	// TODO Implement
}

ASTBinaryExpression* ASTBinaryExpression_create(int op, ASTValue* left, ASTValue* right) {
	ASTBinaryExpression* bin_expr = (ASTBinaryExpression*) malloc(sizeof(ASTBinaryExpression));
	bin_expr->op = op;
	bin_expr->left = left;
	bin_expr->right = right;
	return bin_expr;
}
void ASTBinaryExpression_destroy(ASTBinaryExpression* bin_expr) {
	ASTValue_destroy(bin_expr->left);
	ASTValue_destroy(bin_expr->right);
	free(bin_expr);
}

ASTStatement* ASTStatement_create() {
	return (ASTStatement*)malloc(sizeof(ASTStatement));
}
void ASTStatement_destroy(ASTStatement* statement) {
	if(statement->type == AST_DECL_STATEMENT) {
		ASTValue_destroy(statement->statement.declaration->left);
		ASTValue_destroy(statement->statement.declaration->right);
		free(statement->statement.declaration);
	} else if(statement->type == AST_EXPR_STATEMENT) {
		ASTValue_destroy(statement->statement.expression->expression);
		free(statement->statement.expression);
	} else if(statement->type == AST_IF_STATEMENT) {
		ASTValue_destroy(statement->statement.ifelse->expression);
		ASTBlock_destroy(statement->statement.ifelse->block);
		free(statement->statement.ifelse);
	}
	free(statement);
}
ASTStatement* ASTDeclarationStatement_create(ASTValue* left, ASTValue* right) {
	ASTStatement* statement = ASTStatement_create();
	statement->type = AST_DECL_STATEMENT;
	statement->statement.declaration = (ASTDeclarationStatement*)malloc(sizeof(ASTDeclarationStatement));
	statement->statement.declaration->left = left;
	statement->statement.declaration->right = right;
	return statement;
}
ASTStatement* ASTExpressionStatement_create(ASTValue* expression) {
	ASTStatement* statement = ASTStatement_create();
	statement->type = AST_EXPR_STATEMENT;
	statement->statement.expression = (ASTExpressionStatement*)malloc(sizeof(ASTExpressionStatement));
	statement->statement.expression->expression = expression;
	return statement;
}
ASTStatement* ASTIfStatement_create(ASTValue* expression, ASTBlock* block, ASTBlock* block_else) {
	ASTStatement* statement = ASTStatement_create();
	statement->type = AST_IF_STATEMENT;
	statement->statement.ifelse = (ASTIfStatement*)malloc(sizeof(ASTIfStatement));
	statement->statement.ifelse->expression = expression;
	statement->statement.ifelse->block = block;
	statement->statement.ifelse->block_else = block_else;
	return statement;
}
ASTStatement* ASTWhileStatement_create(ASTValue* expression, ASTBlock* block) {
	ASTStatement* statement = ASTStatement_create();
	statement->type = AST_WHILE_STATEMENT;
	statement->statement.whiles = (ASTWhileStatement*)malloc(sizeof(ASTWhileStatement));
	statement->statement.whiles->expression = expression;
	statement->statement.whiles->block = block;
	return statement;
}

ASTBlock* ASTBlock_create() {
	ASTBlock* block = (ASTBlock*)malloc(sizeof(ASTBlock));
	block->first = NULL;
	block->last = NULL;
	return block;
}
void ASTBlock_destroy(ASTBlock* block) {
	ASTBlockStatement* current = block->first;
	ASTBlockStatement* next;
	while(current != NULL) {
		next = current->next;
		ASTBlockStatement_destroy(current);
		current = next;
	}
	free(block);
}
void ASTBlock_push(ASTBlock* block, ASTBlockStatement* blockStmt) {
	if(block->first == NULL) {
		block->first = block->last = blockStmt;
	} else {
		block->last->next = blockStmt;
		block->last = blockStmt;
	}
}

ASTBlockStatement* ASTBlockStatement_create(ASTStatement* statement) {
	ASTBlockStatement* blockStmt = (ASTBlockStatement*)malloc(sizeof(ASTBlockStatement));
	blockStmt->statement = statement;
	blockStmt->next = NULL;
	return blockStmt;
}
void ASTBlockStatement_destroy(ASTBlockStatement* blockStmt) {
	ASTStatement_destroy(blockStmt->statement);
	free(blockStmt);
}

ASTArgument* ASTArgument_create(ASTValue* key, ASTValue* value) {
	ASTArgument* argument = (ASTArgument*)malloc(sizeof(ASTArgument));
	argument->key = key;
	argument->value = value;
	argument->next = NULL;
	return argument;
}
void ASTArgument_destroy(ASTArgument* argument) {
	ASTValue_destroy(argument->key);
	ASTValue_destroy(argument->value);
	free(argument);
}
ASTArgumentList* ASTArgumentList_create() {
	ASTArgumentList* argument_list = (ASTArgumentList*)malloc(sizeof(ASTArgumentList));
	argument_list->first = NULL;
	argument_list->last = NULL;
	return argument_list;
}
void ASTArgumentList_destroy(ASTArgumentList* argument_list) {
	ASTArgument* current = argument_list->first;
	ASTArgument* next;
	while(current != NULL) {
		next = current->next;
		ASTArgument_destroy(current);
		current = next;
	}
	free(argument_list);
}
ASTArgument* ASTArgumentList_get(ASTArgumentList* argument_list, ASTValue* key) {
	ASTArgument* current = argument_list->first;
	while(current != NULL) {
		if( (current->key->type == AST_STRING_VALUE && strcmp(current->key->value.string, key->value.string) == 0) ||
			(current->key->type == AST_NUMBER_VALUE && current->key->value.number == key->value.number) ) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
void ASTArgumentList_set(ASTArgumentList* argument_list, ASTArgument* argument) {
	if(argument_list->first == NULL) {
		argument_list->first = argument_list->last = argument;
	} else if (argument->key == NULL) {
		argument_list->last->next = argument;
		argument_list->last = argument;
	} else {
		ASTArgument* found = ASTArgumentList_get(argument_list, argument->key);
		if(found == NULL) {
			argument_list->last->next = argument;
			argument_list->last = argument;
		} else {
			log_error("Argument list redefinition!");
			Error_throw();
		}
	}
}
ASTFunctionExpression* ASTFunctionExpression_create(ASTValue* name, ASTArgumentList* argument_list) {
	// TODO MAKE VALUE_FN_EXPR_CREATE()
	ASTFunctionExpression* fn_expr = (ASTFunctionExpression*)malloc(sizeof(ASTFunctionExpression));
	fn_expr->name = name;
	fn_expr->argument_list = argument_list;
	return fn_expr;
}

ASTProgram* ASTProgram_create(ASTBlock* block) {
	ASTProgram* program = (ASTProgram*)malloc(sizeof(ASTProgram));
	program->block = block;
	return program;
}
void ASTProgram_destroy(ASTProgram* program) {
	ASTBlock_destroy(program->block);
	free(program);
}