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

    cout << endl << "NDOF: " << structure.getNDOF() << endl;
    return 0;
}
