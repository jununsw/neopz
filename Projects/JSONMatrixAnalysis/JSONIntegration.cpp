/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions that integrate JSON with the structure classes. */

#include "JSONIntegration.h"

// Displays a JSON script.
void printJSON(const nlohmann::json& J) {
    std::cout << std::setw(4) << J << std::endl;
    std::cout << std::flush;
}

// Converts JSON into a TStructure object and vice-versa.
void readStructure(const nlohmann::json& J, TStructure& S) {
    
	// Reads the vector of TNode.
    std::vector<TNode> nodes;
    for (int i = 0; i < J["Nodes"].size(); i++)
    {       
        TNode node = J["Nodes"][i];
        nodes.push_back(node);
    }
	S.setNodes(nodes);

    // Reads the vector of TMaterial.
    std::vector<TMaterial> materials(0);
    for (int i = 0; i < J["Materials"].size(); i++)
    {
        TMaterial material= J["Materials"][i];
        materials.push_back(material);
    }
    S.setMaterials(materials);

	// Reads the vector of TSupport.
	std::vector<TSupport> supports(0);
	for (int i = 0; i < J["Supports"].size(); i++)
	{
		TSupport support = J["Supports"][i];
		supports.push_back(support);
	}
	S.setSupports(supports);

	// Reads the vector of TElements.
	std::vector<TElement> elements(0);
	for (int i = 0; i < J["Elements"].size(); i++)
	{
        //int c = i;
		/*int node1ID = J["Elements"][i]["Nodes"][0].get<int>();
		int node2ID = J["Elements"][i]["Nodes"][1].get<int>();
        bool hinge1 = J["Elements"][i]["Hinges"][0].get<bool>();
        bool hinge2 = J["Elements"][i]["Hinges"][1].get<bool>();
        int materialID = J["Elements"][i]["Material"].get<int>();
        
		TElement element(node1ID, node2ID, hinge1, hinge2, materialID, &S);*/
        TElement element = J["Elements"][i];
		elements.push_back(element);
	}
	S.setElements(elements);
}

void readLoads(const nlohmann::json& J, std::vector<TNodalLoad>& nodalLoads, std::vector<TElementLoad>& elementLoads, std::vector<TDistributedLoad>& distributedLoads)
{
    
    nlohmann::json jNodalLoads = J["Nodal Loads"];
    for (int i = 0; i < jNodalLoads.size(); i++)
    {
        int nodeID = jNodalLoads.at(i)["Node"].get<int>();
        double fx = jNodalLoads.at(i)["Fx"].get<double>();
        double fy = jNodalLoads.at(i)["Fy"].get<double>();
        double m = jNodalLoads.at(i)["M"].get<double>();
        
        TNodalLoad nLoad(fx, fy, m, nodeID);
        nodalLoads.push_back(nLoad);
    }
}

// TMaterial.
void to_json(nlohmann::json& J, const TMaterial& M) {
    J = nlohmann::json{{"E", M.getE()}, {"A", M.getA()}, {"I", M.getI()}};
}
void from_json(const nlohmann::json& J, TMaterial& M) {
    M.setE(J.at("E").get<double>());
    M.setA(J.at("A").get<double>());
    M.setI(J.at("I").get<double>());
}

// TNode.
void to_json(nlohmann::json& J, const TNode& N){
    J = nlohmann::json{N.getX(), N.getY()};
}

void from_json(const nlohmann::json& J, TNode& N){
    N.setX(J[0].get<double>());
    N.setY(J[1].get<double>());
}

// TSupport.
void to_json(nlohmann::json& J, const TSupport& S) {
    J = nlohmann::json{{"Conditions", {S.getFx(), S.getFy(), S.getM()}}, {"Node", S.getNodeID()}};
}

void from_json(const nlohmann::json& J, TSupport& S) {
    S.setFx(J["Conditions"][0].get<bool>());
    S.setFy(J["Conditions"][1].get<bool>());
    S.setM(J["Conditions"][2].get<bool>());
    S.setNodeID(J["Node"].get<int>());
}

// TElement.
void to_json(nlohmann::json& J, const TElement& E) {
    J = nlohmann::json{{"Nodes", {E.getNode0ID(), E.getNode1ID()}}, {"Hinges", {E.getHinge0(), E.getHinge1()}}, {"Material", E.getMaterialID()}};
}

void from_json(const nlohmann::json& J, TElement& E) {
    E.setNode0ID(J["Nodes"][0].get<int>());
    E.setNode1ID(J["Nodes"][1].get<int>());
    E.setHinge0(J["Hinges"][0].get<bool>());
    E.setHinge1(J["Hinges"][1].get<bool>());
    E.setMaterialID(J["Material"].get<int>());
}






