
#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "interpreter.h"




void dumpTokenlist(tokenList list){
	printf("========================dumpTokenList BEGIN======================\n");
	int i = 0;
	if(list == NULL)
		printf("list is NULL\n");
	while(list){
		printf("token[%d]:%s\n",i,list->symbol);
		i++;
		list = list->next;
	}
	printf("========================dumpTokenList END========================\n");
}

void dumpNodelistInternal(nodeList list,int level){
	printf("========================dumpNodeList LEVEL %d======================\n",level);
	if(list == NULL)
		printf("list is NULL\n");
	while(list){
		switch(list->type){
			case INTEGER:
				printf("type:%s  value:%d\n",nodeType[list->type],list->u.iValue);
				break;
			case SYMBOL:
				printf("type:%s  value:%s\n",nodeType[list->type],list->u.symbol);
				break;
			case LIST:
				dumpNodelistInternal(list->u.nList,level+1);
				break;
			default:
				error("DumpNodelist error: unknown type:%d",list->type);
				break;
		}
		list = list->next;
	}
	printf("========================dumpNodeList LEVEL %d  END======================\n",level);
}

void dumpNodelist(nodeList list){
	printf("\n\n\n");
	if(list == NULL){
		printf("node list is NULL\n");
		return;
	}
	printf("========================dumpNodeList BEGIN========================\n");
	dumpNodelistInternal(list,0);
	printf("========================dumpNodeList END========================\n");
}

void dumpArgList(expr argList){
	if(argList == NULL){
		printf("Arg List is NULL\n");
		return;
	}
	if(argList->type != EARGS){
		error("dumpArgList argList[%s]",exprType[argList->type]);
	}
	scheme_log("-------------------------dumpArgList BEGIN--------------------------------");
	arg a = argList->u.a;
	int count = 0;
	while(a){
		printf("args[%d]:%d\n",count,a->iarg->u.iValue); //NOW ONLY SUPPORT INTEGER ARGS
		a = a->next;
		count++;
	}
	scheme_log("-------------------------dumpArgList END--------------------------------");
}


void dumpEnv(env e){
	int count = 0;
	scheme_log("-------------------------dumpENV BEGIN--------------------------------");
	while(e){
		scheme_log("envEntry[%d]:  [%s:%s]",count,e->symbol,exprType[e->e->type]);
		e = e->next;
		count++;
	}
	scheme_log("-------------------------dumpENV END--------------------------------");
}




/**********************begin of environment****************************************/
env global_env = NULL;

void installSymbol(char* symbol,expr exp,env* e){
	scheme_log("installSymbol:%s",symbol);
	envEntry entry = (envEntry)schemeMalloc(sizeof(struct _env));
	entry->symbol = strdup(symbol);
	entry->e = exp;
	entry->next = *e;

	*e = entry;
}

void  installGlobalSymbol(char* symbol,expr e){
	installSymbol(symbol,e,&global_env);
}

expr lookupSymbol(char* symbol,env e){
	while(e){
		if(equal(e->symbol,symbol))
			return e->e;
		e = e->next;
	}
	return NULL;
}


/**********************end of environment****************************************/

/**********************begin of tokenize****************************************/

struct token* createToken(char* symbol){
	struct token* t = (struct token*)schemeMalloc(sizeof(struct token));
	t->symbol = strdup(symbol);
	t->next = NULL;
	return t;
}

int tokenListLen(tokenList list){
	int count = 0;
	while(list){
		list = list->next;
		count++;
	}
	return count;
}


void addToList(tokenList* list,struct token* t){
	tokenList save = *list;
	if(!*list){
		*list = t;
		return;
	}
	while((*list)->next){
		*list = (*list)->next;
	}

	(*list)->next = t;

	*list = save;
}

tokenList tokenize(char* input){
	char tokenBuffer[MAX_TOKEN_SIZE];
	int i = 0;
	tokenList list = NULL;
	struct token* t = NULL;
	while(*input){
		while(*input == ' ' || *input == '\t' || *input == '\n')
			input++;
		if(*input == 0)
			break;
		if(*input == '('){
			t = createToken("(");
			addToList(&list,t);
			input++;
			continue;
		}
		if(*input == ')'){
			t = createToken(")");
			addToList(&list,t);
			input++;
			continue;
		}
		memset(tokenBuffer,0,MAX_TOKEN_SIZE);
		i = 0;
    	while(*input && *input != ' ' && *input != '\t' && *input != '\n' && *input != '(' && *input!=')'){
    		tokenBuffer[i] = *input;
    		i++;
    		input++;
    	}
    	t = createToken(tokenBuffer);
    	addToList(&list,t);
	}
	return list;
}
/**********************end of tokenize****************************************/
/**********************begin of parse****************************************/
void printNode(struct node* n){
	switch(n->type){
		case INTEGER:
			printf("Node %s@%d\n",nodeType[n->type],n->u.iValue);
			break;
		case SYMBOL:
			printf("Node %s@%s\n",nodeType[n->type],n->u.symbol);
			break;
		case LIST:
			printf("Node List %x\n",n);
			break;
		default:
			error("Unknown node");
	}
}

