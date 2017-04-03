#include "AST.c"
#include "Lexer.c"

// ----------------------------------------
typedef struct {
	
	Token* first;
	Token* current;
	ASTProgram* program;
	
} Parser;

// ----------------------------------------
Parser* Parser_create(Token* current);
void Parser_destroy(Parser* parser);

ASTProgram* Parser_parse(Parser* parser);
void* Parser_parse_next(Parser* parser);

ASTValue* Parser_parse_value(Parser* parser);
ASTValue* Parser_parse_literal(Parser* parser);

ASTValue* Parser_parse_binary_expression(Parser* parser);
ASTValue* Parser_parse_additive_expression(Parser* parser);
ASTValue* Parser_parse_term_expression(Parser* parser);

ASTObjectPair* Parser_parse_objectPair(Parser* parser);
ASTObject* Parser_parse_object(Parser* parser);

ASTStatement* Parser_parse_statement(Parser* parser);
ASTStatement* Parser_parse_declaration_stmt(Parser* parser);
ASTStatement* Parser_parse_expression_stmt(Parser* parser);

ASTProgram* Parser_parse_program(Parser* parser);

void Parser_parse_EOS(Parser* parser);

Token* Parser_advance(Parser* parser);