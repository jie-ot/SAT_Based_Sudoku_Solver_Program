#define _CRT_SECURE_NO_WARNINGS
#include"xsudoku.h"

//内部共享的全局数组，用于回溯构建数独解
static int full[N][N];

//将三维i,j,n变量转换为一维连续的编号
static inline int var_to_num(int i, int j, int n)
{
	return (i - 1) * 81 + (j - 1) * 9 + n;
}

//将一个81字符的百分号数独转换成cnf并写入cnf_path，成功返回1，否则返回0
bool xsudoku_to_cnf(const char* xsudoku_str, const char* cnf_path)
{
	if (!xsudoku_str || !cnf_path) return false;

	//校验输入长度与字符
	for (int k = 0; k < SUDOKU_LENGTH; ++k) {
		char c = xsudoku_str[k];
		if (c == '\0') return false; // 输入长度不足
		else if (c >= '1' && c <= '9') { } //合法数字
		else if (c == '.' || c == '0' || c == '_') { } //合法空格字符
		else return false; // 非法字符
	}

	FILE* fp = fopen(cnf_path, "wb");
	if (!fp) return false;

	fprintf(fp, "p cnf %d %10d\n", VAR_NUM, 0);
	fflush(fp);

	int clauses = 0;
	//每格至少一个数
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			for (int n = 1; n <= 9; n++) {
				fprintf(fp, "%d ", var_to_num(i, j, n));
			}
			fprintf(fp, "0\n");
			clauses++;
		}
	}
	//每格至多一个数
	for (int i = 1; i <= 9; i++) for (int j = 1; j <= 9; j++) {
		for (int a = 1; a <= 9; a++) for (int b = a + 1; b <= 9; b++) {
			fprintf(fp, "-%d -%d 0\n", var_to_num(i, j, a), var_to_num(i, j, b));
			clauses++;
		}
	}
	//行不重复
	for (int i = 1; i <= 9; i++) for (int n = 1; n <= 9; n++) {
		for (int j = 1; j <= 9; j++) fprintf(fp, "%d ", var_to_num(i, j, n));
		fprintf(fp, "0\n"); clauses++;
		for (int j1 = 1; j1 <= 9; j1++) for (int j2 = j1 + 1; j2 <= 9; j2++) {
			fprintf(fp, "-%d -%d 0\n", var_to_num(i, j1, n), var_to_num(i, j2, n));
			clauses++;
		}
	}
	//列不重复
	for (int j = 1; j <= 9; j++) for (int n = 1; n <= 9; n++) {
		for (int i = 1; i <= 9; i++) fprintf(fp, "%d ", var_to_num(i, j, n));
		fprintf(fp, "0\n"); clauses++;
		for (int i1 = 1; i1 <= 9; i1++) for (int i2 = i1 + 1; i2 <= 9; i2++) {
			fprintf(fp, "-%d -%d 0\n", var_to_num(i1, j, n), var_to_num(i2, j, n));
			clauses++;
		}
	}
	//九宫格内不重复	
	for (int bi = 0; bi < 3; bi++) for (int bj = 0; bj < 3; bj++) {
		for (int n = 1; n <= 9; n++) {
			//至少一个
			for (int di = 1; di <= 3; di++) for (int dj = 1; dj <= 3; dj++) {
				fprintf(fp, "%d ", var_to_num(bi * 3 + di, bj * 3 + dj, n));
			}
			fprintf(fp, "0\n"); clauses++;
			//至多一个
			int idx = 0, cells[9][2];
			for (int di = 1; di <= 3; di++) for (int dj = 1; dj <= 3; dj++) {
				cells[idx][0] = bi * 3 + di; cells[idx++][1] = bj * 3 + dj;
			}
			for (int x = 0; x < 9; x++) for (int y = x + 1; y < 9; y++) {
				fprintf(fp, "-%d -%d 0\n", var_to_num(cells[x][0], cells[x][1], n), var_to_num(cells[y][0], cells[y][1], n));
				clauses++;
			}
		}
	}
	//xsudoku_str中的已知线索
	for (int k = 0; k < SUDOKU_LENGTH; k++) {
		char c = xsudoku_str[k];
		if (c >= '1' && c <= '9') {
			int i = k / 9 + 1, j = k % 9 + 1, n = c - '0';
			fprintf(fp, "%d 0\n", var_to_num(i, j, n)); clauses++;
		}
	}
	//cnf公式限制条件7：百分号内不重复
	int diag_i[9] = { 1,2,3,4,5,6,7,8,9 };
	int diag_j[9] = { 9,8,7,6,5,4,3,2,1 };
	for (int n = 1; n <= 9; n++) {
		// 至少一个
		for (int k = 0; k < 9; k++) {
			fprintf(fp, "%d ", var_to_num(diag_i[k], diag_j[k], n));
		}
		fprintf(fp, "0\n");
		clauses++;
		// 至多一个
		for (int x = 0; x < 9; x++) {
			for (int y = x + 1; y < 9; y++) {
				fprintf(fp, "-%d -%d 0\n",
					var_to_num(diag_i[x], diag_j[x], n),
					var_to_num(diag_i[y], diag_j[y], n));
				clauses++;
			}
		}
	}
	int up_i[9] = { 2,2,2,3,3,3,4,4,4 };
	int up_j[9] = { 2,3,4,2,3,4,2,3,4 };
	for (int n = 1; n <= 9; n++) {
		// 至少一个
		for (int k = 0; k < 9; k++) {
			fprintf(fp, "%d ", var_to_num(up_i[k], up_j[k], n));
		}
		fprintf(fp, "0\n");
		clauses++;
		// 至多一个
		for (int x = 0; x < 9; x++) {
			for (int y = x + 1; y < 9; y++) {
				fprintf(fp, "-%d -%d 0\n",
					var_to_num(up_i[x], up_j[x], n),
					var_to_num(up_i[y], up_j[y], n));
				clauses++;
			}
		}
	}
	int down_i[9] = { 6,6,6,7,7,7,8,8,8 };
	int down_j[9] = { 6,7,8,6,7,8,6,7,8 };
	for (int n = 1; n <= 9; n++) {
		// 至少一个
		for (int k = 0; k < 9; k++) {
			fprintf(fp, "%d ", var_to_num(down_i[k], down_j[k], n));
		}
		fprintf(fp, "0\n");
		clauses++;
		// 至多一个
		for (int x = 0; x < 9; x++) {
			for (int y = x + 1; y < 9; y++) {
				fprintf(fp, "-%d -%d 0\n",
					var_to_num(down_i[x], down_j[x], n),
					var_to_num(down_i[y], down_j[y], n));
				clauses++;
			}
		}
	}

	//信息行写入
	fflush(fp);
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "p cnf %d %10d\n", VAR_NUM, clauses);
	fclose(fp);
	return true;
}

