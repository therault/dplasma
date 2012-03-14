/*
 * Copyright (c) 2009-2011 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 *
 * @precisions normal z -> s d c
 *
 */

#include "common.h"
#include "data_dist/matrix/two_dim_rectangle_cyclic.h"

static int check_solution( dague_context_t *dague, int loud,
                           tiled_matrix_desc_t *ddescA,
                           tiled_matrix_desc_t *ddescB,
                           tiled_matrix_desc_t *ddescX );

int main(int argc, char ** argv)
{
    dague_context_t* dague;
    int iparam[IPARAM_SIZEOF];
    int info = 0;
    int ret = 0;
    double criteria = 0;

    /* Set defaults for non argv iparams */
    iparam_default_facto(iparam);
    iparam_default_ibnbmb(iparam, 40, 200, 200);
    iparam[IPARAM_LDA] = -'m';
    iparam[IPARAM_LDB] = -'m';
#if defined(HAVE_CUDA) && defined(PRECISION_s) && 0
    iparam[IPARAM_NGPUS] = 0;
#endif
    /* Initialize DAGuE */
    dague = setup_dague(argc, argv, iparam);
    PASTE_CODE_IPARAM_LOCALS(iparam)
    PASTE_CODE_FLOPS(FLOPS_ZGETRF, ((DagDouble_t)M, (DagDouble_t)N))

    if ( M != N && check ) {
        fprintf(stderr, "Check cannot be perfomed with M != N\n");
        check = 0;
    }

    /* initializing matrix structure */
    PASTE_CODE_ALLOCATE_MATRIX(ddescA, 1,
        two_dim_block_cyclic, (&ddescA, matrix_ComplexDouble, matrix_Lapack,
                               nodes, cores, rank, MB, NB, M, N, 0, 0,
                               LDA, N, SMB, SNB, P));

    PASTE_CODE_ALLOCATE_MATRIX(ddescA0, check,
        two_dim_block_cyclic, (&ddescA0, matrix_ComplexDouble, matrix_Lapack,
                               nodes, cores, rank, MB, NB, LDA, N, 0, 0,
                               LDA, N, SMB, SNB, P));

    PASTE_CODE_ALLOCATE_MATRIX(ddescB, check,
        two_dim_block_cyclic, (&ddescB, matrix_ComplexDouble, matrix_Lapack,
                               nodes, cores, rank, MB, NB, LDB, NRHS, 0, 0,
                               N, NRHS, SMB, SNB, P));

    PASTE_CODE_ALLOCATE_MATRIX(ddescX, check,
        two_dim_block_cyclic, (&ddescX, matrix_ComplexDouble, matrix_Lapack,
                               nodes, cores, rank, MB, NB, LDB, NRHS, 0, 0,
                               N, NRHS, SMB, SNB, P));

    /* matrix generation */
    if(loud > 2) printf("+++ Generate matrices ... ");
    dplasma_zplrnt( dague, (tiled_matrix_desc_t *)&ddescA, 7657);

    tiled_matrix_desc_t * descA = (tiled_matrix_desc_t *)&ddescA;
    Dague_Complex64_t *tab = (Dague_Complex64_t *) ddescA.mat;
    int t,e;
    for(t = 0; t < ((descA->lmt < descA->lnt)?descA->lmt:descA->lnt); t++)
        for(e = 0; e < descA->mb; e++)
            tab[(t * descA->lmt +t) * (descA->mb*descA->nb) + e * descA->mb + e] += ((M<N)?M:N);

    if ( check )
    {
        dplasma_zlacpy( dague, PlasmaUpperLower,
                        (tiled_matrix_desc_t *)&ddescA,
                        (tiled_matrix_desc_t *)&ddescA0 );
        dplasma_zplrnt( dague, (tiled_matrix_desc_t *)&ddescB, 2354);
        dplasma_zlacpy( dague, PlasmaUpperLower,
                        (tiled_matrix_desc_t *)&ddescB,
                        (tiled_matrix_desc_t *)&ddescX );
    }
    if(loud > 2) printf("Done\n");

    /* Create DAGuE */
    if(loud > 2) printf("+++ Computing getrf ... ");
    PASTE_CODE_ENQUEUE_KERNEL(dague, zgetrf_sp,
                              (criteria,(tiled_matrix_desc_t*)&ddescA,
                               &info));
    /* lets rock! */
    PASTE_CODE_PROGRESS_KERNEL(dague, zgetrf_sp);
    dplasma_zgetrf_sp_Destruct( DAGUE_zgetrf_sp );
    if(loud > 2) printf("Done.\n");

    if ( check && info != 0 ) {
        if( rank == 0 && loud ) printf("-- Factorization is done with static pivoting (info = %d) ! \n", info );
        ret |= 1;
    }
    else if ( check ) {
        dague_object_t *dague_ztrsm1 = NULL;
        dague_object_t *dague_ztrsm2 = NULL;

        dague_ztrsm1 = dplasma_ztrsm_New(PlasmaLeft, PlasmaLower, PlasmaNoTrans, PlasmaUnit,
                                         1.0, (tiled_matrix_desc_t *)&ddescA,
                                         (tiled_matrix_desc_t *)&ddescX);
        dague_ztrsm2 = dplasma_ztrsm_New(PlasmaLeft, PlasmaUpper, PlasmaNoTrans, PlasmaNonUnit,
                                         1.0, (tiled_matrix_desc_t *)&ddescA,
                                         (tiled_matrix_desc_t *)&ddescX);

        dague_enqueue( dague, dague_ztrsm1 );
        dague_enqueue( dague, dague_ztrsm2 );

        dague_progress( dague );

        dplasma_ztrsm_Destruct( dague_ztrsm1 );
        dplasma_ztrsm_Destruct( dague_ztrsm2 );

        /* dplasma_zgetrs(dague, PlasmaNoTrans, */
        /*                (tiled_matrix_desc_t *)&ddescA, */
        /*                (tiled_matrix_desc_t *)&ddescIPIV, */
        /*                (tiled_matrix_desc_t *)&ddescX ); */

        /* Check the solution */
        ret |= check_solution( dague, (rank == 0) ? loud : 0,
                               (tiled_matrix_desc_t *)&ddescA0,
                               (tiled_matrix_desc_t *)&ddescB,
                               (tiled_matrix_desc_t *)&ddescX);
    }
    cleanup_dague(dague, iparam);

    if ( check ) {
        dague_data_free(ddescA0.mat);
        dague_ddesc_destroy( (dague_ddesc_t*)&ddescA0);
        dague_data_free(ddescB.mat);
        dague_ddesc_destroy( (dague_ddesc_t*)&ddescB);
        dague_data_free(ddescX.mat);
        dague_ddesc_destroy( (dague_ddesc_t*)&ddescX);
    }

    //    cleanup_dague(dague, iparam);

    dague_data_free(ddescA.mat);
    dague_ddesc_destroy((dague_ddesc_t*)&ddescA);

    return EXIT_SUCCESS;
}



