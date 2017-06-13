/* This file was created by Gustavo BATISTELA.
 It is a header file containing the definition of the TNode class
 and the declaration of its members. */

#ifndef TNODE_H
#define TNODE_H

#include <iostream>			// Basic input and output operations.
#include <string>			// Operations with strings.
#include "json.hpp"			// JSON compatibility.

// TNode class and declarations of its functions.
class TNode {
    
public:
    // Default constructor.
    TNode(double X = 0, double Y = 0);
    //Copy constructor.
    TNode(const TNode& N);
    // Destructor.
    ~TNode();
    
    // getX - accesses the node X coord.
    double getX() const;
    // getY - accesses the node Y coord.
    double getY() const;
    // getCoord - accesses the vector of coordinates.
    double* const getCoord();
    
    // setX - modifies the X coord of the node.
    void setX(double X);
    // setY - modifies the Y coord of the node.
    void setY(double Y);
    // setCoord - modifies the vector of coordinates.
    void setCoord(double* Coordinates);
    
    // Assignment operator.
    TNode& operator=(const TNode& N);
    
    // Function that prints the node information.
    void print();
    
private:
    double  fCoordinates[2];
};

#endif