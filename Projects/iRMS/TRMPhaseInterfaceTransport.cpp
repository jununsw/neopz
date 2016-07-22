//
//  TRMPhaseInterfaceTransport.cpp
//  PZ
//
//  Created by omar duran on 5/05/2015.
//
//

#include "TRMPhaseInterfaceTransport.h"

/**
 * Empty Constructor
 */
TRMPhaseInterfaceTransport::TRMPhaseInterfaceTransport() : TPZMatWithMem<TRMPhaseInterfaceMemory, TPZDiscontinuousGalerkin>()
{
    
}

/** Creates a material object and inserts it in the vector of
 *  material pointers of the mesh.
 */
TRMPhaseInterfaceTransport::TRMPhaseInterfaceTransport(int matid) : TPZMatWithMem<TRMPhaseInterfaceMemory, TPZDiscontinuousGalerkin>(matid)
{
    
}


/** Creates a material object based on the referred object and
 *  inserts it in the vector of material pointers of the mesh.
 */
TRMPhaseInterfaceTransport::TRMPhaseInterfaceTransport(const TRMPhaseInterfaceTransport &mat) : TPZMatWithMem<TRMPhaseInterfaceMemory, TPZDiscontinuousGalerkin>(mat)
{
    
}

/**
 * Destructor
 */
TRMPhaseInterfaceTransport::~TRMPhaseInterfaceTransport()
{
    DebugStop();
}

/** Fill material data parameter with necessary requirements for the
 * Contribute method. Here, in base class, all requirements are considered
 * as necessary. Each derived class may optimize performance by selecting
 * only the necessary data.
 * @since April 10, 2007
 */
void TRMPhaseInterfaceTransport::FillDataRequirements(TPZVec<TPZMaterialData> &datavec)
{
    DebugStop();
}

void TRMPhaseInterfaceTransport::FillBoundaryConditionDataRequirement(int type,TPZVec<TPZMaterialData> &datavec)
{
    DebugStop();
}

/** print out the data associated with the material */
void TRMPhaseInterfaceTransport::Print(std::ostream &out)
{
    out << "\t Base class print:\n";
    out << " name of material : " << this->Name() << "\n";
    TPZMaterial::Print(out);
}


/** returns the variable index associated with the name */
int TRMPhaseInterfaceTransport::VariableIndex(const std::string &name)
{
    DebugStop();
    return 0;
}


/** returns the number of variables associated with the variable
 indexed by var.  var is obtained by calling VariableIndex */
int TRMPhaseInterfaceTransport::NSolutionVariables(int var)
{
    DebugStop();
    return 0;
}


/** Computes the divergence over the parametric space */
void TRMPhaseInterfaceTransport::ComputeDivergenceOnMaster(TPZVec<TPZMaterialData> &datavec, TPZFMatrix<STATE> &DivergenceofPhi, STATE &DivergenceofU)
{
    DebugStop();
}


/** returns the solution associated with the var index based on
 * the finite element approximation */
void TRMPhaseInterfaceTransport::Solution(TPZVec<TPZMaterialData> &datavec, int var, TPZVec<REAL> &Solout)
{
    DebugStop();
}





// Contribute Methods being used

