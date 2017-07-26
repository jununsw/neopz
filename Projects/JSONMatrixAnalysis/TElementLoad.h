/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TElementLoad class
 and the declaration of its members. */

#ifndef TELEMENTLOAD_H
#define TELEMENTLOAD_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

class TStructure;

// TElementLoad class and declarations of its functions.
class TElementLoad {
    
public:
    // Default constructor.
    TElementLoad(double Fx = 0, double Fy = 0, double M = 0, int ElementID = -1, int Node = -1, TStructure* Structure = nullptr);
    // Copy constructor.
    TElementLoad(const TElementLoad& EL);
    // Destructor.
    ~TElementLoad();
	// Assignment operator.
	TElementLoad& operator= (const TElementLoad& EL);
    
    // getFx - returns the horizontal load applied to the load.
    double getFx() const;
    // getFy - returns the vertical load applied to the load.
    double getFy() const;
    // getM - returns the moment applied to the load.
    double getM() const;
    // getElementID - returns the ID of the element which the load is applied to.
    int getElementID() const;
    // getNode - returns the local ID (0 or 1) of the node which the load is applied to.
    int getNode() const;
    
    // setFx - modifies the horizontal load applied to the load.
    void setFx(double Fx);
    // setFy - modifies the vertical load applied to the load.
    void setFy(double Fy);
    // setM - modifies the moment applied to the load.
    void setM(double M);
    // setElementID - modifies the ID of the element which the load is applied to.
    void setElementID(int ElementID);
    // setNode - modifies the local ID (0 or 1) of the node which the load is applied to.
    void setNode(int Node);
	// setStructure - modifies the element's Structure.
	void setStructure(TStructure* Structure);

	// store - adds the effects of the nodal load to the vector of loads.
	void store(TPZFMatrix<double>& L);
    
    // Function that prints the load information.
    void print();
    
private:
    // Member variables:
    double fFx;
    double fFy;
    double fM;
    int fElementID;
    int fNode;
	TStructure* fStructure;
};

#endif