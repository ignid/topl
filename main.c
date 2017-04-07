#include <stdio.h>
#include <string.h>
#define LOG_USE_COLOR 1
#include "log.c/src/log.c"
#include "Error.c"
#include "AST.c"
#include "Object.c"
#include "Lexer.c"
#include "Parser.c"
#include "Interpreter.c"

void help() {
	printf("Usage:\n");
	printf("    ./main (d) [FILE]\n");
	printf("where FILE is the file for TOPL to interpret\n");
	printf("      pass d to output debugging information\n");
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		help();
		exit(0);
	}
	
	char* filename;
	if(strcmp(argv[1], "d") == 0) {
		filename = argv[2];
	} else {
		filename = argv[1];
		log_set_level(LOG_ERROR);
	}
	
	FILE *f;
	if ( (f = fopen(filename,"rb")) == NULL ){
		log_fatal("Error loading file!\n");
		exit(1);
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	
	Lexer* lexer = NULL;
	Parser* parser = NULL;
	Interpreter* interpreter = NULL;
	if(setjmp(env) == 0) {
		lexer = Lexer_create(string);
		Lexer_parse(lexer);
		
		parser = Parser_create(lexer->first);
		Parser_parse(parser);
		
		Scope* scope = Scope_create();
		Program* program = Program_create(parser->program, scope);
		interpreter = Interpreter_create(program);
		Interpreter_interpret(interpreter);
		
		Lexer_destroy(lexer);
		Parser_destroy(parser);
		Interpreter_destroy(interpreter);
	} else {
		if(lexer != NULL) Lexer_destroy(lexer);
		if(parser != NULL) Parser_destroy(parser);
		if(interpreter != NULL) Interpreter_destroy(interpreter);
	}
	
	return 0;
}