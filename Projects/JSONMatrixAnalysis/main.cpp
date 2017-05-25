#include <iostream>
#include <fstream>
#include "TMaterial.h"
#include "TNode.h"
#include "TSupport.h"
#include "TElement.h"
#include "JSONIntegration.h"
#include "pzfmatrix.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main()
{
	ifstream input("InputJSON.json");

	json j;
	input >> j;
    
	TStructure structure;
	readJSON(j, structure);
    structure.setEquations();
    
	vector<TNode> nodes = structure.getNodes();
	vector<TMaterial> materials = structure.getMaterials();
	vector<TSupport> supports = structure.getSupports();
	vector<TElement> elements = structure.getElements();
    
    cout << endl << "Elements: " << elements.size() << endl;
    cout << "Nodes: " << nodes.size() << endl;
    cout << "Materials: " << materials.size() << endl;
    cout << "Supports: " << supports.size() << endl << endl;
    
	for (int i = 0; i < elements.size(); i++)
	{
		elements[i].print();
		cout << endl;
        TPZFMatrix<double> k = elements[i].getK();
        k.Print(cout);
	}
    int eq[6] = {4,6,5,1,2,3};
    elements[0].setEquations(eq);
    int eq2[6] = {1,2,3,7,8,9};
    elements[1].setEquations(eq2);
    structure.getK().Print(cout);

    cout << endl << "NDOF: " << structure.getNDOF() << endl;
    return 0;
}