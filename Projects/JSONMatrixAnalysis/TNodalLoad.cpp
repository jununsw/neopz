/* This file was created by Gustavo BATISTELA.
It contains the definitions of functions of the TNodalLoad class. */

#include "TStructure.h"
#include "TNodalLoad.h"

// Default constructor.
TNodalLoad::TNodalLoad(double Fx, double Fy, double M, int NodeID,
					   TStructure* Structure)
	: fFx(Fx),
	  fFy(Fy), 
	  fM(M), 
	  fNodeID(NodeID), 
	  fStructure(Structure) {}

// Copy constructor.
TNodalLoad::TNodalLoad(const TNodalLoad& NL)
	: fFx(NL.fFx),
	  fFy(NL.fFy),
	  fM(NL.fM),
	  fNodeID(NL.fNodeID),
	  fStructure(NL.fStructure) {}

// Destructor.
TNodalLoad::~TNodalLoad() {}

// Assignment operator.
TNodalLoad& TNodalLoad::operator=(const TNodalLoad& NL) {
	if (this != &NL) {
		fFx = NL.fFx;
		fFy = NL.fFy;
		fM = NL.fM;
		fNodeID = NL.fNodeID;
		fStructure = NL.fStructure;
	}
	return *this;
}

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

// getNodeID - returns the ID of the node the load is applied to.
int TNodalLoad::getNodeID() const {
	return fNodeID;
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

// setNodeID - modifies the ID of the node the load is applied to.
void TNodalLoad::setNodeID(int NodeID) {
	fNodeID = NodeID;
}

// setStructure - modifies the element's structure.
void TNodalLoad::setStructure(TStructure* Structure) { 
	fStructure = Structure;
}

// store - adds the effects of the nodal load to the vector of loads.
void TNodalLoad::store(TPZFMatrix<double>& L) {
	int fxDOF = fStructure->getNodeEquations()(fNodeID, 0);
	int fyDOF = fStructure->getNodeEquations()(fNodeID, 1);
	int mDOF = fStructure->getNodeEquations()(fNodeID, 2);

	L(fxDOF, 0) = L(fxDOF, 0) + fFx;
	L(fyDOF, 0) = L(fyDOF, 0) + fFy;
	if (mDOF != -1) {
		L(mDOF, 0) = L(mDOF, 0) + fM;
	}
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