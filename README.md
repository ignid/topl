# Totally Original Programming Language

**Totally Original Programming Language** is a scripting language that is, well, totally original (pls dont steal).

This is a personal project for me to learn about implementing a parser and interpreter in C.

Feel free to criticize or fork this.

(logging tool: https://github.com/rxi/log.c)

## Compilation

You will need:

* gcc

* a linux system ~~because i'm a poor bastard who cant afford windows~~

Type this into terminal:

```
git clone https://github.com/ignid/topl.git
cd topl
gcc main.c -o main -w -lm
```

## Todo List

- [x] Error handling
- [ ] Statements
	- [x] Declaration statements
	- [x] Expression statements
	- [ ] Function declaration statements
	- [x] If statements
	- [x] While statements
- [x] Expressions
	- [ ] Unary expressions (+, -, !)
	- [x] Function call expressions
	- [x] Equality expressions (==, !=)
	- [x] Comparison expressions (>, <, >=, <=)
	- [x] Logical expressions (&&, ||)
- [ ] Data structures
	- [x] Basic structures (numbers, strings)
	- [ ] Objects (prototypical inheritance like in JS)
	- [ ] Arrays
- [ ] Standard API
	- [ ] Input/output API
	- [ ] Math API
- [x] Add an actual REPL/CLI interpreter
- [x] Add working Hello World example

## Examples

(See examples folder for more examples)

```
print("Hello World");
```

## Grammar

```
Program = Statement

Block =
	"{" Statement* "}"

Statement
	= (DefinitionStatement
	| IfStatement
	| FunctionStatement
	| ExpressionStatement)
	

DefinitionStatement
	= Identifier "=" BinaryExpression ";"

IfStatement
	= "if" "(" BinaryExpression ")" Block

FunctionArguments
	= "(" Identifier ("," Identifier)+ ")"

FunctionStatement
	= "fn" Identifier FunctionArguments Block

ExpressionStatement
	= BinaryExpression ";"

BinaryExpression = Term ("+"/"-" Term)+
Term = Unary ("*"/"/" Unary)+
Unary = ("+"/"-") CallExpression
CallArguments = "(" BinaryExpression ("," BinaryExpression)+ ")"
CallExpression = (Identifier | Literal | "(" BinaryExpression ")") CallArguments?
```