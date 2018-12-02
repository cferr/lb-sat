/*
 * SMT Computation of IO Lower Bounds
 * Corentin Ferry - 2018
 *
*/

#include "sat-version.h"
#include <algorithm>

// Internal functions : processing.
// We only expose the launcher function and a symbol helper to the exterior, cf. header

// Utilitary functions on symbols
std::string symbolName(node* n, rule _rule, uint32_t time);
void addRegisteredSymbol(registered_symbol rs, symbol_list& symbols);
expr ruleSymbol(node* n, rule _rule, uint32_t time, context& ctx, symbol_list& symbols);
expr_vector freshBoolSymbols(node* n, rule _rule, uint32_t maxTime, context& ctx, symbol_list& symbols);

// DAG pre-processing : ASAP and ALAP computation
void preProcessDAG(dag* d, uint32_t maxTime);
void preProcessASAP(node* n, uint32_t t);
void preProcessALAP(node* n, uint32_t t);

// Heavy functions.
void noTwoSimultaneousNodes(expr_vector& constraints, symbol_list symbols, uint32_t maxTime, context& ctx);
void buildConstraintsComputable(node* n, uint32_t maxTime, context& ctx, expr_vector& constraints, symbol_list& symbols);
void buildConstraintsInput(node* n, uint32_t maxTime, context& ctx, expr_vector& constraints, symbol_list& symbols);
void createLimitedPebbleConstraint(expr_vector& constraints, symbol_list symbols, uint32_t maxTime, uint32_t nbRedPebbles, context& ctx);

//#define DEBUG

std::string ruleToString(rule r) {
	switch(r) {
		case RULE_R1:
			return "R1";
			break;
		case RULE_R2:
			return "R2";
			break;
		case RULE_R3:
			return "R3";
			break;
		case RULE_R4:
			return "R4";
			break;
		default:
			return "INVALID";
	}
}

std::string symbolName(node* n, rule _rule, uint32_t time) {
	return ruleToString(_rule)  + "(" + std::to_string(n->num) + "," + std::to_string(time) + ")";
}

registered_symbol lookupRegisteredSymbol(std::string name, symbol_list& symbols) {
	std::vector<registered_symbol>::iterator i;
	for(i = symbols.begin();
				(i < symbols.end()) ; ++i) {
		std::string symName = symbolName(i->n, i->r, i->date);
		if(symName == name)
			break;
	}

	if(i < symbols.end())
		return *i;

	std::string err = "Symbol not found: " + name;
	throw exception(err.c_str());
}

void addRegisteredSymbol(registered_symbol rs, symbol_list& symbols) {
	std::vector<registered_symbol>::iterator i;
	for(i = symbols.begin();
			(i < symbols.end()) && (rs.n != i->n || rs.r != i->r || rs.date != i->date); ++i) {
	}
	if(i == symbols.end()) {
		symbols.push_back(rs);
	}
}

expr ruleSymbol(node* n, rule _rule, uint32_t time, context& ctx, symbol_list& symbols) {
	expr ret =  ctx.bool_const(symbolName(n, _rule, time).c_str());
	registered_symbol rsym = { ret, n, _rule, time };
	addRegisteredSymbol(rsym, symbols);
	return ret;
}

expr_vector freshBoolSymbols(node* n, rule _rule, uint32_t maxTime, context& ctx, symbol_list& symbols) {
	expr_vector ret(ctx);
	uint32_t t;
	for(t = 0; t < maxTime; ++t) {
		std::string sn = symbolName(n, _rule, t);
		expr sym = ctx.bool_const(sn.c_str());
		registered_symbol rsym = { sym, n, _rule, t };
		addRegisteredSymbol(rsym, symbols);
		ret.push_back(sym);
	}
	return ret;
}

//// ARCHITECTURE: express the limited number of pebbles

bool compareSymbolsByDate(registered_symbol a, registered_symbol b) {
	return a.date < b.date;
}

