#include <stdlib.h>
#include "Error.h"
#include "log.c/src/log.h"
#include "AST.h"
#include "Parser.h"
#include "Conversion.h"

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
	//log_debug("value: %s", current->value);
	if(current == NULL) {
		return NULL;
	} else if(current->type == TOK_STRING_TYPE) {
		log_debug("STRING TOKEN %s", current->value);
		value = ASTString_create(current->value);
	} else if (current->type == TOK_INTEGER_TYPE) {
		log_debug("INTEGER %s", current->value);
		value = ASTInteger_create(atoi(current->value));
	} else if (current->type == TOK_NUMBER_TYPE) {
		log_debug("NUMBER %s", current->value);
		value = ASTNumber_create(strtod(current->value, NULL));
	} else if (current->type == TOK_IDENTIFIER_TYPE) {
		log_debug("IDENTIFIER %s", current->value);
		value = ASTIdentifier_create(current->value);
	} else if (strcmp(current->value, "{") == 0) {
		log_debug("OBJECT");
		value = Parser_parse_object(parser);
		return value;
	} else if (strcmp(current->value, "(") == 0) {
		Parser_advance(parser);
		value = Parser_parse_binary_expression(parser);
		
		if (strcmp(current->value, ")") != 0) {
			log_error("expected ) at the end of expression");
			Error_throw();
		}
		Parser_advance(parser);
		return value;
	} else {
		return NULL;
	}
	Parser_advance(parser);
	return value;
}

