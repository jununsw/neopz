/* This file was created by Gustavo BATISTELA.
 It contains the definitions of functions of the TDistributedLoad class. */

#include "TDistributedLoad.h"

// Default constructor.
TDistributedLoad::TDistributedLoad(int ElementID, double LoadNode0, double LoadNode1, LoadType LT)
    : fElementID(ElementID), fLoadNode0(LoadNode0), fLoadNode1(LoadNode1), fLT(LT) { }

//Copy constructor.
TDistributedLoad::TDistributedLoad(const TDistributedLoad& DL)
    : fElementID(DL.fElementID), fLoadNode0(DL.fLoadNode0), fLoadNode1(DL.fLoadNode1), fLT(DL.fLT) { }

// Destructor.
TDistributedLoad::~TDistributedLoad() { }

// Assignment operator.
TDistributedLoad& TDistributedLoad::operator= (const TDistributedLoad& DL) {
    if (this != &DL) {
        fElementID = DL.fElementID;
        fLoadNode0 = DL.fLoadNode0;
        fLoadNode1 = DL.fLoadNode1;
        fLT = DL.fLT;
    }
    return *this;
}

// Function that prints the load information.
void TDistributedLoad::print() {
    std::cout << "Distributed Load Info: " << std::endl
    << " Element ID: " << fElementID << std::endl
    << " Node 0: " << fLoadNode0 << std::endl
    << " Node 1: " << fLoadNode1 << std::endl
    << " Load Type: " << fLT << std::endl;
    std::cout << std::flush;
}