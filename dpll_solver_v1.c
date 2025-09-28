#define _CRT_SECURE_NO_WARNINGS
#include"dpll_solver_v1.h"

//内部函数声明
static formula* clone_formula(const formula* F);
static void assign_and_simplify(formula* F, Assignment* A, int lit); //为A[abs(lit)]赋值（1或者0），同时对子句进行化简，去掉含lit的子句，和文字-lit
static bool unit_simplify(formula* F, Assignment* A); //单子句规则，不断寻找单子句并assign_and_simplify，若发现空子句则返回false
static bool pure_literal_elimination(formula* F, Assignment* A); //纯文字规则，扫描所有子句看是否存在纯文字，存在则assign_and_simplify并返回true，否则返回false
static int select_literal(const formula* F); //分裂策略：选取最短子句的第一个文字
static bool dpll_recursive(formula* F, Assignment* A); //dpll递归主流程

static size_t conflict_count = 0; //冲突计数器
static const size_t LOG_INTERVAL = 10; //每遇到多少次冲突打印一次
static clock_t start_time; //记录算法开始的时钟

//调用dpll递归求解并计时
bool solve_dpll_v1(formula* F, Assignment* A, double* time_ms)
{
    A->values = malloc((F->var_num + 1) * sizeof(int));
    for (int i = 1; i <= F->var_num; i++) A->values[i] = -1;

    start_time = clock();
    bool SAT = dpll_recursive(F, A);
    clock_t t1 = clock();
    *time_ms = 1000.0 * (t1 - start_time) / CLOCKS_PER_SEC;
    return SAT;
}

//释放链表内存
void free_formula(formula* F)
{
    clause* c = F->clauses;
    while (c) {
        clause* cn = c->next;
        //释放子句内文字链表
        literal* l = c->lists;
        while (l) {
            literal* ln = l->next;
            free(l);
            l = ln;
        }
        free(c);
        c = cn;
    }
    free(F);
}

//拷贝cnf公式F
static formula* clone_formula(const formula* F)
{
    formula* C = malloc(sizeof(formula));
    C->var_num = F->var_num;
    C->clause_num = F->clause_num;
    C->clauses = NULL;

    clause** tail = &C->clauses;
    //复制每个子句节点
    for (clause* c = F->clauses; c; c = c->next) {
        clause* nc = malloc(sizeof(clause));
        nc->lists = NULL;
        nc->next = NULL;
        *tail = nc;
        tail = &nc->next;

        //复制文字链
        literal** lp = &nc->lists;
        for (literal* L = c->lists; L; L = L->next) {
            literal* nL = malloc(sizeof(literal));
            nL->var = L->var;
            nL->next = NULL;
            *lp = nL;
            lp = &nL->next;
        }
    }
    return C;
}

//为A[abs(lit)]赋值（1或者0），同时对子句进行化简，去掉含lit的子句，和文字-lit
static void assign_and_simplify(formula* F, Assignment* A, int lit)
{
    int v = abs(lit);
    A->values[v] = (lit > 0) ? 1 : 0; //单子句lit为真，若lit是正数则lit表示true，反之若lit为负数则lit表示false

    clause** pc = &F->clauses;
    while (*pc) {
        clause* c = *pc;
        //若子句包含lit，则删除子句
        bool has_lit = false;
        for (literal* L = c->lists; L; L = L->next) {
            if (L->var == lit) {
                has_lit = true;
                break;
            }
        }
        if (has_lit) {
            *pc = c->next; //将*pc所指的内容改成c->next指向的内容，完成了子句的删除。用二级指针的好处是不用分是否为头结点讨论。
            //删除c所指的子句内存
            literal* L = c->lists;
            while (L) {
                literal* ln = L->next;
                free(L);
                L = ln;
            }
            free(c);
            continue; //返回while(*pc)那部，继续找下一个有lit的子句
        }
        //否则删去-lit
        literal** lp = &c->lists;
        while (*lp) {
            if ((*lp)->var == -lit) {
                literal* tmp = *lp;
                *lp = tmp->next; //和上面一句同理，保证删除后前后文字正确相连
                free(tmp);
                break;
            }
            lp = &(*lp)->next;
        }
        pc = &(*pc)->next;
    }
}

//单子句规则，不断寻找单子句并assign_and_simplify，若发现空子句则返回false
static bool unit_simplify(formula* F, Assignment* A)
{
    bool finded;
    do {
        finded = false;
        for (clause* c = F->clauses; c; c = c->next) {
            //若c为单子句，则assign_and_simplify
            if (c->lists != NULL && c->lists->next == NULL) {
                assign_and_simplify(F, A, c->lists->var);
                finded = true;
                break;
            }
        }
        //检查空子句
        for (clause* c = F->clauses; c; c = c->next) {
            if (c->lists == NULL) return false;
        }
    } while (finded);
    return true;
}

