/* This file was created by Gustavo ALCALA BATISTELA.
It is a header file containing the definition of the _________
and the declaration of its members.*/

#ifndef TSTRUCTURE_H
#define TSTRUCTURE_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

#include "pzfmatrix.h"		// PZ Matrix operations.
#include "TNode.h"			// TNode class.
#include "TMaterial.h"		// TMaterial class.
#include "TSupport.h"		// TSupport class.
#include "TElement.h"		// TElement class.

// TSupport class and declarations of its functions.
class TStructure {

public:
	// Default constructor.
	TStructure(const std::vector<TNode>& Nodes = {}, const std::vector<TMaterial>& Materials = {},
		const std::vector<TSupport>& Supports = {}, 	const std::vector<TElement>& Elements = {});
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
    
    // getNDOF - returns the number of degrees of freedom of the structure.
    int getNDOF() const;
    // getK - returns the global stiffness matrix.
    TPZFMatrix<double> getK() const;

	// setNodes - modifies the vector of nodes.
	void setNodes(const std::vector<TNode>& Nodes);
	// setMaterials - modifies the vector of materials.
	void setMaterials(const std::vector<TMaterial>& Materials);
	// setSupports - modifies the vector of supports.
	void setSupports(const std::vector<TSupport>& Supports);
	// setElements - modifies the vector of elements.
	void setElements(const std::vector<TElement>& Elements);

private:
	// Member variables:
	std::vector<TNode> fNodes;
	std::vector<TMaterial> fMaterials;
	std::vector<TSupport> fSupports;
	std::vector<TElement> fElements;
};

#endif