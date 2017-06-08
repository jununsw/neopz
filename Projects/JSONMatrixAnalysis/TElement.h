/* This file was created by Gustavo ALCALA BATISTELA.
 It is a header file containing the definition of the TElement class
 and the declaration of its members.*/

#ifndef TELEMENT_H
#define TELEMENT_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include <math.h>			// Mathematical operations.

#include "pzfmatrix.h"		// PZ Matrix operations.
#include "TNode.h"          // TNode class.
#include "TMaterial.h"      // TMaterial class.
#include "json.hpp"			// JSON compatibility.

class TStructure;
// TElement class and declarations of its functions.
class TElement {
    
public:
    // Default constructor.
    TElement(const int& Node1ID = -1, const int& Node2ID = -1, const bool& LHinge = false,
             const bool& RHinge = false, const int& MaterialID = -1, TStructure* Structure = nullptr);
    //Copy constructor.
    TElement(const TElement& E);
    // Destructor.
    ~TElement();
    
	// getNodes - accesses the nodes of the element.
	int* const getLocalNodesIDs();
    // getNode1 - accesses node1 of the element.
    int getNode1ID() const;
    // getNode2 - accesses node2 of the element.
    int getNode2ID() const;
    // getHinge1 - accesses the left hinge of the element.
    bool getHinge1() const;
    // getHinge2 - accesses the right hinge of the element.
    bool getHinge2() const;
    // getMaterial - accesses the material of the element.
    int getMaterialID() const;
    // getEquations - accesses the equations of the element.
    int* const getEquations();
    
    // getX1 - accesses the x1 coordinate of the element.
    double getX1() const;
    // getY1 - accesses the y1 coordinate of the element.
    double getY1() const;
    // getX2 - accesses the x2 coordinate of the element.
    double getX2() const;
    // getY2 - accesses the y2 coordinate of the element.
    double getY2() const;
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
    // setNode1 - modifies the first node of the element.
    void setNode1ID(const int& Node1ID);
    // setNode2 - modifies the second node of the element.
    void setNode2ID(const int& Node2ID);
    // setMaterial - modifies the element's material.
    void setMaterialID(const int& MaterialID);
    
    // Assignment operator.
    TElement& operator= (const TElement& E);
    
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