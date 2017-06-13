/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TElementLoad class. */

#include "TElementLoad.h"

// Default constructor.
TElementLoad::TElementLoad(double Fx, double Fy, double M, int ElementID, int Node)
    : fFx(Fx), fFy(Fy), fM(M), fElementID(ElementID), fNode(Node) { }

//Copy constructor.
TElementLoad::TElementLoad(const TElementLoad& EL)
    : fFx(EL.fFx), fFy(EL.fFy), fM(EL.fM), fElementID(EL.fElementID), fNode(EL.fNode) { }

// Destructor.
TElementLoad::~TElementLoad() { }

// getFx - returns the horizontal load applied to the load.
double TElementLoad::getFx() const {
    return fFx;
}

// getFy - returns the vertical load applied to the load.
double TElementLoad::getFy() const {
    return fFy;
}

// getM - returns the moment applied to the load.
double TElementLoad::getM() const {
    return fM;
}

// getElementID - returns the ID of the element which the load is applied to.
int TElementLoad::getElementID() const {
    return fElementID;
}

// getNode - returns the local ID (0 or 1) of the node which the load is applied to.
int TElementLoad::getNode() const {
    return fNode;
}

// setFx - modifies the horizontal load applied to the load.
void TElementLoad::setFx(double Fx) {
    fFx = Fx;
}

// setFy - modifies the vertical load applied to the load.
void TElementLoad::setFy(double Fy) {
    fFy = Fy;
}

// setM - modifies the moment applied to the load.
void TElementLoad::setM(double M) {
    fM = M;
}

// setElementID - modifies the ID of the element which the load is applied to.
void TElementLoad::setElementID(int ElementID) {
    fElementID = ElementID;
}

// setNode - modifies the local ID (0 or 1) of the node which the load is applied to.
void TElementLoad::setNode(int Node) {
    fNode = Node;
}

// Assignment operator.
TElementLoad& TElementLoad::operator= (const TElementLoad& EL) {
    if (this != &EL) {
        fFx = EL.fFx;
        fFy = EL.fFy;
        fM = EL.fM;
        fElementID = EL.fElementID;
        fNode = EL.fNode;
    }
    return *this;
}

// Function that prints the load information.
void TElementLoad::print() {
    std::cout << "Element Load Info: " << std::endl
    << " Element ID: " << fElementID << std::endl
    << " Node: " << fNode << std::endl
    << " Fx: " << fFx << std::endl
    << " Fy: " << fFy << std::endl
    << " M: " << fM << std::endl;
    std::cout << std::flush;
}
