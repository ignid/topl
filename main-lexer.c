#include <stdio.h>
#include "Lexer.c"

int main() {
	Lexer* lexer = Lexer_create("{ 'hello':1000 }");
	printf("%i/%i\n", lexer->position, lexer->length);
	
	Lexer_parse(lexer);
	return 0;
}