ASTObjectPair* Parser_parse_objectPair(Parser* parser) {
	Token* key = parser->current;
	if(key->type != TOK_STRING_TYPE) {
		log_error("Expected string type for object key, got %i (%s)", key->type, key->value);
		Error_throw();
	}
	Parser_advance(parser);
	
	if(parser->current == NULL || strcmp(parser->current->value, ":") != 0) {
		log_error("Expected object pair separator ':'");
		Error_throw();
	}
	Parser_advance(parser);
	
	ASTValue* value = Parser_parse_binary_expression(parser);
	
	if(strcmp(parser->current->value, ",") == 0) {
		Parser_advance(parser);
	}
	
	log_debug("OBJECT PAIR %s", key->value);
	
	return ASTObjectPair_create(key->value, value);
}
ASTValue* Parser_parse_object(Parser* parser) {
	ASTObject* object = ASTObject_create();
	
	Parser_advance(parser); // {
	
	Token* current;
	while( (current = parser->current) != NULL && strcmp(current->value, "}") != 0 ) {
		ASTObject_set(object, Parser_parse_objectPair(parser));
	}
	
	if(parser->current == NULL || strcmp(parser->current->value, "}") != 0) {
		log_error("Expected }");
		Error_throw();
	}
	Parser_advance(parser);
	
	return ASTValue_object_create(object);
}
ASTValue* Parser_parse_object_expression(Parser* parser) {
	// TODO: expand this to access indexed array
	// TODO: square brackets notation
	ASTValue* value = Parser_parse_literal(parser);
	Token* current;
	while ( (current = parser->current) != NULL && strcmp(current->value, ".") == 0) {
		Parser_advance(parser);
		
		Token* tail = parser->current;
		log_debug("%s", tail->value);
		if(tail == NULL || tail->type != TOK_IDENTIFIER_TYPE) {
			log_error("Expected identifier literal after .");
			Error_throw();
		}
		Parser_advance(parser);
		
		value = ASTValue_object_expr_create(ASTObjectExpression_create(value, ASTIdentifier_create(tail->value)));
		log_debug("value %s", value->value.object_expr->tail->value.string);
	}
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
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if(strcmp(current->value, ",") == 0) {
			log_debug("new");
			Parser_advance(parser);
			ASTValue* value = Parser_parse_binary_expression(parser);
			if(value == NULL) {
				log_error("Expected expression");
				Error_throw();
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
	ASTValue* value = Parser_parse_object_expression(parser);
	Token* current = parser->current;
	if(current != NULL && strcmp(current->value, "(") == 0) {
		log_debug("Call expression");
		Parser_advance(parser);
		ASTArgumentList* argument_list = Parser_parse_call_argument_list(parser);
		
		current = parser->current;
		if(current == NULL || strcmp(current->value, ")") != 0) {
			log_error("Expected )");
			Error_throw();
		}
		
		Parser_advance(parser);
		return ASTValue_fn_expr_create(ASTFunctionExpression_create(value, argument_list));
	}
	return value;
}

ASTValue* Parser_parse_binary_expression(Parser* parser) {
	return Parser_parse_logical_or_expression(parser);
}

ASTValue* Parser_parse_logical_or_expression(Parser* parser) {
	log_debug("LOGICAL OR EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_logical_and_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if(strcmp(current->value, "||") == 0) {
			Parser_advance(parser);
			int op = AST_OR_OP;
			
			ASTValue* value = Parser_parse_logical_and_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}
ASTValue* Parser_parse_logical_and_expression(Parser* parser) {
	log_debug("LOGICAL AND EXPRESSION IN");
	ASTValue* bin_expr = Parser_parse_equality_expression(parser);
	Token* current;
	while( (current = parser->current) != NULL ) {
		log_debug("%s", current->value);
		if(strcmp(current->value, "&&") == 0) {
			Parser_advance(parser);
			int op = AST_AND_OP;
			
			ASTValue* value = Parser_parse_equality_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
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
		if(strcmp(current->value, "*") == 0 || strcmp(current->value, "/") == 0 || strcmp(current->value, "%") == 0) {
			Parser_advance(parser);
			int op;
			if(strcmp(current->value, "*") == 0) op = AST_MULT_OP;
			else if(strcmp(current->value, "/") == 0) op = AST_DIV_OP;
			else op = AST_MODULO_OP;
			
			ASTValue* value = Parser_parse_call_expression(parser);
			bin_expr = ASTValue_bin_expr_create(ASTBinaryExpression_create(op, bin_expr, value));
		} else {
			break;
		}
	}
	return bin_expr;
}

ASTStatement* Parser_parse_statement(Parser* parser) {
	Token* current = parser->current;
	if (strcmp(current->value, "if") == 0) {
		log_debug("if");
		return Parser_parse_if_stmt(parser);
	} else if (strcmp(current->value, "while") == 0) {
		log_debug("while");
		return Parser_parse_while_stmt(parser);
	} else {
		return Parser_parse_declaration_expression_stmt(parser);
		//log_debug("expression");
		//return Parser_parse_expression_stmt(parser);
	}
}
ASTStatement* Parser_parse_declaration_expression_stmt(Parser* parser) {
	log_debug("declaration stmt");
	ASTValue* left = Parser_parse_binary_expression(parser);
	log_info("%s", parser->current->value);
	if(parser->current != NULL && strcmp(parser->current->value, "=") == 0) {
		Parser_advance(parser);
		if(parser->current == NULL) {
			log_error("expected right hand side expression");
			Error_throw();
		}
		ASTValue* right = Parser_parse_binary_expression(parser);
		Parser_parse_EOS(parser);
		return ASTDeclarationStatement_create(left, right);
	} else {
		Parser_parse_EOS(parser);
		return ASTExpressionStatement_create(left);
	}
}
ASTStatement* Parser_parse_if_stmt(Parser* parser) {
	Parser_advance(parser); // if
	if(parser->current == NULL || strcmp(parser->current->value, "(") != 0) {
		log_error("missing ( before condition");
		Error_throw();
	}
	Parser_advance(parser); // (
	
	ASTValue* expression = Parser_parse_binary_expression(parser);
	
	if(parser->current == NULL || strcmp(parser->current->value, ")") != 0) {
		log_error("missing ) after condition");
		Error_throw();
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
ASTStatement* Parser_parse_while_stmt(Parser* parser) {
	Parser_advance(parser); // if
	if(parser->current == NULL || strcmp(parser->current->value, "(") != 0) {
		log_error("missing ( before condition");
		Error_throw();
	}
	Parser_advance(parser); // (
	
	ASTValue* expression = Parser_parse_binary_expression(parser);
	
	if(parser->current == NULL || strcmp(parser->current->value, ")") != 0) {
		log_error("missing ) after condition");
		Error_throw();
	}
	Parser_advance(parser); // )
	
	ASTBlock* block = Parser_parse_block(parser);
	
	return ASTWhileStatement_create(expression, block);
}

ASTBlock* Parser_parse_block(Parser* parser) {
	if(parser->current == NULL) {
		log_error("expected expression");
		Error_throw();
	}
	
	ASTBlock* block = ASTBlock_create();
	if(strcmp(parser->current->value, "{") == 0) {
		Parser_advance(parser);
		ASTStatement* statement;
		
		while ( (parser->current != NULL && strcmp(parser->current->value, "}") != 0) &&
			(statement = Parser_parse_statement(parser)) != NULL ) {
			ASTBlock_push(block, ASTBlockStatement_create(statement));
		}
		
		if(parser->current == NULL || strcmp(parser->current->value, "}") != 0) {
			log_error("missing } after block");
			Error_throw();
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
	while ( parser->current != NULL ) {
		log_debug("stmt %i", parser->current == NULL);
		if((statement = Parser_parse_statement(parser)) != NULL)
			ASTBlock_push(block, ASTBlockStatement_create(statement));
	}
	return program;
}

void Parser_parse_EOS(Parser* parser) { // end of statement: could be end of file or ";"
	Token* current = parser->current;
	log_debug("EOS");
	if(parser->current == NULL) {
		log_debug("EOF");
		return;
	}
	
	log_debug("%s", current->value);
	if(strcmp(current->value, ";") == 0) {
		log_debug("semicolon");
		if(parser->current->next == NULL) { // eof fix
			log_debug("EOF");
			parser->current = NULL;
		} else {
			Parser_advance(parser);
		}
	} else {
		log_error("Unexpected character %s", current->value);
		Error_throw();
	}
}

void Parser_advance(Parser* parser) {
	parser->current = parser->current->next;
}