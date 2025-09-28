#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include<time.h>
#include<ctype.h>

#define MAX_LINE_SIZE 1000

//���֡��Ӿ䡢CNF��ʽ���ݽṹ
typedef struct literal{
	int var;                //����Ϊtrue������Ϊfalse
	struct literal* next;   //ָ����һ������
}literal;
typedef struct clause {
	literal* lists;         //�Ӿ����������
	struct clause* next;    //ָ����һ���Ӿ�
}clause;
typedef struct {
	int var_num;            //��������
	int clause_num;         //�Ӿ�����
	clause* clauses;        //�Ӿ�����ͷ
}formula;                   //��ʾ����CNF��ʽ

//���������ԡ�c����ͷ��ע����
void skip_comment(FILE* fp); 

//����p��ͷ��cnf�ļ�˵����Ϣ��nΪ��������mΪ�Ӿ���
bool paresr_p_cnf(FILE* fp, int* n, int* m);

//���ж��Ӿ䣬ÿ���Ӿ乹��һ������
//���һ�����ֵ��Ӿ��ĩβ
void add_literal(clause* cl, int lit);
//���������Ӿ䣬����CNF��ʽ
formula* parse_cnf(const char* filename);

//������ӡ����
void print_clause(const clause* cl);
void print_formula(const formula* F);