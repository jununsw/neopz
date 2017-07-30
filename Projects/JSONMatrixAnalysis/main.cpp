#include <iostream>
#include <fstream>
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
	// Reads input JSON file and converts into the structure and the different loads objects.
	ifstream input("InputJSON.json");
	json J;
	input >> J;

	TStructure Structure;
	importStructure(J, Structure);

	vector<TNodalLoad> nLoads;
	vector<TDistributedLoad> dLoads;
	vector<TElementLoad> eLoads;
	importLoads(J, nLoads, dLoads, eLoads);

	// Enumerates the DOF and computes the Global Stiffness Matrix K.
	Structure.enumerateEquations();
	Structure.getK().Print(cout);

	// Creates the global vector of forces and displacements and calculates known external loads QK, unknown unconstrained displacements DU and unknown support loads QU.
	TPZFMatrix<double> Q = Structure.getQ();
	TPZFMatrix<double> D = Structure.getD();
	Structure.calculateQK(Q, nLoads, dLoads, eLoads);
	Q.Print(cout);
	Structure.calculateDU(Q, D);
	Structure.calculateQU(Q, D);

	// Displays results.
	D.Print(cout);
	Q.Print(cout);

	system("pause");
    return 0;
}