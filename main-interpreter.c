#include <stdio.h>
#include "Interpreter.c"

int main() {
	Lexer* lexer = Lexer_create("a=10;12*a;");
	Lexer_parse(lexer);
	
	printf("----------------------------------------\n");
	
	Parser* parser = Parser_create(lexer->first);
	Parser_parse(parser);
	
	printf("----------------------------------------\n");
	
	Scope* scope = Scope_create();
	Program* program = Program_create(parser->program, scope);
	Interpreter* interpreter = Interpreter_create(program);
	Interpreter_interpret(interpreter);
	
	Lexer_destroy(lexer);
	Parser_destroy(parser);
	
	return 0;
}