#include <stdio.h>
#include "Object.c"

int main() {
	printf("Literal tests\n");
	
	// string
	Value* string = String_create("Hello World");
	printf("String %s\n",Value_get(string));
	
	// number
	Value* number = Number_create(69420);
	printf("Number %f\n",*(float*)Value_get(number));
	
	// object
	Object* object_lit = Object_create();
	ObjectPair* pair_lit = ObjectPair_create("hello", String_create("goodbye"));
	Object_set(object_lit, pair_lit);
	Value* object_value = Value_object_create(object_lit);
	printf("Object: { hello: %s }\n", Object_get((Object*)Value_get(object_value), "hello"));
	
	printf("\n");
	
	// object test
	Object* object = Object_create();
	ObjectPair* pair = ObjectPair_create("hello", string);
	Object_set(object, pair);
	printf("hello: %s\n", Object_get(object, "hello"));
	
	// pushing object pair test
	ObjectPair* pair2 = ObjectPair_create("goodbye", number);
	Object_set(object, pair2);
	printf("goodbye: %f\n", *(double*)Object_get(object, "goodbye"));
	
	// object value mutation test
	Value* string2 = String_create("WOW!");
	ObjectPair* pair3 = ObjectPair_create("hello", string2);
	Object_set(object, pair3);
	printf("hello: %s\n", Object_get(object, "hello"));
	
	printf("\n");
	
	// array test
	Array* array = Array_create();
	Value* string3 = String_create("Hello World");
	ArrayElement* string_elem = ArrayElement_create(string3);
	Array_push(array, string_elem);
	printf("Array element #0: %s\n", Array_get(array, 0));
	
	Value* string4 = String_create("Hey there!");
	ArrayElement* string2_elem = ArrayElement_create(string4);
	Array_push(array, string2_elem);
	printf("Array element #1: %s\n", Array_get(array, 1));
	
	Value* string5 = String_create("Woah!");
	ArrayElement* string5_elem = ArrayElement_create(string5);
	Array_push(array, string5_elem);
	printf("Array element #2: %s\n", Array_get(array, 2));
	printf("Array size: %i/%i\n", array->length, array->capacity);
	
	Array_destroy(array);
	Object_destroy(object);
	return 0;
}