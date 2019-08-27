//! This implementation of the Pardiso solver is for the version
//! available in the Intel MKL.

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "PardisoSolver.h"

#ifdef PARDISO

//////////////////////////////////////////////////////////////
// PardisoSolver
//////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PardisoSolver::PardisoSolver()
{
	/* If both PARDISO AND PARDISODL are defined, print a warning */
#ifdef PARDISODL
	fprintf(stderr, "WARNING: The MKL version of the Pardiso solver is being used\n\n");
	exit(1);
#endif
}

//-----------------------------------------------------------------------------
bool PardisoSolver::PreProcess()
{
	m_mtype = (m_bsymm ? -2 : 11); /* Real symmetric matrix */
	m_iparm[0] = 0; /* Use default values for parameters */

	//fprintf(stderr, "In PreProcess\n");

	pardisoinit_(m_pt, &m_mtype, m_iparm);

	m_n = m_pA->Size();
	m_nnz = m_pA->NonZeroes();
	m_nrhs = 1;

	// number of processors: This parameter is no longer used.
	// Use OMP_NUM_THREADS
	// m_iparm[2] = m_numthreads;

	m_maxfct = 1;	/* Maximum number of numerical factorizations */
	m_mnum = 1;	/* Which factorization to use */

	m_msglvl = 0;	/* 0 Suppress printing, 1 Print statistical information */
	m_error = 0;	/* Initialize m_error flag */

	return LinearSolver::PreProcess();
}

//-----------------------------------------------------------------------------
bool PardisoSolver::Factor()
{

	CompactMatrix* A = dynamic_cast<CompactMatrix*> (m_pA);

// ------------------------------------------------------------------------------
// Reordering and Symbolic Factorization.  This step also allocates all memory
// that is necessary for the factorization.
// ------------------------------------------------------------------------------

	int phase = 11;

	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, A->Values(), A->Pointers(), A->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &m_error);

	if (m_error)
	{
		fprintf(stderr, "\nERROR during symbolic factorization: ");
		print_err();
		exit(2);
	}

// ------------------------------------------------------------------------------
// This step does the factorization
// ------------------------------------------------------------------------------

	phase = 22;

#ifdef PRINTHB
	A->print_hb();
#endif

	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, A->Values(), A->Pointers(), A->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &m_error);

	if (m_error)
	{
		fprintf(stderr, "\nERROR during factorization: ");
		print_err();
		exit(2);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool PardisoSolver::BackSolve(vector<double>& x, vector<double>& b)
{

	CompactMatrix* A = dynamic_cast<CompactMatrix*> (m_pA);

	int phase = 33;

	m_iparm[7] = 1;	/* Maximum number of iterative refinement steps */

	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, A->Values(), A->Pointers(), A->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, &b[0], &x[0], &m_error);

	if (m_error)
	{
		fprintf(stderr, "\nERROR during solution: ");
		print_err();
		exit(3);
	}

	return true;
}

//-----------------------------------------------------------------------------
void PardisoSolver::Destroy()
{
	// make sure we actually initialized the matrix data.
	if (m_bvalid == false) return;

	CompactMatrix* A = dynamic_cast<CompactMatrix*> (m_pA);

	int phase = -1;

	//fprintf(stderr, "In Destroy\n");

	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, NULL, A->Pointers(), A->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &m_error);

	LinearSolver::Destroy();

}
#endif
