/*
 * Copyright (c) 2011      The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 *
 * @precisions normal z -> c d s
 *
 */
#include "dague.h"
#include <plasma.h>
#include "dplasma.h"
#include "dplasma/lib/dplasmatypes.h"
#include "dplasma/lib/dplasmaaux.h"

#include "zplrnt.h"

/***************************************************************************//**
 *
 * @ingroup DPLASMA_Complex64_t
 *
 *  dplasma_zplrnt_New - Generate a random matrix by tiles.
 *
 *******************************************************************************
 *
 * @param[out] A
 *          On exit, The random hermitian matrix A generated.
 *
 * @param[in] seed
 *          The seed used in the random generation.
 *
 ******************************************************************************/
dague_object_t* dplasma_zplrnt_New( tiled_matrix_desc_t *A,
                                    unsigned long long int seed)
{
    dague_zplrnt_object_t* object;
    
    object = dague_zplrnt_new( seed, *A, (dague_ddesc_t*)A);

    /* Default type */
    dplasma_add2arena_tile( object->arenas[DAGUE_zplrnt_DEFAULT_ARENA], 
                            A->mb*A->nb*sizeof(Dague_Complex64_t),
                            DAGUE_ARENA_ALIGNMENT_SSE,
                            MPI_DOUBLE_COMPLEX, A->mb );
    
    return (dague_object_t*)object;
}

int dplasma_zplrnt( dague_context_t *dague, 
                    tiled_matrix_desc_t *A,
                    unsigned long long int seed) 
{
    dague_object_t *dague_zplrnt = NULL;

    dague_zplrnt = dplasma_zplrnt_New(A, seed);

    dague_enqueue(dague, (dague_object_t*)dague_zplrnt);
    dague_progress(dague);

    dplasma_zplrnt_Destruct( dague_zplrnt );
    return 0;
}

void
dplasma_zplrnt_Destruct( dague_object_t *o )
{
    dague_zplrnt_object_t *dague_zplrnt = (dague_zplrnt_object_t *)o;
    dague_zplrnt_destroy(dague_zplrnt);
}