static struct node* createNode(atom type,struct token* t){
	struct node* n = (struct node*)schemeMalloc(sizeof(struct node));
	scheme_log("create node symbol:%s",t->symbol);
	n->type = type;
	n->next = NULL;
	switch(type){
		case INTEGER:
			n->u.iValue = atoi(t->symbol);
			break;
		case SYMBOL:
			n->u.symbol = strdup(t->symbol);
			break;
		default:
			error("Unknown node : %s",t->symbol);
	}
	return n;
}

struct node* createListNode(nodeList nList){
	struct node* n = (struct node*)schemeMalloc(sizeof(struct node));
	scheme_log("create list node");
	n->type = LIST;
	n->next = NULL;
	n->u.nList = nList;
	return n;
}

static void addToNodeList(nodeList* list,struct node* node){
	nodeList save = *list;
	if(!*list){
		*list = node;
		return;
	}
	while((*list)->next){
		*list = (*list)->next;
	}
	(*list)->next = node;
	*list = save;
}

int isInteger(struct token* t){
	char* symbol = t->symbol;
	while(*symbol){
		if(!isdigit(*symbol)){
			return false;
		}
		symbol++;
	}
	return true;
}

#define eat(list,s) do{if(list && equal((list)->symbol,s)) advance(list); else {error("excepted %s",s);exit(1);}}while(0)
#define advance(list) do{(list) = (list)->next;}while(0)
struct node* parseItem(tokenList*);

nodeList parseList(tokenList* list){
	nodeList nList = NULL;
	eat(*list,"(");
	while(*list && !equal((*list)->symbol,")")){
		scheme_log("parse token %s",(*list)->symbol);
		struct node* n = parseItem(list);
		addToNodeList(&nList,n);
		if(n->type != LIST){
			advance(*list);
		}
	}
	eat(*list,")");
	return nList;
}



struct node* parseItem(tokenList* list){
	struct token* t =  *list;
	if(isInteger(t)){
		return createNode(INTEGER,t);
	}

	else if(!equal(t->symbol,"(")){
		return createNode(SYMBOL,t);
	}

	else return createListNode(parseList(list));
}

nodeList parse(tokenList* list){
	return parseList(list);
}

/**********************end of parse****************************************/



/**********************begin of eval****************************************/
void printExpr(expr e){
	if(!e){
		printf("Null expression.\n");
		return;
	}

	switch(e->type){
		case EINTEGER:
			printf("Integer Expr: %d\n",e->u.iValue);
			break;
		case EFUNCTION:
			printf("Function Expr\n");
			break;
		case EARGS:
		case EBOOLEAN:
		case ESTRING:
			printf("STRING Expr: %s\n",e->u.s);
			break;
		default:
			scheme_log("todo...");
			break;
	}
}

expr createBooleanExpr(unsigned int b){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = EBOOLEAN;
	e->u.bValue = b;
	return e;
}

expr createStringExpr(char* s){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = ESTRING;
	e->u.s = strdup(s);
	return e;
}

expr createIntegerExpr(int iValue){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = EINTEGER;
	e->u.iValue = iValue;
	return e;
}

expr createFunctionExpr(expr (*proc)(int,char**)){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = EFUNCTION;
	e->u.proc = proc;
	return e;
}

expr createArgExpr(){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = EARGS;
	e->u.a = NULL;
	return e;
}

arg createArg(expr e){
	arg a = schemeMalloc(sizeof(struct _arg));
	a->iarg = e;
	a->next = NULL;
}

void addArgs(expr args,arg func_arg){
	if(args->u.a == NULL){
		args->u.a = func_arg;
	}
	else{
		arg a = args->u.a;
		while(a->next)
			a = a->next;
		a->next = func_arg;
	}
}

int argSize(expr args_list){
	int count = 0;
	arg a = args_list->u.a;
	while(a){
		count++;
		a = a->next;
	}
	return count;
}

expr apply(expr func_name,expr args_list){
	int i=0;
	int argc = argSize(args_list);
	arg a = args_list->u.a;
	if(func_name->type != EFUNCTION || args_list->type != EARGS)
		error("apply func[%s] to args[%s]",exprType[func_name->type],exprType[args_list->type]);
	
	char* argv[argc];
	for(i=0;i<argc;i++){
		if(a){
			argv[i] = (char*)schemeMalloc(sizeof(char)*20);
			switch(a->iarg->type){
				case EINTEGER:
					snprintf(argv[i],20,"%d",a->iarg->u.iValue);
					break;
				case ESTRING:
					snprintf(argv[i],20,"%s",a->iarg->u.s);
					break;
				case EBOOLEAN:
					snprintf(argv[i],20,"%d",a->iarg->u.bValue);
					break;
				default:
					error("unknown arg type!");
			}
			a = a->next;
		}
		else{
			error("apply error ");
		}
	}
	expr res = func_name->u.proc(argc,argv);

	for(i=0;i<argc;i++){
		free(argv[i]);
	}
	return res;
}