//调用dpll求解cnf_path下的百分号数独，var_count为总变元数，model用来存储cnf公式的解
bool xsudoku_solver(const char* cnf_path, int var_count, int* model)
{
	formula* F = parse_cnf(cnf_path);
	if (!F) return false;
	Assignment A;
	double time_ms;
	bool SAT = solve_dpll_v2(F, &A, &time_ms);
	if (!SAT) return false; //数独无解
	for (int i = 1; i <= var_count; i++) model[i] = A.values[i];
	return true;
}
//从model（1到var_count）中提取出百分号数独的解，写入grid_res（1到var_count）中
bool xsudoku_model_to_grid(const int* model, int var_count, char* grid_res)
{
	if (!model || !grid_res) return false;
	memset(grid_res, '.', SUDOKU_LENGTH);
	for (int v = 1; v <= var_count; v++) {
		if (model[v] > 0) {
			int idx = v - 1;
			int row = idx / 81 + 1, reminder = idx % 81;
			int col = reminder / 9 + 1, num = reminder % 9 + 1;
			grid_res[(row - 1) * 9 + (col - 1)] = '0' + num;
		}
	}
	grid_res[SUDOKU_LENGTH] = '\0';
	return true;
}
//从字符串grid_res中打印百分号数独
void xsudoku_print(const char* grid_res)
{
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			putchar(grid_res[i * 9 + j]);
			if (j < 8) putchar(' ');
		}
		putchar('\n');
	}
}

