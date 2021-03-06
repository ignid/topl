#ifndef AST_H
#define AST_H

#define AST_UNDEFINED_VALUE -1
#define AST_STRING_VALUE 0
#define AST_NUMBER_VALUE 1
#define AST_OBJECT_VALUE 2
#define AST_ARRAY_VALUE 3
#define AST_BINEXPR_VALUE 4
#define AST_IDENTIFIER_VALUE 5
#define AST_FN_VALUE 6
#define AST_INTEGER_VALUE 7
#define AST_OBJECT_EXPR_VALUE 8

#define AST_PLUS_OP 0
#define AST_MINUS_OP 1
#define AST_DIV_OP 2
#define AST_MULT_OP 3
#define AST_LESS_OP 4
#define AST_MORE_OP 5
#define AST_LESS_EQ_OP 6
#define AST_MORE_EQ_OP 7
#define AST_EQUAL_OP 8
#define AST_NOT_EQUAL_OP 9
#define AST_MODULO_OP 10
#define AST_AND_OP 11
#define AST_OR_OP 12

#define AST_DECL_STATEMENT 0
#define AST_EXPR_STATEMENT 1
#define AST_IF_STATEMENT 2
#define AST_WHILE_STATEMENT 3

struct ASTValue;
struct ASTObjectPair;
struct ASTObject;
struct ASTArrayElement;
struct ASTArray;
struct ASTBinaryExpression;
struct ASTArgument;
struct ASTArgumentList;
struct ASTFunctionExpression;
struct ASTObjectExpression;

struct ASTDeclarationStatement;
struct ASTExpressionStatement;
struct ASTFunctionStatement;
struct ASTIfStatement;
struct ASTWhileStatement;
struct ASTStatement;

struct ASTProgram;

// ----------------------------------------
// Value
typedef struct ASTValue {
	
	union {
		double number;
		int integer;
		char* string;
		struct ASTObject* object;
		struct ASTArray* array;
		struct ASTBinaryExpression* bin_expr;
		struct ASTFunctionExpression* fn_expr;
		struct ASTObjectExpression* object_expr;
	} value;
	int type;
	
} ASTValue;

// Object
typedef struct ASTObjectPair {
	
	char* key;
	ASTValue* value;
	struct ASTObjectPair* next;
	
} ASTObjectPair;

typedef struct ASTObject {
	
	ASTObjectPair* first;
	ASTObjectPair* last;
	
} ASTObject;

// Array
typedef struct ASTArrayElement {
	
	ASTValue* value;
	int index;
	struct ArrayElement* next;
	
} ASTArrayElement;

typedef struct ASTArray {
	
	ASTArrayElement* first;
	ASTArrayElement* last;
	
} ASTArray;

// ----------------------------------------
// Function
typedef struct ASTArgument {
	
	ASTValue* key;
	ASTValue* value;
	struct ASTArgument* next;
	
} ASTArgument;

typedef struct ASTArgumentList {
	
	ASTArgument* first;
	ASTArgument* last;
	
} ASTArgumentList;

typedef struct ASTFunctionExpression {
	
	ASTValue* name;
	ASTArgumentList* argument_list;
	
} ASTFunctionExpression;

// ----------------------------------------
// Binary expression tree
typedef struct ASTBinaryExpression {
	
	int op;
	ASTValue* left;
	ASTValue* right;
	
} ASTBinaryExpression;

// ----------------------------------------
// Object accessors
// a.b or a[b]
typedef struct ASTObjectExpression {
	
	ASTValue* head;
	ASTValue* tail;
	
} ASTObjectExpression;

// ----------------------------------------
struct ASTBlockStatement;
struct ASTBlock;

// Blocks
typedef struct ASTBlockStatement {
	
	struct ASTStatement* statement;
	struct ASTBlockStatement* next;
	
} ASTBlockStatement;

typedef struct ASTBlock {
	
	ASTBlockStatement* first;
	ASTBlockStatement* last;
	
} ASTBlock;

