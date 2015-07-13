#ifndef EXPR_H
#define EXPR_H

typedef struct _expr* expr;
typedef struct _procArg* procArg;
typedef struct _procedure* procedure;
typedef struct _arg* arg;
typedef struct token* tokenList;
typedef struct node* nodeList;

typedef enum{
	EINTEGER = 0,
	EBOOLEAN,
	EFUNCTION,
	EARGS,
	ESTRING,
	EPROCEDURE
}exprTy;

struct _arg{
	expr iarg;
	struct _arg* next;
};

struct _procArg{
	char* argName;
	struct _procArg* next;
};

struct _procedure{
	char* name;
	int argc;
	procArg argv;
	nodeList body;
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


#endif