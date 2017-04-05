#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Object.h"
#include "Parser.h"

// ----------------------------------------
struct ScopeVariable;
struct Scope;
struct Block;
struct Program;
struct Interpreter;

// ----------------------------------------
// Represents a hashtable mapping identifiers to a certain variable
typedef struct ScopeVariable {
	
	char* key;
	Value* value;
	
	struct ScopeVariable* next;
	struct Scope* scope;
	
} ScopeVariable;

typedef struct Scope {
	
	ScopeVariable* first;
	ScopeVariable* last;
	
} Scope;

// ----------------------------------------
typedef struct Block {
	
	ASTBlock* ast;
	Scope* scope;
	
	struct Block* parent;
	
} Block;

// ----------------------------------------
typedef struct Program {
	
	ASTProgram* ast;
	Block* block;
	
} Program;

// ----------------------------------------
typedef struct Interpreter {
	
	Program* program;
	
} Interpreter;

Interpreter* Interpreter_create(Program* program);
void Interpreter_interpret(Interpreter* interpreter);
void Interpreter_destroy(Interpreter* interpreter);

Block* Block_create(ASTBlock* ast, Scope* scope);
void Block_destroy(Block* block);
Block* Block_inherit(Block* block);

Scope* Scope_create();
void Scope_destroy(Scope* scope);
Scope* Scope_inherit(Scope* scope);
ScopeVariable* Scope_get(Scope* scope, char* key);
void Scope_set(Scope* scope, char* key, Value* value);
void Scope_push(Scope* scope, char* key, Value* value);
ScopeVariable* ScopeVariable_create(Scope* scope, char* key, Value* value);

void Block_evaluate(Block* block);
Value* BlockStatement_evaluate(ASTBlockStatement* statement, Block* block);
Value* Statement_evaluate(ASTStatement* statement, Block* block);
Value* Value_evaluate(ASTValue* value, Block* block);

Program* Program_create(ASTProgram* program, Scope* scope);
void Program_destroy(Program* program);

#endif