/**
 *
 * @file
 *
 *  PLASMA core_blas kernel
 *  PLASMA is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.7.1
 * @author Mathieu Faverge
 * @date 2010-11-15
 **/
/*
 * @precisions normal z -> c d s
 */
#include "common.h"

/**
 ******************************************************************************
 *
 * @ingroup dplasma_cores_complex64
 *
 *  CORE_ztradd adds to matrices together as in PBLAS pztradd.
 *
 *       B <- alpha * op(A)  + beta * B
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies the shape of A and B matrices:
 *          = PlasmaUpperLower: A and B are general matrices.
 *          = PlasmaUpper: op(A) and B are upper trapezoidal matrices.
 *          = PlasmaLower: op(A) and B are lower trapezoidal matrices.
 *
 * @param[in] trans
 *          Specifies whether the matrix A is non-transposed, transposed, or
 *          conjugate transposed
 *          = PlasmaNoTrans:   op(A) = A
 *          = PlasmaTrans:     op(A) = A'
 *          = PlasmaConjTrans: op(A) = conj(A')
 *
 * @param[in] M
 *          Number of rows of the matrices A and B.
 *
 * @param[in] N
 *          Number of columns of the matrices A and B.
 *
 * @param[in] alpha
 *          Scalar factor of A.
 *
 * @param[in] A
 *          Matrix of size LDA-by-N.
 *
 * @param[in] LDA
 *          Leading dimension of the array A. LDA >= max(1,M)
 *
 * @param[in] beta
 *          Scalar factor of B.
 *
 * @param[in,out] B
 *          Matrix of size LDB-by-N.
 *          On exit, B = alpha * op(A) + beta * B
 *
 * @param[in] LDB
 *          Leading dimension of the array B. LDB >= max(1,M)
 *
 *******************************************************************************
 *
 * @return
 *          \retval PLASMA_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
int CORE_ztradd(PLASMA_enum uplo, PLASMA_enum trans, int M, int N,
                              PLASMA_Complex64_t  alpha,
                        const PLASMA_Complex64_t *A, int LDA,
                              PLASMA_Complex64_t  beta,
                              PLASMA_Complex64_t *B, int LDB)
{
    static PLASMA_Complex64_t zone = (PLASMA_Complex64_t)1.;
    int j;

    if (uplo == PlasmaUpperLower){
        return CORE_zgeadd( trans, M, N, alpha, A, LDA, beta, B, LDB );
    }

    if ((uplo != PlasmaUpper) &&
        (uplo != PlasmaLower))
    {
        coreblas_error(1, "illegal value of uplo");
        return -1;
    }

    if ((trans != PlasmaNoTrans) &&
        (trans != PlasmaTrans)   &&
        (trans != PlasmaConjTrans))
    {
        coreblas_error(2, "illegal value of trans");
        return -2;
    }

    if (M < 0) {
        coreblas_error(3, "Illegal value of M");
        return -3;
    }
    if (N < 0) {
        coreblas_error(4, "Illegal value of N");
        return -4;
    }
    if ( ((trans == PlasmaNoTrans) && (LDA < max(1,M)) && (M > 0)) ||
         ((trans != PlasmaNoTrans) && (LDA < max(1,N)) && (N > 0)) )
    {
        coreblas_error(7, "Illegal value of LDA");
        return -7;
    }
    if ( (LDB < max(1,M)) && (M > 0) ) {
        coreblas_error(9, "Illegal value of LDB");
        return -9;
    }

    if (uplo == PlasmaLower) {
        switch( trans ) {
#if defined(PRECISION_z) || defined(PRECISION_c)
        case PlasmaConjTrans:
            for (j=0; j<N; j++, M--, A+=LDA+1, B+=LDB+1) {
                for(int i=0; i<M; i++) {
                    B[i] = beta * B[i] + alpha * conj(A[LDA*i]);
                }
            }
            break;
#endif /* defined(PRECISION_z) || defined(PRECISION_c) */

        case PlasmaTrans:
            for (j=0; j<N; j++, M--, A+=LDA+1, B+=LDB+1) {
                if (beta != zone) {
                    cblas_zscal(M, CBLAS_SADDR(beta), B, 1);
                }
                cblas_zaxpy(M, CBLAS_SADDR(alpha), A, LDA, B, 1);
            }
            break;

        case PlasmaNoTrans:
        default:
            for (j=0; j<N; j++, M--, A+=LDA+1, B+=LDB+1) {
                if (beta != zone) {
                    cblas_zscal(M, CBLAS_SADDR(beta), B, 1);
                }
                cblas_zaxpy(M, CBLAS_SADDR(alpha), A, 1, B, 1);
            }
        }
    }
    else {
        switch( trans ) {
#if defined(PRECISION_z) || defined(PRECISION_c)
        case PlasmaConjTrans:
            for (j=0; j<N; j++, A++, B+=LDB) {
                for(int i=0; i<=j; i++) {
                    B[i] = beta * B[i] + alpha * conj(A[LDA*i]);
                }
            }
            break;
#endif /* defined(PRECISION_z) || defined(PRECISION_c) */

        case PlasmaTrans:
            for (j=0; j<N; j++, A++, B+=LDB) {
                if (beta != zone) {
                    cblas_zscal(j+1, CBLAS_SADDR(beta), B, 1);
                }
                cblas_zaxpy(j+1, CBLAS_SADDR(alpha), A, LDA, B, 1);
            }
            break;

        case PlasmaNoTrans:
        default:
            for (j=0; j<N; j++, A+=LDA, B+=LDB) {
                if (beta != zone) {
                    cblas_zscal(j+1, CBLAS_SADDR(beta), B, 1);
                }
                cblas_zaxpy(j+1, CBLAS_SADDR(alpha), A, 1, B, 1);
            }
        }
    }

    return 0;
}
