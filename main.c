#include <stdio.h>
#include <string.h>
#include "log.c/src/log.c"
#include "Error.c"
#include "AST.c"
#include "Object.c"
#include "Lexer.c"
#include "Parser.c"
#include "Interpreter.c"
#define BUFFER_SIZE 1024

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
		log_set_quiet(0);
	} else {
		filename = argv[1];
		log_set_quiet(1);
	}
	
	FILE* file;
	if ( (file = fopen(filename,"rb")) == NULL ){
		printf("Error loading file!\n");
		exit(1);
	}
	
	
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, file);
	fclose(file);
	string[fsize] = '\0';
	
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