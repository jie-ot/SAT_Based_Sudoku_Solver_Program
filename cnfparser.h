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

//文字、子句、CNF公式数据结构
typedef struct literal{
	int var;                //正数为true，负数为false
	struct literal* next;   //指向下一个文字
}literal;
typedef struct clause {
	literal* lists;         //子句的文字链表
	struct clause* next;    //指向下一个子句
}clause;
typedef struct {
	int var_num;            //文字数量
	int clause_num;         //子句数量
	clause* clauses;        //子句链表头
}formula;                   //表示整个CNF公式

//跳过所有以‘c’开头的注释行
void skip_comment(FILE* fp); 

//解析p开头的cnf文件说明信息，n为文字数，m为子句数
bool paresr_p_cnf(FILE* fp, int* n, int* m);

//按行读子句，每个子句构造一个链表
//添加一个文字到子句的末尾
void add_literal(clause* cl, int lit);
//解析所有子句，构建CNF公式
formula* parse_cnf(const char* filename);

//辅助打印函数
void print_clause(const clause* cl);
void print_formula(const formula* F);