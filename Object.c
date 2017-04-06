#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.c/src/log.h"
#include "Error.h"
#include "Object.h"
#include "Interpreter.h"

// objects
Object* Object_create() {
	Object* object = (Object*)malloc(sizeof(Object));
	object->first = NULL;
	object->last = NULL;
	return object;
}
ObjectPair* Object_get_pair(Object* object, char* key) {
	ObjectPair* current = object->first;
	while(current != NULL) {
		if(strcmp(current->key, key) == 0) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
void* Object_get(Object* object, char* key) {
	ObjectPair* pair = Object_get_pair(object, key);
	if(pair == NULL) {
		return NULL;
	} else {
		return Value_get(pair->value);
	}
}
void Object_set(Object* object, ObjectPair* objectPair) {
	if(object->first == NULL) {
		object->first = object->last = objectPair;
	} else {
		ObjectPair* found = Object_get_pair(object, objectPair->key);
		if(found == NULL) {
			object->last->next = objectPair;
			object->last = objectPair;
		} else {
			Value_destroy(found->value);
			found->value = objectPair->value;
		}
	}
}
void Object_destroy(Object* object) {
	ObjectPair* current = object->first;
	ObjectPair* next;
	while(current != NULL) {
		next = current->next;
		ObjectPair_destroy(current);
		current = next;
	}
	free(object);
}

// object pairs
ObjectPair* ObjectPair_create(char* key, Value* value) {
	ObjectPair* objectPair = (ObjectPair*)malloc(sizeof(ObjectPair));
	objectPair->key = key;
	objectPair->value = value;
	return objectPair;
}
void ObjectPair_destroy(ObjectPair* objectPair) {
	Value_destroy(objectPair->value);
	free(objectPair);
}

// Value
Value* String_create(char* string) {
	Value* value = Value_create();
	value->value.string = strdup(string);
	value->type = OBJ_STRING_TYPE;
	return value;
}
Value* Number_create(double number) {
	Value* value = Value_create();
	value->value.number = (double*)malloc(sizeof(double));
	*value->value.number = number;
	value->type = OBJ_NUMBER_TYPE;
	return value;
}
Value* Value_object_create(Object* object) {
	Value* value = Value_create();
	value->value.object = object;
	value->type = OBJ_OBJECT_TYPE;
	return value;
}
Value* Value_fn_create(Function* function) {
	Value* value = Value_create();
	value->value.function = function;
	value->type = OBJ_FN_TYPE;
	return value;
}
Value* Value_create() {
	return (Value*)malloc(sizeof(Value*));
}
void Value_destroy(Value* value) {
	if(value->type == OBJ_STRING_TYPE) {
		free(value->value.string);
	} else if(value->type == OBJ_NUMBER_TYPE) {
		free(value->value.number);
	} else if(value->type == OBJ_OBJECT_TYPE) {
		Object_destroy(value->value.object);
	} else if(value->type == OBJ_ARRAY_TYPE) {
		Array_destroy(value->value.array);
	} else if(value->type == OBJ_FN_TYPE) {
		Function_destroy(value->value.function);
	}
	free(value);
}
void* Value_get(Value* value) {
	int type = value->type;
	if(type == OBJ_STRING_TYPE) {
		return (void*)value->value.string;
	} else if(type == OBJ_NUMBER_TYPE) {
		return (void*)value->value.number;
	} else if(type == OBJ_OBJECT_TYPE) {
		return (void*)value->value.object;
	} else if(type == OBJ_ARRAY_TYPE) {
		return (void*)value->value.array;
	} else {
		return NULL;
	}
}

// array elements
ArrayElement* ArrayElement_create(Value* value) {
	ArrayElement* arrayElement = (ArrayElement*)malloc(sizeof(ArrayElement));
	arrayElement->value = value;
	arrayElement->index = 0;
	return arrayElement;
}
void ArrayElement_destroy(ArrayElement* arrayElement) {
	Value_destroy(arrayElement->value);
	free(arrayElement);
}

// arrays
Array* Array_create() {
	Array* array = (Array*)malloc(sizeof(Array));
	
	array->length = 0;
	array->capacity = 2;
	
	array->elements = (ArrayElement**)malloc(sizeof(ArrayElement*) * array->capacity);
	if(array->elements == NULL) {
		log_error("Error on malloc'ing array");
		Error_throw(1);
	}
	
	return array;
}
void Array_push(Array* array, ArrayElement* arrayElement) {
	if(array->length == array->capacity) {
		array->capacity *= 2;
		void* temp_elements = (ArrayElement**)realloc(array->elements, sizeof(ArrayElement*) * array->capacity);
		if(temp_elements == NULL) {
			log_error("Error on realloc'ing new array");
			Error_throw(1);
		} else {
			array->elements = temp_elements;
		}
	}
	array->elements[array->length] = arrayElement;
	arrayElement->index = array->length;
	array->length++;
}
ArrayElement* Array_get_element(Array* array, int id) {
	return array->elements[id];
}
void* Array_get(Array* array, int id) {
	return Value_get(Array_get_element(array, id)->value);
}
void Array_destroy(Array* array) {
	size_t i;
	for(i = 0; i < array->length; i++) {
		ArrayElement_destroy(array->elements[i]);
	}
	free(array);
}

Argument* Argument_create(Value* key) {
	Argument* argument = (Argument*)malloc(sizeof(Argument));
	argument->key = key;
	argument->value = NULL;
	argument->next = NULL;
	return argument;
}
void Argument_destroy(Argument* argument) {
	if(argument->value != NULL)
		Value_destroy(argument->value);
	free(argument);
}
ArgumentList* ArgumentList_create() {
	ArgumentList* argument_list = (ArgumentList*)malloc(sizeof(ArgumentList));
	argument_list->first = NULL;
	argument_list->last = NULL;
	return argument_list;
}
ArgumentList* ArgumentList_inherit(ArgumentList* argument_list) {
	ArgumentList* argument_list1 = ArgumentList_create();
	argument_list1->first = argument_list1->last = argument_list->first;
	
	Argument* current = argument_list->first->next;
	while(current != NULL) {
		argument_list->last->next = current;
		argument_list->last = current;
		current = current->next;
	}
	
	return argument_list1;
}
Argument* ArgumentList_get(ArgumentList* argument_list, Value* key) {
	Argument* current = argument_list->first;
	while(current != NULL) {
		// TODO value_compare?
		if( (current->key->type == OBJ_STRING_TYPE && key->type == OBJ_STRING_TYPE && strcmp(Value_get(current->key), Value_get(key)) == 0) ||
			(current->key->type == OBJ_NUMBER_TYPE && key->type == OBJ_NUMBER_TYPE && (*(double*)Value_get(current->key) == *(double*)Value_get(key)) ) ) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
Argument* ArgumentList_get_by_index(ArgumentList* argument_list, int index) {
	Argument* current;
	int i;
	// 0 <= 0 -> 
	for(i = 0, current = argument_list->first; i <= index; ++i) {
		//log_info("I %i\n", i);
		if(current == NULL) {
			break;
		} else if (i == index) {
			return current;
		} else {
			current = current->next;
		}
	}
	return NULL;
}
void ArgumentList_set(ArgumentList* argument_list, Argument* argument) {
	if(argument_list->first == NULL) {
		argument_list->first = argument_list->last = argument;
	} else {
		Argument* found = ArgumentList_get(argument_list, argument->key);
		if(found == NULL) {
			argument_list->last->next = argument;
			argument_list->last = argument;
		} else {
			Value_destroy(found->value);
			found->value = argument->value;
		}
	}
}
void ArgumentList_push(ArgumentList* argument_list, Argument* argument) {
	argument_list->last->next = argument;
	argument_list->last = argument;
}
void ArgumentList_destroy(ArgumentList* argument_list) {
	Argument* current = argument_list->first;
	Argument* next;
	while(current != NULL) {
		next = current->next;
		Argument_destroy(current);
		current = next;
	}
	free(argument_list);
}

Function* Function_create(struct Block* block, ArgumentList* arguments) {
	Function* function = (Function*)malloc(sizeof(Function));
	function->type = OBJ_FN_REGULAR;
	function->code.block = block;
	function->arguments = arguments;
	return function;
}
Function* NativeFunction_create(Value* (*native)(struct Scope* scope, ArgumentList* arguments), ArgumentList* arguments) {
	Function* function = (Function*)malloc(sizeof(Function));
	function->type = OBJ_FN_NATIVE;
	function->code.native = native;
	function->arguments = arguments;
	return function;
}
void Function_destroy(Function* function) {
	if(function->type == OBJ_FN_REGULAR)
		Block_destroy(function->code.block);
	if(function->arguments != NULL)
		ArgumentList_destroy(function->arguments);
	free(function);
}
Function* Function_inherit(Function* function) {
	return Function_create(Block_inherit(function->code.block), ArgumentList_inherit(function->arguments));
}