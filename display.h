#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"
#include"dpll_solver_v1.h"

//打印函数，SAT为true时打印结果，为false时表示不成立
void print_result(bool SAT, const Assignment* A, int var_num, double time_ms);

//将结果以.res形式的文件输出，path为文件输出路径
void write_to_file(const char* path, bool SAT, const Assignment* A, int var_num, double time_ms);