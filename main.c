#define _CRT_SECURE_NO_WARNINGS
#include"cnfparser.h"
#include"display.h"
#include"dpll_solver_v1.h"
#include"dpll_solver_v2.h"
//#include"dpll_solver_v3.h"
#include"sudoku.h"
#include"xsudoku.h"

int main()
{
    printf("-------------------------------------------\n");
    printf("********����һ��SAT�������������Ϸ********\n");
    printf("-------------------------------------------\n");
    while (1) {
        printf("1.���cnf�ļ�\n2.������Ϸ\n0.�˳�����\n");
        printf("����������ѡ��");
        int choice1;
        scanf("%d", &choice1);
        if (choice1 == 0) break;
        switch (choice1) {
            case 1: {
                printf("������cnf�ļ�·����");
                char cnf_file_path[512];
                scanf("%511s", cnf_file_path);
                getchar();
                printf("\n");
                size_t len_file_path = strlen(cnf_file_path);
                // �����β����˫���ţ��Ͱ�����ȥ��
                if (len_file_path >= 2 && cnf_file_path[0] == '"' && cnf_file_path[len_file_path - 1] == '"') {
                    // �� path[1..len-2] ���Ƶ� path[0]
                    memmove(cnf_file_path, cnf_file_path + 1, len_file_path - 2);
                    cnf_file_path[len_file_path - 2] = '\0';
                }
                formula* F = parse_cnf(cnf_file_path);
                if (!F) {
                    fprintf(stderr, "����CNF�ļ�ʧ��\n");
                    break;
                }
                Assignment A;
                printf("�㷨1��\n");
                double time_ms, time_ms_optimized;
                bool SAT_optimized = solve_dpll_v2(F, &A, &time_ms_optimized);
                print_result(SAT_optimized, &A, F->var_num, time_ms_optimized);
                //��.cnf�ļ�����Ϊͬ����.res�ļ�
                size_t len = strlen(cnf_file_path);
                char res_file_path[512];
                if (len > 4) {
                    memcpy(res_file_path, cnf_file_path, len - 4);
                    strcpy(res_file_path + (len - 4), ".res");
                }
                write_to_file(res_file_path, SAT_optimized, &A, F->var_num, time_ms_optimized);
                printf("�㷨2��\n");
                bool SAT = solve_dpll_v1(F, &A, &time_ms);
                print_result(SAT, &A, F->var_num, time_ms);
                if (time_ms == time_ms_optimized) printf("ʱ���Ż���Ϊ0\n");
                else printf("ʱ���Ż���Ϊ��[(t-t0)/t]*100%% = %.2f%%\n", (time_ms - time_ms_optimized) / time_ms * 100.00);
                //printf("�㷨3��\n");
                //bool SAT_ = solve_dpll_v3(F, &A, &time_ms);
                //print_result(SAT_, &A, F->var_num, time_ms);
                free(A.values);
                free_formula(F);
                printf("\n");
                break;
            }
            case 2: {
                while (1) {
                    printf("�������ࣺ 1.��ͨ����  2.�ٷֺ�����\n���� 0 �˳�������Ϸ\n");
                    printf("����������ѡ��");
                    int choice2;
                    scanf("%d", &choice2);
                    getchar();
                    printf("\n");
                    if (choice2 == 0) break;
                    switch (choice2) {
                        case 1: {
                            while (1) {
                                printf("����ѡ�� 1.�������  2.�������һ������\n���� 0 ����ѡ����������\n");
                                printf("����������ѡ��");
                                int choice3;
                                scanf("%d", &choice3);
                                getchar();
                                printf("\n");
                                if (choice3 == 0) break;
                                switch (choice3) {
                                    case 1: {
                                        while (1) {
                                            printf("������Դ�� 1.�������ļ�  2.�ֶ������������\n���� 0 ������һ��\n");
                                            printf("����������ѡ��");
                                            int choice4;
                                            scanf("%d", &choice4);
                                            getchar();
                                            printf("\n");
                                            if (choice4 == 0) break;
                                            switch (choice4) {
                                                case 1: {
                                                    printf("�������ļ�·����");
                                                    char sudoku_file_path[512];
                                                    if (fgets(sudoku_file_path, sizeof(sudoku_file_path), stdin) == NULL)  sudoku_file_path[0] = '\0';
                                                    else {
                                                        size_t len = strlen(sudoku_file_path);
                                                        if (len > 0 && (sudoku_file_path[len - 1] == '\n' || sudoku_file_path[len - 1] == '\r')) sudoku_file_path[--len] = '\0';
                                                    }

                                                    size_t len_file_path = strlen(sudoku_file_path);
                                                    //ȥ����β��˫����
                                                    if (len_file_path >= 2 && sudoku_file_path[0] == '"' && sudoku_file_path[len_file_path - 1] == '"') {
                                                        memmove(sudoku_file_path, sudoku_file_path + 1, len_file_path - 2);
                                                        sudoku_file_path[len_file_path - 2] = '\0';
                                                    }
                                                    int count = sudoku_solve_from_file(sudoku_file_path);
                                                    if (count == -1) {
                                                        printf("�ļ���ʧ�ܣ�\n");
                                                        break;
                                                    }
                                                    else {
                                                        printf("�ɹ��������������%d \n", count);
                                                        break;
                                                    }
                                                }
                                                case 2: {
                                                    printf("��������������������ʽҪ��81�����Ű����������У�����"".""��""0""��ʾ����ո�\n");
                                                    char sudoku_str[100];
                                                    scanf("%s", sudoku_str);
                                                    bool SAT = sudoku_solve_from_string(sudoku_str);
                                                    break;
                                                }
                                                default:
                                                    printf("��������Ч��ѡ�\n\n");
                                                    break;
                                            }
                                        }
                                        break;
                                    }
                                    case 2: {
                                        while (1) {
                                            printf("��ѡ���Ѷȣ� 1.��  2.�е�  3.����\n���� 0 ������һ��\n");
                                            printf("����������ѡ��");
                                            int choice5;
                                            scanf("%d", &choice5);
                                            getchar();
                                            printf("\n");
                                            if (choice5 == 0) break;
                                            switch (choice5) {
                                                case 1:
                                                case 2:
                                                case 3: {
                                                    char random_sudoku[82];
                                                    generate_random_sudoku(choice5, random_sudoku);
                                                    for (int i = 0; i < 9; i++) {
                                                        for (int j = 0; j < 9; j++) {
                                                            putchar(random_sudoku[i * 9 + j]);
                                                            putchar(' ');
                                                        }
                                                        printf("\n");
                                                    }
                                                    printf("�鿴�������� 1 ���˳������� 0 ��");
                                                    int choice6;
                                                    scanf("%d", &choice6);
                                                    getchar();
                                                    printf("\n");
                                                    if (choice6 == 1) {
                                                        sudoku_solve_from_string(random_sudoku);
                                                        break;
                                                    }
                                                    else break;
                                                }
                                                default: {
                                                    printf("��������Ч��ѡ�\n\n");
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    default:
                                        printf("��������Ч��ѡ�\n\n");
                                        break;
                                }
                            }
                            break;
                        }
                        case 2: {
                            while (1) {
                                printf("����ѡ�� 1.����ٷֺ�����  2.�������һ���ٷֺ�����\n���� 0 ����ѡ����������\n");
                                printf("����������ѡ��");
                                int choice7;
                                scanf("%d", &choice7);
                                getchar();
                                printf("\n");
                                if (choice7 == 0) break;
                                switch (choice7) {
                                    case 1: {
                                        while (1) {
                                            printf("������Դ�� 1.�������ļ�  2.�ֶ�����ٷֺ��������\n���� 0 ������һ��\n");
                                            printf("����������ѡ��");
                                            int choice8;
                                            scanf("%d", &choice8);
                                            getchar();
                                            printf("\n");
                                            if (choice8 == 0) break;
                                            switch (choice8) {
                                            case 1: {
                                                printf("�������ļ�·����");
                                                char xsudoku_file_path[512];
                                                if (fgets(xsudoku_file_path, sizeof(xsudoku_file_path), stdin) == NULL)  xsudoku_file_path[0] = '\0';
                                                else {
                                                    size_t len = strlen(xsudoku_file_path);
                                                    if (len > 0 && (xsudoku_file_path[len - 1] == '\n' || xsudoku_file_path[len - 1] == '\r')) xsudoku_file_path[--len] = '\0';
                                                }

                                                size_t len_file_path = strlen(xsudoku_file_path);
                                                //ȥ����β��˫����
                                                if (len_file_path >= 2 && xsudoku_file_path[0] == '"' && xsudoku_file_path[len_file_path - 1] == '"') {
                                                    memmove(xsudoku_file_path, xsudoku_file_path + 1, len_file_path - 2);
                                                    xsudoku_file_path[len_file_path - 2] = '\0';
                                                }
                                                int count = xsudoku_solve_from_file(xsudoku_file_path);
                                                if (count == -1) {
                                                    printf("�ļ���ʧ�ܣ�\n\n");
                                                    break;
                                                }
                                                else {
                                                    printf("�ɹ����ٷֺ�����������%d \n\n", count);
                                                    break;
                                                }
                                            }
                                            case 2: {
                                                printf("�����������İٷֺ���������ʽҪ��81�����Ű����������У�����"".""��""0""��ʾ����ո�\n");
                                                char xsudoku_str[100];
                                                scanf("%s", xsudoku_str);
                                                bool SAT = xsudoku_solve_from_string(xsudoku_str);
                                                break;
                                            }
                                            default:
                                                printf("��������Ч��ѡ�\n\n");
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                    case 2: {
                                        while (1) {
                                            printf("��ѡ���Ѷȣ� 1.��  2.�е�  3.����\n���� 0 ������һ��\n");
                                            printf("����������ѡ��");
                                            int choice9;
                                            scanf("%d", &choice9);
                                            getchar();
                                            printf("\n");
                                            if (choice9 == 0) break;
                                            switch (choice9) {
                                                case 1:
                                                case 2:
                                                case 3: {
                                                    char random_xsudoku[82];
                                                    generate_random_xsudoku(choice9, random_xsudoku);
                                                    for (int i = 0; i < 9; i++) {
                                                        for (int j = 0; j < 9; j++) {
                                                            putchar(random_xsudoku[i * 9 + j]);
                                                            putchar(' ');
                                                        }
                                                        printf("\n");
                                                    }
                                                    printf("���뻥�������� 2 ���鿴�������� 1 ��");
                                                    int choice10;
                                                    scanf("%d", &choice10);
                                                    getchar();
                                                    if (choice10 == 1) {
                                                        xsudoku_solve_from_string(random_xsudoku);
                                                        break;
                                                    }
                                                    else if (choice10 == 2) {
                                                        char s[82];
                                                        strcpy(s, random_xsudoku);
                                                        while (1) {
                                                            printf("�������С��С�ֵ���˳�����0����");
                                                            int c, r, value;
                                                            scanf("%d", &c);
                                                            if (c == 0) break;
                                                            scanf("%d %d", &r, &value);
                                                            if (s[(c - 1) * 9 + (r - 1)] != '_') {
                                                                printf("�Ƿ����룡\n");
                                                                continue;
                                                            }
                                                            s[(c - 1) * 9 + (r - 1)] = value + '0';
                                                            for (int i = 0; i < 9; i++) {
                                                                for (int j = 0; j < 9; j++) {
                                                                    putchar(s[i * 9 + j]);
                                                                    putchar(' ');
                                                                }
                                                                printf("\n");
                                                            }
                                                        }
                                                        printf("�鿴�������� 1���˳����� 0:");
                                                        int k;
                                                        scanf("%d", &k);
                                                        if (k == 1) xsudoku_solve_from_string(random_xsudoku);
                                                        else break;
                                                    }
                                                    else break;
                                                }
                                                default: {
                                                    //printf("��������Ч��ѡ�\n\n");
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    default:
                                        printf("��������Ч��ѡ�\n\n");
                                        break;
                                }
                            }
                            break;
                        }
                        default:
                            printf("��������Ч��ѡ�\n\n");
                            break;
                    }
                }
                break;
            }
            default:
                printf("��������Ч��ѡ�\n\n");
                break;
        }
    }
    return 0;
}