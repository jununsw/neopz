/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of _ classes.*/

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
    
    int NDOF = 2*fNodes.size();  // Initial number of DOF, considering only vertical and horizontal displacements.
    
    bool *aux = new bool[fNodes.size()];     // Auxiliar variable that keeps track if there are already non-hinged elements connected to a certain node.
    std::fill_n(aux, fNodes.size(), false);
    
    for (int i = 0; i < fElements.size(); i++) {
        TElement elem = fElements[i];
        int Node1ID = elem.getNode1ID();
        int Node2ID = elem.getNode2ID();
        
        if (elem.getHinge1() == true)
        {
            NDOF++;
        }
        else
        {
            if (aux[Node1ID] == false)
            {
                NDOF++;
                aux[Node1ID] = true;
            }
        }
        
        if (elem.getHinge2() == true)
        {
            NDOF++;
        }
        else
        {
            if (aux[Node2ID] == false)
            {
                NDOF++;
                aux[Node2ID] = true;
            }
        }
        
    }
    delete aux;
    return NDOF;
}

// setEquations - enumerates the equations related to each element.
void TStructure::setEquations() {

    int DOF = 1;
    TPZFMatrix<int> equations(fElements.size(), 6, 0);
    
    // Enumerate the unknown degrees of freedom.
    for (int i = 0; i < fElements.size(); i++)
    {
        int node1ID = fElements[i].getNode1ID();
        int node2ID = fElements[i].getNode2ID();
        // Node 1 enumeration.
        if (this->getSupportID(node1ID) == -1)
        {
            equations(i, 0) = DOF;
            DOF++;
            equations(i, 1) = DOF;
            DOF++;
            equations(i, 2) = DOF;
            DOF++;
        }
        else
        {
            if (fSupports[getSupportID(node1ID)].getFx() != true) {
                equations(i, 0) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getFy() != true) {
                equations(i, 1) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getMx() != true) {
                equations(i, 2) = DOF;
                DOF++;
            }
        }
        // Node 2 enumeration.
        if (this->getSupportID(node2ID) == -1)
        {
            equations(i, 3) = DOF;
            DOF++;
            equations(i, 4) = DOF;
            DOF++;
            equations(i, 5) = DOF;
            DOF++;
        }
        else
        {
            if (fSupports[getSupportID(node1ID)].getFx() != true) {
                equations(i, 3) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getFy() != true) {
                equations(i, 4) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getMx() != true) {
                equations(i, 5) = DOF;
                DOF++;
            }
        }
        
    }
    // Enumerate the restrict degrees of freedom.
    for (int i = 0; i < fElements.size(); i++)
    {
        int node1ID = fElements[i].getNode1ID();
        int node2ID = fElements[i].getNode2ID();
        // Node 1 enumeration.
        if (this->getSupportID(node1ID) != -1)
        {
            if (fSupports[getSupportID(node1ID)].getFx() == true) {
                equations(i, 0) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getFy() == true) {
                equations(i, 1) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getMx() == true) {
                equations(i, 2) = DOF;
                DOF++;
            }
        }
        // Node 2 enumeration.
        if (this->getSupportID(node2ID) != -1)
        {
            if (fSupports[getSupportID(node1ID)].getFx() == true) {
                equations(i, 3) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getFy() == true) {
                equations(i, 4) = DOF;
                DOF++;
            }
            if (fSupports[getSupportID(node1ID)].getMx() == true) {
                equations(i, 5) = DOF;
                DOF++;
            }
        }
        int localEquations[6] = {equations(i, 0), equations(i, 1), equations(i, 2), equations(i, 3), equations(i, 4), equations(i, 5)};
        fElements[i].setEquations(localEquations);
    }
}

// getK - returns the global stiffness matrix.
TPZFMatrix<double> TStructure::getK() const {
    int nDOF = this->getNDOF();
    TPZFMatrix<double> K(nDOF, nDOF, 0);
    
    return K;
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
