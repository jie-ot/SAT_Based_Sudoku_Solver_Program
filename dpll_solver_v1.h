#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"

//���ÿ���������յĸ�ֵ��1����0����
//    values[i] =  1 ��ʾ xi=true
//    values[i] =  0 ��ʾ xi=false
//    values[i] = -1 ��ʾ xi δ��ֵ
typedef struct {
    int* values;
} Assignment;


//���ô˺���ִ��DPLL���
//����true��ʾSAT��false��ʾUNSAT
//solve_time_ms�������غ�ʱ�����룩
bool solve_dpll_v1(formula* F, Assignment* A, double* time_ms);

//�ͷ��ڴ�
void free_formula(formula* F);