/**
 * It computes a contribution to the stiffness matrix and load vector at one integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ek[out] is the stiffness matrix
 * @param ef[out] is the load vector
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::Contribute(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef)
{
    DebugStop();
}


/**
 * It computes a contribution to the load vector at one integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ef[out] is the load vector
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::Contribute(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ef)
{
    DebugStop();
}


/**
 * It computes a contribution to the stiffness matrix and load vector at one BC integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ek[out] is the stiffness matrix
 * @param ef[out] is the load vector
 * @param bc[in] is the boundary condition material
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::ContributeBC(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef, TPZBndCond &bc)
{
    DebugStop();
}


/**
 * It computes a contribution to the stiffness matrix and load vector at one BC interface integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ek[out] is the stiffness matrix
 * @param ef[out] is the load vector
 * @param bc[in] is the boundary condition material
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::ContributeBCInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef, TPZBndCond &bc)
{
    DebugStop();
}


/**
 * It computes a contribution to the stiffness matrix and load vector at one BC interface integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ek[out] is the stiffness matrix
 * @param ef[out] is the load vector
 * @param bc[in] is the boundary condition material
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::ContributeBCInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, REAL weight, TPZFMatrix<STATE> &ef, TPZBndCond &bc)
{
    DebugStop();
}



/**
 * It computes a contribution to the stiffness matrix and load vector at one internal interface integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ek[out] is the stiffness matrix
 * @param ef[out] is the load vector
 * @param bc[in] is the boundary condition material
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::ContributeInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, TPZVec<TPZMaterialData> &datavecright, REAL weight, TPZFMatrix<STATE> &ek,TPZFMatrix<STATE> &ef)
{
    if (!fSimulationData->IsCurrentStateQ()) {
        return;
    }
    
    int nvars = 4; // {p,sa,sb,t}
    int sb_a    = 0;
    
    TPZFNMatrix<100,STATE> phi_ss_l       = datavecleft[sb_a].phi;
    TPZFNMatrix<100,STATE> phi_ss_r       = datavecright[sb_a].phi;
    
    int nphis_a_l     = phi_ss_l.Rows();
    int firsts_a_l    = 0;
    
    int nphis_a_r     = phi_ss_r.Rows();
    int firsts_a_r    = firsts_a_l + nphis_a_l;
    
    TPZManVector<STATE,3> n = data.normal;

    REAL s_l                  = datavecleft[sb_a].sol[0][0];
    
    REAL s_r                  = datavecright[sb_a].sol[0][0];
    
    STATE un_l = 0.0, un_r = 0.0;// memory
    DebugStop();
    
    
    //  Average values p_a
    
    STATE p_a_l    = 0.0;
    STATE s_a_l    = 0.0;
    STATE p_a_r    = 0.0;
    STATE s_a_r    = 0.0;
    
    STATE beta = 0.0;
    // upwinding
    if (un_l > 0.0) {
        beta = 1.0;
    }
    
    TPZManVector<STATE, 10> fa_l,v_l(nvars+1),fa_r,v_r(nvars+1);
    v_l[0] = p_a_l;
    v_l[1] = s_a_l;
    v_r[0] = p_a_r;
    v_r[1] = s_a_r;
    
    this->fSimulationData->PetroPhysics()->fa(fa_l, v_l);
    this->fSimulationData->PetroPhysics()->fa(fa_r, v_r);
    
    TPZFNMatrix<3,STATE> phi_u_i_l(3,1);
    STATE phi_un_l = 0.0;
    int s_j;
    int v_j;
    
    for (int is = 0; is < nphis_a_l; is++) {
        
        ef(is + firsts_a_l) += +1.0*weight * (beta*fa_l[0] + (1.0-beta)*fa_r[0])*phi_ss_l(is,0)*un_l;
        
        for (int js = 0; js < nphis_a_l; js++) {
            ek(is + firsts_a_l, js + firsts_a_l) += +1.0*weight * beta * fa_l[2] * phi_ss_l(js,0) * phi_ss_l(is,0)*un_l;
        }
        
        for (int js = 0; js < nphis_a_r; js++) {
            ek(is + firsts_a_l, js + firsts_a_r) += +1.0*weight * (1.0-beta) * fa_r[2] * phi_ss_r(js,0) * phi_ss_l(is,0)*un_l;
        }
        
    }
    
    for (int is = 0; is < nphis_a_r; is++) {
        
        ef(is + firsts_a_r) += -1.0*weight * (beta*fa_l[0] + (1.0-beta)*fa_r[0])*phi_ss_r(is,0)*un_l;
        
        for (int js = 0; js < nphis_a_l; js++) {
            ek(is + firsts_a_r, js + firsts_a_l) += -1.0*weight * beta * fa_l[2] * phi_ss_l(js,0) * phi_ss_r(is,0)*un_l;
        }
        
        for (int js = 0; js < nphis_a_r; js++) {
            ek(is + firsts_a_r, js + firsts_a_r) += -1.0*weight * (1.0-beta) * fa_r[2] * phi_ss_r(js,0) * phi_ss_r(is,0)*un_l;
        }
        
    }
}


/**
 * It computes a contribution to the stiffness matrix and load vector at one internal interface integration point.
 * @param data[in] stores all input data
 * @param weight[in] is the weight of the integration rule
 * @param ef[out] is the load vector
 * @param bc[in] is the boundary condition material
 * @since April 16, 2007
 */
void TRMPhaseInterfaceTransport::ContributeInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, TPZVec<TPZMaterialData> &datavecright, REAL weight,TPZFMatrix<STATE> &ef)
{
    if (!fSimulationData->IsCurrentStateQ()) {
        return;
    }
    
    int nvars = 4; // {p,sa,sb,t}
    int sb_a    = 0;
    
    TPZFNMatrix<100,STATE> phi_ss_l       = datavecleft[sb_a].phi;
    TPZFNMatrix<100,STATE> phi_ss_r       = datavecright[sb_a].phi;
    
    int nphis_a_l     = phi_ss_l.Rows();
    int firsts_a_l    = 0;
    
    int nphis_a_r     = phi_ss_r.Rows();
    int firsts_a_r    = firsts_a_l + nphis_a_l;
    
    TPZManVector<STATE,3> n = data.normal;
    
    REAL s_l                  = datavecleft[sb_a].sol[0][0];
    
    REAL s_r                  = datavecright[sb_a].sol[0][0];
    
    STATE un_l = 0.0, un_r = 0.0;// memory
    DebugStop();
    
    
    //  Average values p_a
    
    STATE p_a_l    = 0.0;
    STATE s_a_l    = 0.0;
    STATE p_a_r    = 0.0;
    STATE s_a_r    = 0.0;
    
    STATE beta = 0.0;
    // upwinding
    if (un_l > 0.0) {
        beta = 1.0;
    }
    
    TPZManVector<STATE, 10> fa_l,v_l(nvars+1),fa_r,v_r(nvars+1);
    v_l[0] = p_a_l;
    v_l[1] = s_a_l;
    v_r[0] = p_a_r;
    v_r[1] = s_a_r;
    
    this->fSimulationData->PetroPhysics()->fa(fa_l, v_l);
    this->fSimulationData->PetroPhysics()->fa(fa_r, v_r);
    
    for (int is = 0; is < nphis_a_l; is++) {
        
        ef(is + firsts_a_l) += +1.0*weight * (beta*fa_l[0] + (1.0-beta)*fa_r[0])*phi_ss_l(is,0)*un_l;
        
    }
    
    for (int is = 0; is < nphis_a_r; is++) {
        
        ef(is + firsts_a_r) += -1.0*weight * (beta*fa_l[0] + (1.0-beta)*fa_r[0])*phi_ss_r(is,0)*un_l;
        
    }

}


/**
 * Unique identifier for serialization purposes
 */
int TRMPhaseInterfaceTransport::ClassId() const
{
    DebugStop();
}


/**
 * Save the element data to a stream
 */
void TRMPhaseInterfaceTransport::Write(TPZStream &buf, int withclassid)
{
    DebugStop();
}


/**
 * Read the element data from a stream
 */
void TRMPhaseInterfaceTransport::Read(TPZStream &buf, void *context)
{
    DebugStop();
}



/// Copy the n+1 data to the n data
void TRMPhaseInterfaceTransport::UpdateMemory()
{
    DebugStop();
}

