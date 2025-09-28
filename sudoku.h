#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"dpll_solver_v2.h"

#define SUDOKU_LENGTH 81
#define VAR_NUM 729
#define N 9

//��һ��81�ַ�����ͨ����ת����cnf��д��cnf_path���ɹ�����1�����򷵻�0
bool sudoku_to_cnf(const char* xsudoku_str, const char* cnf_path);

//����dpll���cnf_path�µ�������var_countΪ�ܱ�Ԫ����model�����洢cnf��ʽ�Ľ�
bool sudoku_solver(const char* cnf_path, int var_count, int* model);

//��model��1��var_count������ȡ�������Ľ⣬д��grid_res��
bool sudoku_model_to_grid(const int* model, int var_count, char* grid_res);

//���ַ���grid_res�д�ӡ����
void sudoku_print(const char* grid_res);

//��һ���ַ���sudoku_str�����ͨ����
bool sudoku_solve_from_string(const char* sudoku_str);

//���ı��ļ������ж�ȡ�����ͨ������������Ⲣ��ӡ�����سɹ����ĸ���
int sudoku_solve_from_file(const char* sudoku_file);

//��������������򣬿�ѡ���Ѷ�difficulty
void generate_random_sudoku(int difficulty, char* sudoku_str);