// This one should yield a pretty big structure. This is actually one of the reasons why
// no compilers perform simultaneous scheduling and register allocation.
void createLimitedPebbleConstraint(expr_vector& constraints, symbol_list symbols, uint32_t maxTime, uint32_t nbRedPebbles, context& ctx) {
	// for all t's before maxTime
	// sum those symbols with epsilons
	// impose the result must not exceed the number of red pebbles

	// Sort by date
	std::sort(symbols.begin(), symbols.end(), compareSymbolsByDate);

	uint32_t t;
	expr r1val = ctx.int_val(1);
	expr r2val = ctx.int_val(-1);
	expr r3val = ctx.int_val(1);
	expr r4val = ctx.int_val(-1);
	expr zero = ctx.int_val(0);

	expr redPebblesExpr = ctx.int_val(nbRedPebbles);

	for(t = 0; t < maxTime; ++t) {

		expr_vector pebbleVariation_v(ctx);
			for(symbol_list::iterator i = symbols.begin(); (i < symbols.end()) && (i->date <= t); ++i) {
				switch(i->r) {
				case RULE_R1:
					pebbleVariation_v.push_back(ite(i->symbol, r1val, zero));
					break;
				case RULE_R2:
					pebbleVariation_v.push_back(ite(i->symbol, r2val, zero));
					break;
				case RULE_R3:
					pebbleVariation_v.push_back(ite(i->symbol, r3val, zero));
					break;
				case RULE_R4:
					pebbleVariation_v.push_back(ite(i->symbol, r4val, zero));
					break;
				default:
					break;
				}
			}
		expr takenPebblesAtDateT = sum(pebbleVariation_v);
		expr constraintOnPebbles = (takenPebblesAtDateT <= redPebblesExpr);
		//std::cout << constraintOnPebbles << std::endl;
		constraints.push_back(constraintOnPebbles);
	}
}

//// SCHEDULING: express the scheduling problem, respecting the dependences

void noTwoSimultaneousNodes(expr_vector& constraints, symbol_list symbols, uint32_t maxTime, context& ctx) {
	// Symbols are our xi's (in the paper). This function makes them mutually exclusive by date,
	// i.e. no two operations can happen at the same time. This makes a lot of constraints, but it's
	// essential so that we can count the spills and restores.


	uint32_t t;
	for(t = 0; t < maxTime; ++t) {
		expr_vector possibleOpsAtT(ctx);
		// Find those symbols that have t as timestamp
		for(std::vector<registered_symbol>::iterator i = symbols.begin(); i != symbols.end(); ++i) {
			//std::cout << "Symbol: " << i->symbol << std::endl;
			if(i->date == t)
				possibleOpsAtT.push_back(i->symbol);
		}
		// Makeshift XOR : "atmost" one should be true.
		expr oneOnlyAtT = atmost(possibleOpsAtT, 1);
		constraints.push_back(oneOnlyAtT);
	}

}



