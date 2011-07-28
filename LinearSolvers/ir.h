//*****************************************************************
// Iterative template routine -- Preconditioned Richardson
//
// @ingroup solvers
// @brief IR solves the unsymmetric linear system Ax = b using 
// Iterative Refinement (preconditioned Richardson iteration).
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//  
// @param x  --  approximate solution to Ax = b
// @param max_iter  --  the number of iterations performed before the
//               tolerance was reached
// @param tol  --  the residual after the final iteration
//  
//*****************************************************************

template < class Matrix, class Vector, class Preconditioner, class Real >
int 
IR( Matrix &A, Vector &x,const Vector &b,
   Preconditioner &M, Vector *residual, int &max_iter, Real &tol,const int FromCurrent)
{
	Real resid;
	Vector z;
	
	Real normb = Norm(b);
	Vector resbackup;
	Vector *res = residual;
	if(!res) res = &resbackup;
	Vector &r = *res;
	//  Vector r = b - A*x;
	if(FromCurrent) A.MultAdd(x,b,r,-1.,1.);
	else {
		x.Zero();
		r = b;
	}
	//  Vector r = b - A*x;
	
	if (normb == 0.0)
		normb = 1;
	
	if ((resid = Norm(r) / normb) <= tol) {
		tol = resid;
		max_iter = 0;
		return 0;
	}
	
	for (int i = 1; i <= max_iter; i++) {
		//	 z = M.solve(r);
		M.Solve(r,z);
		x += z;
		A.MultAdd(x,b,r,-1.,1.);
		//	 r = b - A * x;
		
		if ((resid = Norm(r) / normb) <= tol) {
			tol = resid;
			max_iter = i;
			return 0;
		}
	}
	
	tol = resid;
	return 1;
}



