/* This file was created by Gustavo BATISTELA.
It is a header file containing the definition of the TStructure class
and the declaration of its members. */

#ifndef TSTRUCTURE_H
#define TSTRUCTURE_H

#include <iostream>
#include "pzfmatrix.h"
#include "TNode.h"
#include "TMaterial.h"
#include "TSupport.h"
#include "TElement.h"
#include "TNodalLoad.h"
#include "TDistributedLoad.h"
#include "TElementLoad.h"

// TStructure class and declarations of its functions.
class TStructure {
public:
	// Default constructor.
	TStructure(const std::vector<TNode>& Nodes = {},
			   const std::vector<TMaterial>& Materials = {},
			   const std::vector<TSupport>& Supports = {},
		       const std::vector<TElement>& Elements = {});
	// Copy constructor.
	TStructure(const TStructure& S);
	// Destructor.
	~TStructure();

	// getNodes - returns the vector of nodes.
	std::vector<TNode> getNodes() const;
	// getMaterials - returns the vector of materials.
	std::vector<TMaterial> getMaterials() const;
	// getSupports - returns the vector of supports.
	std::vector<TSupport> getSupports() const;
	// getElements - returns the vector of elements.
	std::vector<TElement> getElements() const;
	// getElement - returns an element.
	TElement getElement(int elementID) const;
	// getSupport - returns a support ID by giving the reference node ID.
	int getSupportID(int NodeID);
	// getNodeEquations - returns the matrix of DOFs of the nodes.
	TPZFMatrix<int> getNodeEquations() const;

	// setNodes - modifies the vector of nodes.
	void setNodes(const std::vector<TNode>& Nodes);
	// setMaterials - modifies the vector of materials.
	void setMaterials(const std::vector<TMaterial>& Materials);
	// setSupports - modifies the vector of supports.
	void setSupports(const std::vector<TSupport>& Supports);
	// setElements - modifies the vector of elements.
	void setElements(const std::vector<TElement>& Elements);

	// getNDOF - returns the number of DOF of the structure.
	int getNDOF() const;
	// getCDOF - returns the number of constrained DOF of the structure.
	int getCDOF() const;
	// getUDOF - returns the number of unconstrained DOF of the structure.
	int getUDOF() const;
	// enumerateEquations - enumerates the DOF associated with each element.
	void enumerateEquations();

	// getK - returns the global stiffness matrix.
	TPZFMatrix<double> getK() const;
	// getK11 - returns the left upper block of the global stiffness matrix K.
	TPZFMatrix<double> getK11() const;
	// getK21 - returns the left down block of the global stiffness matrix K.
	TPZFMatrix<double> getK21() const;

	// getQ - returns the vector of loads with null elements.
	TPZFMatrix<double> getQ();
	// calculateQK - calculates the known external loads and stores them in the
	// vector of global forces Q.
	void calculateQK(TPZFMatrix<double>& Q, std::vector<TNodalLoad>& nLoads,
				     std::vector<TDistributedLoad>& dLoads,
					 std::vector<TElementLoad>& eLoads);
	// getD - returns the vector of displacements with null elements.
	TPZFMatrix<double> getD();
	// calculateDU - solves the unknown displacements DU and stores them in the
	// vector of global displacements D.
	void calculateDU(TPZFMatrix<double>& Q, TPZFMatrix<double>& D);
	// calculateQU - solves the support reactions and stores them in the vector of
	// global forces Q.
	void calculateQU(TPZFMatrix<double>& Q, TPZFMatrix<double>& D);

private:
	// fNodes - vector containing the structure nodes.
	std::vector<TNode> fNodes;
	// fMaterials - vector containing the available materials.
	std::vector<TMaterial> fMaterials;
	// fSupports - vector containing the structure supports.
	std::vector<TSupport> fSupports;
	// fElements - vector containing the structure elements.
	std::vector<TElement> fElements;
	// fNodeEquations - matrix containing the DOFs of the nodes.
	TPZFMatrix<int> fNodeEquations;
};

#endif