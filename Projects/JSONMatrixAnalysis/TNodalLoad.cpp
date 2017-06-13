/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TNodalLoad class. */

#include "TNodalLoad.h"

// Default constructor.
TNodalLoad::TNodalLoad(double Fx, double Fy, double M, int NodeID) : fFx(Fx), fFy(Fy), fM(M), fNodeID(NodeID) { }

//Copy constructor.
TNodalLoad::TNodalLoad(const TNodalLoad& NL) : fFx(NL.fFx), fFy(NL.fFy), fM(NL.fM), fNodeID(NL.fNodeID) { }

// Destructor.
TNodalLoad::~TNodalLoad() { }

// getFx - returns the horizontal load applied to the load.
double TNodalLoad::getFx() const {
    return fFx;
}

// getFy - returns the vertical load applied to the load.
double TNodalLoad::getFy() const {
    return fFy;
}

// getM - returns the moment applied to the load.
double TNodalLoad::getM() const {
    return fM;
}

// setFx - modifies the horizontal load applied to the load.
void TNodalLoad::setFx(double Fx) {
    fFx = Fx;
}

// setFy - modifies the vertical load applied to the load.
void TNodalLoad::setFy(double Fy) {
    fFy = Fy;
}

// setM - modifies the moment applied to the load.
void TNodalLoad::setM(double M) {
    fM = M;
}

// Assignment operator.
TNodalLoad& TNodalLoad::operator= (const TNodalLoad& NL) {
    if (this != &NL) {
        fFx = NL.fFx;
        fFy = NL.fFy;
        fM = NL.fM;
        fNodeID = NL.fNodeID;

    }
    return *this;
}

// Function that prints the load information.
void TNodalLoad::print() {
    std::cout << "Nodal Load Info: " << std::endl
    << " Fx: " << fFx << std::endl
    << " Fy: " << fFy << std::endl
    << " M: " << fM << std::endl
    << " Node ID: " << fNodeID << std::endl;
    std::cout << std::flush;
}