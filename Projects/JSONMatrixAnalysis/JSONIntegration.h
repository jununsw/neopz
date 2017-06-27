/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of functions
 that integrate JSON with the structure classes. */

#ifndef JSONINTEGRATION_H
#define JSONINTEGRATION_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "pzmatrix.h"
#include "pzfmatrix.h"		// PZ Matrix operations.
#include "json.hpp"			// JSON compatibility.

#include "TNode.h"			// TNode class.
#include "TMaterial.h"		// TMaterial class.
#include "TSupport.h"		// TSupport class.
#include "TElement.h"		// TElement class.
#include "TStructure.h"		// TStructure class.

#include "TNodalLoad.h"         // TNodalLoad class.
#include "TDistributedLoad.h"	// TDistributedLoad class.
#include "TElementLoad.h"       // TElementLoad class.

// Displays a JSON script. 
void printJSON(const nlohmann::json& J);

// Converts JSON into a TStructure object.
void readStructure(const nlohmann::json& J, TStructure& S);
// Converts JSON into a load vector.
void readLoads(const nlohmann::json& J, std::vector<double>& L, const TStructure& S);

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

#endif