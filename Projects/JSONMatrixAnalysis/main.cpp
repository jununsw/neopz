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
	ifstream input("InputJSON.json");
	json j;
	input >> j;
    
	TStructure st;
	importStructure(j, st);

    st.enumerateEquations();
    st.getK().Print(cout);
    st.getK11().Print(cout);
	st.getK21().Print(cout);

    cout << "NDOF: " << st.getNDOF() << endl;
    cout << "CDOF: " << st.getCDOF() << endl;
    cout << "UDOF: " << st.getUDOF() << endl;
    st.getNodeEquations().Print(cout);

	vector<TNodalLoad> nLoads;
	vector<TDistributedLoad> dLoads;
	vector<TElementLoad> eLoads;
	importLoads(j, nLoads, dLoads, eLoads);

	TPZFMatrix<double> Q = st.getQ();
	Q.Print(cout);
	st.calculateQK(Q, nLoads, dLoads, eLoads);
	Q.Print(cout);

	TPZFMatrix<double> D = st.getD();
	D.Print(cout);
	st.calculateDU(Q, D);
	D.Print(cout);

	st.calculateQU(Q, D);
	Q.Print(cout);

	system("pause");
    return 0;
}