#include <stdlib.h>
#include "Error.h"
#include "log.c/src/log.h"
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
		log_debug("STRING TOKEN %s", current->value);
		value = ASTString_create(current->value);
	} else if (current->type == TOK_NUMBER_TYPE) {
		value = ASTNumber_create(strtod(current->value, NULL));
		log_debug("NUMBER %f", *(value->value.number));
	} else if (current->type == TOK_IDENTIFIER_TYPE) {
		value = ASTIdentifier_create(current->value);
		log_debug("IDENTIFIER %s", current->value);
	} else {
		return NULL;
	}
	Parser_advance(parser);
	return value;
}

ASTArgumentList* Parser_parse_call_argument_list(Parser* parser) {
	ASTArgumentList* argument_list = ASTArgumentList_create();
	size_t length = 1;
	
	ASTValue* first = Parser_parse_binary_expression(parser);
	if(first == NULL) {
		return argument_list;
	}
	ASTArgumentList_set(argument_list, ASTArgument_create(NULL, first));
	
	Token* current;
	while( (current = parser->current) != NULL) {
		if (strcmp(current->value, ",") == 0) {
			Parser_advance(parser);
			ASTValue* value = Parser_parse_binary_expression(parser);
			if(value == NULL) {
				log_error("Expected expression");
				Error_throw(1);
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
	log_debug("Call expression");
	ASTValue* value = Parser_parse_literal(parser);
	Token* current = parser->current;
	if(strcmp(current->value, "(") == 0) {
		Parser_advance(parser);
		ASTArgumentList* argument_list = Parser_parse_call_argument_list(parser);
		
		current = parser->current;
		if(current == NULL || strcmp(current->value, ")") != 0) {
			log_error("Expected )");
			Error_throw(1);
		}
		
		Parser_advance(parser);
		return ASTValue_fn_expr_create(ASTFunctionExpression_create(value, argument_list));
	}
	return value;
}

ASTValue* Parser_parse_binary_expression(Parser* parser) {
	return Parser_parse_equality_expression(parser);
}

ASTValue* Parser_parse_equality_expression(Parser* parser) {
	log_debug("EQUALITY EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_comparison_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if( strcmp(current->value, "==") == 0  || strcmp(current->value, "!=") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "==") == 0) op = AST_EQUAL_OP;
			else op = AST_NOT_EQUAL_OP;
			
			ASTValue* value = Parser_parse_comparison_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}
ASTValue* Parser_parse_comparison_expression(Parser* parser) {
	log_debug("COMPARISON EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_additive_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if( strcmp(current->value, "<") == 0  || strcmp(current->value, ">") == 0 ||
			strcmp(current->value, "<=") == 0 || strcmp(current->value, ">=") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "<") == 0) op = AST_LESS_OP;
			else if(strcmp(current->value, ">") == 0) op = AST_MORE_OP;
			else if(strcmp(current->value, "<=") == 0) op = AST_LESS_EQ_OP;
			else op = AST_MORE_EQ_OP;
			
			ASTValue* value = Parser_parse_additive_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}
ASTValue* Parser_parse_additive_expression(Parser* parser) {
	log_debug("ADDITIVE EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_term_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if(strcmp(current->value, "+") == 0 || strcmp(current->value, "-") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "+") == 0) op = AST_PLUS_OP;
			else op = AST_MINUS_OP;
			
			ASTValue* value = Parser_parse_term_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}
ASTValue* Parser_parse_term_expression(Parser* parser) {
	log_debug("MULTIPLICATIVE EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_call_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		if(strcmp(current->value, "*") == 0 || strcmp(current->value, "/") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "*") == 0) op = AST_MULT_OP;
			else op = AST_DIV_OP;
			
			ASTValue* value = Parser_parse_call_expression(parser);
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
		log_debug("EXPECTED OBJECT KEY: STRING TYPE %s", key->value);
		return NULL;
	}
	Parser_advance(parser);
	
	Token* current = parser->current;
	if(strcmp(current->value, ":") != 0) {
		log_debug("EXPECTED OBJECT ':'");
		return NULL;
	}
	Parser_advance(parser);
	
	ASTValue* value = Parser_parse_literal(parser);
	
	current = parser->current;
	if(strcmp(current->value, ",") == 0) {
		Parser_advance(parser);
	}
	
	log_debug("COMPLETE! %s %f", key->value, *(value->value.number));
	
	return ASTObjectPair_create(key->value, value);
}

ASTObject* Parser_parse_object(Parser* parser) {
	ASTObject* object = ASTObject_create();
	ASTObjectPair* objectPair;
	
	Token* current = parser->current;
	if(strcmp(current->value, "{") == 0) {
		Parser_advance(parser);
	} else {
		log_error("Expected object start {");
		Error_throw(1);
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
		log_debug("declaration");
		return Parser_parse_declaration_stmt(parser);
	} else if (strcmp(current->value, "if") == 0) {
		log_debug("if");
		return Parser_parse_if_stmt(parser);
	} else {
		log_debug("expression");
		return Parser_parse_expression_stmt(parser);
	}
}
ASTStatement* Parser_parse_declaration_stmt(Parser* parser) {
	char* identifier = parser->current->value;
	Parser_advance(parser);
	log_debug("DECL IDENTIFIER = %s", identifier);
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
ASTStatement* Parser_parse_if_stmt(Parser* parser) {
	Parser_advance(parser); // if
	if(parser->current == NULL || strcmp(parser->current->value, "(") != 0) {
		log_error("missing ( before condition");
		Error_throw(1);
	}
	Parser_advance(parser); // (
	
	ASTValue* expression = Parser_parse_binary_expression(parser);
	
	if(parser->current == NULL || strcmp(parser->current->value, ")") != 0) {
		log_error("missing ) after condition");
		Error_throw(1);
	}
	Parser_advance(parser); // )
	
	ASTBlock* block = Parser_parse_block(parser);
	
	if(parser->current != NULL && strcmp(parser->current->value, "else") == 0) {
		log_debug("else statement");
		Parser_advance(parser);
		ASTBlock* block_else = Parser_parse_block(parser);
		return ASTIfStatement_create(expression, block, block_else);
	}
	
	return ASTIfStatement_create(expression, block, NULL);
}

ASTBlock* Parser_parse_block(Parser* parser) {
	if(parser->current == NULL) {
		log_error("expected expression");
		Error_throw(1);
	}
	
	ASTBlock* block = ASTBlock_create();
	//log_error("%s", parser->current->value);
	if(strcmp(parser->current->value, "{") == 0) {
		Parser_advance(parser);
		ASTStatement* statement;
		
		while ( (parser->current != NULL && strcmp(parser->current->value, "}") != 0) &&
			(statement = Parser_parse_statement(parser)) != NULL ) {
			ASTBlock_push(block, ASTBlockStatement_create(statement));
		}
		
		if(parser->current == NULL || strcmp(parser->current->value, "}") != 0) {
			log_error("missing } after block");
			Error_throw(1);
		}
		Parser_advance(parser); // }
	} else {
		ASTBlock_push(block, ASTBlockStatement_create(Parser_parse_statement(parser)));
	}
	return block;
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
	log_debug("EOS");
	if(current == NULL) return;
	//if(current == NULL || strcmp(current->value, "}") == 0) return;
	if(strcmp(current->value, ";") == 0) {
		Parser_advance(parser);
	} else if (strcmp(current->value, "}") == 0) {
		Parser_advance(parser);
	} else {
		log_error("Unexpected character %s", current->value);
		Error_throw(1);
	}
}

void Parser_advance(Parser* parser) {
	parser->current = parser->current->next;
}