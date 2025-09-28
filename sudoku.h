#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"dpll_solver_v2.h"

#define SUDOKU_LENGTH 81
#define VAR_NUM 729
#define N 9

//将一个81字符的普通数独转换成cnf并写入cnf_path，成功返回1，否则返回0
bool sudoku_to_cnf(const char* xsudoku_str, const char* cnf_path);

//调用dpll求解cnf_path下的数独，var_count为总变元数，model用来存储cnf公式的解
bool sudoku_solver(const char* cnf_path, int var_count, int* model);

//从model（1到var_count）中提取出数独的解，写入grid_res中
bool sudoku_model_to_grid(const int* model, int var_count, char* grid_res);

//从字符串grid_res中打印数独
void sudoku_print(const char* grid_res);

//从一个字符串sudoku_str求解普通数独
bool sudoku_solve_from_string(const char* sudoku_str);

//从文本文件中逐行读取多个普通数独，批量求解并打印，返回成功求解的个数
int sudoku_solve_from_file(const char* sudoku_file);

//随机生成数独程序，可选择难度difficulty
void generate_random_sudoku(int difficulty, char* sudoku_str);