//$Id: pzl2projection.h,v 1.13 2011-03-02 11:41:14 fortiago Exp $

#ifndef PZL2PROJECTION_H
#define PZL2PROJECTION_H

#include "pzmaterial.h"
#include "pzdiscgal.h"

/**
 * Implements an L2 projection to constant solution values.
 * @since April 23, 2007
*/
class TPZL2Projection : public TPZDiscontinuousGalerkin{

protected:

  /** Problem dimension */
  int fDim;

 /** Number of state variables */
  int fNStateVars;

 /** Constant solution vector */
  TPZVec<REAL> fSol;

 /** Argument defining this material is a referred material */
  bool fIsReferred;

  /** Order for setting the integration rule */
  int fIntegrationOrder;

public:

  /** Class constructor 
   * @param id material id
   * @param dim problem dimension
   * @param nstate number of state variables
   * @param sol constant solution vector
   */
  TPZL2Projection(int id, int dim, int nstate, TPZVec<REAL> &sol,
                  int IntegrationOrder = -1);

  /** Class destructor
   */
  ~TPZL2Projection();

  /** Copy constructor
   */
  TPZL2Projection(const TPZL2Projection &cp);

  /** Solution indices of post-processing
    */
  enum ESolutionVars { ENone = 0, ESolution = 1 };

  /** See base class
   */
  virtual void SetIntegrationRule(TPZAutoPointer<TPZIntPoints> rule,
                          int elPMaxOrder,
                          int elDimension);

  /** Contribute method
   */
  virtual void Contribute(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef);

  /** Contribute method
   */
  virtual void Contribute(TPZMaterialData &data, REAL weight, TPZFMatrix &ef)
  {
	  TPZDiscontinuousGalerkin::Contribute(data,weight,ef);
  }


  /** To satisfy base class interface.
   */
  virtual void ContributeInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef){
	//NOTHING TO BE DONE HERE
  }

  /** To satisfy base class interface.
   */
  virtual void ContributeBCInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc){
	//NOTHING TO BE DONE HERE
  }

  /** To satisfy base class interface.
   */
  virtual void ContributeInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ef){
	//NOTHING TO BE DONE HERE
  }

  /** To satisfy base class interface.
   */
  virtual void ContributeBCInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ef,TPZBndCond &bc){
	//NOTHING TO BE DONE HERE
  }

  /** Returns problem dimension
   */
  virtual int Dimension(){ return this->fDim; }

  /** Returns number of state variables
   */
  virtual int NStateVariables(){ return this->fNStateVars; }
      /**
   * It computes a contribution to the stiffness matrix and load vector at one BC integration point.
   * @param data[in] stores all input data
   * @param weight[in] is the weight of the integration rule
   * @param ek[out] is the stiffness matrix
   * @param ef[out] is the load vector
   * @param bc[in] is the boundary condition material
   * @since April 16, 2007
       */
  virtual void ContributeBC(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef, TPZBndCond &bc);
  virtual void ContributeBC(TPZMaterialData &data, REAL weight, TPZFMatrix &ef, TPZBndCond &bc)
  {
      TPZDiscontinuousGalerkin::ContributeBC(data,weight,ef,bc);
  }

  /** Define if material is referred or not */
  void SetIsReferred(bool val);

  /** To create another material of the same type */
  virtual TPZAutoPointer<TPZMaterial> NewMaterial();

  /** It returns the variable index associated with the name */
  virtual int VariableIndex(const std::string &name);

  /** It returns the number of variables associated with the variable
   *  indexed by var.  
   * var is obtained by calling VariableIndex
   */
  virtual int NSolutionVariables(int var);

protected:
  /** It returns the solution associated with the var index based on
   * the finite element approximation
   */
  virtual void Solution(TPZVec<REAL> &Sol, TPZFMatrix &DSol,
						TPZFMatrix &axes, int var, TPZVec<REAL> &Solout);
public:
      /**returns the solution associated with the var index based on
       * the finite element approximation*/
virtual void Solution(TPZMaterialData &data, int var, TPZVec<REAL> &Solout)
{
    TPZMaterial::Solution(data,var,Solout);
}




};

#endif
