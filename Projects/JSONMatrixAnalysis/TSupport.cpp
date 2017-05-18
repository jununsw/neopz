/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of _ classes.*/

#include "TSupport.h"
#include "TStructure.h"

// Default constructor.
TSupport::TSupport(const bool Fx, const bool Fy, const bool Mx, 
	const int NodeID, TStructure* Structure) {
    fConditions[0] = Fx;
    fConditions[1] = Fy;
    fConditions[2] = Mx;
    fNodeID = NodeID;
    fStructure = Structure;
}

//Copy constructor.
TSupport::TSupport(const TSupport& S) {
    fConditions[0] = S.fConditions[0];
    fConditions[1] = S.fConditions[1];
    fConditions[2] = S.fConditions[2];
    fNodeID = S.fNodeID;
    fStructure = S.fStructure;
}

// Destructor.
TSupport::~TSupport() { }

// getFx - accesses the Fx DOF of the support.
bool TSupport::getFx() const {
    return fConditions[0];
}
// getFy - accesses the Fy DOF of the support.
bool TSupport::getFy() const {
	return fConditions[1];
}
// getMx - accesses the Mx DOF of the support.
bool TSupport::getMx() const {
    return fConditions[2];
}
// getConditions - accesses the vector of support conditoins.
bool* const TSupport::getConditions() {
	return fConditions;
}
// getNodeID - accesses the node of the support.
int TSupport::getNodeID() const {
    return fNodeID;
}

// setFx - modifies the Fx DOF of the support.
void TSupport::setFx(const bool Fx) {
    fConditions[0] = Fx;
}
// setFy - modifies the Fy DOF of the support.
void TSupport::setFy(const bool Fy) {
    fConditions[1] = Fy;
}
// setMx - modifies the Mx DOF of the support.
void TSupport::setMx(const bool Mx) {
    fConditions[1] = Mx;
}
// setNode - modifies the node of the support.
void TSupport::setNodeID(const int NodeID) {
    fNodeID = NodeID;
}

// Assignment operator.
TSupport& TSupport::operator= (const TSupport& S) {
    if (this != &S) {
        fConditions[0] = S.fConditions[0];
        fConditions[1] = S.fConditions[1];
        fConditions[2] = S.fConditions[2];
        fNodeID = S.fNodeID;
        fStructure = S.fStructure;
    }
    return *this;
}

// Function that prints the support information.
void TSupport::print() {
	std::cout << "Support Info: " << std::endl
		<< " Fx: " << this->getFx() << std::endl
		<< " Fy: " << this->getFy() << std::endl
		<< " Mx: " << this->getMx() << std::endl
		<< " Node ID: " << this->getNodeID() << std::endl;
    std::cout << std::flush;
}