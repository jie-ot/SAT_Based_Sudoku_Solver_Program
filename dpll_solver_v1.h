#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"

//存放每个变量最终的赋值（1或者0）：
//    values[i] =  1 表示 xi=true
//    values[i] =  0 表示 xi=false
//    values[i] = -1 表示 xi 未赋值
typedef struct {
    int* values;
} Assignment;


//调用此函数执行DPLL求解
//返回true表示SAT，false表示UNSAT
//solve_time_ms用来返回耗时（毫秒）
bool solve_dpll_v1(formula* F, Assignment* A, double* time_ms);

//释放内存
void free_formula(formula* F);