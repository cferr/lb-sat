/*
 * SMT Computation of IO Lower Bounds
 * Corentin Ferry - 2018
 *
*/

/*
 * This code comes straight from last year's internship.
 * It has been modified to add ASAP/ALAP pre-processing
 * (that wasn't done last year as only valid schedules
 * were computed)
 */

#ifndef DATASTRUCT_H_
#define DATASTRUCT_H_

#include <cstdint>

// Hardcoded value for the max number of dependences for one single node.
#define MAX_DEPS 2

typedef enum rule {
    NONE, // WARNING do not move !!
    RULE_R1,
    RULE_R2,
    RULE_R3,
    RULE_R4
} rule;

typedef struct node {
    struct node** predecessors;
    uint32_t nbPredecessors;

    // redundant ! but useful !
    struct node** successors;
    uint32_t nbSuccessors;

    uint32_t num;

    uint32_t asap;
    uint32_t alap;

    bool deleted;

} node;

typedef struct dag {
    node** inputNodes;
    uint32_t nbInputNodes;

    node** outputNodes;
    uint32_t nbOutputNodes;

    node* allNodes;
    uint32_t nbNodes;
} dag;


dag* createDAGStructure(node* nodes, uint32_t nbNodes);
node* matrixToNodes(uint32_t deps[][MAX_DEPS], uint32_t nbNodes);

#endif /* DATASTRUCT_H_ */
