/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TNodalLoad class
 and the declaration of its members. */

#ifndef TNODALLOAD_H
#define TNODALLOAD_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

class TStructure;

// TNodalLoad class and declarations of its functions.
class TNodalLoad {
    
public:
    // Default constructor.
    TNodalLoad(double Fx = 0, double Fy = 0, double M = 0, int NodeID = -1, TStructure* Structure = nullptr);
    // Copy constructor.
    TNodalLoad(const TNodalLoad& NL);
    // Destructor.
    ~TNodalLoad();
	// Assignment operator.
	TNodalLoad& operator= (const TNodalLoad& NL);
    
    // getFx - returns the horizontal load applied to the load.
    double getFx() const;
    // getFy - returns the vertical load applied to the load.
    double getFy() const;
    // getM - returns the moment applied to the load.
    double getM() const;
	// getNodeID - returns the ID of the node the load is applied to.
	int getNodeID() const;
    
    // setFx - modifies the horizontal load applied to the load.
    void setFx(double Fx);
    // setFy - modifies the vertical load applied to the load.
    void setFy(double Fy);
    // setM - modifies the moment applied to the load.
    void setM(double M);
	// setNodeID - modifies the ID of the node the load is applied to.
	void setNodeID(int NodeID);
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
    int fNodeID;
	TStructure* fStructure;
};

#endif