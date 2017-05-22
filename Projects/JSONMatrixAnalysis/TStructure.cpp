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
    
    std::vector<TElement> elements = this->getElements();
    std::vector<TNode> nodes = this->getNodes();
    std::vector<TSupport> supports = this->getSupports();
    
    int NDOF = 2*nodes.size();  // Initial number of DOF, considering only vertical and horizontal displacements.
    
    bool aux[nodes.size()];     // Auxiliar variable that keeps track if there are already non-hinged elements connected to a certain node.
    std::fill_n(aux, nodes.size(), false);
    
    for (int i = 0; i < elements.size(); i++) {
        TElement elem = elements[i];
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
    
    return NDOF;
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
