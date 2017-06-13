/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions that integrate JSON with the structure classes. */

#include "JSONIntegration.h"

// JSON integration.

// Displays a JSON script.
void printJSON(const nlohmann::json& J) {
    std::cout << std::setw(4) << J << std::endl;
    std::cout << std::flush;
}

// Converts JSON into a TStructure object and vice-versa.
void readJSON(const nlohmann::json& J, TStructure& S) {
	// Reads the vector of TNode.
    nlohmann::json jNodes = J["Nodes"];
    std::vector<TNode> nodes;
    for (int i = 0; i < jNodes.size(); i++)
    {
        TNode node(jNodes.at(i)["X"][0].get<double>(), jNodes.at(i)["X"][1].get<double>());
        nodes.push_back(node);
    }
	S.setNodes(nodes);

    // Reads the vector of TMaterial.
    nlohmann::json jMaterials = J["Materials"];
    std::vector<TMaterial> materials(0);
    for (int i = 0; i < jMaterials.size(); i++)
    {
        TMaterial material(jMaterials.at(i)["E"].get<double>(), jMaterials.at(i)["A"].get<double>(), jMaterials.at(i)["I"].get<double>());
        materials.push_back(material);
    }
    S.setMaterials(materials);

	// Reads the vector of TSupport.
	nlohmann::json jSupports = J["Supports"];
	std::vector<TSupport> supports(0);
	for (int i = 0; i < jSupports.size(); i++)
	{
        bool fx = jSupports.at(i)["Conditions"][0].get<bool>();
        bool fy = jSupports.at(i)["Conditions"][1].get<bool>();
        bool m = jSupports.at(i)["Conditions"][2].get<bool>();
        int nodeID = jSupports.at(i)["Node"].get<int>();
		TSupport support(fx, fy, m, nodeID, &S);
		supports.push_back(support);
	}
	S.setSupports(supports);

	// Reads the vector of TElements.
	nlohmann::json jElements = J["Elements"];
	std::vector<TElement> elements(0);
	for (int i = 0; i < jElements.size(); i++)
	{
		int node1ID = jElements.at(i)["Nodes"][0].get<int>();
		int node2ID = jElements.at(i)["Nodes"][1].get<int>();
        bool hinge1 = jElements.at(i)["Hinges"][0].get<bool>();
        bool hinge2 = jElements.at(i)["Hinges"][1].get<bool>();
        int materialID = jElements.at(i)["Material"].get<int>();
        
		TElement element(node1ID, node2ID, hinge1, hinge2, materialID, &S);
		elements.push_back(element);
	}
	S.setElements(elements);
}

void writeJSON(nlohmann::json& J, const TStructure& S) {
    std::cout << "FOO" << std::endl << "FOO" << std::endl << "FOO" << std::endl;
}






/* TMaterial.
void to_json(nlohmann::json& J, const TMaterial& M) {
    J = nlohmann::json{ { "E", M.getE() },{ "A", M.getA() },
        { "I", M.getI() } };
}

void from_json(const nlohmann::json& J, TMaterial& M) {
    M.setE(J["E"].get<double>());
    M.setA(J["A"].get<double>());
    M.setI(J["I"].get<double>());
}

// TNode.
void to_json(nlohmann::json& J, TNode& N) {
    J = nlohmann::json{ { "X", N.getCoord() } };
}

void from_json(const nlohmann::json& J, TNode& N) {
    double coord[2] = {J["X"][0].get<double>(), J["X"][1].get<double>()};
    N.setCoord(coord);
}

// TSupport.
void to_json(nlohmann::json& J, const TSupport& S) {
    J = nlohmann::json{ { "Conditions", S.getConditions() },{ "Node", S.getNodeID() };
}

void from_json(const nlohmann::json& J, TSupport& S) {
    S.setFx(J.at("Fx").get<bool>());
    S.setFy(J.at("Fy").get<bool>());
    S.setM(J.at("M").get<bool>());
    S.setNodeID(J.at("Node").get<int>());
}

// TElement.
void to_json(nlohmann::json& J, const TElement& E) {
    J = nlohmann::json{ { "Nodes", E.getLocalNodesIDs() }, { "Material", E.getMaterialID() } };
}

void from_json(const nlohmann::json& J, TElement& E) {
    E.setLocalNodesIDs(J["Nodes"].get<std::vector<int>>());
    E.setMaterialID(J["Material"].get<TMaterial>());
}*/