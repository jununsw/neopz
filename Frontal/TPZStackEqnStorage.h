/* Generated by Together */

#ifndef TPZSTACKEQNSTORAGE_H
#define TPZSTACKEQNSTORAGE_H
//#include "TPZFrontMatrix.h"

#include "tpzeqnarray.h"
#include "pzstack.h"
/** @brief Responsible for storing arrays of equations (mostly in a decomposed form)
 * It has methods for operating over a set of equations
 * The arrays of equations are in the form of a Stack of EqnArrays
 * @ingroup frontal
 */
class TPZStackEqnStorage {
public:

     /**
      * method to make this class "template compatible" with the file equation storage
      */
     void ReOpen();

     /**
     *  Reinitialize the object
     */
     void Zero();
    /**
     * It closes the opened binary file. 
     */
	void FinishWriting();

    /**
     * Only to make both possible templates similar in terms of methods and constructors 
     */
	TPZStackEqnStorage(char option, const char * name);
    /** Static main for testing */
	static void main();
    /** Simple Destructor */
    ~TPZStackEqnStorage();
    /** Simple Constructor */
    TPZStackEqnStorage();
    
    TPZStackEqnStorage(const TPZStackEqnStorage &cp) : fEqnStack(cp.fEqnStack)
    {
    }
    /** Adds an EqnArray to EqnStack object
    *@param *EqnArray Pointer to EqnArray to be added to the Stack
    */

    void AddEqnArray(
        TPZEqnArray *EqnArray //!Pointer to EqnArray to be added to the Stack
        );

    /**
     * It prints TPZEqnStorage data. 
     */
    void Print(
        const char *name //!file title to print to
        , std::ostream& out   //!object type file
        );
    /**Resets data structure */
    void Reset();
    /** Executes a Backward substitution Stack object */
    void Backward(
        TPZFMatrix &f //!Matrix to apply Backward substitution on
        , DecomposeType dec /** Decomposition type of f,
                                depends on what decomposition method
                                was used to decompose f
                               */
        ) const ;

    /** Executes a Forward substitution Stack object */
    void Forward(
        TPZFMatrix &f //!Matrix to apply Forward substitution on
        , DecomposeType dec /** Decomposition type of f
                                Depends on what decomposition method
                                was used to decompose f
                               */
        ) const;

	//Only to make it compatible with FIleEqnStorage


    /**
     * Only to make both possible templates similar in terms of methods and constructors 
     */
	//void SetFileName(const char *name);

    /**
     * Only to make both possible templates similar in terms of methods and constructors 
     */
	void OpenGeneric(char option, const char * name);

    /**
     * Only to make both possible templates similar in terms of methods and constructors 
     */
	void ReadBlockPositions();
	
	/**
	 *Name of Storage
	 */
	std::string GetStorage();
private:
    /** Sets the block size to be used */
    void SetBlockSize();
    /** Defines a stack of EqnArrays */
    TPZStack<TPZEqnArray> fEqnStack;

    /** @label Several objects are stored within a stack object
     * @directed
     * @link association*/
    /*#  TPZEqnArray lnkTPZEqnArray; */
};
#endif //TPZSTACKEQNSTORAGE_H
