// -*- c++ -*-

// $ Id: $
#ifndef PZINTEL_H
#define PZINTEL_H

#include "pzcompel.h"
struct TPZElementMatrix;

class TPZIntPoints;
class TPZBlockDiagonal;


/**
 * @brief Class TPZInterpolatedElement implemments computational element based on an interpolation space

 * all h and p adaptive methods are implemented in this class\n
 * derived classes need to implement the behaviour (interface) defined in this class\n
 * this makes the adaptive process extremely general\n
 * @ingroup CompElement
 */
class TPZInterpolatedElement : public TPZCompEl {

 protected:

  /**
   * Geometric element to which this element refers
   */
  TPZGeoEl *fReference;
  /**
   * Material object of this element
   */
  TPZMaterial *fMaterial;

  /**
   * Updates the interpolation order of all neighbouring elements along side
   * to have side order equal to the side order of the current element
   * @param side of the the element
   * @param elvec vector of elements whose side order will be set
   */
  void UpdateNeighbourSideOrder(int side, TPZVec<TPZCompElSide> &elvec);

  /**
   * Computes the minimum interpolation order of the elements contained in elementset
   * this method is used to identify the side order of a set of equal level elements
   * @param elementset vector of element/sides which will be used to compute the maximum order
   * @return resulting side order
   */
  static int ComputeSideOrder(TPZVec<TPZCompElSide> &elementset);


public:
  /**
   * Constructor with a mesh and geometric element as arguments
   * @param mesh mesh object into which the element will insert itself
   * @param reference reference object to which this element will refer
   * @param index index in the vector of elements of mesh where this element was inserted
   */
  TPZInterpolatedElement(TPZCompMesh &mesh, TPZGeoEl *reference, int &index);

  /**
   * Constructor aimed at creating a copy of an interpolated element within a new mesh
   */
  TPZInterpolatedElement(TPZCompMesh &mesh, const TPZInterpolatedElement &copy);
  /**
   * destructor, does nothing
   */
  virtual ~TPZInterpolatedElement();

  /** @name data access methods
   * methods which allow to access the internal data structure of the element
   */
  //@{

  /** 
   * @return return the index of the material
   */
  int MaterialId() const;

  /**
   * Print the relevant data of the element to the output stream
   */
  virtual void Print(ostream &out = cout);

  /**return the geometric element to which this element references*/
  virtual TPZGeoEl *Reference() const { return fReference;}

  /**declare the element as interpolated or not.
   * You may not redefine this method, because a lot of "unsafe" casts depend
   * on the result of this method\n
   * Wherever possible, use dynamic_cast instead of this method
   * @return 0 if the element is not interpolated
   */
  virtual int IsInterpolated() const {return 1;}

  /**returns the number of shapefunctions associated with a connect*/
  virtual int NConnectShapeF(int inod) const  = 0;

  /**returns the total number of shapefunctions*/
  int NShapeF() const;
  
  /**returns the number of shape functions on a side*/
  int NSideShapeF(int side) const;

  /**return the number of dof nodes along side iside*/
  virtual int NSideConnects(int iside) const = 0;

  /**
   * returns the local node number of icon along is
   * @param icon connect number along side is
   * @is side which is being queried
   */
  virtual int SideConnectLocId(int icon,int is) const  = 0;

  /**
   * returns the local id of the connect in the middle of the side
   * @param is side which is being queried
   */
  virtual int MidSideConnectLocId(int is) const;
  
  /**return the index of the c th connect object along side is*/
  int SideConnectIndex(int icon,int is) const;

  /**
   * return a pointer to the icon th connect object along side is
   */
  TPZConnect *SideConnect(int icon,int is) const;

  /**
   * returns the dimension of the element
   */
  virtual int Dimension() const = 0;

  /**return the number of corner connects of the element*/
  virtual int NCornerConnects() const = 0;

  /**return the number of connect objects of the element*/
  virtual int NConnects() const = 0;
	
  /**identify the material object associated with the element*/
  TPZMaterial *Material() const {return fMaterial;}


