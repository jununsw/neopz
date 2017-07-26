/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of functions
 that integrate JSON with the structure classes. */

#ifndef JSONINTEGRATION_H
#define JSONINTEGRATION_H

#include "pzfmatrix.h"		// PZ Matrix operations.
#include "json.hpp"			// JSON compatibility.

#include "TStructure.h"			// TStructure class.
#include "TNode.h"				// TNode class.
#include "TMaterial.h"			// TMaterial class.
#include "TSupport.h"			// TSupport class.
#include "TElement.h"			// TElement class.
#include "TNodalLoad.h"			// TNodalLoad class.
#include "TDistributedLoad.h"	// TDistributedLoad class.
#include "TElementLoad.h"		// TElementLoad class.

// Displays a JSON script. 
void printJSON(const nlohmann::json& J);

// Converts JSON into a TStructure object.
void readStructure(const nlohmann::json& J, TStructure& S);
// Converts JSON into a load vector.
void readLoads(const nlohmann::json& J, std::vector<TNodalLoad>& nLoads, std::vector<TDistributedLoad>& dLoads, std::vector<TElementLoad>& eLoads);

// TMaterial.
void to_json(nlohmann::json& J, const TMaterial& M);
void from_json(const nlohmann::json& J, TMaterial& M);
// TNode.
void to_json(nlohmann::json& J, const TNode& N);
void from_json(const nlohmann::json& J, TNode& N);
// TSupport.
void to_json(nlohmann::json& J, const TSupport& S);
void from_json(const nlohmann::json& J, TSupport& S);
// TElement.
void to_json(nlohmann::json& J, const TElement& E);
void from_json(const nlohmann::json& J, TElement& E);
// TNodalLoad.
void to_json(nlohmann::json& J, const TNodalLoad& NL);
void from_json(const nlohmann::json& J, TNodalLoad& NL);
// TDistributedLoad.
void to_json(nlohmann::json& J, const TDistributedLoad& DL);
void from_json(const nlohmann::json& J, TDistributedLoad& DL);
// TElementLoad.
void to_json(nlohmann::json& J, const TElementLoad& EL);
void from_json(const nlohmann::json& J, TElementLoad& EL);
#endif