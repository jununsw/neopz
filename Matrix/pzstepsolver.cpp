/* Generated by Together */

#include "pzstepsolver.h"
#include <stdlib.h>

TPZStepSolver::TPZStepSolver(TPZMatrix *refmat) : TPZMatrixSolver(refmat) {
  fPrecond = 0;
  ResetSolver();
}

TPZStepSolver::TPZStepSolver(const TPZStepSolver & copy) : TPZMatrixSolver(copy) {
    fSolver = copy.fSolver;
    fDecompose = copy.fDecompose;
    fNumIterations = copy.fNumIterations;
    fTol = copy.fTol;
    fOverRelax = copy.fOverRelax;
    fPrecond = 0;
    if(copy.fPrecond) fPrecond = copy.fPrecond->Clone();
    fFromCurrent = copy.fFromCurrent;
    fNumVectors = copy.fNumVectors;//Cedric: 24/04/2003 - 12:39
}

TPZStepSolver::~TPZStepSolver() {
  if(fPrecond) delete fPrecond;
}
  /**
  This method will reset the matrix associated with the solver
  This is useful when the matrix needs to be recomputed in a non linear problem
  */
void TPZStepSolver::ResetMatrix()
{
  TPZMatrixSolver::ResetMatrix();
}
  
  /**
  This method gives a preconditioner to share a matrix with the referring solver object
  */
void TPZStepSolver::SetMatrix(TPZMatrixSolver *solver)
{
  switch(fSolver)
  {
    case ECG:
    case EDirect:
    case EGMRES:
    case EJacobi:
    case ESOR:
    case ESSOR:
      ShareMatrix(*solver);
      break;
    default:
      ;    
  }
}
  

void TPZStepSolver::Solve(const TPZFMatrix &F, TPZFMatrix &result, TPZFMatrix *residual){
  if(!Matrix()) {
    cout << "TPZMatrixSolver::Solve called without a matrix pointer\n";
    exit(-1);
  }
  TPZMatrix *mat = Matrix();
  if(result.Rows() != mat->Rows() || result.Cols() != F.Cols()) {
    result.Redim(mat->Rows(),F.Cols());
  }

  if(fScratch.Rows() != result.Rows() || fScratch.Cols() != result.Cols()) {
	  fScratch.Redim(result.Rows(),result.Cols());
  }

  REAL tol = fTol;
  int numiterations = fNumIterations;
  switch(fSolver) {
  case ENoSolver:
  default:
    cout << "TPZMatrixSolver::Solve called without initialized solver, Jacobi used\n";
    SetJacobi(1,0.,0);
  case EJacobi:
    //    cout << "fScratch dimension " << fScratch.Rows() << ' ' << fScratch.Cols() << endl;
    mat->SolveJacobi(numiterations,F,result,residual,fScratch,tol,fFromCurrent);
    break;
  case ESOR:
    mat->SolveSOR(numiterations,F,result,residual,fScratch,fOverRelax,tol,fFromCurrent);
    break;
  case ESSOR:
    mat->SolveSSOR(numiterations,F,result,residual,fScratch,fOverRelax,tol,fFromCurrent);
    break;
  case ECG:
    mat->SolveCG(numiterations,*fPrecond,F,result,residual,tol,fFromCurrent);
    break;
  case EGMRES: {
    TPZFMatrix H(fNumVectors+1,fNumVectors+1,0.);
    mat->SolveGMRES(numiterations,*fPrecond,H,fNumVectors,F,result,residual,tol,fFromCurrent);
  }
    break;
  case EDirect:
    result = F;
    mat->SolveDirect(result,fDecompose);
    if(residual) residual->Redim(F.Rows(),F.Cols());
    break;
  case EMultiply:
    mat->Multiply(F,result);
    if(residual) mat->Residual(result,F,*residual);

  }
}
void TPZStepSolver::ResetSolver() {
  fSolver = ENoSolver;
  fDecompose  = ENoDecompose;
  fNumIterations = 0;
  fTol = 0.;
  fNumVectors = 0;
  fOverRelax = 0.;
  if(fPrecond) delete fPrecond;
  fPrecond = 0;
  fFromCurrent = 0;
}
void TPZStepSolver::SetDirect (const DecomposeType decomp){
  ResetSolver();
  fSolver = EDirect;
  fDecompose = decomp;
}
void TPZStepSolver::SetCG(const int numiterations, const TPZMatrixSolver &pre, const REAL tol, const int FromCurrent){
  ResetSolver();
  fSolver = ECG;
  fNumIterations = numiterations;
  fTol = tol;
  //	fPrecond = &pre;
  if(fPrecond) delete fPrecond;
  fPrecond = pre.Clone();
  fFromCurrent = FromCurrent;
}
void TPZStepSolver::SetGMRES(const int numiterations, const int numvectors, const TPZMatrixSolver &pre, const REAL tol, const int FromCurrent){
  ResetSolver();
  fSolver = EGMRES;
  fNumVectors = numvectors;
  fNumIterations = numiterations;
  fTol = tol;
  //	fPrecond = &pre;
  if(fPrecond) delete fPrecond;
  fPrecond = pre.Clone();
  fFromCurrent = FromCurrent;
}
void TPZStepSolver::SetJacobi(const int numiterations, const REAL tol, const int FromCurrent) {
  ResetSolver();
  fSolver = EJacobi;
  fNumIterations = numiterations;
  fTol = tol;
  fFromCurrent = FromCurrent;
}
void TPZStepSolver::SetSSOR(const int numiterations,const REAL overrelax,const REAL tol,const int FromCurrent) {
  ResetSolver();
  fSolver = ESSOR;
  fOverRelax = overrelax;
  fNumIterations = numiterations;
  fTol = tol;
  fFromCurrent = FromCurrent;
}
void TPZStepSolver::SetSOR(const int numiterations,const REAL overrelax,const REAL tol,const int FromCurrent){
  ResetSolver();
  fSolver = ESOR;
  fNumIterations = numiterations;
  fOverRelax = overrelax;
  fTol = tol;
  fFromCurrent = FromCurrent;
}
void TPZStepSolver::SetMultiply() {
  ResetSolver();
  fSolver = EMultiply;
}
