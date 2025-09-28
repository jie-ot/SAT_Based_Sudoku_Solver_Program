#define _CRT_SECURE_NO_WARNINGS
#include"sudoku.h"

//将三维i,j,n变量转换为一维连续的编号
static inline int var_to_num(int i, int j, int n)
{
	return (i - 1) * 81 + (j - 1) * 9 + n;
}
// 将一个81字符的数独转换成cnf并写入cnf_path，成功返回 true，否则返回 false
bool sudoku_to_cnf(const char* sudoku_str, const char* cnf_path)
{
	if (!sudoku_str || !cnf_path) return false;

	//校验输入长度与字符
	for (int k = 0; k < SUDOKU_LENGTH; ++k) {
		char c = sudoku_str[k];
		if (c == '\0') return false; // 输入长度不足
		else if (c >= '1' && c <= '9') {} //合法数字
		else if (c == '.' || c == '0' || c == '_') { /* 空格 */ }
		else return false; // 非法字符
	}

	FILE* fp = fopen(cnf_path, "wb");
	if (!fp) return false;

	// 先写占位 header（clauses 字段用固定宽度 10，以便之后安全覆盖）
	if (fprintf(fp, "p cnf %d %10d\n", VAR_NUM, 0) < 0) { fclose(fp); return false; }
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

	//九宫内不重复
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

	//sudoku_str中的已知线索
	for (int k = 0; k < SUDOKU_LENGTH; k++) {
		char c = sudoku_str[k];
		if (c >= '1' && c <= '9') {
			int i = k / 9 + 1, j = k % 9 + 1, n = c - '0';
			fprintf(fp, "%d 0\n", var_to_num(i, j, n));
			clauses++;
		}
	}

	//写出缓冲并回写header
	if (fflush(fp) != 0 || ferror(fp)) { fclose(fp); return false; }
	fseek(fp, 0, SEEK_SET);
	if (fprintf(fp, "p cnf %d %10d\n", VAR_NUM, clauses) < 0) { fclose(fp); return false; }

	fclose(fp);
	return true;
}

//调用dpll求解cnf_path下的数独，var_count为总变元数，model用来存储cnf公式的解
bool sudoku_solver(const char* cnf_path, int var_count, int* model)
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
//从model（1到var_count）中提取出数独的解，写入grid_res（1到var_count）中
bool sudoku_model_to_grid(const int* model, int var_count, char* grid_res)
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
//从字符串grid_res中打印数独
void sudoku_print(const char* grid_res)
{
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			putchar(grid_res[i * 9 + j]);
			if (j < 8) putchar(' ');
		}
		putchar('\n');
	}
}

//从一个字符串sudoku_str求解普通数独
bool sudoku_solve_from_string(const char* sudoku_str)
{
	int model[VAR_NUM + 1];
	if (sudoku_to_cnf(sudoku_str, "sudoku_cnf.cnf") == false) {
		printf("无法生成CNF文件\n");
		return false;
	}
	bool SAT = sudoku_solver("sudoku_cnf.cnf", VAR_NUM, model);
	if (!SAT) {
		printf("数独无结果\n\n");
		return false;
	}
	char grid_res[SUDOKU_LENGTH + 1];
	sudoku_model_to_grid(model, VAR_NUM, grid_res);
	sudoku_print(grid_res);
	return true;
}

//从文本文件中逐行读取多个普通数独，批量求解并打印，返回成功求解的个数
int sudoku_solve_from_file(const char* sudoku_file)
{
	FILE* fp = fopen(sudoku_file, "rb");
	if (!fp) return -1;
	char line[SUDOKU_LENGTH + 11]; //多留10个字节的缓冲区余量
	int count = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/') continue;
		if (strlen(line) < SUDOKU_LENGTH) continue;
		char sudoku_str[SUDOKU_LENGTH + 1];
		memcpy(sudoku_str, line, SUDOKU_LENGTH);
		sudoku_str[SUDOKU_LENGTH] = '\0';
		if (sudoku_solve_from_string(sudoku_str) == true) count++;
		printf("\n");
	}
	fclose(fp);
	return count;
}

//内部共享的全局数组，用于回溯构建数独解
static int full[N][N];
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

//填满普通数独
static bool fill_grid(int idx) 
{
	if (idx == N * N) return true;
	int r = idx / N, c = idx % N;
	int nums[N];
	for (int i = 0; i < N; i++) nums[i] = i + 1; //保存1-9的数字，用于后序随机排列

	//此处for循环更改nums中1-9的排序顺序，生成均匀随机排列，用于后续尝试填入的顺序
	for (int i = N - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int t = nums[i]; nums[i] = nums[j]; nums[j] = t;
	}
	for (int k = 0; k < N; k++) {
		if (ok(r, c, nums[k])) {  //如果符合数独的格式，就填入
			full[r][c] = nums[k];
			if (fill_grid(idx + 1)) return true;
		}
	}

	full[r][c] = 0; //回溯的关键！for循环结束依然没有找到合适的数字，表明上一层填入有问题，要回溯！
	return false;
}

//挖洞函数
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

//难度→挖洞数映射函数
static int holes_for_diff(int diff) 
{
	switch (diff) {
		case 1: return 45;
		case 2: return 50;
		case 3: return 60;
		default: return 50;
	}
}

void generate_random_sudoku(int difficulty, char* sudoku_str) 
{
	srand((unsigned)time(NULL));
	memset(full, 0, sizeof full);
	fill_grid(0);
	dig_holes(holes_for_diff(difficulty));
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			sudoku_str[i * 9 + j] = full[i][j] ? '0' + full[i][j] : '_';
	sudoku_str[81] = '\0';
}
