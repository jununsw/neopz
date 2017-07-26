#include <iostream>
#include <fstream>
#include <iostream>

#include "TMaterial.h"
#include "TNode.h"
#include "TSupport.h"
#include "TElement.h"
#include "TNodalLoad.h"
#include "TDistributedLoad.h"
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
	readStructure(j, structure);

    structure.setEquations();
    structure.getK().Print(cout);
    structure.getK11().Print(cout);
	structure.getK21().Print(cout);

    cout << "NDOF: " << structure.getNDOF() << endl;
    cout << "CDOF: " << structure.getCDOF() << endl;
    cout << "UDOF: " << structure.getUDOF() << endl;
    structure.getNodeEquations().Print(cout);

	vector<TNodalLoad> nLoads;
	vector<TDistributedLoad> dLoads;
	vector<TElementLoad> eLoads;
	readLoads(j, nLoads, dLoads, eLoads);

	TPZFMatrix<double> loads(structure.getNDOF(), 1, 0);
	structure.getLoads(loads, nLoads, dLoads, eLoads);
	loads.Print(cout);

	TPZFMatrix<double> DU(structure.getUDOF(), 1, 0);
	structure.getDU(loads, DU);
	DU.Print(cout);

	TPZFMatrix<double> SupportReactions(structure.getCDOF(), 1, 0);
	structure.getSupportLoads(DU, SupportReactions);
	SupportReactions.Print(cout);

	system("pause");
    return 0;
}