void buildConstraintsComputable(node* n, uint32_t maxTime, context& ctx, expr_vector& constraints, symbol_list& symbols) {
	// For each node, its dependences must have been computed before it is.

	uint32_t i;
	uint32_t t, tt, ttt, tttt;

	try {
		expr_vector constraintsToScheduleNodeAtT(ctx);
		for(t = n->asap; t <= n->alap; ++t) {

			// There must be an R3 at some time.
			// We have to do it after all the dependencies are satisfied.
			expr computeAtT = ruleSymbol(n, RULE_R3, t, ctx, symbols);
#ifdef DEBUG
			std::cout <<  "Node " << std::to_string(n->num) << " is computed at date " << std::to_string(t) << std::endl;
#endif
			expr_vector constraintsOnPredecessors(ctx);

			for(i = 0; i < n->nbPredecessors; ++i) {
				node* predecessor = n->predecessors[i];
				// If we are computing this node at time t, then:
				// - this predecessor must have been computed or loaded (be in memory)
				// - it musn't have been spilled since then

				// This translates to: for all t' < t, (compute at date t' OR load at date t')
				//                                     AND (do not spill between t and t')
				expr_vector P3OrP4_v(ctx);

				for(tt = predecessor->asap; tt < t; ++tt) {

					// Common to P3 and P4 - load at date t (mandatory for P3, will be ORed for P4)
					// This R1 may come from a spill OR from a pure load (input)

					// Dependence is an intermediary node: Verify P4
					if(predecessor->nbPredecessors > 0) {
						expr scheduleLoadAtTT = ruleSymbol(predecessor, RULE_R1, tt, ctx, symbols);
						expr scheduleComputeAtTT = ruleSymbol(predecessor, RULE_R3, tt, ctx, symbols);


						expr_vector computeDepStrictlyBeforeTT_v(ctx);
						for(ttt = predecessor->asap; ttt < tt; ++ttt) {
							// If loaded at date tt, then has been computed at date ttt < tt.
							expr computePredecessorAtTTT = ruleSymbol(predecessor, RULE_R3, ttt, ctx, symbols);
							// the predecessor must not be deleted between ttt and t.
							expr_vector deleteBetweenTTTAndT_v(ctx);
							for(tttt = ttt; tttt <= t; ++tttt) {
								deleteBetweenTTTAndT_v.push_back(ruleSymbol(predecessor, RULE_R4, tttt, ctx, symbols));
							}
							expr doNotDeleteBetweenTTTAndTT = not(mk_or(deleteBetweenTTTAndT_v));
							computeDepStrictlyBeforeTT_v.push_back(computePredecessorAtTTT && doNotDeleteBetweenTTTAndTT);
						}

						// There exists a ttt < tt such that dependence computed at ttt
						expr computeDepStrictlyBeforeTT = mk_or(computeDepStrictlyBeforeTT_v);

						// No storeback between tt and t
						expr_vector spillsBetweenTTAndT(ctx);
						for(ttt = tt; ttt <= t; ++ttt) {
							expr spillAtTTT = ruleSymbol(predecessor, RULE_R2, ttt, ctx, symbols);
							spillsBetweenTTAndT.push_back(spillAtTTT);
						}
						expr noSpillsBetweenTTAndT = not(mk_or(spillsBetweenTTAndT));

						expr P4 = (scheduleLoadAtTT && computeDepStrictlyBeforeTT) || scheduleComputeAtTT;
						P3OrP4_v.push_back(P4);

					} else {
						// Dependence is an input node: Verify P3

						// load at tt < t
						expr scheduleLoadAtTT = ruleSymbol(predecessor, RULE_R1, tt, ctx, symbols);

						// no delete before t
						expr_vector deleteBeforeT_v(ctx);
						for(ttt = 0; ttt < t; ++ttt) {
							deleteBeforeT_v.push_back(ruleSymbol(predecessor, RULE_R4, ttt, ctx, symbols));
						}
						expr doNotDeleteBeforeT = not(mk_or(deleteBeforeT_v));

						expr P3 = scheduleLoadAtTT && doNotDeleteBeforeT;
						P3OrP4_v.push_back(P3);
					}
				}
				// There exists a t'<t, such that this predecessor have been loaded or computed at t' and no spill
				// has been performed between t and t'
				expr P3OrP4 = mk_or(P3OrP4_v);

				// Schedule deletion - P5
				expr_vector deleteAfterT_v(ctx);
				for(ttt = t+1; ttt < maxTime; ++ttt) {
					deleteAfterT_v.push_back(ruleSymbol(predecessor, RULE_R4, ttt, ctx, symbols));
				}
				expr P5 = mk_or(deleteAfterT_v);

				constraintsOnPredecessors.push_back(P3OrP4 && P5);

			}

			// Are we scheduling an output node ? In this case, we'll store it.
			// Otherwise, we'll end up deleting it, but this will be handled by a successor.
			expr storeAfterT = ctx.bool_val(true); // no constraint if not output node
			if(n->nbSuccessors == 0) {
				expr_vector storeAfterT_v(ctx);
				for(tt = t+1; tt < maxTime; ++tt) {
					expr storeAtTT = ruleSymbol(n, RULE_R2, tt, ctx, symbols);
					storeAfterT_v.push_back(storeAtTT);
				}
				storeAfterT = mk_or(storeAfterT_v);
			}
			// Compute current node at date t, given all the scheduling constraints above on predecessors
			constraintsToScheduleNodeAtT.push_back(computeAtT && mk_and(constraintsOnPredecessors) && storeAfterT);

		}
		// This OR models that we'll end up scheduling this node. This is the first part of P1 or P2. Predecessors have been taken care of already.
		constraints.push_back(mk_or(constraintsToScheduleNodeAtT));
		constraints.push_back(atmost(constraintsToScheduleNodeAtT, 1));

	} catch(exception e) {
		std::cout << e.msg() << std::endl;
	}

}

