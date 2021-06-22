#include <stdio.h>
#include <stdlib.h>
#include "Scanner.h"
#include "MiniC.tbl"
#include "SymTab.c"

#define PS_SIZE  100

enum nodeNumber {
	ACTUAL_PARAM, ADD, ADD_ASSIGN, ARRAY_VAR, ASSIGN_OP,
	CALL, COMPOUND_ST, CONST_NODE, DCL, DCL_ITEM,
	DCL_LIST, DCL_SPEC, DIV, DIV_ASSIGN, EQ,
	ERROR_NODE, EXP_ST, FORMAL_PARA, FUNC_DEF, FUNC_HEAD,
	GE, GT, IDENT, IF_ELSE_ST, IF_ST,
	INDEX, INT_NODE, LE, LOGICAL_AND, LOGICAL_NOT,
	LOGICAL_OR, LT, MOD, MOD_ASSIGN, MUL,
	MUL_ASSIGN, NE, NUMBER, PARAM_DCL, POST_DEC,
	POST_INC, PRE_DEC, PRE_INC, PROGRAM, RETURN_ST,
	SIMPLE_VAR, STAT_LIST, SUB, SUB_ASSIGN, UNARY_MINUS,
	VOID_NODE, WHILE_ST
};
char *nodeName[] = {
	"ACTUAL_PARAM",	"ADD",			"ADD_ASSIGN",	"ARRAY_VAR",	"ASSIGN_OP",
	"CALL",			"COMPOUND_ST",	"CONST_NODE",	"DCL",			"DCL_ITEM",
	"DCL_LIST",		"DCL_SPEC",		"DIV",			"DIV_ASSIGN",	"EQ",
	"ERROR_NODE",	"EXP_ST",		"FORMAL_PARA",	"FUNC_DEF",		"FUNC_HEAD",
	"GE",			"GT",			"IDENT",		"IF_ELSE_ST",	"IF_ST",
	"INDEX",		"INT_NODE",		"LE",			"LOGICAL_AND",	"LOGICAL_NOT",
	"LOGICAL_OR",	"LT",			"MOD",			"MOD_ASSIGN",	"MUL",
	"MUL_ASSIGN",	"NE",			"NUMBER",		"PARAM_DCL",	"POST_DEC",
	"POST_INC",		"PRE_DEC",		"PRE_INC",		"PROGRAM",		"RETURN_ST",
	"SIMPLE_VAR",	"STAT_LIST",	"SUB",			"SUB_ASSIGN",	"UNARY_MINUS",
	"VOID_NODE",	"WHILE_ST"
};
int ruleName[] = {
	/*0					1				2				3				4*/
	0,					PROGRAM,		0,				0,				0,
	/*5					6				7				8				9*/
	0,					FUNC_DEF,		FUNC_HEAD,		DCL_SPEC,		0,
	/*10				11				12				13				14*/
	0,					0,				0,				CONST_NODE,		INT_NODE,
	/*15				16				17				18				19*/
	VOID_NODE,			0,				FORMAL_PARA,	0,				0,
	/*20				21				22				23				24*/
	0,					0,				PARAM_DCL,		COMPOUND_ST,	DCL_LIST,
	/*25				26				27				28				29*/
	DCL_LIST,			0,				0,				DCL,			0,
	/*30				31				32				33				34*/
	0,					DCL_ITEM,		DCL_ITEM,		SIMPLE_VAR,		ARRAY_VAR,
	/*35				36				37				38				39*/
	0,					0,				STAT_LIST,		0,				0,
	/*40				41				42				43				44*/
	0,					0,				0,				0,				0,
	/*45				46				47				48				49*/
	0,					EXP_ST,			0,				0,				IF_ST,
	/*50				51				52				53				54*/
	IF_ELSE_ST,			WHILE_ST,		RETURN_ST,		0,				0,
	/*55				56				57				58				59*/
	ASSIGN_OP,			ADD_ASSIGN,		SUB_ASSIGN,		MUL_ASSIGN,		DIV_ASSIGN,
	/*60				61				62				63				64*/
	MOD_ASSIGN,			0,				LOGICAL_OR,		0,				LOGICAL_AND,
	/*65				66				67				68				69*/
	0,					EQ,				NE,				0,				GT,
	/*70				71				72				73				74*/
	LT,					GE,				LE,				0,				ADD,
	/*75				76				77				78				79*/
	SUB,				0,				MUL,			DIV,			MOD,
	/*80				81				82				83				84*/
	0,					UNARY_MINUS,	LOGICAL_NOT,	PRE_INC,		PRE_DEC,
	/*85				86				87				88				89*/
	0,					INDEX,			CALL,			POST_INC,		POST_DEC,
	/*90				91				92				93				94*/
	0,					0,				ACTUAL_PARAM,	0,				0,
	/*95				96				97*/
	0,					0,				0
};

