/* This file was created by Gustavo ALCALA BATISTELA.
 It is a header file containing the definition of the TDistributedLoad class
 and the declaration of its members.*/

#ifndef TDISTRIBUTEDLOAD_H
#define TDISTRIBUTEDLOAD_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

// TNodalLoad class and declarations of its functions.
class TDistributedLoad {
    
public:
    // Default constructor.
    TDistributedLoad(int ElementID = -1, double LoadNode1 = 0, double LoadNode2 = 0, LoadType LT = EGlobal);
    //Copy constructor.
    TDistributedLoad(const TDistributedLoad& D);
    // Destructor.
    TDistributedLoad();
        
    // Assignment operator.
    TDistributedLoad& operator= (const TDistributedLoad& D);
    
    // Function that prints the load information.
    void print();
    
private:
    // Member variables:
    int fElementID;
    double fLoadNode1;
    double fLoadNode2;
    enum LoadType {ELocal, EGlobal};
};

#endif