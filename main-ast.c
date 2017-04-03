#include <stdio.h>
#include "Parser.c"

int main() {
	Lexer* lexer = Lexer_create("a='a'; a=1+1;");
	Lexer_parse(lexer);
	
	printf("----------------------------------------\n");
	
	Parser* parser = Parser_create(lexer->first);
	Parser_parse(parser);
	
	printf("----------------------------------------\n");
	
	Lexer_destroy(lexer);
	Parser_destroy(parser);
	
	return 0;
}