// -*- c++ -*-

//$Id: pzpoisson3d.h,v 1.10 2005-01-28 16:48:37 tiago Exp $

#ifndef MATPOISSON3DH
#define MATPOISSON3DH

#include <iostream>
#include "pzdiscgal.h"
#include "pzfmatrix.h"

//#include "pzmanvector.h"

#ifdef _AUTODIFF
#include "fadType.h"
#endif


class TPZMatPoisson3d : public TPZDiscontinuousGalerkin {
 
  protected :

  /** Forcing function value */
  REAL fXf;
  
  /** Problem dimension */
  int fDim;
  
  /** Coeficient which multiplies the Laplacian operator */
  REAL fK;
  
  /** Variable which multiplies the convection term of the equation */
  REAL fC;
  
  /** Direction of the convection operator */
  REAL fConvDir[3];
  
  /** Symmetry coefficient of elliptic term.
   * Symmetrical formulation - Global element method - has coefficient = -1.
   * Non-symmetrical formulation - Baumann's formulation - has coefficient = +1.
   */
  REAL fSymmetry;
  
  public :

  /** Usado em InterfaceErrors */
  static REAL gAlfa;

  TPZMatPoisson3d(int nummat, int dim);

  virtual ~TPZMatPoisson3d();

  TPZMatPoisson3d(TPZMatPoisson3d &copy) : TPZDiscontinuousGalerkin(copy){
    fXf  = copy.fXf;
    fDim = copy.fDim;
    fK   = copy.fK;
    fC   = copy.fC;
    for (int i = 0; i < 3; i++) fConvDir[i] = copy.fConvDir[i];
    fSymmetry = copy.fSymmetry;

  }
  
  /** Set material elliptic term as the global element method, i.e. the symmetrical formulation.
  */
  void SetSymmetric(){
    this->fSymmetry = -1.0;
  }
  
  /** Set material elliptic term as the Baumann's formulation, i.e. the non-symmetrical formulation.
  */  
  void SetNonSymmetric() {
    this->fSymmetry = +1.0; 
  }

  virtual TPZMaterial *NewMaterial(){
    return new TPZMatPoisson3d(*this);
  }
    
  int Dimension() { return fDim;}

  int NStateVariables();

  void SetParameters(REAL diff, REAL conv, TPZVec<REAL> &convdir);
  
  virtual void Print(ostream & out);
  
  char *Name() { return "TPZMatPoisson3d"; }
  
  virtual void Contribute(TPZVec<REAL> &x,TPZFMatrix &,TPZVec<REAL> &sol,TPZFMatrix &dsol,REAL weight,
			  TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef);
#ifdef _AUTODIFF
  /**Compute contribution to the energy at an integration point*/
  void TPZMatPoisson3d::ContributeEnergy(TPZVec<REAL> &x,
			      TPZVec<FADFADREAL> &sol,
			      TPZVec<FADFADREAL> &dsol,
			      FADFADREAL &U,
			      REAL weight);
#endif

  virtual void ContributeBC(TPZVec<REAL> &x,TPZVec<REAL> &sol,REAL weight,
			    TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);

#ifdef _AUTODIFF

  virtual void ContributeBCEnergy(TPZVec<REAL> & x,
	TPZVec<FADFADREAL> & sol, FADFADREAL &U,
	REAL weight, TPZBndCond &bc);

#endif

  virtual int VariableIndex(char *name);
  
  virtual int NSolutionVariables(int var);
  
  virtual int NFluxes(){ return 3;}
  
  virtual void Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout);
  
  /**compute the value of the flux function to be used by ZZ error estimator*/
  virtual void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux);
  
  void Errors(TPZVec<REAL> &x,TPZVec<REAL> &u,
	      TPZFMatrix &dudx, TPZFMatrix &axes, TPZVec<REAL> &flux,
	      TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values);//Cedric

  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
				   TPZFMatrix &ek,TPZFMatrix &ef);
  
  virtual void ContributeBCInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL, TPZFMatrix &dsolL, REAL weight, TPZVec<REAL> &normal,
			    TPZFMatrix &phiL,TPZFMatrix &dphiL, TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);

  void InterfaceErrors(TPZVec<REAL> &/*x*/,
		       TPZVec<REAL> &leftu, TPZFMatrix &leftdudx, /* TPZFMatrix &leftaxes,*/ 
		       TPZVec<REAL> &rightu, TPZFMatrix &rightdudx, /* TPZFMatrix &rightaxes,*/ 
		       TPZVec<REAL> &/*flux*/,
		       TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values, 
		       TPZVec<REAL> normal, REAL elsize);

  virtual int IsInterfaceConservative(){ return 1;}

};

#endif
