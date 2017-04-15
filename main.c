#include <stdio.h>
#include <string.h>

#ifdef LOGGING_ENABLED
#define LOG_USE_COLOR
#include "log.c/src/log.c"
#else
#include <stdarg.h>
#define log_set_level(X)
#define ERROR_COLOR "\x1b[31m"
#define FATAL_COLOR "\x1b[35m"
void log_log(int level, const char *file, int line, const char *fmt, ...) {
	if(level < 4) { // LOG_ERROR
		return;
	}
	
	if(level == 4) {
		fprintf(stdout, "%sERROR\x1b[0m \x1b[90m%s:%d:\x1b[0m ", ERROR_COLOR, file, line);
	} else {
		fprintf(stdout, "%sFATAL\x1b[0m \x1b[90m%s:%d:\x1b[0m ", FATAL_COLOR, file, line);
	}
	
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "\n");
}
#endif

#include "Conversion.c"
#include "Error.c"
#include "AST.c"
#include "Object.c"
#include "Lexer.c"
#include "Parser.c"
#include "Interpreter.c"

void help() {
	printf("Usage:\n");
	#ifdef LOGGING_ENABLED
	printf("    ./main (d) [FILE]\n");
	#else
	printf("    ./main [FILE]\n");
	#endif
	
	printf("where FILE is the file for TOPL to interpret\n");
	#ifdef LOGGING_ENABLED
	printf("      pass d to output debugging information\n");
	#endif
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
		// no need to free(string) because Lexer_destroy do it for us
	} else {
		if(lexer != NULL) Lexer_destroy(lexer);
		if(parser != NULL) Parser_destroy(parser);
		if(interpreter != NULL) Interpreter_destroy(interpreter);
	}
	
	return 0;
}