void dagToConstraints(dag* _dag, uint32_t nbRedPebbles, uint32_t maxTime, context& ctx, expr_vector& constraints, symbol_list& symbols) {
	std::cout << "## Pre-processing DAG: computing ASAP, ALAP" << std::endl;
	preProcessDAG(_dag, maxTime);

	std::cout << "## Building individual constraints for dependences and computation" << std::endl;
	uint32_t i;
	node* n;
	for(i = 0; i < _dag->nbNodes; ++i) {
		n = &(_dag->allNodes[i]);
		if(n->nbPredecessors > 0) {
			std::cout << "### Processing node " << std::to_string(n->num) << std::endl;
			buildConstraintsComputable(n, maxTime, ctx, constraints, symbols);
		}
	}

	std::cout << "## Building sequentiality constraints" << std::endl;
	noTwoSimultaneousNodes(constraints, symbols, maxTime, ctx);

	std::cout << "## Building architectural constraints" << std::endl;
	createLimitedPebbleConstraint(constraints, symbols, maxTime, nbRedPebbles, ctx);

}



/// OPTIMIZATION : Pre-process the DAG to compute nodes mobility

void preProcessASAP(node* n) {
	if(n->nbPredecessors > 0) {
		uint32_t minPredAsap = UINT32_MAX;
		uint32_t maxPredAsap = 0;
		for(uint32_t i = 0; i < n->nbPredecessors; ++i) {
			preProcessASAP(n->predecessors[i]);//, t+1);
			minPredAsap = std::min(minPredAsap, n->predecessors[i]->asap);
			maxPredAsap = std::max(maxPredAsap, n->predecessors[i]->asap);
		}

		n->asap = std::max(minPredAsap + n->nbPredecessors, 1 + maxPredAsap);
	} else n->asap = 0;
#ifdef DEBUG
	std::cout << "Node " << std::to_string(n->num) << " ASAP " << std::to_string(n->asap) << std::endl;
#endif
}

void preProcessALAP(node* n, uint32_t maxTime) {
	if(n->nbSuccessors > 0) {
		uint32_t minSuccAlap = UINT32_MAX;
		uint32_t maxSuccAlap = 0;
		for(uint32_t i = 0; i < n->nbSuccessors; ++i) {
			preProcessALAP(n->successors[i], maxTime);//, t+1);
			minSuccAlap = std::min(minSuccAlap, n->successors[i]->alap);
			maxSuccAlap = std::max(maxSuccAlap, n->successors[i]->alap);
		}
		n->alap = std::min(maxSuccAlap - 2*n->nbSuccessors, minSuccAlap - 1);
		if(n->alap > maxSuccAlap) {
			std::cout << "WARNING: ALAP computation overflows. Any result will be wrong. Please relax maximum deadline." << std::endl;
		}
	} else n->alap = maxTime;
	// WARNING the negative (overflow) case CAN happen and isn't handled here

#ifdef DEBUG
	std::cout << "Node " << std::to_string(n->num) << " ALAP " << std::to_string(n->alap) << std::endl;
#endif
}

void preProcessDAG(dag* d, uint32_t maxTime) {
	// compute ALAP
	for(uint32_t i = 0; i < d->nbInputNodes; ++i) {
		preProcessALAP(d->inputNodes[i], maxTime);//, 0);
	}

	// compute ASAP
	for(uint32_t i = 0; i < d->nbOutputNodes; ++i) {
		preProcessASAP(d->outputNodes[i]);//, maxTime-1);
	}
}
