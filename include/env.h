#ifndef ENV_H
#define ENV_H

typedef struct _expr* expr;
typedef struct _env* env;
typedef struct _envEntry* envEntry;

struct _envEntry{
	char* symbol;
	expr e;
	struct _envEntry* next;
};

struct _env{
	envEntry entry;
	struct _env* next;
};


extern env global_env;

expr lookupSymbol(char* symbol,env e);
void  installGlobalSymbol(char* symbol,expr e);
void installSymbol(char* symbol,expr exp,env e);
void releaseEnv(env e);
env createEnv(env top);
#endif