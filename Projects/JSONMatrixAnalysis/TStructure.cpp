/* This file was created by Gustavo BATISTELA.
It contains the definitions of functions of the TStructure class. */

#include "TStructure.h"

// Default constructor.
TStructure::TStructure(const std::vector<TNode>& Nodes,
					   const std::vector<TMaterial>& Materials,
					   const std::vector<TSupport>& Supports,
					   const std::vector<TElement>& Elements)
	: fNodes(Nodes),
	  fMaterials(Materials),
	  fSupports(Supports),
	  fElements(Elements) {}

// Copy constructor.
TStructure::TStructure(const TStructure& S)
	: fNodes(S.fNodes),
	  fMaterials(S.fMaterials),
	  fSupports(S.fSupports),
	  fElements(S.fElements),
	  fNodeEquations(S.fNodeEquations) {}

// Destructor.
TStructure::~TStructure() {}

// getNodes - returns the vector of nodes.
std::vector<TNode> TStructure::getNodes() const {
	return fNodes;
}

// getMaterials - returns the vector of materials.
std::vector<TMaterial> TStructure::getMaterials() const {
	return fMaterials;
}

// getSupports - returns the vector of supports.
std::vector<TSupport> TStructure::getSupports() const {
	return fSupports;
}

// getElements - returns the vector of elements.
std::vector<TElement> TStructure::getElements() const { 
	return fElements;
}

// getElement - returns an element.
TElement TStructure::getElement(int elementID) const {
	return fElements[elementID];
}

// getNodeEquations - returns the matrix of DOFs of the nodes.
TPZFMatrix<int> TStructure::getNodeEquations() const { 
	return fNodeEquations;
}

// getSupport - returns a support ID by giving the reference node ID.
int TStructure::getSupportID(int NodeID) {
	int SupportID = -1;

	for (int i = 0; i < fSupports.size(); i++) {
		if (fSupports[i].getNodeID() == NodeID) {
			SupportID = i;
			break;
		}
	}
	return SupportID;
}

// getNDOF - returns the number of degrees of freedom of the structure.
int TStructure::getNDOF() const {
	// DOF counter variable, intialized considering vertical and horizontal
	// displacements.
	int NDOF = 2 * fNodes.size();

	// Auxiliar that keeps track if there are already non-hinged elements
	// connected to a certain node.
	std::vector<bool> aux(fNodes.size());
	std::fill(aux.begin(), aux.end(), false);

	// Counts the number of constrained rotation DOF.
	for (int i = 0; i < fSupports.size(); i++) {
		if (fSupports[i].getM() == true) {
			aux[fSupports[i].getNodeID()] = true;
			NDOF++;
		}
	}

	// Counts the number of unconstrained rotation DOF.
	for (int i = 0; i < fElements.size(); i++) {
		int Node0ID = fElements[i].getNode0ID();
		int Node1ID = fElements[i].getNode1ID();

		// Checks if the element is non-hinged at its node 0.
		if (fElements[i].getHinge0() == false) {
			// Ensure element is the first non-hinged element to be
			// connected at its node 0.
			if (aux[Node0ID] == false) {
				aux[Node0ID] = true;
				NDOF++;
			}
		}
		// Case the element is hinged at its node 0.
		else {
			NDOF++;
		}

		// Checks if the element is non-hinged at its node 1.
		if (fElements[i].getHinge1() == false) {
			// Ensure element is the first non-hinged element to be 
			// connected at its node 1.
			if (aux[Node1ID] == false) {
				aux[Node1ID] = true;
				NDOF++;
			}
		}
		// Case the element is hinged at its node 1.
		else { 
			NDOF++;
		}
	}
	return NDOF;
}

// getCDOF - returns the number of constrained DOF of the structure.
int TStructure::getCDOF() const {
	int CDOF = 0;
	// Counts the number of constrained DOF.
	for (int i = 0; i < fSupports.size(); i++) {
		if (fSupports[i].getFx() == true) {
			CDOF++;
		}
		if (fSupports[i].getFy() == true) {
			CDOF++;
		}
		if (fSupports[i].getM() == true) {
			CDOF++;
		}
	}
	return CDOF;
}

// getUDOF - returns the number of unconstrained DOF of the structure.
int TStructure::getUDOF() const { 
	return (this->getNDOF() - this->getCDOF());
}

// enumerateEquations - enumerates the DOF associated with each element.
void TStructure::enumerateEquations() {
	// Matrix that stores the equations associated with each node.
	TPZFMatrix<int> equations(fNodes.size(), 3, -1);
	int CDOF = this->getCDOF();
	int count = this->getNDOF() - CDOF;

	// Enumerates the constrained DOF.
	for (int i = 0; i < fSupports.size(); i++) {
		if (fSupports[i].getFx() == true) {
			equations(fSupports[i].getNodeID(), 0) = count;
			count++;
		}
		if (fSupports[i].getFy() == true) {
			equations(fSupports[i].getNodeID(), 1) = count;
			count++;
		}
		if (fSupports[i].getM() == true) {
			equations(fSupports[i].getNodeID(), 2) = count;
			count++;
		}
	}

	count = 0;
	// Enumerates the unconstrained DOF.
	for (int i = 0; i < fElements.size(); i++) {
		int Node0ID = fElements[i].getNode0ID();
		int Node1ID = fElements[i].getNode1ID();
		int Node0RotationDOF;
		int Node1RotationDOF;

		if (equations(Node0ID, 0) == -1) {
			equations(Node0ID, 0) = count;
			count++;
		}
		if (equations(Node0ID, 1) == -1) {
			equations(Node0ID, 1) = count;
			count++;
		}
		if (fElements[i].getHinge0() == false) {
			if (equations(Node0ID, 2) == -1) {
				equations(Node0ID, 2) = count;
				count++;
			}
			Node0RotationDOF = equations(Node0ID, 2);
		}
		else {
			Node0RotationDOF = count;
			count++;
		}

		if (equations(Node1ID, 0) == -1) {
			equations(Node1ID, 0) = count;
			count++;
		}
		if (equations(Node1ID, 1) == -1) {
			equations(Node1ID, 1) = count;
			count++;
		}
		if (fElements[i].getHinge1() == false) {
			if (equations(Node1ID, 2) == -1) {
				equations(Node1ID, 2) = count;
				count++;
			}
			Node1RotationDOF = equations(Node1ID, 2);
		}
		else {
			Node1RotationDOF = count;
			count++;
		}
		fElements[i].setEquations(equations(Node0ID, 0), equations(Node0ID, 1),
								  Node0RotationDOF, equations(Node1ID, 0),
								  equations(Node1ID, 1), Node1RotationDOF);
	}
	fNodeEquations = equations;
}

