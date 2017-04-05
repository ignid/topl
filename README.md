# Totally Original Programming Language

**Totally Original Programming Language** is a scripting language that is, well, totally original (pls dont steal).

This is a personal project for me to learn about implementing a parser and interpreter in C.

Feel free to criticize or fork this.

## Compilation

(note: debugging purposes)

You will need:

* gcc

* a linux system ~~because i'm a poor piece of shit who cant afford windows~~

Type this into terminal:

```
git clone 
cd topl
gcc main.c -o main -w
```

## Todo List

- [ ] Error handling
- [ ] Statements
	- [x] Declaration statements
	- [x] Expression statements
	- [ ] Function declaration statements
	- [ ] If statements
	- [ ] While statements
- [x] Expressions
	- [ ] Unary expressions
	- [x] Function call expressions
- [ ] Data structures
	- [x] Basic structures (numbers, strings)
	- [ ] Objects (prototypical inheritance like in JS)
	- [ ] Arrays
- [ ] Add an actual REPL/CLI interpreter
- [x] Add working Hello World example

## Examples

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
	| ExpressionStatement)
	";"

DefinitionStatement
	= Identifier "=" Value

IfStatement
	= "if" "(" Value ")" Block

ExpressionStatement
	= Value

Value = BinaryExpression
BinaryExpression = Term ("+"/"-" Term)+
Term = Unary ("*"/"/" Unary)+
Unary = ("+"/"-") (Identifier | Literal | "(" Value ")")
```