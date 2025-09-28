#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "dpll_solver_v1.h"

bool solve_dpll_v2(const formula* F, Assignment* A, double* time_ms);