//从一个字符串xsudoku_str求解百分号数独
bool xsudoku_solve_from_string(const char* xsudoku_str)
{
	int model[VAR_NUM + 1];
	if (xsudoku_to_cnf(xsudoku_str, "xsudoku_cnf.cnf") == false) {
		printf("无法生成CNF文件\n");
		return false;
	}
	bool SAT = xsudoku_solver("xsudoku_cnf.cnf", VAR_NUM, model);
	if (!SAT) {
		printf("数独无结果\n");
		return false;
	}
	char grid_res[SUDOKU_LENGTH + 1];
	xsudoku_model_to_grid(model, VAR_NUM, grid_res);
	xsudoku_print(grid_res);
	return true;
}
//从文本文件中逐行读取多个百分号数独，批量求解并打印，返回成功求解的个数
int xsudoku_solve_from_file(const char* xsudoku_file)
{
	FILE* fp = fopen(xsudoku_file, "rb");
	if (!fp) return -1;
	char line[SUDOKU_LENGTH + 11]; //多留10个字节的缓冲区余量
	int count = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/') continue;
		if (strlen(line) < SUDOKU_LENGTH) continue;
		char xsudoku_str[SUDOKU_LENGTH + 1];
		memcpy(xsudoku_str, line, SUDOKU_LENGTH);
		xsudoku_str[SUDOKU_LENGTH] = '\0';
		if (xsudoku_solve_from_string(xsudoku_str) == true) count++;
		printf("\n");
	}
	fclose(fp);
	return count;
}

//普通九宫格合法性检查
static bool ok(int row, int col, int num)
{
	for (int k = 0; k < N; k++)
		if (full[row][k] == num || full[k][col] == num) return false;
	int br = (row / 3) * 3, bc = (col / 3) * 3;
	for (int i = br; i < br + 3; i++)
		for (int j = bc; j < bc + 3; j++)
			if (full[i][j] == num) return false;
	return true;
}
//百分号数独额外约束区域
static const int diag_i[9] = { 0,1,2,3,4,5,6,7,8 };
static const int diag_j[9] = { 8,7,6,5,4,3,2,1,0 };
static const int up_i[9] = { 1,1,1,2,2,2,3,3,3 };
static const int up_j[9] = { 1,2,3,1,2,3,1,2,3 };
static const int dn_i[9] = { 5,5,5,6,6,6,7,7,7 };
static const int dn_j[9] = { 5,6,7,5,6,7,5,6,7 };

//检查百分号数独合法性（含行/列/宫/撇对角/上下窗口）
static bool ok_percent(int r, int c, int num)
{
	if (!ok(r, c, num)) return false;
	//撇对角线
	for (int k = 0; k < 9; k++)
		if (diag_i[k] == r && diag_j[k] == c) {
			for (int x = 0; x < 9; x++)
				if ((diag_i[x] != r || diag_j[x] != c) && full[diag_i[x]][diag_j[x]] == num) return false;
		}
	//上方窗口
	for (int k = 0; k < 9; k++)
		if (up_i[k] == r && up_j[k] == c) {
			for (int x = 0; x < 9; x++)
				if ((up_i[x] != r || up_j[x] != c) && full[up_i[x]][up_j[x]] == num) return false;
		}
	//下方窗口
	for (int k = 0; k < 9; k++)
		if (dn_i[k] == r && dn_j[k] == c) {
			for (int x = 0; x < 9; x++)
				if ((dn_i[x] != r || dn_j[x] != c) && full[dn_i[x]][dn_j[x]] == num) return false;
		}
	return true;
}
//回溯填满百分号数独
static bool fill_percent(int idx)
{
	if (idx == N * N) return true;
	int r = idx / N, c = idx % N;
	int nums[N];
	for (int i = 0; i < N; i++) nums[i] = i + 1;
	for (int i = N - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int t = nums[i]; nums[i] = nums[j]; nums[j] = t;
	}
	for (int k = 0; k < N; k++) {
		if (ok_percent(r, c, nums[k])) {
			full[r][c] = nums[k];
			if (fill_percent(idx + 1)) return true;
		}
	}
	full[r][c] = 0;
	return false;
}

//挖洞辅助
static void dig_holes(int holes)
{
	while (holes > 0) {
		int i = rand() % N, j = rand() % N;
		if (full[i][j] != 0) {
			full[i][j] = 0;
			holes--;
		}
	}
}

//难度->挖洞数映射函数
static int holes_for_diff(int diff)
{
	switch (diff) {
	case 1: return 45;
	case 2: return 50;
	case 3: return 60;
	default: return 50;
	}
}

void generate_random_xsudoku(int difficulty, char* xsudoku_str)
{
	srand((unsigned)time(NULL) + 12345);
	memset(full, 0, sizeof full);
	fill_percent(0);
	dig_holes(holes_for_diff(difficulty));
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			xsudoku_str[i * 9 + j] = full[i][j] ? '0' + full[i][j] : '_';
	xsudoku_str[81] = '\0';
}