//纯文字规则，扫描所有子句看是否存在纯文字，存在则assign_and_simplify并返回true，否则返回false
static bool pure_literal_elimination(formula* F, Assignment* A)
{
    //统计子句每个文字的真假，若v为负则sign[v]=-1；若v为正则sign[v]=1；若出现有正有负，则sign[v]=2
    int* sign = (int*)calloc(F->var_num + 1,sizeof(int)); //用calloc初始化为0，这样即使某个文字不出现，也会默认为不符合要求

    for (clause* c = F->clauses; c; c = c->next) {
        for (literal* l = c->lists; l; l = l->next) {
            int v = abs(l->var);
            int p_or_n = l->var > 0 ? 1 : -1;
            if (sign[v] == 0) sign[v] = p_or_n;
            else if (sign[v] == 2) continue;
            else if (sign[v] != p_or_n) sign[v] = 2;
        }
    }
    
    for (int v = 1; v <= F->var_num; v++) {
        if (sign[v] == -1 || sign[v] == 1) {
            assign_and_simplify(F, A, sign[v] * v); //第三个参数的传递一定要注意！！！
            free(sign);
            return true;
        }
    }
    free(sign);
    return false;
}

//分裂策略：选取最短子句的第一个文字
static int select_literal(const formula* F)
{
    int min_len = INT_MAX, pick = 0;
    for (clause* c = F->clauses; c; c = c->next) {
        int len = 0;
        for (literal* L = c->lists; L; L = L->next) len++;
        if (len > 0 && len < min_len) {
            min_len = len;
            pick = c->lists->var;
        }
    }
    return pick;
}

//dpll递归主流程
static bool dpll_recursive(formula* F, Assignment* A)
{
    //单子句规则
    if (!unit_simplify(F, A)) return false;
    //纯文字规则+单子句规则
    while (pure_literal_elimination(F, A)) {
        if (!unit_simplify(F, A)) return false;
    }
    //检查结果
    if (F->clauses == NULL) return true;
    for (clause* c = F->clauses; c; c = c->next) {
        if (c->lists == NULL) return false;
    }

    /************************************************************************************************
    *      分裂，假设lit为真和假设lit为假都要走，每个进行递归，递归的同时将结果（A1或者A2）向上拷贝到A，
    *  若两个有一个能递归到全部子句成立，则返回true；反之若两个都无法递归成立，则返回false。
    *************************************************************************************************/
    int lit = select_literal(F);
    //假设lit为真，则可以看作存在单子句lit，其为真
    {
        formula* F1 = clone_formula(F);
        Assignment A1;
        A1.values = (int*)malloc((F1->var_num + 1) * sizeof(int));
        memcpy(A1.values, A->values, (F1->var_num + 1) * sizeof(int));
        assign_and_simplify(F1, &A1, lit);
        if (dpll_recursive(F1, &A1)) {
            memcpy(A->values, A1.values, sizeof(int) * (F1->var_num + 1));
            free(A1.values);
            free_formula(F1);
            return true;
        }
        free(A1.values);
        free_formula(F1);
    }

    //假设lit为假，则可以看作存在单子句非lit，其为真
    {
        formula* F2 = clone_formula(F);
        Assignment A2;
        A2.values = (int*)malloc((F2->var_num + 1) * sizeof(int));
        memcpy(A2.values, A->values, (F2->var_num + 1) * sizeof(int));
        assign_and_simplify(F2, &A2, -lit);
        if (dpll_recursive(F2, &A2)) {
            memcpy(A->values, A2.values, sizeof(int) * (F2->var_num + 1));
            free(A2.values);
            free_formula(F2);
            return true;
        }
        free(A2.values);
        free_formula(F2);
    }
    return false;
}

/*
* 原算法时间复杂度分析:
*     深拷贝公式，每做一次分裂，都要用clone_formula复制所有子句和文字，复杂度约为O(m·k)，其中m是子句数，k是平均文字数。
*     单子句传播：每次都要扫描整个子句链表寻找单子句，最坏O(m·k)。
*     纯文字消除：同样扫描所有子句和文字，最坏 O(m·k)。
*     递归分裂：在最坏情况下会产生2ⁿ个分支（n 为变量数），每个分支又做上述 O(m·k) 的拷贝与扫描。
* 综上，原算法的时间复杂度可粗略估计为：
*     T(n) ≈ 2·T(n−1) + O(m·k·n) ⇒ O(2ⁿ · m · k · n)
*/