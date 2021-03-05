

#include "../misc/misc.h"
#include "matrix.h"

double rmatrix_get_sample(r_matrix* mtx){
    
    /*

        Possible r_matrix realization.
    
        1. Implement callbacks for any signal modifiers/generators (e.g. oscillators, filters, envelopes)
        2. Store pointers for callbacks and processors in a r_node structure; 
            This structure can have references to other node, and stores type of node: generator, envelope, etc.
        3. Perform next routine:
            -> Start from core node (probably, it's the summer).
            -> Check length (child count)
            -> Recoursively call callbacks for every branch in a tree.
            -> Sum.
        4. Return final sample.

        Possibly, if running in non-mono mode, each node parameters could be randomized, making each voice sound individually. 
    
    */
    
    r_node* core = mtx->core;
    double out = rmatrix_recursive_get (core);
    

    return 0.f;

}

double rmatrix_recursive_get(r_node* node){
    switch (node->type){
        case mtx_generator:
            break;
        case mtx_envelope:
            break;
        case mtx_filter:
            break;
        case mtx_summer:
            break;
        case mtx_dafx:
            break;
    }
    for (int i = 0; i < node->depth; i++){
        
    }

}