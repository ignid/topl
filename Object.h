#ifndef OBJECT_H
#define OBJECT_H

#define OBJ_STRING_TYPE 0
#define OBJ_NUMBER_TYPE 1
#define OBJ_OBJECT_TYPE 2
#define OBJ_ARRAY_TYPE 3
#define OBJ_FN_TYPE 4
#define OBJ_INTEGER_TYPE 5

#define OBJ_FN_NATIVE 0
#define OBJ_FN_REGULAR 1

struct Value;
struct ObjectPair;
struct Object;
struct ArrayElement;
struct Array;
struct Function;

// ----------------------------------------
// Object value: a string, number, object or array
typedef struct Value {
	
	union {
		char* string;
		int integer;
		double number;
		struct Object* object;
		struct Array* array;
		struct Function* function;
	} value;
	int type;
	
} Value;

// ----------------------------------------
// An object structure is represented as a pair of curly brackets
// surrounding zero or more name/value pairs (or members).
typedef struct ObjectPair {
	
	char* key;
	Value* value;
	struct ObjectPair* next;
	
} ObjectPair;

typedef struct Object {
	
	ObjectPair* first;
	ObjectPair* last;
	
} Object;

// ----------------------------------------
// Array structure is represented with "[" elements "]" with elements
// separated by ","
typedef struct ArrayElement {
	
	Value* value;
	size_t index;
	
} ArrayElement;

typedef struct Array {
	
	ArrayElement** elements;
	size_t length;
	size_t capacity;
	
} Array;

// ----------------------------------------
// Arguments
struct Argument;
typedef struct Argument {
	
	Value* key;
	Value* value;
	struct Argument* next;
	
} Argument;

struct ArgumentList;
typedef struct ArgumentList {
	
	Argument* first;
	Argument* last;
	
} ArgumentList;

// Function
struct Scope;
typedef struct Function {
	
	union {
		struct Block* block;
		Value* (*native)(struct Scope* scope, ArgumentList* arguments);
	} code;
	int type;
	ArgumentList* arguments; // call arguments maps to function arguments
	// aka calling a(1) with fn a(b) {return b;} evaluates to 1
	// if arguments is NULL, does not do mapping when function calls
	
} Function;

// ----------------------------------------
Object* Object_create();
ObjectPair* Object_get_pair(Object* object, char* key);
Value* Object_get(Object* object, char* key);
void Object_set(Object* object, ObjectPair* objectPair);
void Object_destroy(Object* object);

ObjectPair* ObjectPair_create();
void ObjectPair_destroy(ObjectPair* objectPair);

Value* String_create(char* string);
Value* Integer_create(int number);
Value* Number_create(double number);
Value* Value_object_create(Object* object);
Value* Value_fn_create(Function* function);
Value* Value_create();
void Value_destroy(Value* value);

ArrayElement* ArrayElement_create(Value* value);
void ArrayElement_destroy(ArrayElement* arrayElement);

Array* Array_create();
void Array_push(Array* array, ArrayElement* arrayElement);
ArrayElement* Array_get_element(Array* array, int id);
Value* Array_get(Array* array, int id);
void Array_destroy(Array* array);

Argument* Argument_create(Value* key);
void Argument_destroy(Argument* argument);
ArgumentList* ArgumentList_create();
ArgumentList* ArgumentList_inherit(ArgumentList* argument_list);
Argument* ArgumentList_get(ArgumentList* argument_list, Value* key);
void ArgumentList_set(ArgumentList* argument_list, Argument* argument);
void ArgumentList_destroy(ArgumentList* argument_list);

Function* Function_create(struct Block* block, ArgumentList* arguments);
Function* NativeFunction_create(Value* (*native)(struct Scope* scope, ArgumentList* arguments), ArgumentList* arguments);
void Function_destroy(Function* function);
Function* Function_inherit(Function* function);

#endif