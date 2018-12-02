/*
 * SMT Computation of IO Lower Bounds
 * Corentin Ferry - 2018
 *
*/

#include <z3++.h>
#include <vector>

#include "datastruct.h"
#include "sat-version.h"

using namespace z3;

int main(int argc, char* argv[])
{

	if(argc < 3) {
		std::cout << "Usage: " << argv[0] << " [io_budget] [nb_registers]" << std::endl;
		std::cout << "See main.cpp to change the DAG" << std::endl;
		exit(1);
	}

	context ctx;
	set_param("parallel.enable", true);

	/*
	 * These examples come straight from last year's internship.
	 */

#define __TEST_FFT4

#ifdef __TEST_SAMPLE14
#define MAX_DEPS 2
#define NB_NODES 14
#define TEST_NAME "sample14"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},   // 1   S
        {0, 0},   // 2   A
        {0, 0},   // 3   B1
        {0, 0},   // 4   B2
        {0, 0},   // 5   B3
        {0, 0},   // 6   B4
        {2, 3},   // 7   AB1
        {2, 4},   // 8   AB2
        {2, 5},   // 9   AB3
        {2, 6},   // 10  AB4
        {1, 7},   // 11  S + AB1
        {8, 11},  // 12  S + AB1 + AB2
        {9, 12},  // 13  S + AB1 + AB2 + AB3
        {10, 13}  // 14  S + AB1 + AB2 + AB3 + AB4
    };
#endif

#ifdef __TEST_SAMPLE3
#define MAX_DEPS 2
#define NB_NODES 3
#define TEST_NAME "sample3"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},   // 1   A
        {0, 0},   // 2   B
        {1, 2},   // 3   A + B
    };
#endif

#ifdef __TEST_SAMPLE5
#define MAX_DEPS 2
#define NB_NODES 5
#define TEST_NAME "sample5"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},   // 1   A
        {0, 0},   // 2   B
        {1, 2},   // 3   A + B
        {3, 2},   // 4   A + 2B
        {1, 3}    // 5   2A + B
    };
#endif

#ifdef __TEST_SAMPLE6
#define MAX_DEPS 2
#define NB_NODES 6
#define TEST_NAME "sample6"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},   // 1   A
        {0, 0},   // 2   B
        {1, 2},   // 3   A + B
        {3, 2},   // 4   A + 2B
        {1, 3},   // 5   2A + B
        {4, 5}    // 6   3A + 3B
    };
#endif


#ifdef __TEST_SAMPLE11
#define MAX_DEPS 2
#define NB_NODES 11
#define TEST_NAME "sample11"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},   // 1   S
        {0, 0},   // 2   A
        {0, 0},   // 3   B1
        {0, 0},   // 4   B2
        {0, 0},   // 5   B3
        {2, 3},   // 6   AB1
        {2, 4},   // 7   AB2
        {2, 5},   // 8   AB3
        {1, 6},   // 9   S + AB1
        {9, 7},   // 10  S + AB1 + AB2
        {8, 10}   // 11  S + AB1 + AB2 + AB3
    };

#endif

#ifdef __TEST_FFT8
    // FFT-8
#define MAX_DEPS 2
#define NB_NODES 32
#define TEST_NAME "fft8"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        { 0, 0}, // i1
        { 0, 0}, // i2
        { 0, 0}, // i3
        { 0, 0}, // i4
        { 0, 0}, // i5
        { 0, 0}, // i6
        { 0, 0}, // i7
        { 0, 0}, // i8

        { 1, 2}, // 09
        { 1, 2}, // 10
        { 3, 4}, // 11
        { 3, 4}, // 12
        { 5, 6}, // 13
        { 5, 6}, // 14
        { 7, 8}, // 15
        { 7, 8}, // 16

        {9, 11}, // 17
        {10,12}, // 18
        {9, 11}, // 19
        {10,12}, // 20
        {13,15}, // 21
        {14,16}, // 22
        {13,15}, // 23
        {14,16}, // 24

        {17,21}, // 25
        {18,22}, // 26
        {19,23}, // 27
        {20,24}, // 28
        {21,17}, // 29
        {22,18}, // 30
        {23,19}, // 31
        {24,20}  // 32
    };