typedef struct nodeType {
	struct tokentype token; // 토큰 종류
	enum { terminal, nonterm } noderep; // 노드 종류
	struct nodeType *son; // 왼쪽 링크
	struct nodeType *brother; // 오른쪽 링크
} Node;

int returnWithValue;
int labelNumber=0;
int multAssignFlag = 0;
int block;
int offset;
int lvalue;
int sp, pstk[PS_SIZE];
int stateStack[PS_SIZE];
int symbolStack[PS_SIZE];
Node *valueStack[PS_SIZE];

Node *parser();
Node *buildNode(struct tokentype token);
Node *buildTree(int nodeNumber, int rhsLength);
int meaningfulToken(struct tokentype token);
void printTree(Node *pt, int indent);
void printNode(Node *pt, int indent);
void codeGen(Node* ptr);
void processDeclaration(Node* ptr);
void processSimpleVariable(Node* ptr, int typeSpecifier, int typeQualifier);
void processArrayVariable(Node *ptr, int typeSpecifier, int typeQualifier);
void processFuncHeader(Node* ptr);
void processFunction(Node* ptr);
void processOperator(Node* ptr);
void processStatement(Node* ptr);
void processCondition(Node* ptr);
void rv_emit(Node* ptr);
void emit0(char* value);
void emit1(char* value, int p);
void emit2(char* value, int p, int q);
void emitJump(char* value, char* Label);
void emitLabel(char* value);
void genLabel(char* value);
int checkPredefined(Node * ptr);
void icg_error(int num);
int typeSize(int typeSpecifier);
void emitSym(int base, int offset, int size);

FILE *astFile;
FILE* ucodeFile;

void main(int argc, char* argv[])
{
	char astname[20], ucodename[20];
	sprintf(astname, "%s.ast", argv[1]);
	sprintf(ucodename, "%s.uco", argv[1]);
	printf("*** start of Mini C Compilier\n");
	astFile = fopen(astname, "w");
	ucodeFile = fopen(ucodename, "w");

	Node *root;
	printf("===== start of Parser\n");
	root = parser();
	printTree(root, 2);
	printf("===== start of ICG\n");
	codeGen(root);
	printf("*** end of Mini C Compiler\n");

}

