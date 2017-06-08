/* This file was created by Gustavo ALCALA BATISTELA.
 It is a header file containing the definition of the TNodalLoad class
 and the declaration of its members.*/

#ifndef TNODALLOAD_H
#define TNODALLOAD_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

// TNodalLoad class and declarations of its functions.
class TNodalLoad {
    
public:
    // Default constructor.
    TNodalLoad(int NodeID = -1, double Fx = 0, double Fy = 0, double M = 0);
    //Copy constructor.
    TNodalLoad(const TNodalLoad& N);
    // Destructor.
    ~TNodalLoad();
    
    // getFx - returns the horizontal load applied to the load.
    double getFx() const;
    // getFy - returns the vertical load applied to the load.
    double getFy() const;
    // getM - returns the moment applied to the load.
    double getM() const;
    
    // setFx - modifies the horizontal load applied to the load.
    void setFx(double Fx);
    // setFy - modifies the vertical load applied to the load.
    void setFy(double Fy);
    // setM - modifies the moment applied to the load.
    void setM(double M);
    
    // Assignment operator.
    TNodalLoad& operator= (const TNodalLoad& N);
    
    // Function that prints the load information.
    void print();
    
private:
    // Member variables:
    double fFx;
    double fFy;
    double fM;
    int fNodeID;
};

#endif