/* This file was created by Gustavo BATISTELA.
It contains the definitions of functions of the TElementLoad class. */

#include "TStructure.h"
#include "TElementLoad.h"

// Default constructor.
TElementLoad::TElementLoad(double Fx, double Fy, double M, int ElementID,
						   int Node, TStructure* Structure)
	: fFx(Fx),
	  fFy(Fy),
	  fM(M),
	  fElementID(ElementID),
	  fNode(Node),
	  fStructure(Structure) {}

// Copy constructor.
TElementLoad::TElementLoad(const TElementLoad& EL)
	: fFx(EL.fFx),
	  fFy(EL.fFy),
	  fM(EL.fM),
	  fElementID(EL.fElementID),
	  fNode(EL.fNode),
	  fStructure(EL.fStructure) {}

// Destructor.
TElementLoad::~TElementLoad() {}

// Assignment operator.
TElementLoad& TElementLoad::operator=(const TElementLoad& EL) {
	if (this != &EL) {
		fFx = EL.fFx;
		fFy = EL.fFy;
		fM = EL.fM;
		fElementID = EL.fElementID;
		fNode = EL.fNode;
		fStructure = EL.fStructure;
	}
	return *this;
}

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

// getNode - returns the local ID of the node which the load is applied to.
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

// setNode - modifies the local ID of the node which the load is applied to.
void TElementLoad::setNode(int Node) { 
	fNode = Node; 
}

// setStructure - modifies the element's structure.
void TElementLoad::setStructure(TStructure* Structure) {
	fStructure = Structure;
}

// store - adds the effects of the nodal load to the vector of loads.
void TElementLoad::store(TPZFMatrix<double>& L) {
	int nodeID;
	if (this->fNode == 0) {
		nodeID = fStructure->getElement(this->fElementID).getNode0ID();
	}
	else if (this->fNode == 1) {
		nodeID = fStructure->getElement(this->fElementID).getNode1ID();
	}

	int fxDOF = fStructure->getNodeEquations()(nodeID, 0);
	int fyDOF = fStructure->getNodeEquations()(nodeID, 1);
	int mDOF = fStructure->getNodeEquations()(nodeID, 2);

	L(0, fxDOF) += this->fFx;
	L(0, fyDOF) += this->fFy;
	if (mDOF != -1) {
		L(0, mDOF) += this->fM;
	}
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