/* This file was created by Gustavo ALCALA BATISTELA.
 It contains the definitions of functions of _ classes.*/

#include "TElement.h"
#include "TStructure.h"

// Default constructor.
TElement::TElement(const int& Node1ID, const int& Node2ID, const bool& Hinge1,
                   const bool& Hinge2, const int& MaterialID, TStructure* Structure) {
    fLocalNodesIDs[0] = Node1ID;
    fLocalNodesIDs[1] = Node2ID;
    fHinges[0] = Hinge1;
    fHinges[1] = Hinge2;
	fMaterialID = MaterialID;
	fStructure = Structure;
}

//Copy constructor.
TElement::TElement(const TElement& E) {
    fLocalNodesIDs[0] = E.fLocalNodesIDs[0];
    fLocalNodesIDs[1] = E.fLocalNodesIDs[1];
    fHinges[0] = E.fHinges[0];
    fHinges[1] = E.fHinges[1];
    fMaterialID = E.fMaterialID;
    fStructure = E.fStructure;
}

// Destructor.
TElement::~TElement() { }

// getLocalNodesIDs - accesses the nodes of the element.
int* const TElement::getLocalNodesIDs()
{
	return fLocalNodesIDs;
}

// getNode1 - accesses the node1 of the element.
int TElement::getNode1ID() const {
    return fLocalNodesIDs[0];
}

// getNode2 - accesses the node1 of the element.
int TElement::getNode2ID() const {
    return fLocalNodesIDs[1];
}

// getHinge1 - accesses the left hinge of the element.
bool TElement::getHinge1() const {
    return fHinges[0];
}

// getHinge2 - accesses the right hinge of the element.
bool TElement::getHinge2() const {
    return fHinges[1];
}

// getMaterial - accesses the material of the element.
int TElement::getMaterialID() const {
    return fMaterialID;
}

// getX1 - accesses the x1 coordinate of the element.
double TElement::getX1() const {
    TNode node = (*fStructure).getNodes()[fLocalNodesIDs[0]];
	return node.getX();
}

// getY1 - accesses the y1 coordinate of the element.
double TElement::getY1() const {
    TNode node = (*fStructure).getNodes()[fLocalNodesIDs[0]];
    return node.getY();
}

// getX2 - accesses the x2 coordinate of the element.
double TElement::getX2() const {
    TNode node = (*fStructure).getNodes()[fLocalNodesIDs[1]];
	return node.getX();
}

// getY2 - accesses the y2 coordinate of the element.
double TElement::getY2() const {
    TNode node = (*fStructure).getNodes()[fLocalNodesIDs[1]];
	return node.getY();
}

// getL - accesses the element length.
double TElement::getL() const {
	double x1 = this->getX1();
    double y1 = this->getY1();
    double x2 = this->getX2();
    double y2 = this->getY2();
    return(sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
}

// getCos - accesses the element cosine.
double TElement::getCos() const {
	double x1 = this->getX1();
	double y1 = this->getY1();
	double x2 = this->getX2();
	double y2 = this->getY2();
    return((x2 - x1) / sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
}

// getSin - accesses the element sine.
double TElement::getSin() const {
	double x1 = this->getX1();
	double y1 = this->getY1();
	double x2 = this->getX2();
	double y2 = this->getY2();
    return((y2 - y1) / sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
}

// getT - accesses the transformation matrix of the element.
TPZFMatrix<double> TElement::getT() const {
    
    TPZFMatrix<double> t(6, 6, 0);
    double cos = this->getCos();
    double sin = this->getSin();
    
    t(0, 0) = cos;
    t(0, 1) = sin;
    t(1, 0) = -sin;
    t(1, 1) = cos;
    t(2, 2) = 1;
    t(3, 3) = cos;
    t(3, 4) = sin;
    t(4, 3) = -sin;
    t(4, 4) = cos;
    t(5, 5) = 1;
    
    return t;
}

// getK - accesses the stiffness matrix of the element.
TPZFMatrix<double> TElement::getK() const {
	TMaterial material = (*fStructure).getMaterials()[fMaterialID];
    double E = material.getE();
    double A = material.getA();
    double I = material.getI();
    double L = this->getL();
    TPZFMatrix<double> t = this->getT();
    
    TPZFMatrix<double> k(6, 6, 0);
    k(0, 0) = A*E/L;
    k(0, 3) = -A*E/L;
    k(1, 1) = 12*E*I/(L*L*L);
    k(1, 2) = 6*E*I/(L*L);
    k(1, 4) = -12*E*I/(L*L*L);
    k(1, 5) = 6*E*I/(L*L);
    k(2, 1) = 6*E*I/(L*L);
    k(2, 2) = 4*E*I/L;
    k(2, 4) = -6*E*I/(L*L);
    k(2, 5) = 2*E*I/L;
    k(3, 0) = -A*E/L;
    k(3, 3) = A*E/L;
    k(4, 1) = -12*E*I/(L*L*L);
    k(4, 2) = -6*E*I/(L*L);
    k(4, 4) = 12*E*I/(L*L*L);
    k(4, 5) = -6*E*I/(L*L);
    k(5, 1) = 6*E*I/(L*L);
    k(5, 2) = 2*E*I/L;
    k(5, 4) = -6*E*I/(L*L);
    k(5, 5) = 4*E*I/L;
    
    t.Transpose();
    k = t*k;
    t.Transpose();
    k = k*t;
    
    return k;
}

// setLocalNodesIDs - modifies the IDs of the nodes.
void TElement::setLocalNodesIDs(int* NodesIDs)
{
    fLocalNodesIDs[0] = NodesIDs[0];
    fLocalNodesIDs[1] = NodesIDs[1];
}

// setNode1 - modifies the element's node 1.
void TElement::setNode1ID(const int& Node1ID) {
	fLocalNodesIDs[0] = Node1ID;
}
// setNode2 - modifies the element's node 2.
void TElement::setNode2ID(const int& Node2ID) {
    fLocalNodesIDs[1] = Node2ID;
}
// setMaterial - modifies the element's material.
void TElement::setMaterialID(const int& MaterialID) {
    fMaterialID = MaterialID;
}

// Assignment operator.
TElement& TElement::operator= (const TElement& E) {
    if (this != &E) {
        fLocalNodesIDs[0] = E.fLocalNodesIDs[0];
        fLocalNodesIDs[1] = E.fLocalNodesIDs[1];
        fHinges[0] = E.fHinges[0];
        fHinges[1] = E.fHinges[1];
        fMaterialID = E.fMaterialID;
        fStructure = E.fStructure;
    }
    return *this;
}

// Function that prints the element information.
void TElement::print() {
	std::cout << "Element Info: " << std::endl
		<< " Node 1: " << fLocalNodesIDs[0] << std::endl
		<< " Node 2: " << fLocalNodesIDs[1] << std::endl
        << " Hinge 1: " << fHinges[0] << std::endl
        << " Hinge 2: " << fHinges[1] << std::endl
		<< " Material: " << fMaterialID << std::endl
		<< " L: " << this->getL() << std::endl
		<< " Cos: " << this->getCos() << std::endl
		<< " Sin: " << this->getSin() << std::endl;
    std::cout << std::flush;
}