  /**
   * Identifies the interpolation order of all connects of the element different from the corner connects
   * Note there is a diference between the actual side order returned by this method
   * and the side order which the element naturally would have, which is returned by
   * PreferredSideOrder
   * @param ord vector which will be filled with the side orders of the element
   */
  virtual void GetInterpolationOrder(TPZVec<int> &ord) const  = 0;

  /**return the preferred order of the polynomial along
     side iside*/
  virtual int PreferredSideOrder(int iside) const = 0;

  /** adjusts the preferredSideOrder for faces 
   * @param side : side for which the order needs adjustment
   * @param order : original order which has to be compared with the sides
  */
  int AdjustPreferredSideOrder(int side, int order) const;

  /**returns the actual interpolation order of the polynomial along the side*/
  virtual int SideOrder(int side) const = 0;

  //@}

  /**
   * @name Data modification methods
   * These methods which will modify the local datastructure of the element
   */
  //@{

  /**
   * Sets the node pointer of node i to nod
   */
  virtual void SetConnectIndex(int i, int connectindex)=0;

  /**set the material of the element*/
  virtual void SetMaterial(TPZMaterial *mat) {fMaterial = mat;}

  virtual void SetIntegrationRule(int order) {
    cout << "TPZInterpolatedElement::SetIntegrationRule called\n";
  }
  
  /**Sets the interpolation order for the interior of the element*/
  //  virtual void SetInterpolationOrder(TPZVec<int> &ord) = 0;

  /**Sets the preferred interpolation order along a side
   * This method only updates the datastructure of the element
   * In order to change the interpolation order of an element, use the method PRefine
   */
  virtual void SetPreferredSideOrder(int order) = 0;

public:
  /**
   * Sets the interpolation order of side to order
   * This method only updates the datastructure of the element and
   * updates the blocksize of the associated connect object 
   * @note DO NOT CALL THIS METHOD
   */
  virtual void SetSideOrder(int side, int order) = 0;

  /**
   * Impose an interpolation order on a given side (without using computesideorder)
   */
  virtual void ForceSideOrder(int side, int order);
public:

  //@}

  /**
   * @name Computational methods
   * Methods used to perform computations on the interpolated element
   */
  //@{

  /**
   * CalcStiff computes the element stiffness matrix and right hand side
   * @param ek element matrix
   * @param ef element right hand side
   */
  virtual void CalcStiff(TPZElementMatrix &ek, TPZElementMatrix &ef);

#ifdef _AUTODIFF

  /**
   * @name Computational methods
   * Methods used to perform computations on the interpolated element
   */
  //@{

  /**
   * CalcEnergy computes the element stiffness matrix and right hand side
   * by the energy.
   * @param ek element matrix
   * @param ef element right hand side
   */
  virtual void CalcEnergy(TPZElementMatrix &ek, TPZElementMatrix &ef);

#endif

  /**
   * CalcResidual only computes the element residual
   * @param ef element residual
   */
  virtual void CalcResidual(TPZElementMatrix &ef);

  /**
   * This method computes only the block diagonal entries of the element stiffness matrix
   * and puts the result in the block matrix which is passed as argument. The blocks in the block
   * matrix object correspond to the connect indices of connectlist
   * @param connectlist (output) the connects to which the element will contribute
   * @param block (output) block diagonal matrix which contains the contributions of the element
   */
  virtual void CalcBlockDiagonal(TPZStack<int> &connectlist, TPZBlockDiagonal & block);

  /**returns a reference to an integration rule suitable for integrating
     the interior of the element*/
  virtual TPZIntPoints &GetIntegrationRule() = 0;

  /**
   * Allocates dynamically an integration rule adequate for the side
   * the caller to the method needs to call the delete method!
   * this method is being migrated to the geometric element
   * @param side side along which an integration rule is created
   * @return dynamically allocated integration rule
   */
//  virtual TPZIntPoints *CreateSideIntegrationRule(int side) = 0;

