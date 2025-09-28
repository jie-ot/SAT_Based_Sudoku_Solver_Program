#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"

//���������ԡ�c����ͷ��ע����
void skip_comment(FILE* fp)
{
	long pos;
	char line[MAX_LINE_SIZE];

	while (1) {
		pos = ftell(fp);
		if (fgets(line, MAX_LINE_SIZE, fp) == NULL) return;

		//�����������пհ��ַ�����λ����һ����Ч�ַ�
		char* p = line;
		while (*p && isspace((unsigned char)*p)) p++;

		//�����һ����ע�ͣ���һ���ǿհ��ַ���'c'������У���������һ��
		if (*p == 'c' || *p == '\0' || *p == '\n') continue;

		//���������'p'��ͷ�������õ����ף���parse_p_cnf������
		if (*p == 'p') {
			//printf("DEBUG> next line: %s", line);
			fseek(fp, pos, SEEK_SET);
			return;
		}

		//������Ҳ����ȥ��
		fseek(fp, pos, SEEK_SET);
		return;
	}
}

//����p��ͷ��cnf�ļ�˵����Ϣ��nΪ��������mΪ�Ӿ���
bool paresr_p_cnf(FILE* fp, int* n, int* m)
{
	char pch; //��ȡ��ͷ��"p"
	char format[4]; //��ȡ"cnf"�������ȶԸ�ʽ�Ƿ���ȷ
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

//���һ�����ֵ��Ӿ��ĩβ
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

//���������Ӿ䣬����CNF��ʽ
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

	//���ж��Ӿ�
	for (int i = 0; i < m; i++) {
		clause* cl = (clause*)malloc(sizeof(clause));
		cl->lists = NULL;
		cl->next = NULL;

		int lit;
		while (fscanf(fp, "%d", &lit) == 1 && lit != 0) {
			add_literal(cl, lit);
		}

		//���빫ʽ
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

// ��ӡһ���Ӿ�
void print_clause(const clause* cl)
{
	const literal* p = cl->lists;
	while (p) {
		printf("%d ", p->var);
		p = p->next;
	}
	printf("0\n");
}

// ��ӡ������ʽ
void print_formula(const formula* F) 
{
	printf("p cnf %d %d\n", F->var_num, F->clause_num);
	const clause* cl = F->clauses;
	while (cl) {
		print_clause(cl);
		cl = cl->next;
	}
}