/*
This file was created by Gustavo BATISTELA.
It contains the definitions of functions that integrate JSON with the
structure classes.
*/

#include "JSONIntegration.h"

// Displays a JSON script.
void printJSON(const nlohmann::json& J) {
	std::cout << std::setw(4) << J << std::endl;
	std::cout << std::flush;
}

// Reads the structure from the input JSON file.
void importStructure(const nlohmann::json& J, TStructure& S) {
	// Reads the vector of TNode.
	std::vector<TNode> nodes;
	for (int i = 0; i < J["Nodes"].size(); i++) {
		TNode node = J["Nodes"][i];
		nodes.push_back(node);
	}
	S.setNodes(nodes);

	// Reads the vector of TMaterial.
	std::vector<TMaterial> materials(0);
	for (int i = 0; i < J["Materials"].size(); i++) {
		TMaterial material = J["Materials"][i];
		materials.push_back(material);
	}
	S.setMaterials(materials);

	// Reads the vector of TSupport.
	std::vector<TSupport> supports(0);
	for (int i = 0; i < J["Supports"].size(); i++) {
		TSupport support = J["Supports"][i];
		supports.push_back(support);
	}
	S.setSupports(supports);

	// Reads the vector of TElements.
	std::vector<TElement> elements(0);
	for (int i = 0; i < J["Elements"].size(); i++) {
		TElement element = J["Elements"][i];
		element.setStructure(&S);
		elements.push_back(element);
	}
	S.setElements(elements);
}

// Reads the loads from the input JSON file.
void importLoads(const nlohmann::json& J, std::vector<TNodalLoad>& nLoads,
				 std::vector<TDistributedLoad>& dLoads,
				 std::vector<TElementLoad>& eLoads) {
	if (J.find("Nodal Loads") != J.end()) {
		nlohmann::json jNodalLoads = J["Nodal Loads"];
		for (int i = 0; i < jNodalLoads.size(); i++) {
			nLoads.push_back(TNodalLoad(J["Nodal Loads"][i]));
		}
	}

	if (J.find("Distributed Loads") != J.end()) {
		nlohmann::json jDistributedLoads = J["Distributed Loads"];
		for (int i = 0; i < jDistributedLoads.size(); i++) {
			dLoads.push_back(TDistributedLoad(J["Distributed Loads"][i]));
		}
	}

	if (J.find("Element Loads") != J.end()) {
		nlohmann::json jElementLoads = J["Element Loads"];
		for (int i = 0; i < jElementLoads.size(); i++) {
			eLoads.push_back(TElementLoad(J["Element Loads"][i]));
		}
	}
}

// TMaterial.
void to_json(nlohmann::json& J, const TMaterial& M) {
	J = nlohmann::json{ { "E", M.getE() },{ "A", M.getA() },{ "I", M.getI() } };
}

void from_json(const nlohmann::json& J, TMaterial& M) {
	M.setE(J.at("E").get<double>());
	M.setA(J.at("A").get<double>());
	M.setI(J.at("I").get<double>());
}

// TNode.
void to_json(nlohmann::json& J, const TNode& N) {
	J = nlohmann::json{ N.getX(), N.getY() };
}

void from_json(const nlohmann::json& J, TNode& N) {
	N.setX(J[0].get<double>());
	N.setY(J[1].get<double>());
}

// TSupport.
void to_json(nlohmann::json& J, const TSupport& S) {
	J = nlohmann::json{ { "Conditions",{ S.getFx(), S.getFy(), S.getM() } },
	{ "Node", S.getNodeID() } };
}

void from_json(const nlohmann::json& J, TSupport& S) {
	S.setFx(J["Conditions"][0].get<bool>());
	S.setFy(J["Conditions"][1].get<bool>());
	S.setM(J["Conditions"][2].get<bool>());
	S.setNodeID(J["Node"].get<int>());
}

// TElement.
void to_json(nlohmann::json& J, const TElement& E) {
	J = nlohmann::json{ { "Nodes",{ E.getNode0ID(), E.getNode1ID() } },
	{ "Hinges",{ E.getHinge0(), E.getHinge1() } },
	{ "Material", E.getMaterialID() } };
}

void from_json(const nlohmann::json& J, TElement& E) {
	E.setNode0ID(J["Nodes"][0].get<int>());
	E.setNode1ID(J["Nodes"][1].get<int>());
	E.setHinge0(J["Hinges"][0].get<bool>());
	E.setHinge1(J["Hinges"][1].get<bool>());
	E.setMaterialID(J["Material"].get<int>());
}

// TNodalLoad.
void to_json(nlohmann::json& J, const TNodalLoad& NL) {
	J = nlohmann::json{ { "Node", NL.getNodeID() },
	{ "Fx", NL.getFx() },
	{ "Fy", NL.getFy() },
	{ "M", NL.getM() } };
}

void from_json(const nlohmann::json& J, TNodalLoad& NL) {
	NL.setNodeID(J["Node"].get<int>());
	NL.setFx(J["Fx"].get<double>());
	NL.setFy(J["Fy"].get<double>());
	NL.setM(J["M"].get<double>());
}

// TDistributedLoad.
void to_json(nlohmann::json& J, const TDistributedLoad& DL) {
	J = nlohmann::json{ { "Element", DL.getElementID() },
	{ "Node 0 Load", DL.getNode0Load() },
	{ "Node 1 Load", DL.getNode1Load() },
	{ "Load Plane", DL.getLoadPlane() } };
}

void from_json(const nlohmann::json& J, TDistributedLoad& DL) {
	DL.setElementID(J["Element"].get<int>());
	DL.setNode0Load(J["Node 0 Load"].get<double>());
	DL.setNode1Load(J["Node 1 Load"].get<double>());
	DL.setLoadPlane(J["Load Plane"].get<bool>());
}

// TElementLoad.
void to_json(nlohmann::json& J, const TElementLoad& EL) {
	J = nlohmann::json{ { "Element", EL.getElementID() },
	{ "Node", EL.getNode() },
	{ "Fx", EL.getFx() },
	{ "Fy", EL.getFy() },
	{ "M", EL.getM() } };
}

void from_json(const nlohmann::json& J, TElementLoad& EL) {
	EL.setElementID(J["Element"].get<int>());
	EL.setNode(J["Node"].get<int>());
	EL.setFx(J["Fx"].get<double>());
	EL.setFy(J["Fy"].get<double>());
	EL.setM(J["M"].get<double>());
}