  /**computes the shape function set at the point x. This method uses the order of interpolation
   * of the element along the sides to compute the number of shapefunctions
   * @param x point in master element coordinates
   * @param phi vector of values of shapefunctions, dimension (numshape,1)
   * @param dphi matrix of derivatives of shapefunctions, dimension (dim,numshape)
   */
  virtual void Shape(TPZVec<REAL> &x,TPZFMatrix &phi,TPZFMatrix &dphi) = 0;
  
  /**compute the values of the shape function along the side*/
  virtual void SideShapeFunction(int side, TPZVec<REAL> &point, TPZFMatrix &phi, TPZFMatrix &dphi) = 0;

  //@}

  /**
   * @name Post processing methods
   * The TPZInterpolatedElement class provides the user with a variety of methods for post-processing :\n
   * Methods for error evaluation\n
   * Methods for computing derived post processed values (depending on the variational statement)\n
   */
  //@{

  /**
   * Will project the flux associated with the variational statement onto the finite element interpolation space
   * The ek matrix corresponds to an L2 (scalar) projection, the ef matrix contains multiple right hand sides, one
   * for each component of the flux
   * @param ek projection matrix
   * @param ef inner product of the flux with the finite element interpolation space
   */
  void ProjectFlux(TPZElementMatrix &ek, TPZElementMatrix &ef);

  /**
   * Generic method for error evaluation, based on a function pointer which evaluates to the exact solution and
   * a matrix pointer containing the projected flux values
   * @param fp function which computes the exact solution, if fp=0, this implies that an exact solution is not available
   * @param true_error (output) Energy error computed as the diference between the finite element interpolation and the exact solution
   * @param L2_error (output) L2 error computed as the diference between the finite element interpolation and the exact solution
   * @param flux (input) projected flux function. if flux=0, this means the projected flux is not available
   * @param estimate (output) estimated energy error based on the projected flux function
   * @see TPZMaterial::Flux
   * @see TPZMaterial::NFluxes
   * @see TPZMaterial::Errors which computes the error contributions at an integration point
   */
  void EvaluateError(void (*fp)(TPZVec<REAL> &loc, TPZVec<REAL> &val, TPZFMatrix &deriv),
  		     REAL &true_error, REAL &L2_error, TPZBlock *flux, REAL &estimate);

  /**
   * Post processing method which computes the solution for the var post processed variable. The var index is obtained
   * by calling the TPZMaterial::VariableIndex method with a post processing name
   * @param qsi coordinate of the point in master element space where the solution will be evaluated
   * @param var variable which will be computed
   * @param sol (output) solution computed at the given point
   * @see TPZMaterial::VariableIndex
   * @see TPZMaterial::NSolutionVariables
   * @see TPZMaterial::Solution
   */
  void Solution(TPZVec<REAL> &qsi,int var,TPZManVector<REAL> &sol);

  /**
   * Compare the L2 norm of the difference between the �var� solution of the current element with 
   * the �var� solution of the element which is pointed to by the geometric element
   * In order to use this method, call ResetReference on the geometric mesh to which the geometric reference element belongs
   * and call LoadReference on the mesh of the element with which to compare the solution
   * This method only computes the error is the name of the material is matname
   * @param var variable index indicating which difference is being integrated
   * @param matname reference material name
   */
  virtual REAL CompareElement(int var, char *matname);

  //@}

  /**
   * @name Adaptivity methods
   * methods which help to implement the adaptive process
   */
  //@{

  /**
   * Implement the refinement of an interpolated element
   * Divides the current element into subelements. Inserts the subelements in the mesh of the element
   * and returns their indices
   * @param index (input) index of the current element in the mesh
   * @param pv (output) indices of the subelements (they are already inserted in the mesh)
   * @param interpolatesolution (input) if == 1 the solution of the original element is projected on the
   * solution of the subelements
   * @see PRefine
   * @note This is the user interface to adaptive refinement of this class
   */
  void Divide(int index,TPZVec<int> &pv,int interpolatesolution = 0);



