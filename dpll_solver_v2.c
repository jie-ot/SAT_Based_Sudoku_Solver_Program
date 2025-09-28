#define _CRT_SECURE_NO_WARNINGS
#include "dpll_solver_v2.h"

//Clause结构体：表示一个子句及其双重监视字面
typedef struct Clause {
    int len;                //子句中包含的文字数量
    int* lits;              //文字数组
    int watch[2];           //当前监视的两个文字
    struct Clause* next0;   //对应watch[0]在watch_list中的下一个子句
    struct Clause* next1;   //对应watch[1]在watch_list中的下一个子句
} Clause;

//记录回溯栈中操作类型：赋值或移动监视字面
typedef enum { 
    OP_ASSIGN,              //对变量赋值操作
    OP_MOVE_WATCH           //对子句监视字面移动操作
} OpType;

//Change结构体：用于记录单步操作，便于撤销
typedef struct {
    OpType type;            //操作类型
    /*对于OP_ASSIGN*/
    int var;                //被赋值的变量编号
    int old_val;            //赋值前的旧值
    /*对于OP_MOVE_WATCH*/
    Clause* clause;         //受影响的子句指针
    int watch_idx;          //哪一路监视字面（0或1）
    int old_lit;            //原监视文字
    int new_lit;            //新监视文字
} Change;

//全局求解器状态
static int var_num, clause_num;
static Clause* clauses;      //存放所有子句Clause的数组
static Clause** watchlist;   //watchlist是大小为2*var_num的数组，每个元素是子句链表的头指针。watchlist[idx]是所有子句中监视字面转换成下标是idx的子句组成的链表
static int* values;          //变量赋值（-1 未赋值；0 false；1 true）
static double* jw_score;     //jw分裂启发式数组
static Change* trail;        //操作记录栈，用于回溯
static int trail_top;        //trail栈顶层索引
static int* trail_lim;       //每次分裂失败回溯时，回溯层数trail_top的限制，用于层级撤销

static size_t conflict_count = 0; //全局冲突计数器 
static clock_t start_time; //记录算法开始运行的时钟
static double next_print_time = 10.0; //打印间隔为10.0s

static inline void log_printer(void)
{
    clock_t now = clock();
    double used_time = (double)(now - start_time) / CLOCKS_PER_SEC;
    if (used_time >= next_print_time) {
        printf("conflicts=%zu, used time=%.0f s\n", conflict_count, used_time);
        next_print_time += 10.0;
    }
}

//文字和索引（文字数组中）的相互转换
static inline int lit_idx(int lit) 
{
    return (lit > 0 ? lit - 1 : var_num + (-lit - 1));
}
static inline int idx_lit(int idx) 
{
    return idx < var_num ? idx + 1 : -(idx - var_num + 1);
}

/*记录操作以便回溯*/

//记录一次变量赋值操作
static inline void record_assign(int var, int old_val) 
{
    trail[trail_top].type = OP_ASSIGN;
    trail[trail_top].var = var;
    trail[trail_top++].old_val = old_val;
}
//记录一次对子句监视字面的移动   
static inline void record_move_watch(Clause* c, int w, int o, int n) 
{
    trail[trail_top].type = OP_MOVE_WATCH;
    trail[trail_top].clause = c;
    trail[trail_top].watch_idx = w;
    trail[trail_top].old_lit = o;
    trail[trail_top++].new_lit = n;
}

/*watchlist监视列表维护*/

//从监视列表watchlist[lit_idx(lit)]中删除子句c
static void remove_watch(int lit, Clause* c, int widx) 
{
    int idx = lit_idx(lit);
    Clause** p = &watchlist[idx];
    while (*p && *p != c) {
        if ((*p)->watch[0] == lit) p = &(*p)->next0;
        else p = &(*p)->next1;
    }
    if (*p == c) {
        if (c->watch[0] == lit) *p = c->next0;  //将p所指的内容替换为c->next0，即删除c
        else *p = c->next1;
    }
}
//向watchlist[lit_idx(lit)]插入子句c，widx表示0/1监视字面
static void add_watch(int lit, Clause* c, int widx) 
{
    int idx = lit_idx(lit);
    if (widx == 0) {
        c->next0 = watchlist[idx];
        watchlist[idx] = c; //将c插入到watchlist的第一个
    }
    else {
        c->next1 = watchlist[idx];
        watchlist[idx] = c;
    }
}
//改变子句c中的第widx路监视，由old_lit变成new_lit
static void move_watch(Clause* c, int widx, int new_lit)
{
    int old_lit = c->watch[widx];
    record_move_watch(c, widx, old_lit, new_lit); //记录监视字面移动操作
    remove_watch(old_lit, c, widx);
    c->watch[widx] = new_lit;
    add_watch(new_lit, c, widx);
}

