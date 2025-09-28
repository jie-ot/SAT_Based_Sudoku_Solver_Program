#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"dpll_solver_v2.h"

#define SUDOKU_LENGTH 81
#define VAR_NUM 729
#define N 9

//��һ��81�ַ��İٷֺ�����ת����cnf��д��cnf_path���ɹ�����1�����򷵻�0
bool xsudoku_to_cnf(const char* xsudoku_str, const char* cnf_path);

//����dpll���cnf_path�µİٷֺ�������var_countΪ�ܱ�Ԫ����model�����洢cnf��ʽ�Ľ�
bool xsudoku_solver(const char* cnf_path, int var_count, int* model);

//��model��1��var_count������ȡ���ٷֺ������Ľ⣬д��grid_res��
bool xsudoku_model_to_grid(const int* model, int var_count, char* grid_res);

//���ַ���grid_res�д�ӡ�ٷֺ�����
void xsudoku_print(const char* grid_res);

//��һ���ַ���xsudoku_str���ٷֺ�����
bool xsudoku_solve_from_string(const char* xsudoku_str);

//���ı��ļ������ж�ȡ����ٷֺ�������������Ⲣ��ӡ�����سɹ����ĸ���
int xsudoku_solve_from_file(const char* xsudoku_file);

//������ɰٷֺ��������򣬿�ѡ���Ѷ�difficulty
void generate_random_xsudoku(int difficulty, char* xsudoku_str);