  /**
   * Changes the interpolation order of a side. Updates all constraints and block sizes\n
   * This call will not �necessarily� modify the interpolation order of the side. The interpolation
   * order of neighbouring elements need to remain compatible. The actual order is obtained by calling
   * ComputeSideOrder
   * @param order interpolation order which the user requests
   * @see ComputeSideOrder
   * @note This is the user interface to adaptive refinement of this class
   */
  void PRefine(int order);

 /**compute the shapefunction restraints which need to be applied to the shape functions
     on the side of the element*/
  virtual void RestrainSide(int side, TPZInterpolatedElement *neighbour, int neighbourside);

  /**
     @enum MInsertMode
   * Defines a flag indicating the state of creation/deletion of the element
   * This has an impact on how constraints are being computed
   * @param EInsert The element is being inserted
   * @param EDelete The element is being deleted
   */
  enum MInsertMode {EInsert,EDelete};

  /**delete the restraints on the nodes of the connected elements if necessary
     Depending on the insert mode, the neighbouring elements will need to
     recompute their constraints
     insert indicates whether the element will be refined, coarsened, inserted or deleted
     This method is only called during deletion
   * @param mode indicates insertion or deletion of element
   * @note THIS IS A VERY TRICKY METHOD
   */
  virtual void RemoveSideRestraintsII(MInsertMode mode);

  /**
   * Removes the side restraints of the current element along side with respect to neighbour/side
   * This method checks (extensively) if the relative positions between both elements makes sense
   * @param side side of the current element which contains the constrained connect
   * @param neighbour element/side with respect to which the connect is restrained
   */
  virtual void RemoveSideRestraintWithRespectTo(int side, const TPZCompElSide &neighbour);

  /**
   * Will recompute the restraints of all connects which are restrained by this side
   * This method will be called for a side when a connected lower dimension side is
   * changing order
   * @param side side of the large element
   */
  void RecomputeRestraints(int side);

  /**
   * Accumulates the transfer coefficients between the current element and the
   * coarse element into the transfer matrix, using the transformation t
   * This method forms the basis for the multigrid method
   * @param coarsel larger element with respect to which the transfer matrix is computed
   * @param t transformation which maps the master element space of the current element into the master element space of the coarse element
   * @param transfer transfer matrix mapping the solution of the coarse mesh into the fine mesh
   */
  virtual void BuildTransferMatrix(TPZInterpolatedElement &coarsel, TPZTransform &t, TPZTransfer &transfer);

  /**interpolates the solution into the degrees of freedom nodes from the degrees
     of freedom nodes from the coarse element*/
  virtual void InterpolateSolution(TPZInterpolatedElement &coarse);


  /**
   * Verify the neighbours of the element and create a node along this side
   *  if necessary
   *  this method returns the index of the newly created node
   * @note This is the central method for h-p adaptivity : the constructor of the element
   * simply calls CreateMidSideConnect which does all the necessary adjustments
   */
  virtual int CreateMidSideConnect(int side);

  /**
   * Checks if the side order is consistent with the preferred side order and
   * with the constraints and recomputes the constraints if necessary
   * calls IdentifySideOrder on higher level (i.e. smaller) connected elements recursively
   */
  virtual void IdentifySideOrder(int side);

  //@}

  /**
   * @name Data consistency methods
   * These methods are used during the debugging phase and check the consistency of the data structures
   */

  //@{

  /**Check the consistency of the constrained connects along a side*/
  void CheckConstraintConsistency(int side);

  /**Check the consistency of the constrained connects for all sides*/
  void CheckConstraintConsistency();

  /**
   * Checks element data structure consistancy
   */
  int CheckElementConsistency();

  /**
   * Compare the shape functions of sides of an element
   * @param phis: small side function values
   * @param dphis: small side gradient function values
   * @param phil: large side function values
   * @param dphil: large side gradient function values
   * @param transform: transformation matrix from large side to small side
   */
  int CompareShapeF(int sides, int sidel, TPZFMatrix &phis, TPZFMatrix &dphis, TPZFMatrix &phil, TPZFMatrix &dphil, TPZTransform &transform);

