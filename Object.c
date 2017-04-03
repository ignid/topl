#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Object.h"

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
		printf("Error on malloc'ing array\n");
	}
	
	return array;
}
void Array_push(Array* array, ArrayElement* arrayElement) {
	if(array->length == array->capacity) {
		array->capacity *= 2;
		void* temp_elements = (ArrayElement**)realloc(array->elements, sizeof(ArrayElement*) * array->capacity);
		if(temp_elements == NULL) {
			printf("Error on realloc'ing new array\n");
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