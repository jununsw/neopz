/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TSupport class. */

#include "TSupport.h"
#include "TStructure.h"

// Default constructor.
TSupport::TSupport(const bool Fx, const bool Fy, const bool M,
	const int NodeID, TStructure* Structure) {
    fConditions[0] = Fx;
    fConditions[1] = Fy;
    fConditions[2] = M;
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
// getM - accesses the M DOF of the support.
bool TSupport::getM() const {
    return fConditions[2];
}
// getConditions - accesses the vector of support conditions.
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
// setM - modifies the M DOF of the support.
void TSupport::setM(const bool M) {
    fConditions[2] = M;
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
		<< " Fx: " << fConditions[0] << std::endl
		<< " Fy: " << fConditions[1] << std::endl
		<< " M: " << fConditions[2] << std::endl
		<< " Node ID: " << fNodeID << std::endl;
    std::cout << std::flush;
}