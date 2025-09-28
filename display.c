#define _CRT_SECURE_NO_WARNINGS
#include"display.h"

//打印函数，SAT为true时打印结果，为false时表示不成立
void print_result(bool SAT, const Assignment* A, int var_num, double time_ms)
{
	printf("s %d\n", SAT ? 1 : 0);

	if (SAT) {
		printf("v");
		for (int i = 1; i <= var_num; i++) {
			int lit = A->values[i] ? i : -i;
			printf(" %d", lit);
		}
		printf("\n");
	}

	printf("t %.0f ms\n", time_ms);
}

//将结果以.res形式的文件输出，path为文件输出路径
void write_to_file(const char* path, bool SAT, const Assignment* A, int var_num, double time_ms)
{
    FILE* fp = fopen(path, "w");
    if (!fp) {
        printf("写入结果时文件打开或创建失败！\n");
        return;
    }

    fprintf(fp, "s %d\n", SAT ? 1 : 0);

    if (SAT) {
        fprintf(fp, "v");
        for (int i = 1; i <= var_num; i++) {
            int lit = A->values[i] ? i : -i;
            fprintf(fp, " %d", lit);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "t %.0f\n", time_ms);
    //printf("成功写入文件！文件路径为：%s \n", path);
    fclose(fp);
}