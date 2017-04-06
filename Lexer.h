#ifndef LEXER_H
#define LEXER_H

#define TOK_STRING_TYPE 0
#define TOK_NUMBER_TYPE 1
#define TOK_KEYWORD_TYPE 2
#define TOK_OPERATOR_TYPE 3
#define TOK_IDENTIFIER_TYPE 4

// ----------------------------------------
struct Token;
struct Lexer;

// ----------------------------------------
typedef struct Token {
	int type;
	char* value;
	struct Token* next;
} Token;

// ----------------------------------------
typedef struct Lexer {
	
	char* source;
	size_t position;
	size_t length;
	
	Token* first;
	Token* last;
	
} Lexer;

// ----------------------------------------
Lexer* Lexer_create(char* source);
void Lexer_destroy(Lexer* lexer);
void Lexer_push(Lexer* lexer, Token* token);
Token* Lexer_parse(Lexer* lexer);
Token* Lexer_parse_next(Lexer* lexer);
Token* Lexer_parse_string(Lexer* lexer);
Token* Lexer_parse_number(Lexer* lexer);
Token* Lexer_parse_identifier(Lexer* lexer);
Token* Lexer_parse_operator(Lexer* lexer);
void Lexer_ignore_whitespace(Lexer* lexer);

Token* Token_create(int type, char* value);
void Token_destroy(Token* token);

#endif