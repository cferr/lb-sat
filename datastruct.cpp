/*
 * SMT Computation of IO Lower Bounds
 * Corentin Ferry - 2018
 *
*/

/*
 * This code comes straight from last year's internship.
 */

#include "datastruct.h"
#include <cstdlib>
#include <iostream>
#include <cassert>

using namespace std;

int isInput(node n) {
    return (n.nbPredecessors == 0);
}

int isOutput(node n) {
    return (n.nbSuccessors == 0);
}

dag* createDAGStructure(node* nodes, uint32_t nbNodes) {
    dag* d = (dag*)malloc(sizeof(dag));

    uint32_t i;

    d->allNodes = nodes;
    d->nbNodes = nbNodes;
    d->inputNodes = NULL;
    d->outputNodes = NULL;
    d->nbInputNodes = 0;
    d->nbOutputNodes = 0;


    // Build input nodes
    for(i = 0; i < d->nbNodes; i++) {
        node n = d->allNodes[i];
        if(isInput(n)) {
#ifdef DEBUG
            cout << to_string(i+1) << " is input" << endl;
#endif
            d->nbInputNodes += 1;
            d->inputNodes = (node**)realloc(d->inputNodes, d->nbInputNodes *
                sizeof(node*));
            assert(d->inputNodes != NULL);

            d->inputNodes[d->nbInputNodes - 1] = &(d->allNodes[i]);
        }
    }

    // Build output nodes
    for(i = 0; i < d->nbNodes; i++) {
        node n = d->allNodes[i];
        if(isOutput(n)) {
#ifdef DEBUG
            cout << to_string(i+1) << " is output" << endl;
#endif
            d->nbOutputNodes += 1;
            d->outputNodes = (node**)realloc(d->outputNodes, d->nbOutputNodes *
                sizeof(node*));
            d->outputNodes[d->nbOutputNodes - 1] = &(d->allNodes[i]);
        }
    }
#ifdef DEBUG
    cout << "The DAG has " << to_string(d->nbInputNodes) << " inputs and " <<
    		to_string(d->nbOutputNodes) << " outputs" << endl;
#endif
    return d;
}

node* matrixToNodes(uint32_t deps[][MAX_DEPS], uint32_t nbNodes) {
    uint32_t i = 0, j = 0;
    node* dagNodes = (node*)calloc(nbNodes, sizeof(node));

    for(i = 0; i < nbNodes; i++) {
        dagNodes[i].num = i + 1;
        dagNodes[i].asap = UINT32_MAX;
        dagNodes[i].alap = 0;
        dagNodes[i].deleted = false;
        for(j = 0; j < MAX_DEPS; j++) {
            uint32_t dep = deps[i][j];
            if(dep > 0) {
#ifdef DEBUG
                cout << to_string(i + 1) << " -> " << to_string(dep) << endl;
#endif
                dep -= 1; // convention !!
                dagNodes[i].nbPredecessors += 1;
                dagNodes[i].predecessors = (node**)realloc(dagNodes[i].predecessors,
                                dagNodes[i].nbPredecessors * sizeof(node*));
                dagNodes[i].predecessors[dagNodes[i].nbPredecessors - 1] =
                                &(dagNodes[dep]);

                dagNodes[dep].nbSuccessors += 1;
                dagNodes[dep].successors = (node**)realloc(dagNodes[dep].successors,
                                dagNodes[dep].nbSuccessors * sizeof(node*));
                dagNodes[dep].successors[dagNodes[dep].nbSuccessors - 1] =
                                &(dagNodes[i]);
            }
        }
    }

    return dagNodes;

}
