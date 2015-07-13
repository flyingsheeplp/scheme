#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#define schemeMalloc malloc
#define MAX_PROGRAM_LENGTH 1024
#define MAX_TOKEN_SIZE 128
#define PROMPT "SI>"
#define EXIT "exit"
#define QUIT "quit"

#define true 1
#define false 0

#define equal(s1,s2) !strcmp(s1,s2)


typedef struct _expr* expr;
typedef struct _procArg* procArg;
typedef struct _procedure* procedure;
typedef struct _arg* arg;
typedef struct token* tokenList;
typedef struct node* nodeList;
typedef struct _env* env;
typedef struct _envEntry* envEntry;

typedef enum{
	SYMBOL = 0,
	LIST,
	INTEGER,
	ERROR
}atom;

struct token{
	char* symbol;
	struct token* next;
};


struct node{
	atom type;
	union{
		int iValue;
		char* symbol;
		nodeList nList;
	}u;
	struct node* next;
};

expr eval(nodeList,env*);
#endif