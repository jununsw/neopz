/* This file was created by Gustavo BATISTELA.
It contains the definitions of functions of the TNode class. */

#include "TNode.h"

// Default constructor.
TNode::TNode(double X, double Y) {
	fCoordinates[0] = X;
	fCoordinates[1] = Y;
}

// Copy constructor.
TNode::TNode(const TNode& N) {
	fCoordinates[0] = N.fCoordinates[0];
	fCoordinates[1] = N.fCoordinates[1];
}

// Destructor.
TNode::~TNode() {}

// Assignment operator.
TNode& TNode::operator=(const TNode& N) {
	if (this != &N) {
		fCoordinates[0] = N.fCoordinates[0];
		fCoordinates[1] = N.fCoordinates[1];
	}
	return *this;
}

// getX - accesses the node X coord.
double TNode::getX() const { 
	return fCoordinates[0];
}

// getY - accesses the node Y coord.
double TNode::getY() const {
	return fCoordinates[1];
}

// getCoord - accesses the vector of coordinates.
double* const TNode::getCoord() {
	return fCoordinates; 
}

// setX - modifies the X coord of the node.
void TNode::setX(double X) { 
	fCoordinates[0] = X; 
}

// setY - modifies the Y coord of the node.
void TNode::setY(double Y) {
	fCoordinates[1] = Y; 
}

// setCoord - modifies the vector of coordinates.
void TNode::setCoord(double* Coordinates) {
	fCoordinates[0] = Coordinates[0];
	fCoordinates[1] = Coordinates[1];
}

// Function that prints the node information.
void TNode::print() {
	std::cout << "Node Info:" << std::endl
		<< " X: " << fCoordinates[0] << std::endl
		<< " Y: " << fCoordinates[1] << std::endl;
	std::cout << std::flush;
}