/*
 * SMT Computation of IO Lower Bounds
 * Corentin Ferry - 2018
 *
*/

#ifndef SAT_VERSION_H_
#define SAT_VERSION_H_

#include <z3++.h>
#include <vector>
#include "datastruct.h"

using namespace z3;

typedef struct {
	expr symbol;
	node* n;
	rule r;
	uint32_t date;
} registered_symbol;

typedef std::vector<registered_symbol> symbol_list;

registered_symbol lookupRegisteredSymbol(std::string name, symbol_list& symbols);
void addRegisteredSymbol(registered_symbol rs, symbol_list& symbols);

void dagToConstraints(dag* _dag, uint32_t nbRedPebbles, uint32_t maxTime, context& ctx, expr_vector& existingConstraints, symbol_list& symbols);

#endif /* SAT_VERSION_H_ */