#endif

#ifdef __TEST_FFT4
    // FFT-4
#define MAX_DEPS 2
#define NB_NODES 12
#define TEST_NAME "fft4"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},

        {1, 2},
        {1, 2},
        {3, 4},
        {3, 4},

        {5, 7},
        {6, 8},
        {5, 7},
        {6, 8}

    };

#endif

#ifdef __TEST_JACOBI2D
    // Jacobi-2D
#define MAX_DEPS 7
#define NB_NODES 20
#define TEST_NAME "jacobi2d"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0, 0, 0, 0, 0, 0, 0},  // c1
        {0, 0, 0, 0, 0, 0, 0},  // c2


        {0, 0, 0, 0, 0, 0, 0},  // A[0, 0] = 3
        {0, 0, 0, 0, 0, 0, 0},  // A[0, 1] = 4
        {0, 0, 0, 0, 0, 0, 0},  // A[0, 2] = 5
        {0, 0, 0, 0, 0, 0, 0},  // A[1, 0] = 6
        {0, 0, 0, 0, 0, 0, 0},  // A[1, 1] = 7
        {0, 0, 0, 0, 0, 0, 0},  // A[0, 0] = 8
        {0, 0, 0, 0, 0, 0, 0},  // A[2, 0] = 9
        {0, 0, 0, 0, 0, 0, 0},  // A[2, 1] = 10
        {0, 0, 0, 0, 0, 0, 0},  // A[2, 2] = 11

        {1, 2, 3, 4, 6, 0, 0},  // B[0, 0]
        {1, 2, 3, 4, 5, 7, 0},  // B[0, 1]
        {1, 2, 4, 5, 8, 0, 0},  // B[0, 2]
        {1, 2, 3, 6, 7, 9, 0},  // B[1, 0]
        {1, 2, 4, 6, 7, 8, 10},  // B[1, 1]
        {1, 2, 5, 7, 8, 11, 0},  // B[1, 2]
        {1, 2, 6, 8, 9, 0, 0},  // B[2, 0]
        {1, 2, 7, 9, 10, 11, 0},  // B[2, 1]
        {1, 2, 8, 10, 11, 0, 0}  // B[2, 2]

    };
#endif

#ifdef __TEST_MATMULT4
    // Matmult
#define MAX_DEPS 2
#define NB_NODES 176
#define TEST_NAME "matmult4"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {1,17},
        {33,34},
        {2,21},
        {35,36},
        {3,25},
        {37,38},
        {4,29},
        {39,40},
        {0,0},
        {1,18},
        {42,43},
        {2,22},
        {44,45},
        {3,26},
        {46,47},
        {4,30},
        {48,49},
        {0,0},
        {1,19},
        {51,52},
        {2,23},
        {53,54},
        {3,27},
        {55,56},
        {4,31},
        {57,58},
        {0,0},
        {1,20},
        {60,61},
        {2,24},
        {62,63},
        {3,28},
        {64,65},
        {4,32},
        {66,67},
        {0,0},
        {5,17},
        {69,70},
        {6,21},
        {71,72},
        {7,25},
        {73,74},
        {8,29},
        {75,76},
        {0,0},
        {5,18},
        {78,79},
        {6,22},
        {80,81},
        {7,26},
        {82,83},
        {8,30},
        {84,85},
        {0,0},
        {5,19},
        {87,88},
        {6,23},
        {89,90},
        {7,27},
        {91,92},
        {8,31},
        {93,94},
        {0,0},
        {5,20},
        {96,97},
        {6,24},
        {98,99},
        {7,28},
        {100,101},
        {8,32},
        {102,103},
        {0,0},
        {9,17},
        {105,106},
        {10,21},
        {107,108},
        {11,25},
        {109,110},
        {12,29},
        {111,112},
        {0,0},
        {9,18},
        {114,115},
        {10,22},
        {116,117},
        {11,26},
        {118,119},
        {12,30},
        {120,121},
        {0,0},
        {9,19},
        {123,124},
        {10,23},
        {125,126},
        {11,27},
        {127,128},
        {12,31},
        {129,130},
        {0,0},
        {9,20},
        {132,133},
        {10,24},
        {134,135},
        {11,28},
        {136,137},
        {12,32},
        {138,139},
        {0,0},
        {13,17},
        {141,142},
        {14,21},
        {143,144},
        {15,25},
        {145,146},
        {16,29},
        {147,148},
        {0,0},
        {13,18},
        {150,151},
        {14,22},
        {152,153},
        {15,26},
        {154,155},
        {16,30},
        {156,157},
        {0,0},
        {13,19},
        {159,160},
        {14,23},
        {161,162},
        {15,27},
        {163,164},
        {16,31},
        {165,166},
        {0,0},
        {13,20},
        {168,169},
        {14,24},
        {170,171},
        {15,28},
        {172,173},
        {16,32},
        {174,175}
    };
