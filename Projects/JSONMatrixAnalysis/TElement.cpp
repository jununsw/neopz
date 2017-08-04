/* This file was created by Gustavo BATISTELA.
It contains the definitions of functions of the TElement class. */

#include "TStructure.h"
#include "TElement.h"

// Default constructor.
TElement::TElement(int Node0ID, int Node1ID, bool Hinge0, bool Hinge1,
				   int MaterialID, TStructure* Structure) {
	fLocalNodesIDs[0] = Node0ID;
	fLocalNodesIDs[1] = Node1ID;
	fHinges[0] = Hinge0;
	fHinges[1] = Hinge1;
	fMaterialID = MaterialID;
	fStructure = Structure;
	fEquations[0] = -1;
	fEquations[1] = -1;
	fEquations[2] = -1;
	fEquations[3] = -1;
	fEquations[4] = -1;
	fEquations[5] = -1;
}

// Copy constructor.
TElement::TElement(const TElement& E) {
	fLocalNodesIDs[0] = E.fLocalNodesIDs[0];
	fLocalNodesIDs[1] = E.fLocalNodesIDs[1];
	fHinges[0] = E.fHinges[0];
	fHinges[1] = E.fHinges[1];
	fMaterialID = E.fMaterialID;
	fStructure = E.fStructure;
	fEquations[0] = E.fEquations[0];
	fEquations[1] = E.fEquations[1];
	fEquations[2] = E.fEquations[2];
	fEquations[3] = E.fEquations[3];
	fEquations[4] = E.fEquations[4];
	fEquations[5] = E.fEquations[5];
}

// Destructor.
TElement::~TElement() {}

// Assignment operator.
TElement& TElement::operator=(const TElement& E) {
	if (this != &E) {
		fLocalNodesIDs[0] = E.fLocalNodesIDs[0];
		fLocalNodesIDs[1] = E.fLocalNodesIDs[1];
		fHinges[0] = E.fHinges[0];
		fHinges[1] = E.fHinges[1];
		fMaterialID = E.fMaterialID;
		fStructure = E.fStructure;
		fEquations[0] = E.fEquations[0];
		fEquations[1] = E.fEquations[1];
		fEquations[2] = E.fEquations[2];
		fEquations[3] = E.fEquations[3];
		fEquations[4] = E.fEquations[4];
		fEquations[5] = E.fEquations[5];
	}
	return *this;
}

// getLocalNodesIDs - accesses the nodes of the element.
int* const TElement::getLocalNodesIDs() {
	return fLocalNodesIDs;
}

// getNode0ID - accesses the ID of the element's Node 0.
int TElement::getNode0ID() const { 
	return fLocalNodesIDs[0]; 
}

// getNode1ID - accesses the ID of the element's Node 1.
int TElement::getNode1ID() const { 
	return fLocalNodesIDs[1];
}

// getHinge0 - accesses the hinge of the element at Node 0.
bool TElement::getHinge0() const { 
	return fHinges[0]; 
}

// getHinge1 - accesses the hinge of the element at Node 1.
bool TElement::getHinge1() const {
	return fHinges[1];
}

// getMaterial - accesses the material of the element.
int TElement::getMaterialID() const { 
	return fMaterialID; 
}

// getEquations - accesses the equations of the element.
int* const TElement::getEquations() { 
	return fEquations; 
}

// getX0 - accesses the x coordinate of the element's Node 0.
double TElement::getX0() const {
	TNode node = fStructure->getNodes()[fLocalNodesIDs[0]];
	return node.getX();
}

// getY0 - accesses the y coordinate of the element's Node 0.
double TElement::getY0() const {
	TNode node = fStructure->getNodes()[fLocalNodesIDs[0]];
	return node.getY();
}

// getX1 - accesses the x coordinate of the element's Node 1.
double TElement::getX1() const {
	TNode node = fStructure->getNodes()[fLocalNodesIDs[1]];
	return node.getX();
}

// getY1 - accesses the y coordinate of the element's Node 1.
double TElement::getY1() const {
	TNode node = fStructure->getNodes()[fLocalNodesIDs[1]];
	return node.getY();
}

// getL - accesses the element length.
double TElement::getL() const {
	double x0 = this->getX0();
	double y0 = this->getY0();
	double x1 = this->getX1();
	double y1 = this->getY1();
	return (sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)));
}

// getCos - accesses the element cosine.
double TElement::getCos() const {
	double x0 = this->getX0();
	double y0 = this->getY0();
	double x1 = this->getX1();
	double y1 = this->getY1();
	return ((x1 - x0) / sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)));
}

// getSin - accesses the element sine.
double TElement::getSin() const {
	double x0 = this->getX0();
	double y0 = this->getY0();
	double x1 = this->getX1();
	double y1 = this->getY1();
	return ((y1 - y0) / sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)));
}

// getT - accesses the transformation matrix of the element.
TPZFMatrix<double> TElement::getT() const {
	TPZFMatrix<double> t(6, 6, 0);
	double lx = this->getCos();
	double ly = this->getSin();

	t(0, 0) = lx;
	t(0, 1) = ly;
	t(1, 0) = -ly;
	t(1, 1) = lx;
	t(2, 2) = 1;
	t(3, 3) = lx;
	t(3, 4) = ly;
	t(4, 3) = -ly;
	t(4, 4) = lx;
	t(5, 5) = 1;

	return t;
}

