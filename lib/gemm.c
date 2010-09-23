/*
 * Copyright (c) 2010      The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 */
#include <plasma.h>
#include <dague.h>
#include "dplasma.h"
#include "dplasmatypes.h"
#include "gemm.h"

dague_object_t* 
DAGUEprefix(gemm_New)( int transA, int transB, int m, int n, int k,
		       TYPENAME alpha, const tiled_matrix_desc_t* ddescA,
		       const tiled_matrix_desc_t* ddescB,
		       TYPENAME beta, tiled_matrix_desc_t* ddescC)
{
    if( PlasmaNoTrans == transA ) {
        if( PlasmaNoTrans == transB ) {
            return (dague_object_t*)dagueprefix(gemm_NN_new)((dague_ddesc_t*)ddescC,
							     (dague_ddesc_t*)ddescB,
							     (dague_ddesc_t*)ddescA,
							     ddescA->nb,
							     m, n, k,
							     transA, transB,
							     alpha, beta);
        } else {
            return (dague_object_t*)dagueprefix(gemm_NT_new)((dague_ddesc_t*)ddescC,
							     (dague_ddesc_t*)ddescB,
							     (dague_ddesc_t*)ddescA,
							     ddescA->nb,
							     m, n, k,
							     transA, transB,
							     alpha, beta);
        }
    } else {
        if( PlasmaNoTrans == transB ) {
            return (dague_object_t*)dagueprefix(gemm_TN_new)((dague_ddesc_t*)ddescC,
							     (dague_ddesc_t*)ddescB,
							     (dague_ddesc_t*)ddescA,
							     ddescA->nb,
							     m, n, k,
							     transA, transB,
							     alpha, beta);
        } else {
            return (dague_object_t*)dagueprefix(gemm_TT_new)((dague_ddesc_t*)ddescC,
							     (dague_ddesc_t*)ddescB,
							     (dague_ddesc_t*)ddescA,
							     ddescA->nb,
							     m, n, k,
							     transA, transB,
							     alpha, beta);
        }
    }
}