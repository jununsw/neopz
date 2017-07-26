/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TMaterial class
 and the declaration of its members. */

#ifndef TMATERIAL_H
#define TMATERIAL_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

// TMaterial class and declarations of its functions.
class TMaterial {
    
public:
    // Default constructor.
    TMaterial(double E = 0, double A = 0, double I = 0);
    // Copy constructor.
    TMaterial(const TMaterial& M);
    // Destructor.
    ~TMaterial();
	// Assignment operator.
	TMaterial& operator= (const TMaterial& M);
    
    // getE - accesses the Young's Modulus of the material.
    double getE() const;
    // getA - accesses the cross-sectional area of the material.
    double getA() const;
    // getIx - accesses the second moment of area Ix.
    double getI() const;
    
    // setE - modifies the Young's Modulus of the material.
    void setE(double E);
    // setA - modifies the cross-sectional area of the material.
    void setA(double A);
    // setIx - modifies the second moment of area Ix.
    void setI(double I);
    
    // Function that prints the material information.
    void print();
    
private:
    // Member variables:
    double fE;
    double fA;
    double fI;
};

#endif