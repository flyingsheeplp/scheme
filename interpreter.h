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

#define DEBUG
#define equal(s1,s2) !strcmp(s1,s2)
#ifdef DEBUG
#define scheme_log(...) do{ \
	printf("%s : ",__PRETTY_FUNCTION__); \
	printf(__VA_ARGS__); \
	printf("\n"); \
}while(0)
#else
#define scheme_log(...) void(0)
#endif

#define error(...) do{fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n");exit(1);}while(0)


/*CONSTANTS*/

char* nodeType[] = {"SYMBOL","LIST","INTEGER"};

char* exprType[] = {
	"IntegerExpr",
	"BooleanExpr",
	"FunctionExpr",
	"ArgsExpr"
};


typedef enum{
	SYMBOL = 0,
	LIST,
	INTEGER,
	ERROR
}atom;

typedef enum{
	EINTEGER = 0,
	EBOOLEAN,
	EFUNCTION,
	EARGS,
	ESTRING,
	EPROCEDURE
}exprTy;


typedef struct _expr* expr;
typedef struct _arg* arg;
typedef struct token* tokenList;
typedef struct node* nodeList;


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


struct _arg{
	expr iarg;
	struct _arg* next;
};

struct _procedure{
	char* name;

};


struct _expr{
	exprTy type;
	union{
		int iValue;
		expr (*proc)(int argc,char** argv);
		arg a;
		unsigned int bValue:1;
		char* s;
		procedure p;
	}u;
};

struct _envEntry{
	char* symbol;
	expr e;
	struct _envEntry* next;
};

typedef struct _env* envEntry;

struct _env{
	envEntry e;
	struct _env* next;
};

typedef struct _env* env;

#endif