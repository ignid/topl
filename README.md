# Totally Original Programming Language

**Totally Original Programming Language** is a scripting language that is, well, totally original (pls dont steal).

This is a personal project for me to learn about implementing a parser and interpreter in C.

Feel free to criticize or fork this.

(logging tool: https://github.com/rxi/log.c)

## Compilation

You will need:

* gcc
* a linux system

Type this into terminal:

```
git clone --recursive https://github.com/ignid/topl.git
cd topl
./build.sh
```

## Todo List

- [x] Error handling
- [ ] Statements
	- [x] Declaration statements
		- [x] Declaration statements for object accessors (a.b = c)
	- [x] Expression statements
	- [ ] Function declaration statements
	- [x] If statements
	- [x] While statements
- [x] Expressions
	- [ ] Unary expressions (+, -, !)
	- [x] Arithmetic expressions (+, -, /, *, %)
	- [x] Function call expressions
	- [x] Equality expressions (==, !=)
	- [x] Comparison expressions (>, <, >=, <=)
	- [x] Logical expressions (&&, ||)
	- [x] Object accessors (a.b)
- [ ] Comments (/**/ and //)
- [ ] Data structures
	- [x] Basic structures (numbers, strings)
	- [x] Objects
	- [ ] Arrays
- [ ] Standard API
	- [x] Input/output API
	- [ ] Math API
- [x] Add an actual REPL/CLI interpreter
- [x] Add working Hello World example
- [ ] Makefile?

## Examples

(See examples folder for more examples)

### Hello World

```
print("Hello World");
```

### Fizzbuzz

```
i = 1;
while(i < 100) {
	if(i % 3 == 0 && i % 5 == 0) {
		print("Fizzbuzz\n");
	} else if(i % 3 == 0) {
		print("Fizz\n");
	} else if (i % 5 == 0) {
		print("Buzz\n");
	} else {
		print(i, "\n");
	}
	i = i + 1;
}
```

## Grammar

```
Program = Statement*

Block =
	"{" Statement* "}"

Statement
	= IfStatement
	| WhileStatement
	| FunctionStatement
	| DefinitionStatement
	| ExpressionStatement
	

IfStatement
	= "if" "(" BinaryExpression ")" Block

WhileStatement
	= "while" "(" BinaryExpression ")" Block

FunctionArguments
	= "(" Identifier ("," Identifier)+ ")"
FunctionStatement
	= "fn" Identifier FunctionArguments Block

DefinitionStatement
	= Expression "=" BinaryExpression ";"

ExpressionStatement
	= BinaryExpression ";"

BinaryExpression = LogicalOr
LogicalOr = LogicalAnd ("||" LogicalAnd)+
LogicalAnd = Equality ("&&" Equality)+
Equality = Comparison ("=="/"!=" Comparison)+
Comparison = Addition (">"/"<"/">="/"<=" Addition)+
Addition = Term ("+"/"-" Unary)+
Multiplicative = Unary ("*"/"/" Unary)+

Unary = ("+"/"-") CallExpression

CallArguments = "(" BinaryExpression ("," BinaryExpression)+ ")"
CallExpression = (ObjectAccessor | Literal | Object | "(" BinaryExpression ")") CallArguments?

ObjectPair
	= Identifier/String ":" BinaryExpression ","?
Object
	= "{" ObjectPair* "}"
ObjectAccessor
	= Identifier ("." Identifier)*

```
