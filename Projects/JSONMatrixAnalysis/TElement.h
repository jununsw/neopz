/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TElement class
 and the declaration of its members. */

#ifndef TELEMENT_H
#define TELEMENT_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include <math.h>			// Mathematical operations.

#include "pzfmatrix.h"		// PZ Matrix operations.
#include "TNode.h"          // TNode class.
#include "TMaterial.h"      // TMaterial class.
#include "json.hpp"			// JSON compatibility.

// Reference to TStructure class.
class TStructure;

// TElement class and declarations of its functions.
class TElement {
    
public:
    // Default constructor.
    TElement(const int Node0ID = -1, const int Node1ID = -1, const bool Hinge0 = false,
             const bool Hinge1 = false, const int MaterialID = -1, TStructure* Structure = nullptr);
    // Copy constructor.
    TElement(const TElement& E);
    // Destructor.
    ~TElement();
	// Assignment operator.
	TElement& operator= (const TElement& E);
    
	// getNodes - accesses the nodes of the element.
	int* const getLocalNodesIDs();
    // getNode0ID - accesses the ID of the element's Node 0.
    int getNode0ID() const;
    // getNode1ID - accesses the ID of the element's Node 0.
    int getNode1ID() const;
    // getHinge0 - accesses the hinge of the element at Node 0.
    bool getHinge0() const;
    // getHinge1 - accesses the hinge of the element at Node 1.
    bool getHinge1() const;
    // getMaterial - accesses the material of the element.
    int getMaterialID() const;
    // getEquations - accesses the equations of the element.
    int* const getEquations();
    
    // getX0 - accesses the x coordinate of the element's Node 0.
    double getX0() const;
    // getY0 - accesses the y coordinate of the element's Node 0.
    double getY0() const;
    // getX1 - accesses the x coordinate of the element's Node 1.
    double getX1() const;
    // getY1 - accesses the y coordinate of the element's Node 1.
    double getY1() const;
    // getL - accesses the element length.
    double getL() const;
    // getCos - accesses the element cosine.
    double getCos() const;
    // getSin - accesses the element sine.
    double getSin() const;
    
    // getT - evaluates the transformation matrix of the element.
    TPZFMatrix<double> getT() const;
    // getT - evaluates the stiffness matrix of the element.
    TPZFMatrix<double> getK() const;
    // setEquations - modifies the element's equations indexes.    
    void setEquations(int Eq1, int Eq2, int Eq3, int Eq4, int Eq5, int Eq6);
    
	// setLocalNodesIDs - modifies the IDs of the nodes.
	void setLocalNodesIDs(int* NodesIDs);
    // setNode0ID - modifies the element's Node 0.
    void setNode0ID(int Node0ID);
    // setNode1ID - modifies the element's Node 1.
    void setNode1ID(int Node1ID);
    // setMaterial - modifies the element's material.
    void setMaterialID(int MaterialID);
    // setHinge0 - modifies the element's hinge at Node 0.
    void setHinge0(bool Hinge0);
    // setHinge1 - modifies the element's hinge at Node 1.
    void setHinge1(bool Hinge1);
    // setStructure - modifies the element's Structure.
	void setStructure(TStructure* Structure);
    
    // Function that prints the element information.
    void print();
    
private:
    // Member variables:
	int fLocalNodesIDs[2];
    bool fHinges[2];
    int fMaterialID;
    TStructure* fStructure;
    int fEquations[6];

};

#endif