// getK - returns the global stiffness matrix.
TPZFMatrix<double> TStructure::getK() const {
	int NDOF = this->getNDOF();
	TPZFMatrix<double> K(NDOF, NDOF, 0);

	for (int i = 0; i < fElements.size(); i++) {
		TElement elem = fElements[i];
		TPZFMatrix<double> kLocal = elem.getK();

		for (int aux1 = 0; aux1 < 6; aux1++) {
			for (int aux2 = 0; aux2 < 6; aux2++) {
				K(elem.getEquations()[aux1], elem.getEquations()[aux2]) +=
					kLocal(aux1, aux2);
			}
		}
	}
	return K;
}

// getK11 - returns the left upper block of the global stiffness matrix K.
TPZFMatrix<double> TStructure::getK11() const {
	int UDOF = this->getUDOF();
	TPZFMatrix<double> K11(UDOF, UDOF);
	TPZFMatrix<double> K = this->getK();

	for (int i = 0; i < UDOF; i++) {
		for (int j = 0; j < UDOF; j++) {
			K11(i, j) = K(i, j);
		}
	}
	return K11;
}

// getK21 - returns the left down block of the global stiffness matrix K.
TPZFMatrix<double> TStructure::getK21() const {
	int UDOF = this->getUDOF();
	int CDOF = this->getCDOF();

	TPZFMatrix<double> K21(CDOF, UDOF);
	TPZFMatrix<double> K = this->getK();

	for (int i = 0; i < CDOF; i++) {
		for (int j = 0; j < UDOF; j++) {
			K21(i, j) = K(UDOF + i, j);
		}
	}
	return K21;
}

// setNodes - modifies the vector of nodes.
void TStructure::setNodes(const std::vector<TNode>& Nodes) {
	fNodes = Nodes;
}

// setMaterials - modifies the vector of materials.
void TStructure::setMaterials(const std::vector<TMaterial>& Materials) {
	fMaterials = Materials;
}

// setSupports - modifies the vector of supports.
void TStructure::setSupports(const std::vector<TSupport>& Supports) {
	fSupports = Supports;
}

// setElements - modifies the vector of elements.
void TStructure::setElements(const std::vector<TElement>& Elements) {
	fElements = Elements;
}

// getQ - returns the vector of loads with null elements.
TPZFMatrix<double> TStructure::getQ() {
	return TPZFMatrix<double>(this->getNDOF(), 1, 0);
}

// calculateQK - calculates the known external loads and stores them in the
// vector of global forces Q.
void TStructure::calculateQK(TPZFMatrix<double>& Q,
							 std::vector<TNodalLoad>& nLoads,
							 std::vector<TDistributedLoad>& dLoads,
							 std::vector<TElementLoad>& eLoads) {
	for (int i = 0; i < nLoads.size(); i++) {
		nLoads[i].setStructure(this);
		nLoads[i].store(Q);
	}

	for (int i = 0; i < dLoads.size(); i++) {
		dLoads[i].setStructure(this);
		dLoads[i].store(Q);
	}

	for (int i = 0; i < eLoads.size(); i++) {
		eLoads[i].setStructure(this);
		eLoads[i].store(Q);
	}
}

// getD - returns the vector of displacements with null elements.
TPZFMatrix<double> TStructure::getD() {
	return TPZFMatrix<double>(this->getNDOF(), 1, 0);
}

// calculateDU - solves the unknown displacements DU and stores them in the
// vector of global displacements D.
void TStructure::calculateDU(TPZFMatrix<double>& Q, TPZFMatrix<double>& D) {
	int UDOF = this->getUDOF();
	TPZFMatrix<double> knownLoads(UDOF, 1, 0);

	for (int i = 0; i < UDOF; i++) {
		knownLoads(i, 0) = Q(i, 0);
	}

	TPZFMatrix<double> invK11;
	this->getK11().Inverse(invK11, ELU);

	TPZFMatrix<double> DU;
	DU = invK11 * knownLoads;

	for (int i = 0; i < UDOF; i++) {
		D(i, 0) = DU(i, 0);
	}
}

// calculateQU - solves the support reactions and stores them in the vector of
// global forces Q.
void TStructure::calculateQU(TPZFMatrix<double>& Q, TPZFMatrix<double>& D) {
	int UDOF = this->getUDOF();
	TPZFMatrix<double> DU(UDOF, 1, 0);

	for (int i = 0; i < UDOF; i++) {
		DU(i, 0) = D(i, 0);
	}

	int CDOF = this->getCDOF();
	TPZFMatrix<double> QU(CDOF, 1, 0);

	QU = this->getK21() * DU;

	for (int i = 0; i < CDOF; i++) {
		Q(UDOF + i, 0) = -1 * Q(UDOF + i, 0) + QU(i, 0);
	}
}