//
//  pznlfluidstructure2d.h
//  PZ
//
//  Created by Agnaldo Farias on 9/17/12.
//
//

#ifndef __PZ__TPZPlaneFractCouplingMat__
#define __PZ__TPZPlaneFractCouplingMat__

#include <iostream>

//#include "pzmaterial.h"
//#include "pzdiscgal.h"
//#include "pzvec.h"
//#include "pzfmatrix.h"



#include <iostream>
#include "TPZElast3Dnlinear.h"


class TPZPlaneFractCouplingMat : public TPZElast3Dnlinear
{
protected:
    enum EState { ELastState = 0, ECurrentState = 1 };
	static EState gState;
    
public:
    TPZPlaneFractCouplingMat();
    TPZPlaneFractCouplingMat(int nummat, STATE E, STATE poisson, TPZVec<STATE> &force,
                             STATE preStressXX, STATE preStressYY, STATE preStressZZ, STATE visc);
    
    ~TPZPlaneFractCouplingMat();
    
    //Soh para nao ficar mostrando warnings!
    using TPZElast3Dnlinear::TPZMaterial::Contribute;
    using TPZElast3Dnlinear::TPZMaterial::ContributeBC;
    using TPZElast3Dnlinear::TPZMaterial::Solution;
    using TPZElast3Dnlinear::FillDataRequirements;
    
    virtual void Contribute(TPZVec<TPZMaterialData> &datavec,
                            STATE weight,
                            TPZFMatrix<STATE> &ek,
                            TPZFMatrix<STATE> &ef);
    
    virtual void ContributePressure(TPZVec<TPZMaterialData> &datavec,
                                    REAL weight,
                                    TPZFMatrix<REAL> &ek,
                                    TPZFMatrix<REAL> &ef);
	
	virtual void ContributeBC(TPZVec<TPZMaterialData> &datavec,
                              STATE weight,
                              TPZFMatrix<STATE> &ek,
                              TPZFMatrix<STATE> &ef,
                              TPZBndCond &bc);
    
    virtual void Solution(TPZVec<TPZMaterialData> &datavec,
                          int var,
                          TPZVec<STATE> &Solout);
    
    virtual void ApplyDirichlet_U(TPZVec<TPZMaterialData> &datavec,
                                  STATE weight,
                                  TPZFMatrix<> &ek,
                                  TPZFMatrix<> &ef,
                                  TPZBndCond &bc);
    
    virtual void ApplyNeumann_U(TPZVec<TPZMaterialData> &datavec,
                                STATE weight,
                                TPZFMatrix<> &ek,
                                TPZFMatrix<> &ef,
                                TPZBndCond &bc);
    
    virtual void ApplyMixed_U(TPZVec<TPZMaterialData> &datavec,
                              STATE weight,
                              TPZFMatrix<> &ek,
                              TPZFMatrix<> &ef,
                              TPZBndCond &bc);
    
    virtual void ApplyNeumann_P(TPZVec<TPZMaterialData> &datavec,
                                STATE weight,
                                TPZFMatrix<> &ek,
                                TPZFMatrix<> &ef,
                                TPZBndCond &bc);
    
    virtual void FillDataRequirements(TPZVec<TPZMaterialData > &datavec);
    
    virtual void FillBoundaryConditionDataRequirement(int type,TPZVec<TPZMaterialData > &datavec);
    
private:
    
    STATE fVisc;
};

#endif /* defined(__PZ__TPZPlaneFractCouplingMat__) */