#endif

#ifdef __TEST_MATMULT3
#define NB_NODES 81
#define MAX_DEPS 2
#define TEST_NAME "matmult3"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {1,10},
        {19,20},
        {2,13},
        {21,22},
        {3,16},
        {23,24},
        {0,0},
        {1,11},
        {26,27},
        {2,14},
        {28,29},
        {3,17},
        {30,31},
        {0,0},
        {1,12},
        {33,34},
        {2,15},
        {35,36},
        {3,18},
        {37,38},
        {0,0},
        {4,10},
        {40,41},
        {5,13},
        {42,43},
        {6,16},
        {44,45},
        {0,0},
        {4,11},
        {47,48},
        {5,14},
        {49,50},
        {6,17},
        {51,52},
        {0,0},
        {4,12},
        {54,55},
        {5,15},
        {56,57},
        {6,18},
        {58,59},
        {0,0},
        {7,10},
        {61,62},
        {8,13},
        {63,64},
        {9,16},
        {65,66},
        {0,0},
        {7,11},
        {68,69},
        {8,14},
        {70,71},
        {9,17},
        {72,73},
        {0,0},
        {7,12},
        {75,76},
        {8,15},
        {77,78},
        {9,18},
        {79,80}
    };

#endif

#ifdef __TEST_MATMULT2
#define NB_NODES 28
#define MAX_DEPS 2
#define TEST_NAME "matmult2"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {1,5},
        {9,10},
        {2,7},
        {11,12},
        {0,0},
        {1,6},
        {14,15},
        {2,8},
        {16,17},
        {0,0},
        {3,5},
        {19,20},
        {4,7},
        {21,22},
        {0,0},
        {3,6},
        {24,25},
        {4,8},
        {26,27}
    };

#endif

#ifdef __TEST_MATMULT2_SIMPLE
#define NB_NODES 24
#define MAX_DEPS 2
#define TEST_NAME "matmult2_simple"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {1,5},
        {0,9},
        {2,7},
        {10,11},
        {1,6},
        {0,13},
        {2,8},
        {14,15},
        {3,5},
        {0,17},
        {4,7},
        {18,19},
        {3,6},
        {0,21},
        {4,8},
        {22,23}
    };

#endif

#ifdef __TEST_MATMULT3_SIMPLE
#define NB_NODES 72
#define MAX_DEPS 2
#define TEST_NAME "matmult3_simple"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {1,10},
        {0,19},
        {2,13},
        {20,21},
        {3,16},
        {22,23},
        {1,11},
        {0,25},
        {2,14},
        {26,27},
        {3,17},
        {28,29},
        {1,12},
        {0,31},
        {2,15},
        {32,33},
        {3,18},
        {34,35},
        {4,10},
        {0,37},
        {5,13},
        {38,39},
        {6,16},
        {40,41},
        {4,11},
        {0,43},
        {5,14},
        {44,45},
        {6,17},
        {46,47},
        {4,12},
        {0,49},
        {5,15},
        {50,51},
        {6,18},
        {52,53},
        {7,10},
        {0,55},
        {8,13},
        {56,57},
        {9,16},
        {58,59},
        {7,11},
        {0,61},
        {8,14},
        {62,63},
        {9,17},
        {64,65},
        {7,12},
        {0,67},
        {8,15},
        {68,69},
        {9,18},
        {70,71}
    };

