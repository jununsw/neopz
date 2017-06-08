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
    
    structure.getK().Print(cout);
    structure.getPartitionedK().Print(cout);

    cout << "NDOF: " << structure.getNDOF() << endl;
    cout << "CDOF: " << structure.getCDOF() << endl;
    cout << "UDOF: " << structure.getUDOF() << endl;
    
    return 0;
}