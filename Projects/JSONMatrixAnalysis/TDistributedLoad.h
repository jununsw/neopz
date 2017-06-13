/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TDistributedLoad class
 and the declaration of its members. */

#ifndef TDISTRIBUTEDLOAD_H
#define TDISTRIBUTEDLOAD_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

// TNodalLoad class and declarations of its functions.
class TDistributedLoad {
    
public:
    // LoadType - enum argument which determines if the load will be given in terms of local or global reference axis.
    enum LoadType {ELocal, EGlobal};
    
    // Default constructor.
    TDistributedLoad(int ElementID = -1, double LoadNode0 = 0, double LoadNode1 = 0, LoadType LT = EGlobal);
    //Copy constructor.
    TDistributedLoad(const TDistributedLoad& DL);
    // Destructor.
    ~TDistributedLoad();
        
    // Assignment operator.
    TDistributedLoad& operator= (const TDistributedLoad& DL);
    
    // Function that prints the load information.
    void print();
    
private:
    // Member variables:
    int fElementID;
    double fLoadNode0;
    double fLoadNode1;
    LoadType fLT;
};

#endif