#endif

#ifdef __TEST_JACOBI2D_EXT
#define NB_NODES 72
#define MAX_DEPS 2
#define TEST_NAME "jacobi2d-ext"
    uint32_t deps[NB_NODES][MAX_DEPS] = {
        {0,   0},  // c1
        {0,   0},  // c2
        {0,   0},  // A[0, 0] = 3
        {0,   0},  // A[0, 1] = 4
        {0,   0},  // A[0, 2] = 5
        {0,   0},  // A[1, 0] = 6
        {0,   0},  // A[1, 1] = 7
        {0,   0},  // A[1, 2] = 8
        {0,   0},  // A[2, 0] = 9
        {0,   0},  // A[2, 1] = 10
        {0,   0},  // A[2, 2] = 11
        {1,   4},  // c1 * A[0,1]
        {1,   6},  // c1 * A[1,0]
        {12, 13},  // c1(A[0,1] + A[1, 0])
        {2,   3},  // c2 * A[0,0]
        {14, 15},  // B[0, 0]
        {1,   3},
        {1,   5},
        {17, 18},
        {1,   7},
        {19, 20},
        {2,   4},
        {21, 22}, // B[0,1]
        {1,   4},
        {1,   8},
        {24, 25},
        {2,   5},
        {26, 27}, // B[0, 2]
        {1,   3},
        {1,   7},
        {29, 30},
        {1,   9},
        {31, 32},
        {2,   6},
        {33, 34}, // B[1, 0]
        {1,   4},
        {1,   6},
        {36, 37},
        {1,   8},
        {38, 39},
        {1,  10},
        {40, 41},
        {2,   7},
        {42, 43}, // B[1, 1]
        {1,   5},
        {1,   7},
        {45, 46},
        {1,  11},
        {47, 48},
        {2,   8},
        {49, 50}, // B[1, 2]
        {1,   2},
        {1,   6},
        {52, 53},
        {1,   8},
        {54, 55},
        {1,   9},
        {56, 57},
        {2,   9},
        {58, 59}, // B[2, 0]
        {1,   7},
        {1,   9},
        {61, 62},
        {1,  11},
        {63, 64},
        {2,  10},
        {65, 66}, // B[2, 1]
        {1,   8},
        {1,  10},
        {68, 69},
        {2,  11},
        {70, 71} // B[2, 2]
    };