// getK - accesses the stiffness matrix of the element.
TPZFMatrix<double> TElement::getK() const {
	TMaterial material = fStructure->getMaterials()[fMaterialID];
	double E = material.getE();
	double A = material.getA();
	double I = material.getI();
	double L = this->getL();
	double lx = this->getCos();
	double ly = this->getSin();

	TPZFMatrix<double> k(6, 6, 0);

	k(0, 0) = (A * E / L * (lx * lx) + 12 * E * I * (ly * ly) / (L * L * L));
	k(0, 1) = (A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(0, 2) = -(6 * E * I / (L * L)) * ly;
	k(0, 3) = -(A * E / L * (lx * lx) + 12 * E * I * (ly * ly) / (L * L * L));
	k(0, 4) = -(A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(0, 5) = -(6 * E * I / (L * L)) * ly;

	k(1, 0) = (A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(1, 1) = (A * E / L * (ly * ly) + 12 * E * I * (lx * lx) / (L * L * L));
	k(1, 2) = (6 * E * I / (L * L)) * lx;
	k(1, 3) = -(A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(1, 4) = -(A * E / L * (ly * ly) + 12 * E * I * (lx * lx) / (L * L * L));
	k(1, 5) = (6 * E * I / (L * L)) * lx;

	k(2, 0) = -(6 * E * I / (L * L)) * ly;
	k(2, 1) = (6 * E * I / (L * L)) * lx;
	k(2, 2) = (4 * E * I / (L));
	k(2, 3) = (6 * E * I / (L * L)) * ly;
	k(2, 4) = -(6 * E * I / (L * L)) * lx;
	k(2, 5) = (2 * E * I / (L));

	k(3, 0) = -(A * E / L * (lx * lx) + 12 * E * I * (ly * ly) / (L * L * L));
	k(3, 1) = -(A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(3, 2) = (6 * E * I / (L * L)) * ly;
	k(3, 3) = (A * E / L * (lx * lx) + 12 * E * I * (ly * ly) / (L * L * L));
	k(3, 4) = (A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(3, 5) = (6 * E * I / (L * L)) * ly;

	k(4, 0) = -(A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(4, 1) = -(A * E / L * (ly * ly) + 12 * E * I * (lx * lx) / (L * L * L));
	k(4, 2) = -(6 * E * I / (L * L)) * lx;
	k(4, 3) = (A * E / L - 12 * E * I / (L * L * L)) * (lx * ly);
	k(4, 4) = (A * E / L * (ly * ly) + 12 * E * I * (lx * lx) / (L * L * L));
	k(4, 5) = -(6 * E * I / (L * L)) * lx;

	k(5, 0) = -(6 * E * I / (L * L)) * ly;
	k(5, 1) = (6 * E * I / (L * L)) * lx;
	k(5, 2) = (2 * E * I / (L));
	k(5, 3) = (6 * E * I / (L * L)) * ly;
	k(5, 4) = -(6 * E * I / (L * L)) * lx;
	k(5, 5) = (4 * E * I / (L));

	return k;
}

// setLocalNodesIDs - modifies the IDs of the nodes.
void TElement::setLocalNodesIDs(int* NodesIDs) {
	fLocalNodesIDs[0] = NodesIDs[0];
	fLocalNodesIDs[1] = NodesIDs[1];
}

// setNode0ID - modifies the element's Node 0.
void TElement::setNode0ID(int Node0ID) { 
	fLocalNodesIDs[0] = Node0ID;
}

// setNode1ID - modifies the element's Node 1.
void TElement::setNode1ID(int Node1ID) {
	fLocalNodesIDs[1] = Node1ID; 
}

// setMaterialID - modifies the element material.
void TElement::setMaterialID(int MaterialID) {
	fMaterialID = MaterialID; 
}

// setHinge0 - modifies the element's hinge at Node 0.
void TElement::setHinge0(bool Hinge0) { 
	fHinges[0] = Hinge0; 
}

// setHinge1 - modifies the element's hinge at Node 1.
void TElement::setHinge1(bool Hinge1) {
	fHinges[1] = Hinge1; 
}

// setStructure - modifies the element's Structure.
void TElement::setStructure(TStructure* Structure) { 
	fStructure = Structure; 
}

// setEquations - modifies the element equations indexes.
void TElement::setEquations(int Eq1, int Eq2, int Eq3, 
							int Eq4, int Eq5, int Eq6) {
	fEquations[0] = Eq1;
	fEquations[1] = Eq2;
	fEquations[2] = Eq3;
	fEquations[3] = Eq4;
	fEquations[4] = Eq5;
	fEquations[5] = Eq6;
}

// Function that prints the element information.
void TElement::print() {
	std::cout << "Element Info: " << std::endl
		<< " Node 0: " << fLocalNodesIDs[0] << std::endl
		<< " Node 1: " << fLocalNodesIDs[1] << std::endl
		<< " Hinge 0: " << fHinges[0] << std::endl
		<< " Hinge 1: " << fHinges[1] << std::endl
		<< " Material: " << fMaterialID << std::endl
		<< " L: " << this->getL() << std::endl
		<< " Cos: " << this->getCos() << std::endl
		<< " Sin: " << this->getSin() << std::endl
		<< " Equations: (" << fEquations[0] << ", " << fEquations[1] << ", "
		<< fEquations[2] << "; " << fEquations[3] << ", " << fEquations[4]
		<< ", " << fEquations[5] << ")" << std::endl;
	std::cout << std::flush;
}