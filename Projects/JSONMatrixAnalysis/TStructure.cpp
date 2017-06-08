/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TStructure class.*/

#include "TStructure.h"

// Default constructor.
TStructure::TStructure(const std::vector<TNode>& Nodes, const std::vector<TMaterial>& Materials,
const std::vector<TSupport>& Supports, 	const std::vector<TElement>& Elements)
	: fNodes(Nodes), fMaterials(Materials), fSupports(Supports), fElements(Elements) { }

//Copy constructor.
TStructure::TStructure(const TStructure& S) 
	: fNodes(S.fNodes), fMaterials(S.fMaterials), fSupports(S.fSupports), fElements(S.fElements) { }

// Destructor.
TStructure::~TStructure() { }

// getNodes - returns the vector of nodes.
std::vector<TNode> TStructure::getNodes() const
{
	return fNodes;
}

// getMaterials - returns the vector of materials.
std::vector<TMaterial> TStructure::getMaterials() const
{
	return fMaterials;
}

// getSupports - returns the vector of supports.
std::vector<TSupport> TStructure::getSupports() const
{
	return fSupports;
}

// getElements - returns the vector of elements.
std::vector<TElement> TStructure::getElements() const
{
	return fElements;
}

// getSupport - returns a support ID by giving the reference node ID.
int TStructure::getSupportID(int NodeID) {
    int SupportID = -1;
    
    for (int i = 0; i < fSupports.size(); i++) {
        if (fSupports[i].getNodeID() == NodeID)
        {
            SupportID = i;
            break;
        }
    }
    return SupportID;
}

// getNDOF - returns the number of degrees of freedom of the structure.
int TStructure::getNDOF() const {
    
    int NDOF = 2*fNodes.size();  // DOF counter variable, intialized considering vertical and horizontal displacements.
    
    std::vector<bool> aux(fNodes.size());     // Auxiliar that keeps track if there are already non-hinged elements connected to a certain node.
    std::fill_n(&aux[0], fNodes.size(), false);
    
    for (int i = 0; i < fSupports.size(); i ++) {   // Counts the number of constrained rotation DOF.
        if (fSupports[i].getMx() == true) {
            aux[fSupports[i].getNodeID()] = true;
            NDOF++;
        }
    }
    
    for (int i = 0; i < fElements.size(); i++) {    // Checks the number of unconstrained rotation DOF.
        int Node1ID = fElements[i].getNode1ID();
        int Node2ID = fElements[i].getNode2ID();
        
        if (fElements[i].getHinge1() == false) {    // Checks if the element is non-hinged at its 1st node.
            if (aux[Node1ID] == false) {    // Ensure element is the first non-hinged element to be connected at its 1st node.
                aux[Node1ID] = true;
                NDOF++;
            }
        }
        else {  // Case the element is hinged at its 1st node.
            NDOF++;
        }
        
        if (fElements[i].getHinge2() == false) {    // Checks if the element is non-hinged at its 2nd node.
            if (aux[Node2ID] == false) {    // Ensure element is the first non-hinged element to be connected at its 2nd node.
                aux[Node2ID] = true;
                NDOF++;
            }
        }
        else {  // Case the element is hinged at its 2nd node.
            NDOF++;
        }
    }

    return NDOF;
}

// getCDOF - returns the number of constrained degrees of freedom of the structure.
int TStructure::getCDOF() const {
    int CDOF = 0;    // DOF counter auxiliar variable.
    for (int i = 0; i < fSupports.size(); i ++) {   // Counts the number of constrained DOF.
        if (fSupports[i].getFx() == true) {
            CDOF++;
        }
        if (fSupports[i].getFy() == true) {
            CDOF++;
        }
        if (fSupports[i].getMx() == true) {
            CDOF++;
        }
    }
    return CDOF;
}

// getUDOF - returns the number of unconstrained degrees of freedom of the structure.
int TStructure::getUDOF() const {
    return (this->getNDOF() - this->getCDOF());
}

// setEquations - enumerates the DOF associated with each element.
void TStructure::setEquations() {

    TPZFMatrix<int> equations(fNodes.size(), 3, -1); // Matrix that stores the equations associated with each node.
    int CDOF = this->getCDOF();
    int count = this->getNDOF() - CDOF;
    
    for (int i = 0; i < fSupports.size(); i ++) {   // Enumerates the constrained DOF.
        if (fSupports[i].getFx() == true) {
            equations(fSupports[i].getNodeID(), 0) = count;
            count++;
        }
        if (fSupports[i].getFy() == true) {
            equations(fSupports[i].getNodeID(), 1) = count;
            count++;
        }
        if (fSupports[i].getMx() == true) {
            equations(fSupports[i].getNodeID(), 2) = count;
            count++;
        }
    }
    
    count = 0;
    for (int i = 0; i < fElements.size(); i++) {    // Enumerates the unconstrained DOF.
        int Node1ID = fElements[i].getNode1ID();
        int Node2ID = fElements[i].getNode2ID();
        int Node1RotationDOF;
        int Node2RotationDOF;
        
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
        
        if (equations(Node2ID, 0) == -1) {
            equations(Node2ID, 0) = count;
            count++;
        }
        if (equations(Node2ID, 1) == -1) {
            equations(Node2ID, 1) = count;
            count++;
        }
        if (fElements[i].getHinge2() == false) {
            if (equations(Node2ID, 2) == -1) {
                equations(Node2ID, 2) = count;
                count++;
            }
            Node2RotationDOF = equations(Node2ID, 2);
        }
        else {
            Node2RotationDOF = count;
            count++;
        }
        fElements[i].setEquations(equations(Node1ID, 0), equations(Node1ID, 1), Node1RotationDOF,
                                  equations(Node2ID, 0), equations(Node2ID, 1), Node2RotationDOF);
    }
}

// getK - returns the global stiffness matrix.
TPZFMatrix<double> TStructure::getK() const {
    int NDOF = this->getNDOF();
    TPZFMatrix<double> K(NDOF, NDOF, 0);
    
    for (int i = 0; i < fElements.size(); i++)
    {
        TElement elem = fElements[i];
        TPZFMatrix<double> kLocal = elem.getK();
        
        for (int aux1 = 0; aux1 < 6; aux1++)
        {
            for (int aux2 = 0; aux2 < 6; aux2++)
            {
                K(elem.getEquations()[aux1], elem.getEquations()[aux2]) += kLocal(aux1, aux2);
            }
        }
    }
    return K;
}

// getPartitionedK - returns the left upper block of the global stiffness matrix K.
TPZFMatrix<double> TStructure::getPartitionedK() const {
    int UDOF = this->getUDOF();
    TPZFMatrix<double> partitionedK(UDOF, UDOF);
    TPZFMatrix<double> K = this->getK();
    
    for (int i = 0; i < UDOF; i++) {
        for (int j = 0; j < UDOF; j++)
        {
            partitionedK(i, j) = K(i, j);
        }
    }
    return partitionedK;
}

// setNodes - modifies the vector of nodes.
void TStructure::setNodes(const std::vector<TNode>& Nodes)
{
	fNodes = Nodes;
}

// setMaterials - modifies the vector of materials.
void TStructure::setMaterials(const std::vector<TMaterial>& Materials)
{
	fMaterials = Materials;
}

// setSupports - modifies the vector of supports.
void TStructure::setSupports(const std::vector<TSupport>& Supports)
{
	fSupports = Supports;
}

// setElements - modifies the vector of elements.
void TStructure::setElements(const std::vector<TElement>& Elements)
{
	fElements = Elements;
}
