#include <stdlib.h>
#include "AST.h"
#include "Parser.h"

Parser* Parser_create(Token* token) {
	Parser* parser = (Parser*)malloc(sizeof(Parser));
	parser->first = token;
	parser->current = token;
	parser->program = NULL;
	return parser;
}
void Parser_destroy(Parser* parser) {
	ASTProgram_destroy(parser->program);
	free(parser);
}
ASTProgram* Parser_parse(Parser* parser) {
	ASTProgram* program = Parser_parse_program(parser);
	parser->program = program;
	return program;
}

ASTValue* Parser_parse_literal(Parser* parser) {
	ASTValue* value;
	Token* current = parser->current;
	if(current == NULL) {
		return NULL;
	} else if(current->type == TOK_STRING_TYPE) {
		printf("STRING TOKEN %s\n", current->value);
		value = ASTString_create(current->value);
	} else if (current->type == TOK_NUMBER_TYPE) {
		value = ASTNumber_create(strtod(current->value, NULL));
		printf("NUMBER %f\n", *(value->value.number));
	} else if (current->type == TOK_IDENTIFIER_TYPE) {
		value = ASTIdentifier_create(current->value);
		printf("IDENTIFIER %s\n", current->value);
	} else {
		return NULL;
	}
	Parser_advance(parser);
	return value;
}

ASTArgumentList* Parser_parse_call_argument_list(Parser* parser) {
	ASTArgumentList* argument_list = ASTArgumentList_create();
	size_t length = 1;
	
	ASTValue* first = Parser_parse_literal(parser);
	if(first == NULL) {
		return argument_list;
	}
	ASTArgumentList_set(argument_list, ASTArgument_create(NULL, first));
	
	Token* current;
	while( (current = parser->current) != NULL) {
		if (strcmp(current->value, ",") == 0) {
			Parser_advance(parser);
			ASTValue* value = Parser_parse_literal(parser);
			if(value == NULL) {
				// TODO ERROR
				printf("Expected literal\n");
				return NULL;
			} else { // TODO check for non-positional arguments
				ASTArgumentList_set(argument_list, ASTArgument_create(NULL, value));
				length++;
			}
		} else {
			break;
		}
	}
	
	return argument_list;
}
ASTValue* Parser_parse_call_expression(Parser* parser) {
	printf("Call expression\n");
	ASTValue* value = Parser_parse_literal(parser);
	Token* current = parser->current;
	if(strcmp(current->value, "(") == 0) {
		Parser_advance(parser);
		ASTArgumentList* argument_list = Parser_parse_call_argument_list(parser);
		
		current = parser->current;
		if(current == NULL || strcmp(current->value, ")") != 0) {
			//TODO ERROR
			printf("Expected )\n");
			return NULL;
		}
		
		Parser_advance(parser);
		return ASTValue_fn_expr_create(ASTFunctionExpression_create(value, argument_list));
	}
	return value;
}

ASTValue* Parser_parse_binary_expression(Parser* parser) {
	return Parser_parse_additive_expression(parser);
}
ASTValue* Parser_parse_additive_expression(Parser* parser) {
	printf("ADDITIVE EXPRESSION IN\n");
	ASTValue* bin_expr = Parser_parse_term_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		printf("%s\n", current->value);
		if(strcmp(current->value, "+") == 0 || strcmp(current->value, "-") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "+") == 0) op = AST_PLUS_OP;
			else op = AST_MINUS_OP;
			
			ASTValue* value = Parser_parse_term_expression(parser);
			printf("%s\n", current->value);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}
ASTValue* Parser_parse_term_expression(Parser* parser) {
	printf("MULTIPLICATIVE EXPRESSION IN\n");
	//ASTValue* bin_expr = Parser_parse_literal(parser);
	ASTValue* bin_expr = Parser_parse_call_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		if(strcmp(current->value, "*") == 0 || strcmp(current->value, "/") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "*") == 0) op = AST_MULT_OP;
			else op = AST_DIV_OP;
			
			//ASTValue* value = Parser_parse_literal(parser);
			ASTValue* value = Parser_parse_call_expression(parser);
			printf("%s\n", current->value);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}

ASTObjectPair* Parser_parse_objectPair(Parser* parser) {
	Token* key = parser->current;
	if(strcmp(key->value, "}") == 0) {
		Parser_advance(parser);
		return NULL;
	} else if(key->type != TOK_STRING_TYPE) {
		printf("EXPECTED OBJECT KEY: STRING TYPE %s\n", key->value);
		return NULL;
	}
	Parser_advance(parser);
	
	Token* current = parser->current;
	if(strcmp(current->value, ":") != 0) {
		printf("EXPECTED OBJECT :\n");
		return NULL;
	}
	Parser_advance(parser);
	
	ASTValue* value = Parser_parse_literal(parser);
	
	current = parser->current;
	if(strcmp(current->value, ",") == 0) {
		Parser_advance(parser);
	}
	
	printf("COMPLETE! %s %f\n", key->value, *(value->value.number));
	
	return ASTObjectPair_create(key->value, value);
}

ASTObject* Parser_parse_object(Parser* parser) {
	ASTObject* object = ASTObject_create();
	ASTObjectPair* objectPair;
	
	Token* current = parser->current;
	if(strcmp(current->value, "{") == 0) {
		Parser_advance(parser);
	} else {
		printf("Expected object start {\n");
		return NULL;
	}
	
	current = parser->current;
	while( (objectPair = Parser_parse_objectPair(parser)) != NULL ) {
		ASTObject_set(object, objectPair);
	}
	return object;
}

ASTStatement* Parser_parse_statement(Parser* parser) {
	Token* current = parser->current;
	if(current == NULL) return NULL; // EOF
	if( current->type == TOK_IDENTIFIER_TYPE &&
		current->next != NULL && strcmp(current->next->value, "=") == 0 ) {
		printf("declaration\n");
		return Parser_parse_declaration_stmt(parser);
	} else {
		printf("expression\n");
		return Parser_parse_expression_stmt(parser);
	}
}
ASTStatement* Parser_parse_declaration_stmt(Parser* parser) {
	char* identifier = Parser_advance(parser)->value;
	printf("DECL IDENTIFIER = %s\n", identifier);
	Parser_advance(parser); // =
	ASTValue* expression = Parser_parse_binary_expression(parser);
	Parser_parse_EOS(parser);
	return ASTDeclarationStatement_create(ASTIdentifier_create(identifier), expression);
}
ASTStatement* Parser_parse_expression_stmt(Parser* parser) {
	ASTValue* expression = Parser_parse_binary_expression(parser);
	Parser_parse_EOS(parser);
	return ASTExpressionStatement_create(expression);
}

ASTProgram* Parser_parse_program(Parser* parser) {
	ASTBlock* block = ASTBlock_create();
	ASTProgram* program = ASTProgram_create(block);
	ASTStatement* statement;
	while ( (statement = Parser_parse_statement(parser)) != NULL ) {
		ASTBlock_push(block, ASTBlockStatement_create(statement));
	}
	return program;
}

void Parser_parse_EOS(Parser* parser) { // end of statement: could be end of file or ";"
	Token* current = parser->current;
	printf("EOS\n");
	if(current == NULL) return;
	if(strcmp(current->value, ";") == 0) {
		Parser_advance(parser);
	} else {
		// TODO THROW ERROR
		printf("Unexpected character %s\n", current->value);
	}
}

Token* Parser_advance(Parser* parser) {
	Token* last = parser->current;
	parser->current = parser->current->next;
	return last;
}