#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"
#include"dpll_solver_v1.h"

//��ӡ������SATΪtrueʱ��ӡ�����Ϊfalseʱ��ʾ������
void print_result(bool SAT, const Assignment* A, int var_num, double time_ms);

//�������.res��ʽ���ļ������pathΪ�ļ����·��
void write_to_file(const char* path, bool SAT, const Assignment* A, int var_num, double time_ms);