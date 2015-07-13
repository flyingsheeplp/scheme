#include <dump.h>
#include <expr.h>
#include <env.h>
#include <interpreter.h>
#include <constant.h>

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
	printf("-------------------------dumpArgList BEGIN--------------------------------\n");
	arg a = argList->u.a;
	int count = 0;
	while(a){
		printf("args[%d]:%d\n",count,a->iarg->u.iValue); //NOW ONLY SUPPORT INTEGER ARGS
		a = a->next;
		count++;
	}
	printf("-------------------------dumpArgList END--------------------------------\n");
}


void dumpEnv(env e){
	int count = 0;
	
	while(e){
		printf("-------------------------dump env[%d] BEGIN-------------------------\n",count);
		envEntry ent = e->entry;
		int entryCount = 0;
		while(ent){
			printf("envEntry[%d]:  [%s:%s]",entryCount,ent->symbol,exprType[ent->e->type]);
			e = e->next;
			entryCount++;
		}
		count++;
		printf("-------------------------dump env[%d] END---------------------------\n",count);
	}
}