#endif

    std::cout << "# This is SMT-LB-IO for " << TEST_NAME << std::endl;

    std::cout << "# Creating DAG from matrix representation" << std::endl;
    node* dagNodes = matrixToNodes(deps, (uint32_t)NB_NODES);
    dag* programDag = createDAGStructure(dagNodes, NB_NODES);

    std::cout << "# Creating constraints from the DAG" << std::endl;
    expr_vector constraints(ctx);

    symbol_list symbols;


    uint32_t budget = (uint32_t)atoi(argv[1]); // Maximum I/O budget - deadline
    uint32_t nbRedPebbles = (uint32_t)atoi(argv[2]); // Number of registers

    dagToConstraints(programDag, nbRedPebbles, budget, ctx, constraints, symbols);


    std::cout << "# Built constraints - Simplifying expression" << std::endl;

	expr to_solve = mk_and(constraints);
	expr simpl_to_solve = to_solve.simplify();

	std::cout << "# Solving the problem" << std::endl;

	solver s(ctx);

	s.add(simpl_to_solve);
	//std::cout << s << "\n";
	//std::cout << s.to_smt2() << "\n";
	try {
		check_result solve_result = s.check();
		std::cout << "# Result: ";
		if(solve_result == sat) {
			std::cout << "There is a valid schedule" << std::endl;
			model result = s.get_model();

			symbol_list scheduleSymbols;

			for(uint32_t i = 0; i < result.num_consts(); ++i) {
				func_decl decl = result.get_const_decl(i);
				expr body = result.get_const_interp(decl);
				if(body.is_bool() && body.bool_value() == true) {
					registered_symbol sym = lookupRegisteredSymbol(decl.name().str(), symbols);
					scheduleSymbols.push_back(sym);
				}
			}

			// print by schedule
			for(uint32_t t = 0; t < budget; t++) {
				symbol_list::iterator i;
				for(i = scheduleSymbols.begin(); i < scheduleSymbols.end(); ++i)
					if(i->date == t)
						std::cout << i->symbol.to_string() << " ";
			}
			std::cout << std::endl;

			std::cout << "# Checking for the schedule's validity" << std::endl;
			// Check for schedule validity
			uint32_t t, nR1, nR2;
			node** regs = (node**)calloc(nbRedPebbles, sizeof(node*));
			for(t = 0; t < budget; t++) {
				symbol_list::iterator i;
				for(i = scheduleSymbols.begin(); (i < scheduleSymbols.end()) && (i->date != t); ++i);
				if(i == scheduleSymbols.end())
					break; // we are done
				if(i->date == t) {
					if(i->r == RULE_R1) {
						uint32_t j;
						for(j = 0; (j < nbRedPebbles) && (regs[j] != NULL); ++j);
						if(j == nbRedPebbles) {
							std::cout << "INVALID: Register file full on load" << std::endl;
							break;
						}
						if(i->n->deleted) {
							std::cout << "INVALID: Value deleted on load" << std::endl;
							break;
						}
						regs[j] = i->n;
						nR1 += 1;

					} else if(i->r == RULE_R2) {
						uint32_t j;
						for(j = 0; (j < nbRedPebbles) && (regs[j] != i->n); ++j);
						if(j == nbRedPebbles) {
							std::cout << "INVALID: Value not in registers on store" << std::endl;
							break;
						}
						if(i->n->deleted) {
							std::cout << "INVALID: Value deleted on store" << std::endl;
							break;
						}
						regs[j] = NULL;

						nR2 += 1;

					} else if(i->r == RULE_R3) {
						uint32_t j, k;
						for(k = 0; k < i->n->nbPredecessors; ++k) {
							for(j = 0; (j < nbRedPebbles) && (regs[j] != i->n->predecessors[k]); ++j);
							if(j == nbRedPebbles) {
								std::cout << "INVALID: Dependency not satisfied" << std::endl;
								break;
							}
							if(i->n->predecessors[k]->deleted) {
								std::cout << "INVALID: Dependency deleted" << std::endl;
								break;
							}
						}
						if(k < i->n->nbPredecessors)
							break; // one at least is missing
						for(j = 0; (j < nbRedPebbles) && (regs[j] != NULL); ++j);
						if(j == nbRedPebbles) {
							std::cout << "INVALID: Register file full on compute" << std::endl;
							break;
						}
						regs[j] = i->n;

					} else if(i->r == RULE_R4) {
						uint32_t j;
						for(j = 0; (j < nbRedPebbles) && (regs[j] != i->n); ++j);
						if(j == nbRedPebbles) {
							std::cout << "INVALID: Value not in memory on delete" << std::endl;
							break;
						}
						if(i->n->deleted) {
							std::cout << "INVALID: Value already deleted on delete" << std::endl;
							break;
						}
						regs[j] = NULL;
						i->n->deleted = true;
					}
				}

			}
			if(t == budget)
				std::cout << "Schedule VALID :) I/O cost: " << std::to_string(nR1 + nR2) << std::endl;


		} else if(solve_result == unsat) {
			std::cout << "No valid schedule exists" << std::endl;
		} else { // unknown
			std::cout << "It is unknown whether a valid schedule exists" << std::endl;
		}

    } catch(exception& e) {
    	std::cout << e.msg() << std::endl;
    };

	return 0;
}
