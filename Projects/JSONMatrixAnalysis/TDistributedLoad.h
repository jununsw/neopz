/* This file was created by Gustavo BATISTELA.
It is a header file containing the definition of the TDistributedLoad class
and the declaration of its members. */

#ifndef TDISTRIBUTEDLOAD_H
#define TDISTRIBUTEDLOAD_H

#include <iostream>

// Forward declaration to TStructure class.
class TStructure;

// TNodalLoad class and declarations of its functions.
class TDistributedLoad {
public:
	// Default constructor.
	TDistributedLoad(int ElementID = -1, double Node0Load = 0,
					 double Node1Load = 0, bool LoadPlane = 0,
					 TStructure* Structure = nullptr);
	// Copy constructor.
	TDistributedLoad(const TDistributedLoad& DL);
	// Destructor.
	~TDistributedLoad();
	// Assignment operator.
	TDistributedLoad& operator=(const TDistributedLoad& DL);

	// getElementID - returns the ID of the element the load is applied to.
	int getElementID() const;
	// getNode0Load - returns the load applied to the element's local node 0.
	double getNode0Load() const;
	// getNode1Load - returns the load applied to the element's local node 1.
	double getNode1Load() const;
	// getLoadPlane - returns the boolean associated with the plane of the load.
	bool getLoadPlane() const;

	// setElementID - modifies the ID of the element the load is applied to.
	void setElementID(int ElementID);
	// setNode0Load - modifies the load applied to the element's local node 0.
	void setNode0Load(double Node0Load);
	// setNode1Load - modifies the load applied to the element's local node 1.
	void setNode1Load(double Node1Load);
	// setLoadPlane - modifies the boolean associated with the plane of the load.
	void setLoadPlane(bool LoadPlane);
	// setStructure - modifies the element's structure.
	void setStructure(TStructure* Structure);

	// store - adds the effects of the nodal load to the vector of loads.
	void store(TPZFMatrix<double>& L);

	// Function that prints the load information.
	void print();

private:
	// Member variables:
	int fElementID;
	double fNode0Load;
	double fNode1Load;
	bool fLoadPlane;
	TStructure* fStructure;
};

#endif