expr applyProcedure();


#define isDefine(n) (!strcmp((n)->u.symbol,"define"))
#define isIf(n) (!strcmp((n)->u.symbol,"if"))
#define isTrue(n) (((n)->type)==EBOOLEAN && ((n)->u.bValue==1))

expr eval(nodeList nList,env* e){
	printf("\n\n\n\n\n\n");
	scheme_log("===========start eval===========");

	if(!nList){
		return NULL;
	}

	if(nList->type == SYMBOL){
		if(isDefine(nList)){
			struct node* n = nList;
			advance(n);
			struct node* var = n;
			advance(n);
			printNode(n);
			installSymbol(var->u.symbol,eval(n,e),e);
			return createStringExpr("Done");
		}
		else if(isIf(nList)){
			scheme_log("start  eval if");
			struct node* n = nList;
			advance(n);
			struct node* cond = n;
			advance(n);
			struct node* body = n;
			advance(n);
			struct node* alter =  n;
			expr exp = eval(cond,e);
			if(isTrue(exp)){
				return eval(body,e);
			}else{
				return eval(alter,e);
			}
		}
		else{
			scheme_log("===========start eval symbol : %s",nList->u.symbol);
			expr exp = lookupSymbol(nList->u.symbol,*e);
			printExpr(exp);
			if(exp->type == EFUNCTION){
				scheme_log("===========start eval function===========");
				struct node* n = nList;
				struct node* func = n;
				advance(n);
				struct node* args = n;
				expr func_exp = exp;
				expr args_exp = createArgExpr();
				while(args){
					expr arg_item = eval(args,e);
					arg a = createArg(arg_item);
					addArgs(args_exp,a);
					advance(args);
				}
				dumpArgList(args_exp);
				return apply(func_exp,args_exp);
			}
			return exp;
		}
	}
	else if(nList->type == INTEGER){
		scheme_log("===========start eval integer===========");
		expr exp = createIntegerExpr(nList->u.iValue);
		return exp;
	}
	
	else if(nList->type == LIST){
		scheme_log("===========start eval list===========");
		struct node* n = nList->u.nList;
		return eval(n,e);
	}
}



/**********************begin of primitive procedure****************************************/
expr addProc(int argc,char** argv){
	int i,sum = 0;
	if(argc == 0){
		error("addProc: no arguments");
	}
	for(i=0;i<argc;i++){
		sum += atoi(argv[i]);
	}
	return createIntegerExpr(sum);
}

expr greaterThan(int argc,char** argv){
	if(atoi(argv[0]) > atoi(argv[1]))
		return createBooleanExpr(1);
	else
		return createBooleanExpr(0);
}

void initializeGlobalEnv(){
	installGlobalSymbol("+",createFunctionExpr(addProc));
	installGlobalSymbol(">",createFunctionExpr(greaterThan));
}

/**********************end of primitive procedure****************************************/
/**********************end of eval****************************************/




void repl(){
	char program[MAX_PROGRAM_LENGTH];
	while(1){
		memset(program,0,MAX_PROGRAM_LENGTH);
		printf(PROMPT);
		fflush(stdout);
		read(STDIN_FILENO,program,MAX_PROGRAM_LENGTH);
		if(!strncmp(program,EXIT,4) || !strncmp(program,QUIT,4))
			break;
		tokenList list = tokenize(program);
		if(list == NULL)
			continue;
		dumpTokenlist(list);
		nodeList nlist = parse(&list);
		dumpNodelist(nlist);
		expr res = eval(nlist,&global_env);
		printExpr(res);
	}
}

int main(int argc,char** argv){
	initializeGlobalEnv();
	repl();
#if 0
	initializeGlobalEnv();
	expr e1 = createIntegerExpr(3);
	expr e2 = createIntegerExpr(5);
	expr e3 = createArgExpr();
	arg a1 = createArg(e1);
	arg a2 = createArg(e2);
	addArgs(e3,a1);
	addArgs(e3,a2);
	installGlobalSymbol("arg1",e1);
	installGlobalSymbol("arg2",e2);
	installGlobalSymbol("arg3",e3);
	dumpEnv(global_env);
	dumpArgList(e3);
	expr func = lookupSymbol("+",global_env);
	printExpr(func);
	expr res = apply(func,e3);
	printExpr(res);
#endif
	return 0;
}