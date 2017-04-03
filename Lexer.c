#include <string.h>
#include <ctype.h>
#include "Lexer.h"

Lexer* Lexer_create(char* source) {
	Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
	
	lexer->source = strdup(source);
	lexer->position = 0;
	lexer->length = strlen(lexer->source);
	
	lexer->first = NULL;
	lexer->last = NULL;
	
	return lexer;
}
void Lexer_destroy(Lexer* lexer) {
	if(lexer->first != NULL) {
		Token* current = lexer->first;
		Token* next;
		while(current != NULL) {
			next = current->next;
			Token_destroy(current);
			current = next;
		}
	}
	free(lexer->source);
	free(lexer);
}
void Lexer_push(Lexer* lexer, Token* token) {
	if(lexer->first == NULL) {
		lexer->first = lexer->last = token;
	} else {
		lexer->last->next = token;
		lexer->last = token;
	}
}
Token* Lexer_parse(Lexer* lexer) {
	Token* next = NULL;
	while(1) {
		next = Lexer_parse_next(lexer);
		if(next == NULL) {
			break;
		} else {
			Lexer_push(lexer, next);
		}
	}
	return lexer->first;
}
Token* Lexer_parse_next(Lexer* lexer) {
	if(lexer->position > lexer->length) return NULL;
	
	char current = lexer->source[lexer->position];
	if (isspace(current)) {
		Lexer_ignore_whitespace(lexer);
		current = lexer->source[lexer->position];
	}
	
	if (
		current == '{' || current == '}' || current == ':' ||
		current == '+' || current == '-' || current == '*' || current == '/' ||
		current == '=' ||
		current == ';'
	) {
		return Lexer_parse_operator(lexer);
	} else if (current == '"' || current == '\'') {
		return Lexer_parse_string(lexer);
	} else if (isdigit(current)) {
		return Lexer_parse_number(lexer);
	} else if (isalpha(current)) {
		return Lexer_parse_identifier(lexer);
	} else if (current == '\0') {
	} else {
		printf("Unexpected %c\n", current);
	}
	return NULL;
}
Token* Lexer_parse_string(Lexer* lexer) {
	char start = lexer->source[lexer->position++];
	char* string = (char*)malloc(1);
	size_t length = 0;
	while(lexer->position < lexer->length) {
		char current = lexer->source[lexer->position];
		if(current == start) {
			lexer->position++;
			break;
		} else {
			string = (char*)realloc(string, length + 1);
			string[length++] = current;
			lexer->position++;
		}
	}
	string[length] = '\0';
	printf("STRING %s\n", string);
	return Token_create(TOK_STRING_TYPE, string);
}
Token* Lexer_parse_number(Lexer* lexer) {
	char* string = (char*)malloc(1);
	size_t length = 0;
	while(lexer->position < lexer->length) {
		char current = lexer->source[lexer->position];
		if(isdigit(current)) {
			string = (char*)realloc(string, length + 1);
			string[length++] = current;
			lexer->position++;
		} else {
			break;
		}
	}
	string[length] = '\0';
	printf("NUMBER %s\n", string);
	return Token_create(TOK_NUMBER_TYPE, string);
}
Token* Lexer_parse_identifier(Lexer* lexer) {
	char* string = (char*)malloc(1);
	size_t length = 0;
	while(lexer->position < lexer->length) {
		char current = lexer->source[lexer->position];
		if(isalpha(current)) {
			string = (char*)realloc(string, length + 1);
			string[length++] = current;
			lexer->position++;
		} else {
			break;
		}
	}
	string[length] = '\0';
	printf("IDENTIFIER %s\n", string);
	// keywords: true, false, null
	if(strcmp(string, "false") == 0 ||
		strcmp(string, "true") == 0 ||
		strcmp(string, "null") == 0) {
		return Token_create(TOK_KEYWORD_TYPE, string);
	}
	return Token_create(TOK_IDENTIFIER_TYPE, string);
}
Token* Lexer_parse_operator(Lexer* lexer) {
	char current = lexer->source[lexer->position++];
	char* string = malloc(2);
	string[0] = current;
	string[1] = '\0';
	printf("OPERATOR %s\n", string);
	return Token_create(TOK_OPERATOR_TYPE, string);
}
void Lexer_ignore_whitespace(Lexer* lexer) {
	while(lexer->position < lexer->length) {
		char current = lexer->source[lexer->position];
		if(isspace(current)) {
			lexer->position++;
			continue;
		} else {
			return;
		}
	}
}

Token* Token_create(int type, char* value) {
	Token* token = (Token*)malloc(sizeof(Token));
	token->type = type;
	token->value = value;
	return token;
}
void Token_destroy(Token* token) {
	free(token->value);
	free(token);
}