// ----------------------------------------
// Declaration statement
typedef struct ASTDeclarationStatement {
	
	ASTValue* left; // must be of type identifier
	ASTValue* right;
	
} ASTDeclarationStatement;

// Expression statement
typedef struct ASTExpressionStatement {
	
	ASTValue* expression;
	
} ASTExpressionStatement;

// If statement
typedef struct ASTIfStatement {
	
	ASTValue* expression;
	ASTBlock* block;
	ASTBlock* block_else;
	
} ASTIfStatement;

// While statement
typedef struct ASTWhileStatement {
	
	ASTValue* expression;
	ASTBlock* block;
	
} ASTWhileStatement;

// Statements
typedef struct ASTStatement {
	
	union {
		ASTDeclarationStatement* declaration;
		ASTExpressionStatement* expression;
		ASTIfStatement* ifelse;
		ASTWhileStatement* whiles;
	} statement;
	int type;
	
} ASTStatement;

// ----------------------------------------
// Program
typedef struct ASTProgram {
	
	ASTBlock* block;
	
} ASTProgram;

ASTValue* ASTIdentifier_create(char* string);
ASTValue* ASTString_create(char* string);
ASTValue* ASTNumber_create(double number);
ASTValue* ASTValue_object_create(ASTObject* object);
ASTValue* ASTValue_bin_expr_create(ASTBinaryExpression* bin_expr);
ASTValue* ASTValue_object_expr_create(ASTObjectExpression* object_expr);
ASTValue* ASTValue_create();
void ASTValue_destroy(ASTValue* value);

ASTObjectPair* ASTObjectPair_create(char* key, ASTValue* value);
void ASTObjectPair_destroy(ASTObjectPair* objectPair);
ASTObject* ASTObject_create();
void ASTObject_set(ASTObject* object, ASTObjectPair* objectPair);
void ASTObject_destroy(ASTObject* object);
ASTObjectPair* ASTObject_get(ASTObject* object, char* key);
ASTObjectExpression* ASTObjectExpression_create(ASTValue* head, ASTValue* tail);
void ASTObjectExpression_destroy(ASTObjectExpression* object_expr);

ASTArrayElement* ASTArrayElement_create();
ASTArray* ASTArray_create();
void ASTArray_push(ASTArray* array, ASTArrayElement* arrayElement);
void ASTArray_destroy(ASTArray* array);

ASTBinaryExpression* ASTBinaryExpression_create(int op, ASTValue* left, ASTValue* right);
void ASTBinaryExpression_destroy(ASTBinaryExpression* bin_expr);

ASTStatement* ASTStatement_create();
void ASTStatement_destroy(ASTStatement* statement);
ASTStatement* ASTDeclarationStatement_create(ASTValue* left, ASTValue* right);
ASTStatement* ASTExpressionStatement_create(ASTValue* expression);
ASTStatement* ASTIfStatement_create(ASTValue* expression, ASTBlock* block, ASTBlock* block_else);
ASTStatement* ASTWhileStatement_create(ASTValue* expression, ASTBlock* block);

ASTBlock* ASTBlock_create();
void ASTBlock_destroy(ASTBlock* block);
void ASTBlock_push(ASTBlock* block, ASTBlockStatement* blockStmt);
ASTBlockStatement* ASTBlockStatement_create(ASTStatement* statement);
void ASTBlockStatement_destroy(ASTBlockStatement* blockStmt);

ASTArgument* ASTArgument_create(ASTValue* key, ASTValue* value);
void ASTArgument_destroy(ASTArgument* argument);
ASTArgumentList* ASTArgumentList_create();
void ASTArgumentList_destroy(ASTArgumentList* argument_list);
ASTArgument* ASTArgumentList_get(ASTArgumentList* argument_list, ASTValue* key);
void ASTArgumentList_set(ASTArgumentList* argument_list, ASTArgument* argument);
ASTFunctionExpression* ASTFunctionExpression_create(ASTValue* name, ASTArgumentList* argument_list);

ASTProgram* ASTProgram_create(ASTBlock* block);
void ASTProgram_destroy(ASTProgram* program);

#endif