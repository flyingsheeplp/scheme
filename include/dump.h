#ifndef DUMP_H
#define DUMP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

typedef struct _expr* expr;
typedef struct _procArg* procArg;
typedef struct _procedure* procedure;
typedef struct _arg* arg;
typedef struct token* tokenList;
typedef struct node* nodeList;
typedef struct _envEntry* envEntry;
typedef struct _env* env;

#define DEBUG
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

void dumpTokenlist(tokenList list);
void dumpNodelistInternal(nodeList list,int level);
void dumpNodelist(nodeList list);
void dumpArgList(expr argList);
void dumpEnv(env e);

#endif