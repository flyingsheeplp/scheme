#include <env.h>
#include <expr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <interpreter.h>

env global_env = NULL;

env createEnv(env top){
	env e = (env)schemeMalloc(sizeof(struct _env));
	e->entry = NULL;
	e->next = top;
	return e;
}

void releaseEnv(env e){
	envEntry entry = e->entry;
	envEntry save = NULL;
	while(entry){
		save = entry;
		entry = entry->next;
		free(save);
	}
}

void installSymbol(char* symbol,expr exp,env e){
	envEntry oldEntry = e->entry;
	envEntry newEntry = (envEntry)schemeMalloc(sizeof(struct _envEntry));
	newEntry->symbol = strdup(symbol);
	newEntry->e = exp;
	newEntry->next = oldEntry;

	e->entry = newEntry;
}

void  installGlobalSymbol(char* symbol,expr e){
	installSymbol(symbol,e,global_env);
}

expr lookupSymbol(char* symbol,env e){
	while(e){
		envEntry entry =  e->entry;
		while(entry){
			if(equal(entry->symbol,symbol))
				return entry->e;
			entry = entry->next;
		}
		e = e->next;
	}
	return NULL;
}