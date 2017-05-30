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
    
    bool *aux = new bool[fNodes.size()];     // Auxiliar that keeps track if there are already non-hinged elements connected to a certain node.
    std::fill_n(aux, fNodes.size(), false);
    
    for (int i = 0; i < fElements.size(); i++) {
        TElement elem = fElements[i];
        int Node1ID = elem.getNode1ID();
        int Node2ID = elem.getNode2ID();
        
        if (elem.getHinge1() == true) {     // Checks if the element has a hinge at the first node.
            NDOF++;
        }
        else {  // Element has no hinge at the first node.
            if (aux[Node1ID] == false) {  // Checks if the element is the first non-hinged element to be connected to the node.
                NDOF++;
                aux[Node1ID] = true;
            }
        }
        
        if (elem.getHinge2() == true) {     // Checks if the element has a hinge at the second node.
            NDOF++;
        }
        else {  // Element has no hinge at the second node.
            if (aux[Node2ID] == false) {  // Checks if the element is the first non-hinged element to be connected to the node.
                NDOF++;
                aux[Node2ID] = true;
            }
        }
        
    }
    delete [] aux;
    return NDOF;
}

// setEquations - enumerates the equations related to each element.
void TStructure::setEquations() {

    int DOF = 1;
    TPZFMatrix<int> equations(fElements.size(), 6, -1); // Matrix which stores the equations of each element.
    TPZFMatrix<int> aux(fNodes.size(), 2, -1);  // Auxiliar matrix.
    bool *aux2 = new bool[fNodes.size()];   // Auxiliar that keeps track if there are already non-hinged elements connected to a certain node.
    std::fill_n(aux2, fNodes.size(), false);
    
    // Enumerates the unknown degrees of freedom.
    for (int i = 0; i < fElements.size(); i++)
    {
        int node1ID = fElements[i].getNode1ID();
        int node2ID = fElements[i].getNode2ID();
       
        // Node 1 uDOF enumeration.
        if (aux(node1ID, 0) == -1) // Checks if it is the first element to be connected to the node.
        {
            aux(node1ID, 0) = i; // Stores the first element connected to the node.
            aux(node1ID, 1) = 1; // Stores which node (1 or 2) of the element is connected.
            
            if (this->getSupportID(node1ID) == -1) // Check if there is no support restricting the node.
            {
                equations(i, 0) = DOF;
                DOF++;
                equations(i, 1) = DOF;
                DOF++;
                equations(i, 2) = DOF;
                DOF++;
                
                if (fElements[i].getHinge1() == false)  // If it is the first non-hinged element connected to the node.
                {
                    aux2[node1ID] = true;
                }
            }
            else    // When there is a support at the node.
            {
                if (fSupports[getSupportID(node1ID)].getFx() != true) {
                    equations(i, 0) = DOF;
                    DOF++;
                }
                if (fSupports[getSupportID(node1ID)].getFy() != true) {
                    equations(i, 1) = DOF;
                    DOF++;
                }
                if (fSupports[getSupportID(node1ID)].getMx() != true || fElements[i].getHinge1() == true) {
                    equations(i, 2) = DOF;
                    DOF++;
                }
                if (fElements[i].getHinge1() == false) {  // If it is the first non-hinged element connected to the node.
                    aux2[node1ID] = true;
                }
            }

        }
        else    // When an element has already been connected to the node.
        {
            if (aux(node1ID, 1) == 1)   // Checks if the element's node 1 is the one connected to the node.
            {
                equations(i, 0) = equations(aux(node1ID, 0), 0);
                equations(i, 1) = equations(aux(node1ID, 0), 1);
                
                if (fElements[i].getHinge1() == true) // Checks if the element has a hinge at node 1.
                {
                    equations(i, 2) = DOF;
                    DOF++;
                }
                else if (aux2[node1ID] == false)
                {
                    equations(i, 2) = DOF;
                    DOF++;
                    aux2[node1ID] = true;
                }
                else
                {
                    equations(i, 2) = equations(aux(node1ID, 0), 2);
                }
            }
            else if (aux(node1ID, 1) == 2)   // Checks if the element's node 2 is the one connected to the node.
            {
                equations(i, 0) = equations(aux(node1ID, 0), 3);
                equations(i, 1) = equations(aux(node1ID, 0), 4);
                
                if (fElements[i].getHinge1() == true) // Checks if the element has a hinge at node 1.
                {
                    equations(i, 2) = DOF;
                    DOF++;
                }
                else if (aux2[node1ID] == false)
                {
                    equations(i, 2) = DOF;
                    DOF++;
                    aux2[node1ID] = true;
                }
                else
                {
                    equations(i, 2) = equations(aux(node1ID, 0), 5);
                }
            }
        }
        
        // Node 2 uDOF enumeration.
        if (aux(node2ID, 0) == -1) // Checks if it is the first element to be connected to the node.
        {
            aux(node2ID, 0) = i; // Stores which element is firstly connected.
            aux(node2ID, 1) = 2; // Stores which node of the element is connected.
            
            if (this->getSupportID(node2ID) == -1) // Check if there is any support restricting the node.
            {
                equations(i, 3) = DOF;
                DOF++;
                equations(i, 4) = DOF;
                DOF++;
                equations(i, 5) = DOF;
                DOF++;
                
                if (fElements[i].getHinge2() == false)  // If it is the first non-hinged element connected to the node.
                {
                    aux2[node2ID] = true;
                }
            }
            else
            {
                if (fSupports[getSupportID(node2ID)].getFx() != true) {
                    equations(i, 3) = DOF;
                    DOF++;
                }
                if (fSupports[getSupportID(node2ID)].getFy() != true) {
                    equations(i, 4) = DOF;
                    DOF++;
                }
                if (fSupports[getSupportID(node2ID)].getMx() != true || fElements[i].getHinge2() == true) {
                    equations(i, 5) = DOF;
                    DOF++;
                }
                if (fElements[i].getHinge2() == false) {  // If it is the first non-hinged element connected to the node.
                    aux2[node2ID] = true;
                }
            }
            
        }
        else
        {
            if (aux(node2ID, 1) == 1)   // Checks if the element's node 1 is the one connected to the node.
            {
                equations(i, 3) = equations(aux(node2ID, 0), 0);
                equations(i, 4) = equations(aux(node2ID, 0), 1);
                
                if (fElements[i].getHinge2() == true) // Checks if the element has a hinge at node 1.
                {
                    equations(i, 5) = DOF;
                    DOF++;
                }
                else if (aux2[node2ID] == false)
                {
                    equations(i, 5) = DOF;
                    DOF++;
                    aux2[node2ID] = true;
                }
                else
                {
                    equations(i, 5) = equations(aux(node2ID, 0), 2);
                }
            }
            else if (aux(node2ID, 1) == 2)   // Checks if the element's node 2 is the one connected to the node.
            {
                equations(i, 3) = equations(aux(node2ID, 0), 3);
                equations(i, 4) = equations(aux(node2ID, 0), 4);
                
                if (fElements[i].getHinge2() == true) // Checks if the element has a hinge at node 1.
                {
                    equations(i, 5) = DOF;
                    DOF++;
                }
                else if (aux2[node2ID] == false)
                {
                    equations(i, 5) = DOF;
                    DOF++;
                    aux2[node2ID] = true;
                }
                else
                {
                    equations(i, 5) = equations(aux(node2ID, 0), 5);
                }
            }
        }
    }
    
    // Enumerates the known (restrict) degrees of freedom.
    for (int i = 0; i < fElements.size(); i++)
    {
        int node1ID = fElements[i].getNode1ID();
        int node2ID = fElements[i].getNode2ID();
        
        // Node 1 kDOF enumeration.
        if (aux(node1ID, 0) == i && aux(node1ID, 1) == 1) // Checks if it is the first element to be connected to the node.
        {
            if (this->getSupportID(node1ID) != -1) // Check if there is any support restricting the node.
            {
                if (equations(i, 0) == -1 && fSupports[getSupportID(node1ID)].getFx() == true)
                {
                    equations(i, 0) = DOF;
                    DOF++;
                }
                if (equations(i, 1) == -1 && fSupports[getSupportID(node1ID)].getFy() == true)
                {
                    equations(i, 1) = DOF;
                    DOF++;
                }
                if (equations(i, 2) == -1 && fSupports[getSupportID(node1ID)].getMx() == true)
                {
                    equations(i, 2) = DOF;
                    DOF++;
                }
            }
        }
        else
        {
            if (aux(node1ID, 1) == 1)
            {
                equations(i, 0) = equations(aux(node1ID, 0), 0);
                equations(i, 1) = equations(aux(node1ID, 0), 1);
                
                if (fElements[i].getHinge1() == false)  // Checks if the element is non-hinged.
                {
                    if (fElements[aux(node1ID, 0)].getHinge1() == false)
                    {
                        equations(i, 2) = equations(aux(node1ID, 0), 2);
                    }
                }
            }
            else if (aux(node1ID, 1) == 2)
            {
                equations(i, 0) = equations(aux(node1ID, 0), 3);
                equations(i, 1) = equations(aux(node1ID, 0), 4);
                
                if (fElements[i].getHinge1() == false)  // Checks if the element is non-hinged.
                {
                    if (fElements[aux(node1ID, 0)].getHinge2() == false)
                    {
                        equations(i, 2) = equations(aux(node1ID, 0), 5);
                    }
                }
            }
        }
        
        // Node 2 kDOF enumeration.
        if (aux(node2ID, 0) == i && aux(node2ID, 1) == 2) // Checks if it is the first element to be connected to the node.
        {
            if (this->getSupportID(node2ID) != -1) // Check if there is any support restricting the node.
            {
                if (equations(i, 3) == -1 && fSupports[getSupportID(node2ID)].getFx() == true)
                {
                    equations(i, 3) = DOF;
                    DOF++;
                }
                if (equations(i, 4) == -1 && fSupports[getSupportID(node2ID)].getFy() == true)
                {
                    equations(i, 4) = DOF;
                    DOF++;
                }
                if (equations(i, 5) == -1 && fSupports[getSupportID(node2ID)].getMx() == true)
                {
                    equations(i, 5) = DOF;
                    DOF++;
                }
            }
        }
        else
        {
            if (aux(node2ID, 1) == 1)
            {
                equations(i, 3) = equations(aux(node1ID, 0), 0);
                equations(i, 4) = equations(aux(node1ID, 0), 1);
                
                if (fElements[i].getHinge2() == false)  // Checks if the element is non-hinged.
                {
                    if (fElements[aux(node2ID, 0)].getHinge1() == false)
                    {
                        equations(i, 2) = equations(aux(node2ID, 0), 2);
                    }
                }
            }
            else if (aux(node2ID, 1) == 2)
            {
                equations(i, 3) = equations(aux(node1ID, 0), 3);
                equations(i, 4) = equations(aux(node1ID, 0), 4);
                
                if (fElements[i].getHinge2() == false)  // Checks if the element is non-hinged.
                {
                    if (fElements[aux(node2ID, 0)].getHinge2() == false)
                    {
                        equations(i, 2) = equations(aux(node2ID, 0), 5);
                    }
                }
            }
        }
    
        fElements[i].setEquations(equations(i, 0), equations(i, 1), equations(i, 2), equations(i, 3), equations(i, 4), equations(i, 5));
    }
}

// getK - returns the global stiffness matrix.
TPZFMatrix<double> TStructure::getK() const {
    int nDOF = this->getNDOF();
    TPZFMatrix<double> K(nDOF, nDOF, 0);
    
    for (int i = 0; i < fElements.size(); i++)
    {
        TElement elem = fElements[i];
        TPZFMatrix<double> kLocal = elem.getK();
        
        for (int aux1 = 0; aux1 < 6; aux1++)
        {
            for (int aux2 = 0; aux2 < 6; aux2++)
            {
                K(elem.getEquations()[aux1]-1, elem.getEquations()[aux2]-1) += kLocal(aux1, aux2);
            }
        }
    }
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