int meaningfulToken(struct tokentype token) {
	if ((token.number == tident) || (token.number == tnumber))
		return 1;
	else
		return 0;
}
Node *buildNode(struct tokentype token) {
	Node *ptr;
	ptr = (Node *)malloc(sizeof(Node));
	if (!ptr) {
		printf("malloc error in buildNode()\n");
		exit(1);
	}
	ptr->token = token;
	ptr->noderep = terminal;
	ptr->son = ptr->brother = NULL;
	return ptr;
}
Node *buildTree(int nodeNumber, int rhsLength) {
	int i, j, start;
	Node *first, *ptr;

	i = sp - rhsLength + 1;

	// step 1: find a first index with node in value stack
	while (i <= sp && valueStack[i] == NULL) i++;
	if (!nodeNumber && i > sp) return NULL;
	start = i;

	// step 2: linking brothers
	while (i <= sp - 1) {
		j = i + 1;
		while (j <= sp && valueStack[j] == NULL) j++;
		if (j <= sp) {
			ptr = valueStack[i];
			while (ptr->brother) ptr = ptr->brother;
			ptr->brother = valueStack[j];
		}
		i = j;
	}
	first = (start > sp) ? NULL : valueStack[start];

	// step 3: making subtree root and linking son
	if (nodeNumber) {
		ptr = (Node *)malloc(sizeof(Node));
		if (!ptr) {
			printf("malloc error in buildTree()\n");
			exit(1);
		}

		ptr->token.number = nodeNumber;
		ptr->token.tokenValue = NULL;
		ptr->noderep = nonterm;
		ptr->son = first;
		ptr->brother = NULL;
		return ptr;
	}
	else
		return first;

}
void printNode(Node *pt, int indent) {
	int i;
	extern FILE *astFile;

	for (i = 1; i <= indent; i++) fprintf(astFile, " ");
	if (pt->noderep == terminal) {
		if (pt->token.number == tident)
			fprintf(astFile, " Terminal: %s", pt->token.value.id);
		else if (pt->token.number == tnumber)
			fprintf(astFile, " Terminal: %d", pt->token.value.num);
	}
	else { // nonterminal node    
		int i;
		i = (int)(pt->token.number);
		fprintf(astFile, " Nonterminal: %s", nodeName[i]);
	}
	fprintf(astFile, "\n");
}
void printTree(Node *pt, int indent) {
	Node *p = pt;
	while (p != NULL) {
		printNode(p, indent);
		if (p->noderep == nonterm)
			printTree(p->son, indent + 5);
		p = p->brother;
	}
}
Node *parser() {
	int entry, ruleNumber, lhs;
	int current_state;
	struct tokentype token;
	Node *ptr;

	sp = 0; pstk[sp] = 0;
	token = scanner();
	while (1) {
		current_state = pstk[sp];
		entry = parsingTable[current_state][token.number];
		if (entry > 0)							// shift action    
		{
			if (++sp > PS_SIZE) {
				printf("critical error: parsing stack overflow");
				exit(1);
			}
			symbolStack[sp] = token.number;
			pstk[sp] = entry;
			valueStack[sp] = meaningfulToken(token) ? buildNode(token) : NULL;
			token = scanner();
		}
		else if (entry < 0)						// reduce action    
		{
			ruleNumber = -entry;
			if (ruleNumber == GOAL_RULE)         // accept action         
			{
				return valueStack[sp - 1];
			}
			ptr = buildTree(ruleName[ruleNumber], rightLength[ruleNumber]);
			sp = sp - rightLength[ruleNumber];
			lhs = leftSymbol[ruleNumber];
			current_state = parsingTable[pstk[sp]][lhs];
			symbolStack[++sp] = lhs;
			pstk[sp] = current_state;
			valueStack[sp] = ptr;
		}
		else {
			printf(" === error in source ===\n");
			exit(1);
		}
	}	//while
}		//parser

