#ifndef RMATRIX_H
#define MATRIX_H
#include <stdarg.h>


typedef enum r_node_type{
    mtx_summer,
    mtx_generator,
    mtx_envelope,
    mtx_filter,
    mtx_dafx
} r_node_type;

typedef struct r_node{
    float (*callback)(void*,double[4]);
    void* node;
    int depth;
    r_node_type type;
    r_node** child;
} r_node;

typedef struct r_matrix{
    r_node* core;
}r_matrix;

double rmatrix_get_sample(r_matrix* mtx);

#endif