
/**
 ************************************************************************** TPZEqnArray.h  -  description -------------------
 * begin                : Tue Jun 20 2000 copyright            : (C) 2000 by longhin
 * email                : longhin@fec.unicamp.br
 */

/**
 **************************************************************************
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 */

/* Generated by Together */

#ifndef TPZEQNARRAY_H
#define TPZEQNARRAY_H


#include "pzmatrix.h"
#include "pzstack.h"
#include "pzfmatrix.h"
#include "pzreal.h"
#include "pzmanvector.h"
#include "pzvec.h"

#include <stdlib.h>

#ifdef BORLAND
#include <stdio.h>
#endif

/**
 * @brief It is an equation array, generally in its decomposed form. \n
 * Would be saved and read from disk. \n
 * It contains a forward and a backward method. \n
 * It is sparce, symmetric or not.
 * @ingroup frontal
 */


class TPZEqnArray {     

/**
 *It defines two diferent types of storage Symmetric and NonSymmetric an also an undefined status
 */
enum MSymmetric {  EIsUndefined, EIsSymmetric, EIsNonSymmetric };

public:
    /**Static main function for testing */
	static void main();

    /**
     * Sets EqnArray to a non symmetric form 
     */
    void SetNonSymmetric();

    /**
     * Sets fSymmetric to the symmetric value 
     */
    void SetSymmetric();

    /**
     * Gets the symetry situation of EqnArray 
     */
    int IsSymmetric();

    /** Simple constructor */
    TPZEqnArray();
    /** Simple desctructor */
    ~TPZEqnArray();
    /** Forward substitution on equations stored in EqnArray */
    void EqnForward(
		    TPZFMatrix & F //! Matrix to execute a Forward substitution on
		    , DecomposeType dec /** Type of decomposition, depends on what method was used in its decomposition */
		    );
    /** Backward substitution on equations stored in EqnArray */
    void EqnBackward(
		     TPZFMatrix & U //! Matrix to execute a Forward substitutio
		     , DecomposeType dec /** Type of decomposition, depends on what method was used in its decomposition */
		     );

    /** Resets data structure */
    void Reset();
    /** It starts an equation storage */
    void BeginEquation(
		       int eq //! Indicates what equation is beeing added to the stack
		       );
    /** Add a term to the current equation */
    void AddTerm(
		 int col //! The collumn position of val
		 , REAL val //! The value beeing added itself
		 )
     {
	     fIndex.Push(col);
	     fEqValues.Push(val);
	     fLastTerm++;
     } 
    /** Ends the current equation */
    void EndEquation();

    /** Reads from disk */
    void Read(char * inputfile);

    /** Writes on disk */
    void Write(char * outputfile);

    /**
     * It prints all terms stored in TPZEqnArray 
     */
    void Print(const char * name, std::ostream & out);

    /**
     * Writes to a file in binary mode.
     * Used by FileEqnStorage
     * Receives FILE and position to execute 'C' fwrite function 
     */
    void Write(FILE * outputfile);

   /**
     * Reads from binary file generated by 'WriteBinary'
     * Also receives position and FILE for 'C' fread() function execution
     */
    void Read(FILE * inputfile);

   // static void main();

private:

    /**
     * Indicates the symetry or not of the equationarray 
     */
	MSymmetric fSymmetric;

 
    /** Number of equations */
    int fNumEq;

    /** Equation start point index */
    TPZStack < int , 100 > fEqStart;
    TPZStack < int , 100 > fEqNumber;

    /** Equations coefficients values */
    TPZStack < REAL, 1000 > fEqValues;

    /** Line/Column number associated to each fEqValues values. */
    TPZStack < int , 1000 > fIndex;

    /** Indicates the last used position in fEqValues */
    int fLastTerm;
};
#endif //TPZEQNARRAY_H
