#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"

// ----------------------------------------
struct Parser;
typedef struct Parser {
	
	Token* first;
	Token* current;
	ASTProgram* program;
	
} Parser;

// ----------------------------------------
Parser* Parser_create(Token* current);
void Parser_destroy(Parser* parser);

ASTProgram* Parser_parse(Parser* parser);
void* Parser_parse_next(Parser* parser);

ASTValue* Parser_parse_literal(Parser* parser);

ASTArgumentList* Parser_parse_call_argument_list(Parser* parser);
ASTValue* Parser_parse_call_expression(Parser* parser);

ASTValue* Parser_parse_binary_expression(Parser* parser);
ASTValue* Parser_parse_equality_expression(Parser* parser);
ASTValue* Parser_parse_comparison_expression(Parser* parser);
ASTValue* Parser_parse_additive_expression(Parser* parser);
ASTValue* Parser_parse_term_expression(Parser* parser);

ASTObjectPair* Parser_parse_objectPair(Parser* parser);
ASTObject* Parser_parse_object(Parser* parser);

ASTStatement* Parser_parse_statement(Parser* parser);
ASTStatement* Parser_parse_declaration_stmt(Parser* parser);
ASTStatement* Parser_parse_expression_stmt(Parser* parser);
ASTStatement* Parser_parse_if_stmt(Parser* parser);

ASTBlock* Parser_parse_block(Parser* parser);

ASTProgram* Parser_parse_program(Parser* parser);

void Parser_parse_EOS(Parser* parser);

void Parser_advance(Parser* parser);

#endif