void codeGen(Node* ptr) {
	Node* p;
	int globalSize;

	initSymbolTable();
	//step1 process the declaration part
	for (p = ptr->son; p; p = p->brother) {
		if (p->token.number == DCL) processDeclaration(p->son);
		else if (p->token.number == FUNC_DEF) processFuncHeader(p->son);
		else icg_error(3);
	}

	globalSize = offset - 1;

	genSym(base);

	//step2 process the function part
	for (p = ptr->son; p; p = p->brother)
		if (p->token.number == FUNC_DEF) processFunction(p);
	//step3 generate codes for starting routine
	emit1("bgn", globalSize);
	emit0("ldp");
	emitJump("call", "main");
	emit0("end");
}
void processDeclaration(Node* ptr) {
	int typeSpecifier, typeQualifier;
	Node *p, *q;

	if (ptr->token.number != DCL_SPEC) icg_error(4);
	//step1 process DCL_SPEC
	typeSpecifier = INT_TYPE;
	typeQualifier = VAR_TYPE;
	p = ptr->son;
	while (p) {
		if (p->token.number == INT_NODE) typeSpecifier = INT_TYPE;
		else if (p->token.number == CONST_NODE) typeQualifier = CONST_TYPE;
		else {
			printf("type error\n");
			return;
		}
		p = p->brother;
	}
	p = ptr->brother;
	if (p->token.number != DCL_ITEM) icg_error(5);

	while (p) {
		q = p->son;
		switch (q->token.number){
		case SIMPLE_VAR:
			processSimpleVariable(q, typeSpecifier, typeQualifier);
			break;
		case ARRAY_VAR:
			processArrayVariable(q, typeSpecifier, typeQualifier);
			break;
		default:
			printf("error in SIMPLE_VAR or ARRAY_VAR\n");
			break;
		}
		p = p->brother;
	}
}
void processSimpleVariable(Node* ptr, int typeSpecifier, int typeQualifier) {
	Node* p = ptr->son;
	Node* q = ptr->brother;
	int stIndex, size, initialValue;
	int sign = 1;
	if (ptr->token.number != SIMPLE_VAR) printf("error in SIMPLE_VAR\n");
	if (typeQualifier == CONST_TYPE) {
		if (q == NULL) {
			printf("%s must have a constant value\n", ptr->son->token.value.id);
			return;
		}
		if (q->token.number == UNARY_MINUS) {
			sign = -1;
			q = q->son;
		}
		initialValue = sign * q->token.value.num;

		stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, 0, 0, 0, initialValue);
	}
	else {
		size = typeSize(typeSpecifier);
		stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, size, 0);
		offset += size;
	}
}
void processArrayVariable(Node *ptr, int typeSpecifier, int typeQualifier) {
	Node* p = ptr->son;
	int stIndex, size;

	if (ptr->token.number != ARRAY_VAR) {
		printf("error in ARRAY_VAR\n");
		return;
	}
	if (p->brother == NULL)
		printf("array size must be specified\n");
	else size = p->brother->token.value.num;

	size *= typeSize(typeSpecifier);
	stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, size, 0);
	offset += size;
}
void processFuncHeader(Node* ptr) {
	int noArguments, returnType;
	int stIndex;
	Node* p;

	if (ptr->token.number != FUNC_HEAD)
		printf("error in processFuncHeader\n");
	//step1: process the function return type
	p = ptr->son->son;
	while (p) {
		if (p->token.number == INT_NODE) returnType = INT_TYPE;
		else if (p->token.number == VOID_NODE) returnType = VOID_TYPE;
		else printf("invalid function return type\n");
		p = p->brother;
	}
	//step2: count the number of formal parameters
	p = ptr->son->brother->brother;
	p = p->son;

	noArguments = 0;
	while (p) {
		noArguments++;
		p = p->brother;
	}
	//step3: insert the function name
	stIndex = insert(ptr->son->brother->token.value.id, returnType, FUNC_TYPE, 1, 0, noArguments, 0);
}
void processFunction(Node* ptr) {
	Node* q;
	Node* p = ptr->son->son->brother;
	int stIndex, localVarSize=0, dcl_index;
	char* funcName = p->token.value.id;

	stIndex = lookup(p->token.value.id);
	base = 2; offset = 1;
	returnWithValue = 0;
	//step1: process formal parameters
	localVarSize = symbolTable[stIndex].width;
	//step2: process the declaration part in function body
	for (p = ptr->son->brother->son->son; p; p = p->brother) {
		for (q = p->son->brother; q; q = q->brother) {
			if (q->son->token.number == ARRAY_VAR) {
				localVarSize += q->son->son->brother->token.value.num;
			}
			else
				localVarSize += 1;
		}
		processDeclaration(p->son);
	}
	//step3: emit the function start code
	fprintf(ucodeFile,"%-10s %s     %d     %d     %d\n", funcName, "fun", localVarSize, 2, 2);
	genSym(base);
	//step4: process the statement part in function body
	processStatement(ptr->son->brother);
	//step5: check if return type and return value
	if (!returnWithValue)
		fprintf(ucodeFile, "           ret\n");
	//step6: generate the ending codes
	emit0("end");
}
void processOperator(Node* ptr) {
	int stIndex;
	switch (ptr->token.number){
	case ASSIGN_OP: {
		Node* lhs = ptr->son;
		Node* rhs = ptr->son->brother;
		//step1: left-hand
		if (lhs->noderep == nonterm) {
			lvalue = 1;
			processOperator(lhs);
			lvalue = 0;
		}
		//step2: right-hand
		if (rhs->noderep == nonterm) {
			if (rhs->token.number == ASSIGN_OP) {
				multAssignFlag += 1;
			}
			processOperator(rhs);
		}
		else rv_emit(rhs);
		//step3: henerate a store instruction
		if (lhs->noderep == terminal) {
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->token.value.id);
				return;
			}
			emit2("str", symbolTable[stIndex].base, symbolTable[stIndex].offset);
			if (multAssignFlag) {
				emit2("lod", symbolTable[stIndex].base, symbolTable[stIndex].offset);
				multAssignFlag -= 1;
			}
		}
		else emit0("sti");
		break;
	}
	case ADD_ASSIGN: case SUB_ASSIGN: case MUL_ASSIGN:
	case DIV_ASSIGN: case MOD_ASSIGN: {
		Node* lhs = ptr->son;
		Node* rhs = ptr->son->brother;
		int nodeNumber = ptr->token.number;

		ptr->token.number = ASSIGN_OP;

		//step1: left hand side
		if (lhs->noderep == nonterm) {
			lvalue = 1;
			processOperator(lhs);
			lvalue = 0;
		}
		ptr->token.number = nodeNumber;
		//step2: repeating
		if (lhs->noderep == nonterm)
			processOperator(lhs);
		else rv_emit(lhs);
		//step3: right hand side
		if (rhs->noderep == nonterm)
			processOperator(rhs);
		else rv_emit(rhs);
		//step4: emit the corresponding operation
		switch (ptr->token.number){
		case ADD_ASSIGN: emit0("add"); break;
		case SUB_ASSIGN: emit0("sub"); break;
		case MUL_ASSIGN: emit0("mult"); break;
		case DIV_ASSIGN: emit0("div"); break;
		case MOD_ASSIGN: emit0("mod"); break;
		default:
			break;
		}
		//step5: store
		if (lhs->noderep == terminal) {
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->son->token.value.id);
				return;
			}
			emit2("str", symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else emit0("sti");
		break;
	}
	case ADD: case SUB: case MUL: case DIV: case MOD:
	case EQ: case NE: case GT: case LT: case GE: case LE:
	case LOGICAL_AND: case LOGICAL_OR: {
		Node* lhs = ptr->son;
		Node* rhs = ptr->son->brother;

		//step1: left operand
		if (lhs->noderep == nonterm) processOperator(lhs);
		else rv_emit(lhs);
		//step2: right operand
		if (rhs->noderep == nonterm) processOperator(rhs);
		else rv_emit(rhs);
		//step3: root
		switch (ptr->token.number){
		case ADD: emit0("add"); break;
		case SUB: emit0("sub"); break;
		case MUL: emit0("mult"); break;
		case DIV: emit0("div"); break;
		case MOD: emit0("mod"); break;
		case EQ: emit0("eq"); break;
		case NE: emit0("ne"); break;
		case GT: emit0("gt"); break;
		case LT: emit0("lt"); break;
		case GE: emit0("ge"); break;
		case LE: emit0("le"); break;
		case LOGICAL_AND: emit0("and"); break;
		case LOGICAL_OR: emit0("or"); break;
		default:
			break;
		}
		break;
	}
	case UNARY_MINUS: case LOGICAL_NOT: {
		Node* p = ptr->son;

		if (p->noderep == nonterm) processOperator(p);
		else rv_emit(p);
		switch (ptr->token.number){
		case UNARY_MINUS: emit0("neg"); break;
		case LOGICAL_NOT: emit0("notop"); break;
		default:
			break;
		}
		break;
	}
	case PRE_INC: case PRE_DEC: case POST_INC: case POST_DEC: {
		Node* p = ptr->son;
		Node* q;
		int amount = 1;

		if (p->noderep == nonterm) processOperator(p);
		else rv_emit(p);

		q = p;
		while (q->noderep != terminal) q = q->son;
		if (!q || (q->token.number != tident)) {
			printf("increment/decrement operators can not be applied in expression\n");
			return;
		}
		stIndex = lookup(q->token.value.id);
		if (stIndex == -1) return;
		switch (ptr->token.number){
		case PRE_INC: emit0("inc"); break;
		case PRE_DEC: emit0("dec"); break;
		case POST_INC: emit0("inc"); break;
		case POST_DEC: emit0("dec"); break;
		default:
			break;
		}
		if (p->noderep == terminal) {
			stIndex = lookup(p->token.value.id);
			if (stIndex == -1) return;
			emit2("str", symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else if (p->token.number == INDEX) {
			lvalue = 1;
			processOperator(p);
			lvalue = 0;
			emit0("swp");
			emit0("sti");
		}
		else printf("error in increment/decrement operators\n");
		break;
	}
	case INDEX: {
		Node* indexExp = ptr->son->brother;

		if (indexExp->noderep == nonterm) processOperator(indexExp);
		else rv_emit(indexExp);
		stIndex = lookup(ptr->son->token.value.id);
		if (stIndex == -1) {
			printf("undefined variable : %s", ptr->son->token.value.id);
			return;
		}
		emit2("lda", symbolTable[stIndex].base, symbolTable[stIndex].offset);
		emit0("add");
		if (!lvalue) emit0("ldi");
		break;
	}
	case CALL: {
		Node* p = ptr->son;
		char* functionName;
		int stIndex, noArguments;
		
		if (checkPredefined(p))
			break;

		functionName = p->token.value.id;
		stIndex = lookup(functionName);
		if (stIndex == -1) break;
		noArguments = symbolTable[stIndex].width;

		emit0("ldp");
		p = p->brother;
		while (p) {
			if (p->noderep == nonterm) processOperator(p);
			else rv_emit(p);
			noArguments--;
			p = p->brother;
		}
		if (noArguments > 0)
			printf("%s: too few actual arguments", functionName);
		if (noArguments < 0)
			printf("%s: too many actual arguments", functionName);
		emitJump("call", ptr->son->token.value.id);
		break;
	}
	default:
		break;
	}
}
void processStatement(Node* ptr) {
	Node* p;
	switch (ptr->token.number){
	case COMPOUND_ST:
		p = ptr->son->brother;
		p = p->son;
		while (p) {
			processStatement(p);
			p = p->brother;
		}
		break;
	case EXP_ST:
		if (ptr->son != NULL) processOperator(ptr->son);
		break;
	case RETURN_ST:
		if (ptr->son != NULL) {
			returnWithValue = 1;
			p = ptr->son;
			if (p->noderep == nonterm) processOperator(p);
			else rv_emit(p);
			emit0("retv");
		}
		else emit0("ret");
		break;
	case IF_ST: {
		char label[LABEL_SIZE];

		genLabel(label);
		processCondition(ptr->son);
		emitJump("fjp", label);
		processStatement(ptr->son->brother);
		emitLabel(label);
	}
	break;
	case IF_ELSE_ST: {
		char label1[LABEL_SIZE], label2[LABEL_SIZE];

		genLabel(label1); genLabel(label2);
		processCondition(ptr->son);
		emitJump("fjp", label1);
		processStatement(ptr->son->brother);
		emitJump("ujp", label2);
		emitLabel(label1);
		processStatement(ptr->son->brother->brother);
		emitLabel(label2);
	}
	break;
	case WHILE_ST: {
		char label1[LABEL_SIZE], label2[LABEL_SIZE];

		genLabel(label1); genLabel(label2);
		emitLabel(label1);
		processCondition(ptr->son);
		emitJump("fjp", label2);
		processStatement(ptr->son->brother);
		emitJump("ujp", label1);
		emitLabel(label2);
	}
	break;
	default:
		printf("not yet implemented.\n");
		break;
	}
}
void processCondition(Node* ptr) {
	if (ptr->noderep == nonterm) processOperator(ptr);
	else rv_emit(ptr);
}
void rv_emit(Node* ptr) {
	int stIndex;

	if (ptr->token.number == tnumber)
		emit1("ldc", ptr->token.value.num);
	else {
		stIndex = lookup(ptr->token.value.id);
		if (stIndex == -1) return;
		if (symbolTable[stIndex].typeQualifier == CONST_TYPE)
			emit1("ldc", symbolTable[stIndex].initialValue);
		else if (symbolTable[stIndex].width > 1)
			emit2("lda", symbolTable[stIndex].base, symbolTable[stIndex].offset);
		else
			emit2("lod", symbolTable[stIndex].base, symbolTable[stIndex].offset);
	}
}
void emit0(char* value) {
	fprintf(ucodeFile,"           %s\n",value);
}
void emit1(char* value, int p) {
	fprintf(ucodeFile,"           %s     %d\n", value, p);
}
void emit2(char* value, int p, int q) {
	fprintf(ucodeFile,"           %s     %d     %d\n", value, p, q);
}
void emitJump(char* value, char* Label) {
	fprintf(ucodeFile,"           %s     %s\n", value, Label);
}
void emitLabel(char* name) {
	fprintf(ucodeFile,"%-10s %s\n", name, "nop");
}
void genLabel(char* name) {
	sprintf(name, "$$%d", labelNumber);
	labelNumber++;
}
int checkPredefined(Node * ptr){
	Node *p = ptr;
	char *functionName;
	int noArguments;
	int stIndex;

	functionName = p->token.value.id;

	if (strcmp("read", p->token.value.id) == 0)
	{
		noArguments = 1;

		emit0("ldp");
		p = p->brother;
		while (p)
		{
			if (p->noderep == nonterm)
			{
				processOperator(p);
			}
			else {
				stIndex = lookup(p->token.value.id);
				if (stIndex == -1)
				{
					break;
				}
				emit2("lda", symbolTable[stIndex].base, symbolTable[stIndex].offset);
			}
			noArguments--;
			p = p->brother;
		}

		if (noArguments > 0)
		{
			printf("%s: too few actual arguments\n", functionName);
		}

		if (noArguments < 0)
		{
			printf("%s: too many actual arguments\n", functionName);
		}

		emitJump("call", functionName);
		return 1;
	}
	else if (strcmp("write", p->token.value.id) == 0)
	{
		noArguments = 1;

		emit0("ldp");
		p = p->brother;
		while (p)
		{
			if (p->noderep == nonterm)
				processOperator(p);
			else {
				stIndex = lookup(p->token.value.id);
				if (stIndex == -1) break;;
				emit2("lod", symbolTable[stIndex].base, symbolTable[stIndex].offset);
			}
			noArguments--;
			p = p->brother;
		}

		if (noArguments > 0)
		{
			printf("%s: too few actual arguments\n", functionName);
		}

		if (noArguments < 0)
		{
			printf("%s: too many actual arguments\n", functionName);
		}
		emitJump("call", functionName);
		return 1;
	}
	else if (strcmp(functionName, "lf") == 0)
	{
		emitJump("call", functionName);
		return 1;
	}

	return 0;
}
void icg_error(int num){
	printf("Error error code %d ", num);
}
int typeSize(int typeSpecifier){
	if (typeSpecifier == INT_TYPE) {
		return 1;
	}
	return 0;
}
void emitSym(int base, int offset, int size) {
	fprintf(ucodeFile,"           %s     %d     %d     %d\n", "sym", base, offset, size);
}