
#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <interpreter.h>
#include <env.h>
#include <dump.h>
#include <expr.h>
#include <constant.h>

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
		case EPROCEDURE:
			printf("Procedure expr:%s\n",e->u.p->name);
		default:
			scheme_log("todo...");
			break;
	}
}

void releaseExpr(expr e){
	if(!e){
		printf("Null Expr do not need to release!\n");
		return;
	}
	switch(e->type){
		case EINTEGER:
			printf("release integer expr\n");
			break;
		case EFUNCTION:
			printf("release function expr\n");
			break;
		case EARGS:
			break;
		case EBOOLEAN:
			printf("release boolean expr\n");
			break;
		case ESTRING:
			printf("release boolean expr\n");
			if(e->u.s)
				free(e->u.s);
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

procedure createProcedure(char* name,int argc,procArg pArg,nodeList s){
	procedure p = schemeMalloc(sizeof(struct _procedure));
	p->name = strdup(name);
	p->argc = argc;
	p->argv = pArg;
	p->body = s;
	return p;
}

expr createProcedureExpr(char* name,int argc,procArg pArg,nodeList s){
	expr e = schemeMalloc(sizeof(struct _expr));
	e->type = EPROCEDURE;
	e->u.p = createProcedure(name,argc,pArg,s);
	return e;
}

procArg createProcArg(char* name){
	procArg pa = schemeMalloc(sizeof(struct _procArg));
	pa->argName = strdup(name);
	pa->next = NULL;
	return pa;
}

void addProcArg(procArg* old,procArg new){
	procArg pa = *old;
	if(*old == NULL){
		*old = new;
		return;
	}

	while(pa->next){
		pa = pa->next;
	}

	pa->next = new;
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

expr applyProcedure(expr procedure,env* e){
	if(procedure->type != EPROCEDURE)
		error("apply procedure falied!");

	nodeList s =  procedure->u.p->body;
	expr r;
	while(s){
		r = eval(s,e);
		if(s->next && r)
			releaseExpr(r);
		s = s->next;
	}
	return r;
}


#define isDefine(n) (!strcmp((n)->u.symbol,"define"))
#define isIf(n) (!strcmp((n)->u.symbol,"if"))
#define isTrue(n) (((n)->type)==EBOOLEAN && ((n)->u.bValue==1))
#define isDefun(n) (!strcmp((n)->u.symbol,"defun"))

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
			if(var->type != SYMBOL){
				scheme_log("variable except symbol!");
				return NULL;
			}
			advance(n);
			printNode(n);
			installSymbol(var->u.symbol,eval(n,e),*e);
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
		else if(isDefun(nList)){
			scheme_log("start eval defun");
			struct node* n = nList;
			advance(n);
			// here to consturct procedure
			struct node* def = n;
			int argc = 0;
			if(def->type != LIST){
				error("Define function need list head!");
			}
			struct node* def_start = def->u.nList;
			procArg pArg = NULL;
			char* proc_name =  def_start->u.symbol;
			advance(def_start);
			while(def_start){
				argc++;
				procArg pa = createProcArg(def_start->u.symbol);
				addProcArg(&pArg,pa);
				def_start = def_start->next;
			}
			advance(n);
			expr p = createProcedureExpr(proc_name,argc,pArg,n);
			installSymbol(proc_name,p,*e);
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
			else if(exp->type == EPROCEDURE){
				scheme_log("===========start eval procedure===========");
				struct node* n = nList;
				expr proc_exp = exp;
				procArg pa = proc_exp->u.p->argv;
				int argc = proc_exp->u.p->argc;
				int count = 0;
				advance(n);
				env new_env = createEnv(*e);
				while(n){
					count++;
					if(count > argc)
						error("Too many arguments");
					installSymbol(pa->argName,eval(n,e),new_env);
					advance(pa);
					advance(n);
				}
				if(count < argc)
					error("Too few arguments");

				return applyProcedure(proc_exp,&new_env);
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
	global_env = createEnv(NULL);
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

//#define DEBUG_APPLY_PROCEDURE
int main(int argc,char** argv){
#if 1
	initializeGlobalEnv();
	repl();
#endif
#ifdef DEBUG_APPLY
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
#ifdef DEBUG_APPLY_PROCEDURE
	initializeGlobalEnv();
	env e = createEnv(global_env);
	installSymbol("x",createIntegerExpr(3),e);
	installSymbol("y",createIntegerExpr(4),e);
	nodeList nl = NULL;
	addToNodeList(&nl,createNode(SYMBOL,createToken("+")));
	addToNodeList(&nl,createNode(SYMBOL,createToken("x")));
	addToNodeList(&nl,createNode(SYMBOL,createToken("y")));
	procArg arg1 = createProcArg("x");
	procArg arg2 = createProcArg("y");
	addProcArg(&arg1,arg2);
	expr p = createProcedureExpr("myadd",2,arg1,createListNode(nl));
	expr res = applyProcedure(p,&e);
	printExpr(res);
	return 0;
#endif
}