#include <stdio.h>
#include "log.c/src/log.c"
#include "Error.c"
#include "Lexer.c"

int main() {
	Lexer* lexer = Lexer_create("print('11111111111111111111');");
	Lexer_parse(lexer);
	return 0;
}