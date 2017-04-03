#define AST_UNDEFINED_VALUE -1
#define AST_STRING_VALUE 0
#define AST_NUMBER_VALUE 1
#define AST_OBJECT_VALUE 2
#define AST_ARRAY_VALUE 3
#define AST_BINEXPR_VALUE 4
#define AST_IDENTIFIER_VALUE 5

#define AST_PLUS_OP 0
#define AST_MINUS_OP 1
#define AST_DIV_OP 2
#define AST_MULT_OP 3

#define AST_DECL_STATEMENT 0
#define AST_EXPR_STATEMENT 1
#define AST_IF_STATEMENT 2

struct ASTValue;
struct ASTObjectPair;
struct ASTObject;
struct ASTArrayElement;
struct ASTArray;
struct ASTBinaryExpression;
struct ASTProgram;

// ----------------------------------------
// Value
typedef struct ASTValue {
	
	union {
		double* number;
		char* string;
		struct ASTObject* object;
		struct ASTArray* array;
		struct ASTBinaryExpression* bin_expr;
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
// Binary expression tree
typedef struct ASTBinaryExpression {
	
	int op;
	ASTValue* left;
	ASTValue* right;
	
} ASTBinaryExpression;

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
struct ASTDeclarationStatement;
struct ASTExpressionStatement;
struct ASTStatement;

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
	struct ASTStatement** body;
	
} ASTIfStatement;

// Statements
typedef struct ASTStatement {
	
	union {
		ASTDeclarationStatement* declaration;
		ASTExpressionStatement* expression;
		//ASTIfStatement* if_;
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
ASTValue* ASTValue_create();
void ASTValue_destroy(ASTValue* value);
void* ASTValue_get(ASTValue* value);

ASTObjectPair* ASTObjectPair_create(char* key, ASTValue* value);
void ASTObjectPair_destroy(ASTObjectPair* objectPair);
ASTObject* ASTObject_create();
void ASTObject_set(ASTObject* object, ASTObjectPair* objectPair);
void ASTObject_destroy(ASTObject* object);
ASTObjectPair* ASTObject_get(ASTObject* object, char* key);

ASTArrayElement* ASTArrayElement_create();
ASTArray* ASTArray_create();
void ASTArray_push(ASTArray* array, ASTArrayElement* arrayElement);
void ASTArray_destroy(ASTArray* array);

ASTBinaryExpression* ASTBinaryExpression_create(int op, ASTValue* left, ASTValue* right);
void ASTBinaryExpression_destroy(ASTBinaryExpression* bin_expr);

ASTStatement* ASTStatement_create();
void ASTStatement_destroy(ASTStatement* statement);
void* ASTStatement_get(ASTStatement* statement);
ASTStatement* ASTDeclarationStatement_create(ASTValue* left, ASTValue* right);
ASTStatement* ASTExpressionStatement_create(ASTValue* expression);

ASTBlock* ASTBlock_create();
void ASTBlock_destroy(ASTBlock* block);
void ASTBlock_push(ASTBlock* block, ASTBlockStatement* blockStmt);
ASTBlockStatement* ASTBlockStatement_create(ASTStatement* statement);

ASTProgram* ASTProgram_create(ASTBlock* block);
void ASTProgram_destroy(ASTProgram* program);