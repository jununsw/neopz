/* This file was created by Gustavo ALCALA BATISTELA.
 It is a header file containing the definition of the TSupport class
 and the declaration of its members.*/

#ifndef TSUPPORT_H
#define TSUPPORT_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.
#include "TNode.h"          // TNode class.

class TStructure;
// TSupport class and declarations of its functions.
class TSupport {
    
public:
    // Default constructor.
    TSupport(const bool Fx = false, const bool Fy = false, const bool Mx = false, 
		const int NodeID = 0, TStructure* Structure = nullptr);
    //Copy constructor.
    TSupport(const TSupport& S);
    // Destructor.
    ~TSupport();
    
    // getFx - accesses the Fx DOF of the support.
    bool getFx() const;
    // getFy - accesses the Fy DOF of the support.
    bool getFy() const;
    // getMx - accesses the Mx DOF of the support.
    bool getMx() const;
	// getConditions - accesses the vector of suport conditions.
	bool* const getConditions();
    // getNode - accesses the node of the support.
    int getNodeID() const;
    
    // setFx - modifies the Fx DOF of the support.
    void setFx(const bool Fx);
    // setFy - modifies the Fy DOF of the support.
    void setFy(const bool Fy);
    // setMx - modifies the Mx DOF of the support.
    void setMx(const bool Mx);
    // setNode - modifies the node of the support.
    void setNodeID(const int NodeID);
    
    // Assignment operator.
    TSupport& operator= (const TSupport& S);
    
    // Function that prints the support information.
    void print();
    
private:
    // Member variables:
    bool fConditions[3];
    int fNodeID;
    TStructure *fStructure;
};

#endif