#define OBJ_STRING_TYPE 0
#define OBJ_NUMBER_TYPE 1
#define OBJ_OBJECT_TYPE 2
#define OBJ_ARRAY_TYPE 3

struct Value;
struct ObjectPair;
struct Object;
struct ArrayElement;
struct Array;

// ----------------------------------------
// Object value: a string, number, object or array
typedef struct Value {
	
	union {
		char* string;
		double* number;
		struct Object* object;
		struct Array* array;
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
Object* Object_create();
ObjectPair* Object_get_pair(Object* object, char* key);
void* Object_get(Object* object, char* key);
void Object_set(Object* object, ObjectPair* objectPair);
void Object_destroy(Object* object);

ObjectPair* ObjectPair_create();
void ObjectPair_destroy(ObjectPair* objectPair);

Value* String_create(char* string);
Value* Number_create(double number);
Value* Value_object_create(Object* object);
Value* Value_create();
void Value_destroy(Value* value);
void* Value_get(Value* value);

ArrayElement* ArrayElement_create(Value* value);
void ArrayElement_destroy(ArrayElement* arrayElement);

Array* Array_create();
void Array_push(Array* array, ArrayElement* arrayElement);
ArrayElement* Array_get_element(Array* array, int id);
void* Array_get(Array* array, int id);
void Array_destroy(Array* array);