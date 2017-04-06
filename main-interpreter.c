#include <stdio.h>
#include "log.c/src/log.c"
#include "Error.c"
#include "AST.c"
#include "Object.c"
#include "Lexer.c"
#include "Parser.c"
#include "Interpreter.c"

int main() {
	Lexer* lexer = NULL;
	Parser* parser = NULL;
	Interpreter* interpreter = NULL;
	
	if(setjmp(env) == 0) {
		lexer = Lexer_create("print(10 + 10);");
		Lexer_parse(lexer);
		
		printf("----------------------------------------\n");
		
		parser = Parser_create(lexer->first);
		Parser_parse(parser);
		
		printf("----------------------------------------\n");
		
		Scope* scope = Scope_create();
		Program* program = Program_create(parser->program, scope);
		interpreter = Interpreter_create(program);
		Interpreter_interpret(interpreter);
		
		printf("----------------------------------------\n");
		Lexer_destroy(lexer);
		Parser_destroy(parser);
		Interpreter_destroy(interpreter);
	} else {
		printf("----------------------------------------\n");
		if(lexer != NULL) Lexer_destroy(lexer);
		if(parser != NULL) Parser_destroy(parser);
		if(interpreter != NULL) Interpreter_destroy(interpreter);
	}
	
	return 0;
}