static int check_solution( dague_context_t *dague, int loud,
                           tiled_matrix_desc_t *ddescA,
                           tiled_matrix_desc_t *ddescB,
                           tiled_matrix_desc_t *ddescX )
{
    int info_solution;
    double Rnorm = 0.0;
    double Anorm = 0.0;
    double Bnorm = 0.0;
    double Xnorm, result;
    int m = ddescB->m;
    double eps = LAPACKE_dlamch_work('e');

    Anorm = dplasma_zlange(dague, PlasmaMaxNorm, ddescA);
    Bnorm = dplasma_zlange(dague, PlasmaMaxNorm, ddescB);
    Xnorm = dplasma_zlange(dague, PlasmaMaxNorm, ddescX);

    /* Compute b - A*x */
    dplasma_zgemm( dague, PlasmaNoTrans, PlasmaNoTrans, -1.0, ddescA, ddescX, 1.0, ddescB);

    Rnorm = dplasma_zlange(dague, PlasmaMaxNorm, ddescB);

    result = Rnorm / ( ( Anorm * Xnorm + Bnorm ) * m * eps ) ;

    if ( loud > 2 ) {
        printf("============\n");
        printf("Checking the Residual of the solution \n");
        if ( loud > 3 )
            printf( "-- ||A||_oo = %e, ||X||_oo = %e, ||B||_oo= %e, ||A X - B||_oo = %e\n",
                    Anorm, Xnorm, Bnorm, Rnorm );

        printf("-- ||Ax-B||_oo/((||A||_oo||x||_oo+||B||_oo).N.eps) = %e \n", result);
    }

    if (  isnan(Xnorm) || isinf(Xnorm) || isnan(result) || isinf(result) || (result > 60.0) ) {
        if( loud ) printf("-- Solution is suspicious ! \n");
        info_solution = 1;
    }
    else{
        if( loud ) printf("-- Solution is CORRECT ! \n");
        info_solution = 0;
    }

    return info_solution;
}