  /**
   * Returns the transformation which transform a point from the side to the interior of the element
   * @param side side from which the point will be tranformed (0<=side<=2)
   * @return TPZTransform object
   * @see the class TPZTransform
   */
  virtual TPZTransform TransformSideToElement(int side) = 0;
  
  //@}

  /**
   * @name Discontinuous Galerkin Methods
   * These methods query the element whether it can make a connect discontinuous or not
   */

  //@{

  /**
   * Method which indicates whether a given connect is continuous or not.\n
   * For continuous elements this method always returns 1.\n
   * If the derived class can to be discontinuous this
   *  method need to be reimplemented.
   */
  virtual int IsConnectContinuous(const int icon) { return 1; }
  /**
   * Method which indicates whether a connect can be made discontinuous\n
   * For standard elements, this method always returns 0
   * By default the computational elements are continuous
   */
  virtual int CanBeDiscontinuous() { return 0; }

  /** 
   * Method to force the given connect to be discontinuous. This method will generate an
   * error message if the current element does not support this feature 
   * Set fConnectDisc[i] 1 to indicate the connect i is to be made discontinuous and
   * set the correct positions to point to shape functions
   * @param i index of the connect
   */
  virtual void SetConnectDiscontinuous(int i);

  /**
   * Method to make the connect continuous
   * For traditional elements, this method does nothing
   * @param i index of the connect
   */
  virtual void SetConnectContinuous(int i,TPZFMatrix &mat);


  //@}

private:

  /**
   * Auxiliary method to expand a vector of shapefunctions and their derivatives to acount for constraints
   * As input the regular values of the shapefunctions are given and their derivatives\n
   * if these shapefunctions are dependent upon other shapefunctions (because of constraints) then the vectors
   * are expanded to include the value of the independent shapefunctions and their derivatives as well
   * @param (input) connectlist vector of all connects to which the element will contribute
   * @param (input) dependencyorder vector of indices which indicate the order in which the connects will be processed
   * @param blocksizes (output) number of shapefunctions associated with each connect
   * @param phi (input/output) values of the shapefunctions
   * @param dphi (input/output) values of the derivatives of the shapefunctions
   */
  void ExpandShapeFunctions(TPZVec<int> &connectlist, TPZVec<int> &dependencyorder, TPZVec<int> &blocksizes, TPZFMatrix &phi, TPZFMatrix &dphi);

public:

#ifdef _AUTODIFF
  /**
   * This method fills the matrix ek and load vector ef
   * with the data in the FADFADREAL U second and first derivatives.
   *
   */
 static void FADToMatrix(FADFADREAL &U, TPZFMatrix & ek, TPZFMatrix & ef);
#endif


  /** 
	 * To enable to work with discontinuous element that can to have interface
	 * elements*/
  virtual void SetInterface(int /*side*/, int /*index*/) { }
  virtual int Interface(int /*side*/) { return -1; }
  virtual int CanToHaveInterface() { return 0; }
  virtual void DeleteInterfaces() { }
  /** Return total mass contained into the element */
  REAL MeanSolution(int var);
  /** Compute the integral over the finite element */
  void CalcIntegral(TPZElementMatrix &ef);
  /** To make continuous or discontinuous a connect of the current element and neighbours*/
  virtual void MakeConnectContinuous(int con);
  virtual void MakeConnectDiscontinuous(int con);

};

inline void TPZInterpolatedElement::SetConnectDiscontinuous(int /*i*/) {
  cerr << "TPZInterpolatedElement::SetConnectDiscontinuous is called.\n";
}
inline void TPZInterpolatedElement::SetConnectContinuous(int /*i*/,TPZFMatrix &/*mat*/) {
}
inline void TPZInterpolatedElement::MakeConnectDiscontinuous(int icon) {
  cerr << "TPZInterpolatedElement::MakeConnectDiscontinuous is called, Permission denied.\n";
}

#endif
