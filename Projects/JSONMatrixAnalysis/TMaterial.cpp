/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TMaterial class.*/

#include "TMaterial.h"

// Default constructor.
TMaterial::TMaterial(double E, double A, double I) : fE(E), fA(A), fI(I) { }

// Copy constructor.
TMaterial::TMaterial(const TMaterial& M) : fE(M.fE), fA(M.fA), fI(M.fI) { }

// Destructor.
TMaterial::~TMaterial() { }

// getE - accesses the Young's Modulus of the material.
double TMaterial::getE() const {
    return fE;
}
// getA - accesses the cross-sectional area of the material.
double TMaterial::getA() const {
    return fA;
}
// getI - accesses the second moment of area I.
double TMaterial::getI() const {
    return fI;
}

// setE - modifies the Young's Modulus of the material.
void TMaterial::setE(double E) {
    fE = E;
}
// setA - modifies the cross-sectional area of the material.
void TMaterial::setA(double A) {
    fA = A;
}
// setI - modifies the value of I.
void TMaterial::setI(double I) {
    fI = I;
}

// Assignment operator.
TMaterial& TMaterial::operator=(const TMaterial& M) {
    if (this != &M) {
        fE = M.fE;
        fA = M.fA;
        fI = M.fI;
    }
    return *this;
}

// Function that prints the information of a node.
void TMaterial::print() {
    std::cout << "Material Properties:" << std::endl
		<< " E: " << this->getE() << std::endl
		<< " A: " << this->getA() << std::endl
		<< " I: " << this->getI() << std::endl;
    std::cout << std::flush;
}