/*赋值传播与冲突检测*/
static bool propagate(int lit, int level);
//给lit赋为真，记录赋值操作，然后单元传播和检测冲突
static bool assign_lit(int lit, int level) 
{
    int v = abs(lit), old = values[v];
    record_assign(v, old);
    values[v] = (lit > 0);
    return propagate(lit, level);
}
//在决策层level下，给定文字lit被赋真后，在watchlist[-lit]链表中触发单元传播或检测冲突
static bool propagate(int lit, int level) 
{
    log_printer(); //运行日志打印

    int opp = lit_idx(-lit);
    Clause* c = watchlist[opp];
    while (c) {
        Clause* next = (c->watch[0] == -lit ? c->next0 : c->next1); //记住下次要访问的子句指针
        int widx = (c->watch[0] == -lit ? 0 : 1);
        int oth = c->watch[(widx == 1 ? 0 : 1)];  //另一路监视文字
        int ov = values[abs(oth)];  //另一路监视文字的赋值

        //如果另一路监视文字为真，那么子句已满足条件，跳过
        if ((oth > 0 && ov == 1) || (oth < 0 && ov == 0)) {
            c = next;
            continue;
        }
        
        //在子句中寻找新的监视文字
        bool found = false;
        for (int i = 0; i < c->len; i++) {
            int lit2 = c->lits[i];
            if (lit2 == oth) continue;
            int vv = values[abs(lit2)];
            //未赋值或已满足的文字都可以
            if (vv == -1 || (lit2 > 0 && vv == 1) || (lit2 < 0 && vv == 0)) {
                move_watch(c, widx, lit2);
                found = true;
                break;
            }
        }
        //若没找到新的监视文字，表明是单子句或有冲突
        if (!found) {
            //另一路监视字面未赋值，说明是单子句，进行单元传播，把oth赋为真
            if (values[abs(oth)] == -1) {
                if (!assign_lit(oth, level)) return false;
            }
            else {
                //冲突检测
                conflict_count++;
                return false;
            }
        }
        c = next;
    }
    return true;
}

/*回溯*/
//根据trail数组的记录，撤销到level层
static void undo(int level) 
{
    while (trail_top > trail_lim[level]) {
        Change ch = trail[--trail_top];
        if (ch.type == OP_ASSIGN) {
            //若是赋值操作，则恢复旧值
            values[ch.var] = ch.old_val;
        }
        else {
            //若是监视字面移动，则恢复监视字面
            Clause* c = ch.clause;
            remove_watch(ch.new_lit, c, ch.watch_idx);
            c->watch[ch.watch_idx] = ch.old_lit;
            add_watch(ch.old_lit, c, ch.watch_idx);
        }
    }
}

/*启发式分裂*/
//计算正文字和负文字分数之和，选择最高的文字进行分裂
static int select_literal(void) 
{
    double best = -1.0;
    int pick = 0;
    for (int v = 1; v <= var_num; v++) {
        if (values[v] == -1) {
            int ip = lit_idx(v), in = lit_idx(-v);
            double s = jw_score[ip] + jw_score[in];
            if (s > best) {
                best = s;
                pick = (jw_score[ip] >= jw_score[in] ? v : -v);
            }
        }
    }
    return pick;
}

/*递归dpll搜索*/
static bool dpll_search(int level) 
{
    //检查是否都已赋值
    bool all_assigned = true;
    for (int v = 1; v <= var_num; v++) {
        if (values[v] == -1) { all_assigned = false; break; }
    }
    if (all_assigned) return true;

    int lit = select_literal();
    trail_lim[level] = trail_top;
    if (assign_lit(lit, level) && dpll_search(level + 1)) return true;
    undo(level);

    trail_lim[level] = trail_top;
    if (assign_lit(-lit, level) && dpll_search(level + 1)) return true;
    undo(level);

    return false;
}

bool solve_dpll_v2(const formula* F, Assignment* A, double* time_ms)
{
    start_time = clock();
    next_print_time = 10.0;
    var_num = F->var_num;
    clause_num = F->clause_num;

    //统计总文字数，以估计trail大小
    int sum_lits = 0;
    for (clause* c = F->clauses; c; c = c->next) {
        for (literal* l = c->lists; l; l = l->next) sum_lits++;
    }

    int** in_clauses = malloc(clause_num * sizeof(int*));
    int* clause_lens = malloc(clause_num * sizeof(int));
    jw_score = calloc(2 * var_num, sizeof(double));

    //复制子句文字
    int ci = 0;
    for (clause* c = F->clauses; c; c = c->next, ci++) {
        int len = 0;
        for (literal* l = c->lists; l; l = l->next) len++;
        clause_lens[ci] = len;
        int* arr = malloc(len * sizeof(int));
        int j = 0;
        for (literal* l = c->lists; l; l = l->next) arr[j++] = l->var;
        in_clauses[ci] = arr;
        double w = pow(2.0, -len);
        for (j = 0; j < len; j++) jw_score[lit_idx(arr[j])] += w;
    }

    //构建Clause和监视链表
    clauses = malloc(clause_num * sizeof(Clause));
    watchlist = calloc(2 * var_num, sizeof(Clause*));
    for (int i = 0; i < clause_num; i++) {
        Clause* c = &clauses[i];
        int len = clause_lens[i];
        c->len = len;
        c->lits = in_clauses[i];
        //统一规定初始监视前两个文字
        c->watch[0] = c->lits[0];
        c->watch[1] = (len > 1 ? c->lits[1] : c->lits[0]);
        c->next0 = c->next1 = NULL;
        add_watch(c->watch[0], c, 0);
        add_watch(c->watch[1], c, 1);
    }

    //初始化trail和层级限制
    values = malloc((var_num + 1) * sizeof(int));
    trail = malloc((sum_lits * 2 + var_num + 5) * sizeof(Change));
    trail_lim = malloc((var_num + 2) * sizeof(int));
    for (int i = 1; i <= var_num; i++) values[i] = -1;
    trail_top = 0;
    trail_lim[0] = 0;

    bool sat = dpll_search(1);

    //将结果保存在数组A->value中
    A->values = malloc((var_num + 1) * sizeof(int));
    for (int i = 1; i <= var_num; i++) A->values[i] = values[i];

    clock_t t1 = clock();
    *time_ms = 1000.0 * (t1 - start_time) / CLOCKS_PER_SEC;

    for (int i = 0; i < clause_num; i++) free(clauses[i].lits);
    free(in_clauses);
    free(clause_lens);
    free(clauses);
    free(watchlist);
    free(jw_score);
    free(values);
    free(trail);
    free(trail_lim);

    return sat;
}
