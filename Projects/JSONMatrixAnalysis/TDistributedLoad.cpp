/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TDistributedLoad class. */

#include "TStructure.h"
#include "TDistributedLoad.h"

// Default constructor.
TDistributedLoad::TDistributedLoad(int ElementID, double Node0Load, double Node1Load, bool LoadPlane, TStructure* Structure)
    : fElementID(ElementID), fNode0Load(Node0Load), fNode1Load(Node1Load), fLoadPlane(LoadPlane), fStructure(Structure) { }

// Copy constructor.
TDistributedLoad::TDistributedLoad(const TDistributedLoad& DL)
    : fElementID(DL.fElementID), fNode0Load(DL.fNode0Load), fNode1Load(DL.fNode1Load), fLoadPlane(DL.fLoadPlane), fStructure(DL.fStructure) { }

// Destructor.
TDistributedLoad::~TDistributedLoad() { }

// Assignment operator.
TDistributedLoad& TDistributedLoad::operator= (const TDistributedLoad& DL) {
    if (this != &DL) {
        fElementID = DL.fElementID;
        fNode0Load = DL.fNode0Load;
        fNode1Load = DL.fNode1Load;
		fLoadPlane = DL.fLoadPlane;
		fStructure = DL.fStructure;
    }
    return *this;
}

// getElementID - returns the ID of the element the load is applied to.
int TDistributedLoad::getElementID() const {
	return fElementID;
}

// getNode0Load - returns the load applied to the element's local node 0.
double TDistributedLoad::getNode0Load() const {
	return fNode0Load;
}

// getNode1Load - returns the load applied to the element's local node 1.
double TDistributedLoad::getNode1Load() const {
	return fNode1Load;
}

// getLoadPlane - returns the boolean associated with the plane of the load (Global or Local).
bool TDistributedLoad::getLoadPlane() const {
	return fLoadPlane;
}

// setElementID - modifies the ID of the element the load is applied to.
void TDistributedLoad::setElementID(int ElementID) {
	fElementID = ElementID;
}

// setNode0Load - modifies the load applied to the element's local node 0.
void TDistributedLoad::setNode0Load(double Node0Load) {
	fNode0Load = Node0Load;
}

// setNode1Load - modifies the load applied to the element's local node 1.
void TDistributedLoad::setNode1Load(double Node1Load) {
	fNode1Load = Node1Load;
}

// setLoadPlane - modifies the boolean associated with the plane of the load (Global or Local).
void TDistributedLoad::setLoadPlane(bool LoadPlane) {
	fLoadPlane = LoadPlane;
}

void TDistributedLoad::setStructure(TStructure * Structure) {
	fStructure = Structure;
}

// store - adds the effects of the nodal load to the vector of loads.
void TDistributedLoad::store(TPZFMatrix<double>& L) {
	int node0ID = fStructure->getElement(this->fElementID).getNode0ID();
	int node1ID = fStructure->getElement(this->fElementID).getNode1ID();

	int fx0DOF = fStructure->getNodeEquations()(node0ID, 0);
	int fy0DOF = fStructure->getNodeEquations()(node0ID, 1);
	int m0DOF = fStructure->getNodeEquations()(node0ID, 2);
	int fx1DOF = fStructure->getNodeEquations()(node1ID, 0);
	int fy1DOF = fStructure->getNodeEquations()(node1ID, 1);
	int m1DOF = fStructure->getNodeEquations()(node1ID, 2);

	if (fLoadPlane == true) { // If the load plane is the global plane.
		double lx = fStructure->getElement(this->fElementID).getCos();
		double length = (fStructure->getElement(this->fElementID).getL()) * lx;

		L(fy0DOF, 0) += (7 * fNode0Load*length / 20) + (3 * fNode1Load*length / 20);
		L(m0DOF, 0) += (fNode0Load*length*length / 30) + (fNode1Load*length*length / 20);
		L(fy1DOF, 0) += (3 * fNode0Load*length / 20) + (7 * fNode1Load*length / 20);
		L(m1DOF, 0) += -1 * (fNode0Load*length*length / 20) - 1 * (fNode1Load*length*length / 30);
	}

	else { // If the load plane is the element local plane.
		double lx = fStructure->getElement(this->fElementID).getCos();
		double ly = fStructure->getElement(this->fElementID).getSin();
		double length = fStructure->getElement(this->fElementID).getL();

		L(fx0DOF, 0) += -1 * ly * ((7 * fNode0Load*length / 20) + (3 * fNode1Load*length / 20));
		L(fy0DOF, 0) += lx*((7 * fNode0Load*length / 20) + (3 * fNode1Load*length / 20));
		L(m0DOF, 0) += (fNode0Load*length*length / 30) + (fNode1Load*length*length / 20);
		L(fx1DOF, 0) += -1 * ly*((3 * fNode0Load*length / 20) + (7 * fNode1Load*length / 20));
		L(fy1DOF, 0) += lx * ((3 * fNode0Load*length / 20) + (7 * fNode1Load*length / 20));
		L(m1DOF, 0) += -1 * ((fNode0Load*length*length / 20) + (fNode1Load*length*length / 30));
	}
}

// Function that prints the load information.
void TDistributedLoad::print() {
    std::cout << "Distributed Load Info: " << std::endl
    << " Element ID: " << fElementID << std::endl
    << " Node 0 Load: " << fNode0Load << std::endl
    << " Node 1 Load: " << fNode1Load << std::endl;
    std::cout << std::flush;
}