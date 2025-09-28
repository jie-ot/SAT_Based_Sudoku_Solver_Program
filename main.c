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
    printf("********这是一个SAT求解器和数独游戏********\n");
    printf("-------------------------------------------\n");
    while (1) {
        printf("1.求解cnf文件\n2.数独游戏\n0.退出程序\n");
        printf("请输入您的选择：");
        int choice1;
        scanf("%d", &choice1);
        if (choice1 == 0) break;
        switch (choice1) {
            case 1: {
                printf("请输入cnf文件路径：");
                char cnf_file_path[512];
                scanf("%511s", cnf_file_path);
                getchar();
                printf("\n");
                size_t len_file_path = strlen(cnf_file_path);
                // 如果首尾都是双引号，就把它们去掉
                if (len_file_path >= 2 && cnf_file_path[0] == '"' && cnf_file_path[len_file_path - 1] == '"') {
                    // 把 path[1..len-2] 左移到 path[0]
                    memmove(cnf_file_path, cnf_file_path + 1, len_file_path - 2);
                    cnf_file_path[len_file_path - 2] = '\0';
                }
                formula* F = parse_cnf(cnf_file_path);
                if (!F) {
                    fprintf(stderr, "解析CNF文件失败\n");
                    break;
                }
                Assignment A;
                printf("算法1：\n");
                double time_ms, time_ms_optimized;
                bool SAT_optimized = solve_dpll_v2(F, &A, &time_ms_optimized);
                print_result(SAT_optimized, &A, F->var_num, time_ms_optimized);
                //将.cnf文件名改为同名的.res文件
                size_t len = strlen(cnf_file_path);
                char res_file_path[512];
                if (len > 4) {
                    memcpy(res_file_path, cnf_file_path, len - 4);
                    strcpy(res_file_path + (len - 4), ".res");
                }
                write_to_file(res_file_path, SAT_optimized, &A, F->var_num, time_ms_optimized);
                printf("算法2：\n");
                bool SAT = solve_dpll_v1(F, &A, &time_ms);
                print_result(SAT, &A, F->var_num, time_ms);
                if (time_ms == time_ms_optimized) printf("时间优化率为0\n");
                else printf("时间优化率为：[(t-t0)/t]*100%% = %.2f%%\n", (time_ms - time_ms_optimized) / time_ms * 100.00);
                //printf("算法3：\n");
                //bool SAT_ = solve_dpll_v3(F, &A, &time_ms);
                //print_result(SAT_, &A, F->var_num, time_ms);
                free(A.values);
                free_formula(F);
                printf("\n");
                break;
            }
            case 2: {
                while (1) {
                    printf("数独种类： 1.普通数独  2.百分号数独\n输入 0 退出数独游戏\n");
                    printf("请输入您的选择：");
                    int choice2;
                    scanf("%d", &choice2);
                    getchar();
                    printf("\n");
                    if (choice2 == 0) break;
                    switch (choice2) {
                        case 1: {
                            while (1) {
                                printf("操作选择： 1.解决数独  2.随机生成一个数独\n输入 0 重新选择数独种类\n");
                                printf("请输入您的选择：");
                                int choice3;
                                scanf("%d", &choice3);
                                getchar();
                                printf("\n");
                                if (choice3 == 0) break;
                                switch (choice3) {
                                    case 1: {
                                        while (1) {
                                            printf("数独来源： 1.来自于文件  2.手动输入数独格局\n输入 0 返回上一级\n");
                                            printf("请输入您的选择：");
                                            int choice4;
                                            scanf("%d", &choice4);
                                            getchar();
                                            printf("\n");
                                            if (choice4 == 0) break;
                                            switch (choice4) {
                                                case 1: {
                                                    printf("请输入文件路径：");
                                                    char sudoku_file_path[512];
                                                    if (fgets(sudoku_file_path, sizeof(sudoku_file_path), stdin) == NULL)  sudoku_file_path[0] = '\0';
                                                    else {
                                                        size_t len = strlen(sudoku_file_path);
                                                        if (len > 0 && (sudoku_file_path[len - 1] == '\n' || sudoku_file_path[len - 1] == '\r')) sudoku_file_path[--len] = '\0';
                                                    }

                                                    size_t len_file_path = strlen(sudoku_file_path);
                                                    //去掉首尾的双引号
                                                    if (len_file_path >= 2 && sudoku_file_path[0] == '"' && sudoku_file_path[len_file_path - 1] == '"') {
                                                        memmove(sudoku_file_path, sudoku_file_path + 1, len_file_path - 2);
                                                        sudoku_file_path[len_file_path - 2] = '\0';
                                                    }
                                                    int count = sudoku_solve_from_file(sudoku_file_path);
                                                    if (count == -1) {
                                                        printf("文件打开失败！\n");
                                                        break;
                                                    }
                                                    else {
                                                        printf("成功求解数独个数：%d \n", count);
                                                        break;
                                                    }
                                                }
                                                case 2: {
                                                    printf("请输入待解决的数独，格式要求：81个符号按行优先排列，符号"".""或""0""表示待填空格\n");
                                                    char sudoku_str[100];
                                                    scanf("%s", sudoku_str);
                                                    bool SAT = sudoku_solve_from_string(sudoku_str);
                                                    break;
                                                }
                                                default:
                                                    printf("请输入有效的选项！\n\n");
                                                    break;
                                            }
                                        }
                                        break;
                                    }
                                    case 2: {
                                        while (1) {
                                            printf("请选择难度： 1.简单  2.中等  3.困难\n输入 0 返回上一级\n");
                                            printf("请输入您的选择：");
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
                                                    printf("查看答案请输入 1 ，退出请输入 0 ：");
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
                                                    printf("请输入有效的选项！\n\n");
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    default:
                                        printf("请输入有效的选项！\n\n");
                                        break;
                                }
                            }
                            break;
                        }
                        case 2: {
                            while (1) {
                                printf("操作选择： 1.解决百分号数独  2.随机生成一个百分号数独\n输入 0 重新选择数独种类\n");
                                printf("请输入您的选择：");
                                int choice7;
                                scanf("%d", &choice7);
                                getchar();
                                printf("\n");
                                if (choice7 == 0) break;
                                switch (choice7) {
                                    case 1: {
                                        while (1) {
                                            printf("数独来源： 1.来自于文件  2.手动输入百分号数独格局\n输入 0 返回上一级\n");
                                            printf("请输入您的选择：");
                                            int choice8;
                                            scanf("%d", &choice8);
                                            getchar();
                                            printf("\n");
                                            if (choice8 == 0) break;
                                            switch (choice8) {
                                            case 1: {
                                                printf("请输入文件路径：");
                                                char xsudoku_file_path[512];
                                                if (fgets(xsudoku_file_path, sizeof(xsudoku_file_path), stdin) == NULL)  xsudoku_file_path[0] = '\0';
                                                else {
                                                    size_t len = strlen(xsudoku_file_path);
                                                    if (len > 0 && (xsudoku_file_path[len - 1] == '\n' || xsudoku_file_path[len - 1] == '\r')) xsudoku_file_path[--len] = '\0';
                                                }

                                                size_t len_file_path = strlen(xsudoku_file_path);
                                                //去掉首尾的双引号
                                                if (len_file_path >= 2 && xsudoku_file_path[0] == '"' && xsudoku_file_path[len_file_path - 1] == '"') {
                                                    memmove(xsudoku_file_path, xsudoku_file_path + 1, len_file_path - 2);
                                                    xsudoku_file_path[len_file_path - 2] = '\0';
                                                }
                                                int count = xsudoku_solve_from_file(xsudoku_file_path);
                                                if (count == -1) {
                                                    printf("文件打开失败！\n\n");
                                                    break;
                                                }
                                                else {
                                                    printf("成功求解百分号数独个数：%d \n\n", count);
                                                    break;
                                                }
                                            }
                                            case 2: {
                                                printf("请输入待解决的百分号数独，格式要求：81个符号按行优先排列，符号"".""或""0""表示待填空格\n");
                                                char xsudoku_str[100];
                                                scanf("%s", xsudoku_str);
                                                bool SAT = xsudoku_solve_from_string(xsudoku_str);
                                                break;
                                            }
                                            default:
                                                printf("请输入有效的选项！\n\n");
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                    case 2: {
                                        while (1) {
                                            printf("请选择难度： 1.简单  2.中等  3.困难\n输入 0 返回上一级\n");
                                            printf("请输入您的选择：");
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
                                                    printf("进入互动请输入 2 ，查看答案请输入 1 ：");
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
                                                            printf("请输入行、列、值（退出输入0）：");
                                                            int c, r, value;
                                                            scanf("%d", &c);
                                                            if (c == 0) break;
                                                            scanf("%d %d", &r, &value);
                                                            if (s[(c - 1) * 9 + (r - 1)] != '_') {
                                                                printf("非法输入！\n");
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
                                                        printf("查看答案请输入 1，退出输入 0:");
                                                        int k;
                                                        scanf("%d", &k);
                                                        if (k == 1) xsudoku_solve_from_string(random_xsudoku);
                                                        else break;
                                                    }
                                                    else break;
                                                }
                                                default: {
                                                    //printf("请输入有效的选项！\n\n");
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    default:
                                        printf("请输入有效的选项！\n\n");
                                        break;
                                }
                            }
                            break;
                        }
                        default:
                            printf("请输入有效的选项！\n\n");
                            break;
                    }
                }
                break;
            }
            default:
                printf("请输入有效的选项！\n\n");
                break;
        }
    }
    return 0;
}