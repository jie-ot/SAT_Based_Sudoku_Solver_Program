#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"

//跳过所有以‘c’开头的注释行
void skip_comment(FILE* fp)
{
	long pos;
	char line[MAX_LINE_SIZE];

	while (1) {
		pos = ftell(fp);
		if (fgets(line, MAX_LINE_SIZE, fp) == NULL) return;

		//跳过行首所有空白字符，定位到第一个有效字符
		char* p = line;
		while (*p && isspace((unsigned char)*p)) p++;

		//如果这一行是注释（第一个非空白字符是'c'）或空行，继续读下一行
		if (*p == 'c' || *p == '\0' || *p == '\n') continue;

		//如果看到以'p'开头，就重置到行首，让parse_p_cnf来处理
		if (*p == 'p') {
			//printf("DEBUG> next line: %s", line);
			fseek(fp, pos, SEEK_SET);
			return;
		}

		//其它行也跳过去；
		fseek(fp, pos, SEEK_SET);
		return;
	}
}

//解析p开头的cnf文件说明信息，n为文字数，m为子句数
bool paresr_p_cnf(FILE* fp, int* n, int* m)
{
	char pch; //读取开头的"p"
	char format[4]; //读取"cnf"，用来比对格式是否正确
	if (fscanf(fp, " %c %3s %d %d", &pch, format, n, m) != 4 || pch != 'p') {
		printf("<DEBUG>: %c %s %d %d\n", pch, format, *n, *m);
		fprintf(stderr, "ERROR: invalid p cnf line\n");
		return false;
	}
	if (strcmp(format, "cnf") != 0) {
		fprintf(stderr, "ERROR: format not cnf\n");
		return false;
	}
	return true;
}

//添加一个文字到子句的末尾
void add_literal(clause* cl, int lit)
{
	literal* node = (literal*)malloc(sizeof(literal));
	node->var = lit;
	node->next = NULL;
	if (cl->lists == NULL) {
		cl->lists = node;
	}
	else {
		literal* p = cl->lists;
		while (p->next != NULL) p = p->next;
		p->next = node;
	}
}

//解析所有子句，构建CNF公式
formula* parse_cnf(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) return NULL;
	skip_comment(fp);
	int n, m;
	if (paresr_p_cnf(fp, &n, &m) == false) {
		fclose(fp);
		return NULL;
	}
	formula* F = (formula*)malloc(sizeof(formula));
	F->clause_num = m;
	F->var_num = n;
	F->clauses = NULL;

	//逐行读子句
	for (int i = 0; i < m; i++) {
		clause* cl = (clause*)malloc(sizeof(clause));
		cl->lists = NULL;
		cl->next = NULL;

		int lit;
		while (fscanf(fp, "%d", &lit) == 1 && lit != 0) {
			add_literal(cl, lit);
		}

		//链入公式
		if (F->clauses == NULL) {
			F->clauses = cl;
		}
		else {
			clause* p = F->clauses;
			while (p->next != NULL) p = p->next;
			p->next = cl;
		}
	}
	fclose(fp);
	return F;
}

// 打印一个子句
void print_clause(const clause* cl)
{
	const literal* p = cl->lists;
	while (p) {
		printf("%d ", p->var);
		p = p->next;
	}
	printf("0\n");
}

// 打印整个公式
void print_formula(const formula* F) 
{
	printf("p cnf %d %d\n", F->var_num, F->clause_num);
	const clause* cl = F->clauses;
	while (cl) {
		print_clause(cl);
		cl = cl->next;
	}
}