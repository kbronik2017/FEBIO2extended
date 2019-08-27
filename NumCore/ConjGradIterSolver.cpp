#include "stdafx.h"
#include "ConjGradIterSolver.h"
#include "FECore/vector.h"
#include <algorithm>

//-----------------------------------------------------------------------------
ConjGradIterSolver::ConjGradIterSolver()
{
	m_tol = 0.01;
	m_kmax = 200;
	m_nprint = 0;
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::PreProcess()
{
	assert(m_pA);
	return true;
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::Factor()
{
/*	CompactMatrix& C = dynamic_cast<CompactMatrix&>(*m_pA);
	int neq = K.Size();

	int* pi = C.indices();
	int* pr = C.pointers();
	double* pv = C.values();

	// copy the diagonals to P
	m_P.create(neq);
	int i, j, n, l;

	// store the diagonal elements in P
	for (j=0; j<neq; ++j) m_P[j] = 1.0 / pv[pr[j]];

	for (j=0; j<neq; ++j)
	{
		// get the number of entries on the column
		l = pr[j+1]-pr[j];

		// loop over all rows
		for (i=0; i<l; ++i)
		{
			n = pi[pr[j] + i];
			pv[ pr[j] + i] *= m_P[n];
		}
	}
*/
	return true;
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	int i;

	CompactSymmMatrix& A = dynamic_cast<CompactSymmMatrix&>(*m_pA);

	int N = x.size();

	// intialize x to zero
	std::fill(x.begin(), x.end(), 0);

	// iteration counter
	int k = 0;

	// initial residual vector
	vector<double> r(N);
	A.mult_vector(x, r);
	for (i=0; i<N; ++i) r[i] = b[i] - r[i];

	// initial norms
	double rho1 = r*r;
	double rho2 = rho1;
	double normb = 0;
	for (i=0; i<N; ++i) normb += (b[i])*(b[i]);
	normb = sqrt(normb);

	// search direction
	std::vector<double> p; p.assign(N, 0.0);

	// work vector
	std::vector<double> w(N);

	double alpha, beta;

	if (m_nprint > 0) fprintf(stderr, "Solving linear system ...\n");

	// loop until converged or until max iterations reached
	while ((sqrt(rho1) > m_tol*normb) && (k < m_kmax))
	{
		// increase iteration counter
		++k;

		// calculate search direction
		beta = rho1/rho2;
		for (i=0; i<N; ++i) p[i] = r[i] + p[i]*beta;

		A.mult_vector(p, w);
		alpha = rho1 / (p*w);

		for (i=0; i<N; ++i)
		{
			x[i] += p[i]*alpha;
			r[i] -= w[i]*alpha;
		}
		rho2 = rho1;
		rho1 = r*r;

		if (m_nprint > 0)
			fprintf(stderr, "%d: %lg\n", k, sqrt(rho1)/normb);
	}

	if (m_nprint > 0)
	{
		if ((k >= m_kmax) && (sqrt(rho1) > m_tol*normb))
		{
			fprintf(stderr, "Max iterations reached. Solution has not converged.\n");
		}
		else fprintf(stderr, "Solution converged\n");
	}

	return true;
}

//-----------------------------------------------------------------------------
void ConjGradIterSolver::Destroy()
{
	LinearSolver::Destroy();
}
