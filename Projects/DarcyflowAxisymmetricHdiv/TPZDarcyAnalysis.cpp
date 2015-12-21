    /*
 *  pznondarcyanalysis.cpp
 *  PZ
 *
 *  Created by Omar Duran Triana on 5/21/14.
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#include "TPZDarcyAnalysis.h"
#include "pzcheckgeom.h"
#include "pzlog.h"

#include "TPZReadGIDGrid.h"
#include "tpzgeoelrefpattern.h"
#include "TPZRefPatternTools.h"
#include "TPZGeoLinear.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"
#include "pzgeopoint.h"

#include "tpzhierarquicalgrid.h"
#include "pzelementgroup.h"

#include "TPZVTKGeoMesh.h"
#include "TPZAxiSymmetricDarcyFlow.h"
#include "pzpoisson3d.h"
#include "pzbuildmultiphysicsmesh.h"
#include "TPZSkylineNSymStructMatrix.h"
#include "pzfstrmatrix.h"

#include "TPZParFrontStructMatrix.h"
#include "TPZFrontSym.h"
#include "TPZFrontNonSym.h"

#include "math.h"
#include <boost/cstdfloat.hpp> // For float_64_t. Must be first include!
#include <cmath>  // for pow function.
#include <boost/math/special_functions.hpp> // For gamma function.

#include "TPZMultiphysicsInterfaceEl.h"


#ifdef PZDEBUG
#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.DarcyFlow"));
#endif
#endif

//#define SolutionI
//#define SolutionII

TPZDarcyAnalysis::TPZDarcyAnalysis(TPZAutoPointer<SimulationData> DataSimulation, TPZVec<TPZAutoPointer<ReservoirData> > Layers, TPZVec<TPZAutoPointer<PetroPhysicData> > PetroPhysic)
{
    
    fmeshvecini.Resize(2);
    fmeshvec.Resize(2);
    
    fgmesh=NULL;
    fcmeshinitialdarcy=NULL;
    fcmeshdarcy=NULL;
    fcmesh = NULL;
    
    falpha_fluid=NULL;
    fbeta_fluid=NULL;
    fgamma_fluid=NULL;
    
    // Vector which will store tha residuum in the last state (n)
    fResidualAtn.Resize(0, 0);
    
    // Vector which will store tha residuum in the last state (n+1)
    fResidualAtnplusOne.Resize(0, 0);
    
    fSimulationData     = DataSimulation;
    fLayers             = Layers;
    fRockPetroPhysic    = PetroPhysic;
    
    /** @brief unknowns for n time step */
    falphaAtn.Resize(0, 0);
    
    /** @brief unknowns for n+1 time step */
    falphaAtnplusOne.Resize(0, 0);
    
    /** @brief unknowns saturations for n time step */
    fSAtn.Resize(0, 0);
    
    /** @brief unknowns saturations for n+1 time step */
    fSAtnplusOne.Resize(0, 0);
    
    /** @brief Store DOF associated with active */
    fActiveEquations.Resize(0);
    
    /** @brief Store DOF associated with  non active */
    fNonactiveEquations.Resize(0);
    
    /** @brief L2 norm */
    fL2_norm.Resize(1,0.0);

    /** @brief L2 norm */
    fL2_norm_s.Resize(1,0.0);
    
    /** @brief Hdiv norm */
    fl2_norm_flux.Resize(1,0.0);
    
}


TPZDarcyAnalysis::~TPZDarcyAnalysis()
{
    
}


void TPZDarcyAnalysis::SetFluidData(TPZVec< TPZAutoPointer<Phase> > PVTData){
    
//    PVTData[0] = Water.operator->();
//    PVTData[1] = Oil.operator->();
//    PVTData[2] = Gas.operator->();
    TPZStack<std::string> System =  fSimulationData->GetsystemType();
    int nphases = System.size();
    
    if (fSimulationData->IsOnePhaseQ()) {
        
        for(int iphase = 0; iphase < nphases; iphase++){
        
            if (!strcmp("Water", System[iphase].c_str())){
                falpha_fluid = PVTData[0];
                fbeta_fluid = PVTData[1];
                fgamma_fluid = PVTData[2];
            }
            
            if (!strcmp("Oil", System[iphase].c_str())){
                falpha_fluid = PVTData[1];
                fbeta_fluid  = PVTData[0];
                fgamma_fluid = PVTData[2];                
            }
            
            if (!strcmp("Gas", System[iphase].c_str())){
                falpha_fluid = PVTData[2];
                fbeta_fluid = PVTData[0];
                fgamma_fluid = PVTData[1];
            }

        }

    }
    
    if(fSimulationData->IsTwoPhaseQ()){
        
        fmeshvecini.Resize(3);
        fmeshvec.Resize(3);
        
        for(int iphase = 0; iphase < nphases; iphase++){
            
            switch (iphase) {
                case 0:
                {
                    if (!strcmp("Water", System[iphase].c_str())){
                        falpha_fluid = PVTData[0];
                    }
                    
                    if (!strcmp("Oil", System[iphase].c_str())){
                        falpha_fluid = PVTData[1];
                    }
                    
                    if (!strcmp("Gas", System[iphase].c_str())){
                        falpha_fluid = PVTData[2];
                    }
                    
                    fgamma_fluid = PVTData[2];
                    
                }
                    break;
                    
                case 1:
                {
                    if (!strcmp("Water", System[iphase].c_str())){
                        fbeta_fluid = PVTData[0];
                    }
                    
                    if (!strcmp("Oil", System[iphase].c_str())){
                        fbeta_fluid = PVTData[1];
                    }
                    
                    if (!strcmp("Gas", System[iphase].c_str())){
                        fbeta_fluid = PVTData[2];
                    }
                    
                }
                    fgamma_fluid = PVTData[2];                    
                    break;
                default:
                {
                    DebugStop();
                }
                    break;
            }
            
        }
        
    }
    
    if(fSimulationData->IsThreePhaseQ()){
        
        fmeshvecini.Resize(4);
        fmeshvec.Resize(4);
        
        std::cout << "System not impelmented " << System << std::endl;
        DebugStop();
    }

}

void TPZDarcyAnalysis::SetLastState()
{
    fSimulationData->SetnStep(true);
}

void TPZDarcyAnalysis::SetNextState()
{
    fSimulationData->SetnStep(false);
}

void TPZDarcyAnalysis::Assemble()
{
    
}

/**
 * Push the initial cmesh
 */
void TPZDarcyAnalysis::PushInitialCmesh(){
    fcmesh = NULL;
    fcmesh = fcmeshinitialdarcy;
    
}

/**
 * Push the current cmesh
 */
void TPZDarcyAnalysis::PushCmesh(){
    fcmesh = NULL;
    fcmesh = fcmeshdarcy;
    
}

void TPZDarcyAnalysis::AssembleLastStep(TPZAnalysis *an)
{
    fcmesh->LoadSolution(falphaAtn);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmesh);
    SetLastState();
    an->AssembleResidual();
    fResidualAtn = an->Rhs();
    
//    fResidualAtn.Print("fResidualAtn = ", cout,EMathematicaInput);
    
//#ifdef PZDEBUG
//   #ifdef LOG4CXX
//       if(logger->isDebugEnabled())
//       {
//           std::stringstream sout;
//           falphaAtn.Print("falphaAtn = ", sout,EMathematicaInput);
//           fResidualAtn.Print("fResidualAtn = ", sout,EMathematicaInput);
//           LOGPZ_DEBUG(logger,sout.str())
//       }
//   #endif
//#endif
    
}

void TPZDarcyAnalysis::AssembleResNextStep(TPZAnalysis *an)
{
    fcmesh->LoadSolution(falphaAtnplusOne);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmesh);
    SetNextState();
    an->AssembleResidual();
    fResidualAtnplusOne = an->Rhs();
    
    //#ifdef PZDEBUG
    //  #ifdef LOG4CXX
    //      if(logger->isDebugEnabled())
    //      {
    //          std::stringstream sout;
    //          falphaAtnplusOne.Print("falphaAtnplusOne = ", sout,EMathematicaInput);
    //          fResidualAtnplusOne.Print("fResidualAtnplusOne = ", sout,EMathematicaInput);
    //          LOGPZ_DEBUG(logger,sout.str())
    //      }
    //  #endif
    //#endif
    
}

void TPZDarcyAnalysis::AssembleNextStep(TPZAnalysis *an)
{
    fcmesh->LoadSolution(falphaAtnplusOne);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmesh);
    SetNextState();
    an->Assemble();
    fResidualAtnplusOne = an->Rhs();
    
//    fResidualAtnplusOne.Print("fResidualAtnplusOne = ", cout,EMathematicaInput);
    
    // #ifdef PZDEBUG
    //     #ifdef LOG4CXX
    //         if(logger->isDebugEnabled())
    //         {
    //             std::stringstream sout;
    //             falphaAtnplusOne.Print("falphaAtnplusOne = ", sout,EMathematicaInput);
    //             fResidualAtnplusOne.Print("fResidualAtnplusOne = ", sout,EMathematicaInput);
    //             LOGPZ_DEBUG(logger,sout.str())
    //         }
    //     #endif
    // #endif
    
}

void TPZDarcyAnalysis::UpDateAlphaVec(TPZFMatrix<REAL> &alpha)
{
    falphaAtn = alpha;
    falphaAtnplusOne = alpha;
    
}

void TPZDarcyAnalysis::Residual(TPZFMatrix<STATE> &residual, int icase)
{
    //    TPZNonLinearAnalysis::Residual(residual, icase);
    //    residual = fResidualLastState + residual;
}

void TPZDarcyAnalysis::ComputeTangent(TPZFMatrix<STATE> &tangent, TPZVec<REAL> &coefs, int icase)
{
    this->SetNextState();
    TPZDarcyAnalysis::ComputeTangent(tangent, coefs, icase);
}

void TPZDarcyAnalysis::InitializeSolution(TPZAnalysis *an)
{
    
    REAL TimeStep = fSimulationData->GetDeltaT();
    fcmeshinitialdarcy->Solution().Zero();
    
    if (fSimulationData->IsTwoPhaseQ()) {
        TPZVec<STATE> Swlini(fmeshvec[2]->Solution().Rows());
        TPZCompMesh * L2Sw = L2ProjectionCmesh(Swlini);
        TPZAnalysis * L2Analysis = new TPZAnalysis(L2Sw,false);
        SolveProjection(L2Analysis,L2Sw);
        fmeshvec[2]->LoadSolution(L2Analysis->Solution());
    }
    
    if (fSimulationData->IsThreePhaseQ()) {
        TPZFMatrix<REAL> Soini = fmeshvec[2]->Solution();
        int SoDOF = Soini.Rows();
        for (int iso = 0; iso < SoDOF; iso++)
        {
            Soini(iso,0) = 1.0 - Soini(iso,0);
        }
        fmeshvec[3]->LoadSolution(Soini);
    }
    
    TPZBuildMultiphysicsMesh::TransferFromMeshes(fmeshvec, fcmeshinitialdarcy);
    an->LoadSolution(fcmeshinitialdarcy->Solution());
    
    int n_dt = 1;
    int n_sub_dt = 1;
    int i_time = 0;
    REAL dt = (fSimulationData->GetMaxTime())/REAL(n_sub_dt);
    fSimulationData->SetDeltaT(dt);
    
    if (fSimulationData->GetGR())
    {
        if (fSimulationData->IsImpesQ()) {
            FilterSaturations(fActiveEquations,fNonactiveEquations);
            an->StructMatrix()->EquationFilter().Reset();
            an->StructMatrix()->EquationFilter().SetActiveEquations(fActiveEquations);
        }
        else
        {
            
            FilterSaturationGradients(fActiveEquations,fNonactiveEquations);
            an->StructMatrix()->EquationFilter().Reset();
            an->StructMatrix()->EquationFilter().SetActiveEquations(fActiveEquations);
            CleanUpGradients(an);
            SaturationReconstruction(an);
            
        }
        
    }
    
    falphaAtn = an->Solution();
    falphaAtnplusOne = an->Solution();
    
    while (i_time < n_dt) {
        
        
        
        this->AssembleLastStep(an);
        this->AssembleNextStep(an);
        
        
        if (fSimulationData->IsImpesQ())
        {
            
            const clock_t tinia = clock();
            PicardIterations(an);
            const clock_t tenda = clock();
            const REAL timea = REAL(REAL(tenda - tinia)/CLOCKS_PER_SEC);
            std::cout << "Time for Picard: " << timea << std::endl;
            
        }
        else
        {
            const clock_t tinia = clock();
            NewtonIterations(an);
            const clock_t tenda = clock();
            const REAL timea = REAL(REAL(tenda - tinia)/CLOCKS_PER_SEC);
            std::cout << "Time for Newton: " << timea << std::endl;
            
        }
        i_time++;
    }

    fcmeshinitialdarcy->LoadSolution(an->Solution());
    falphaAtn = fcmeshinitialdarcy->Solution();
    falphaAtnplusOne = fcmeshinitialdarcy->Solution();
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshinitialdarcy);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(fmeshvec, fcmeshdarcy);    
    fSimulationData->SetDeltaT(TimeStep);

    
}

void TPZDarcyAnalysis::RunAnalysis()
{
    
    std::string dirname = PZSOURCEDIR;
    gRefDBase.InitializeUniformRefPattern(EOned);
    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
    gRefDBase.InitializeUniformRefPattern(ETriangle);
    
#ifdef PZDEBUG
    #ifdef LOG4CXX
        
        std::string FileName = dirname;
        FileName = dirname + "/Projects/DarcyflowAxisymmetricHdiv/";
        FileName += "DarcyFlowLog.cfg";
        InitializePZLOG(FileName);
        
    #endif
#endif
    
    //  Reading mesh
    std::string GridFileName;
    GridFileName = dirname + "/Projects/DarcyflowAxisymmetricHdiv/";
    GridFileName += "SingleLayer.dump";
    //GridFileName += "MixLayer.dump";
    //GridFileName += "BatatacoarseQ.dump";
    //GridFileName += "QUAD4.dump";
    
    if(fLayers[0]->GetIsGIDGeometry())
    {
        ReadGeoMesh(GridFileName);
    }
    else
    {
        
        int nx = fSimulationData->GetnElementsx();
        int ny = fSimulationData->GetnElementsy();
        Geometry2D(nx,ny);
        //        CreatedGeoMesh();
    }
    

    REAL deg = fSimulationData->GetRotationAngle();
    RotateGeomesh( deg * M_PI/180.0);
//    ApplyShear(2.0);
    this->UniformRefinement(fSimulationData->GetHrefinement());

//    int hcont = 1;
//    std::set<int> matidstoRef;
//    //    matidstoRef.insert(2);
//    //    matidstoRef.insert(3);
//    //    matidstoRef.insert(4);
//    //    matidstoRef.insert(3);
//    matidstoRef.insert(5);
//    this->UniformRefinement(hcont, matidstoRef);
    
//    TPZCheckGeom check(fgmesh);
//    int isbad = check.PerformCheck();
//    if (isbad) {
//        DebugStop();
//    }
    
    this->PrintGeoMesh();
    
    int q = fSimulationData->Getqorder();
    int p = fSimulationData->Getporder();
    int s = fSimulationData->Getsorder();
    
    CreateMultiphysicsMesh(q,p,s);
    CreateInterfaces();
    
    this->PushInitialCmesh();
    this->PrintCmesh();
    
    if(fSimulationData->GetSC())
    {
        this->ApplyStaticCondensation();
    }
    
    // Analyses
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Computing the Initial conditions " << std::endl;
    
    TPZAnalysis * Initialan = CreateAnalysis(fcmeshinitialdarcy);
    this->PushInitialCmesh();
    const clock_t t_beg_steady = clock();
    this->InitializeSolution(Initialan);
    const clock_t t_end_steady = clock();
    const REAL time_steady = REAL(REAL(t_end_steady - t_beg_steady)/CLOCKS_PER_SEC);
    std::cout << "Time for computing initial condition (minutes): " << time_steady/60.0 << std::endl;
    delete Initialan;

    std::cout << std::endl;
    std::cout << std::endl;    
    std::cout << "Computing the transient solution. " << std::endl;
    
    TPZAnalysis * an = CreateAnalysis(fcmeshdarcy);
    this->PushCmesh();
    //    this->CheckGlobalJacobian(an);
    //    this->CheckGlobalConvergence(an);
    const clock_t t_beg_transient = clock();
    this->TimeForward(an);
    const clock_t t_end_transient = clock();
    const REAL time_transient = REAL(REAL(t_end_transient - t_beg_transient)/CLOCKS_PER_SEC);
    std::cout << "Time for computing transient solution (minutes): " << time_transient/60.0 << std::endl;
    std::cout << std::endl;    

    
    return;
}

TPZAnalysis * TPZDarcyAnalysis::CreateAnalysis(TPZCompMesh * cmesh){
    
    if (!cmesh) {
        std::cout << "NUll computational mesh " << std::endl;
        DebugStop();
    }
    
    bool mustOptimizeBandwidth = fSimulationData->GetOptband();
    TPZAnalysis *an = new TPZAnalysis(cmesh,mustOptimizeBandwidth);
    int numofThreads = fSimulationData->GetNthreads();
    
    bool IsDirecSolver = fSimulationData->GetIsDirect();
    
    if (IsDirecSolver) {
        
        TPZSkylineNSymStructMatrix skylnsym(cmesh);
//            TPZParFrontStructMatrix<TPZFrontNonSym<STATE> > skylnsym(cmesh);
//            skylnsym.SetDecomposeType(ELU);
//            skylnsym.SetQuiet(1);
        TPZStepSolver<STATE> step;
        skylnsym.SetNumThreads(numofThreads);
        step.SetDirect(ELU);
        an->SetStructuralMatrix(skylnsym);
        an->SetSolver(step);
        
    }
    else
    {
        TPZSkylineNSymStructMatrix skylnsym(cmesh);
        skylnsym.SetNumThreads(numofThreads);
        
        TPZAutoPointer<TPZMatrix<STATE> > skylnsyma = skylnsym.Create();
        TPZAutoPointer<TPZMatrix<STATE> > skylnsymaClone = skylnsyma->Clone();
        
        TPZStepSolver<STATE> *stepre = new TPZStepSolver<STATE>(skylnsymaClone);
        TPZStepSolver<STATE> *stepGMRES = new TPZStepSolver<STATE>(skylnsyma);
        TPZStepSolver<STATE> *stepGC = new TPZStepSolver<STATE>(skylnsyma);
        
        stepre->SetDirect(ELU);
        stepre->SetReferenceMatrix(skylnsyma);
        stepGMRES->SetGMRES(10, 20, *stepre, 1.0e-10, 0);
        stepGC->SetCG(10, *stepre, 1.0e-10, 0);
        if (fSimulationData->GetIsCG()) {
            an->SetSolver(*stepGC);
        }
        else{
            an->SetSolver(*stepGMRES);
        }
        
    }
    
    return an;
    
}

void TPZDarcyAnalysis::CreateInterfaces()
{
    if (fSimulationData->IsOnePhaseQ()) {
        return;
    }
    
    fgmesh->ResetReference();
    fcmeshinitialdarcy->LoadReferences();
    
    // Creation of interface elements
    int nel = fcmeshinitialdarcy->ElementVec().NElements();
    for(int el = 0; el < nel; el++)
    {
        TPZCompEl * compEl = fcmeshinitialdarcy->ElementVec()[el];
        if(!compEl) continue;
        TPZGeoEl * gel = compEl->Reference();
        if(!gel) {continue;}
        if(gel->HasSubElement()) {continue;}
        int index = compEl ->Index();
        if(compEl->Dimension() == fcmeshinitialdarcy->Dimension())
        {
            TPZMultiphysicsElement * InterpEl = dynamic_cast<TPZMultiphysicsElement *>(fcmeshinitialdarcy->ElementVec()[index]);
            if(!InterpEl) continue;
            InterpEl->CreateInterfaces();
        }
    }
    
    
    
    fgmesh->ResetReference();
    fcmeshdarcy->LoadReferences();
    nel = fcmeshdarcy->ElementVec().NElements();
    // Creation of interface elements
    for(int el = 0; el < nel; el++)
    {
        TPZCompEl * compEl = fcmeshdarcy->ElementVec()[el];
        if(!compEl) continue;
        TPZGeoEl * gel = compEl->Reference();
        if(!gel) {continue;}
        if(gel->HasSubElement()) {continue;}
        int index = compEl ->Index();
        if(compEl->Dimension() == fcmeshdarcy->Dimension())
        {
            TPZMultiphysicsElement * InterpEl = dynamic_cast<TPZMultiphysicsElement *>(fcmeshdarcy->ElementVec()[index]);
            if(!InterpEl) continue;
            InterpEl->CreateInterfaces();
        }
    }
    
    
    return;
}


void TPZDarcyAnalysis::PrintLS(TPZAnalysis *an)
{
    TPZAutoPointer< TPZMatrix<REAL> > KGlobal;
    TPZFMatrix<STATE> FGlobal;
    KGlobal =   an->Solver().Matrix();
    FGlobal =   fResidualAtn+fResidualAtnplusOne;
    
//    KGlobal->Print("KGlobal = ", std::cout,EMathematicaInput);
//    FGlobal.Print("FGlobal = ", std::cout,EMathematicaInput);
    
#ifdef PZDEBUG
#ifdef LOG4CXX
    if(logger->isDebugEnabled())
    {
        std::stringstream sout;
        KGlobal->Print("KGlobal = ", sout,EMathematicaInput);
        FGlobal.Print("FGlobal = ", sout,EMathematicaInput);
        LOGPZ_DEBUG(logger,sout.str())
    }
#endif
#endif
    
}

void TPZDarcyAnalysis::PrintCmesh(){
    
    
#ifdef PZDEBUG
#ifdef LOG4CXX
    
    std::ofstream out("ComputationalMesh.txt");
    fcmesh->Print(out);
    
#endif
#endif
    
}

void TPZDarcyAnalysis::CreateMultiphysicsMesh(int q, int p, int s)
{
    fmeshvec[0] = CmeshFlux(q);
    fmeshvec[1] = CmeshPressure(p);
    
    if (fSimulationData->IsTwoPhaseQ()) {
        fmeshvec[2] = CmeshSw(s);
    }
    
    if (fSimulationData->IsThreePhaseQ()) {
        fmeshvec[2] = CmeshSw(s);
        fmeshvec[3] = CmeshSo(s);
    }
    
    fcmeshinitialdarcy = CmeshMixedInitial();
    fcmeshdarcy = CmeshMixed();
    
    TPZBuildMultiphysicsMesh::AddElements(fmeshvec, fcmeshinitialdarcy);
    TPZBuildMultiphysicsMesh::AddConnects(fmeshvec, fcmeshinitialdarcy);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(fmeshvec, fcmeshinitialdarcy);
    
    TPZBuildMultiphysicsMesh::AddElements(fmeshvec, fcmeshdarcy);
    TPZBuildMultiphysicsMesh::AddConnects(fmeshvec, fcmeshdarcy);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(fmeshvec, fcmeshdarcy);
    
    int DOF = fcmeshdarcy->Solution().Rows();
    std::cout << "Degrees of freedom: " << DOF << std::endl;
    
}

void TPZDarcyAnalysis::TimeForward(TPZAnalysis *an)
{
    int n_sub_dt = fSimulationData->GetNSubSteps();
    TPZManVector<REAL> reporting_times = fSimulationData->GetTimes();
    REAL tk = fSimulationData->GetTime();
    REAL current_dt = (reporting_times[0] - tk)/n_sub_dt;
    
    if (fSimulationData->GetGR())
    {
        if (fSimulationData->IsImpesQ()) {
            FilterSaturations(fActiveEquations,fNonactiveEquations);
            an->StructMatrix()->EquationFilter().Reset();
            an->StructMatrix()->EquationFilter().SetActiveEquations(fActiveEquations);
        }
        else
        {
            
            FilterSaturationGradients(fActiveEquations,fNonactiveEquations);
            an->StructMatrix()->EquationFilter().Reset();
            an->StructMatrix()->EquationFilter().SetActiveEquations(fActiveEquations);
            CleanUpGradients(an);
            SaturationReconstruction(an);
            
        }

    }
    
    if (fSimulationData->IsImpesQ()) {
        FilterSaturations(fActiveEquations,fNonactiveEquations);
        an->StructMatrix()->EquationFilter().Reset();
        an->StructMatrix()->EquationFilter().SetActiveEquations(fActiveEquations);
    }

    // Out file for initial condition
    this->PostProcessVTK(an);
    std::cout << "Reported time " << tk/(86400.0) << "; dt = " << current_dt/(86400.0) << std::endl;
    std::cout << std::endl;
    
    falphaAtn = an->Solution();
    falphaAtnplusOne = an->Solution();
    
    int i_time = 0;
    int i_sub_dt = 0;
    
    TPZManVector<REAL> velocities(3,0.0);

    TPZFNMatrix<100,REAL> current_v(reporting_times.size()+1,4,0.0);
    TPZFNMatrix<100,REAL> accumul_v(reporting_times.size()+1,4,0.0);
    
    while (i_time < reporting_times.size()) {

        fSimulationData->SetDeltaT(current_dt);
        this->AssembleLastStep(an);
        this->AssembleNextStep(an);
        
        
        if (fSimulationData->IsImpesQ())
        {
            
            const clock_t tinia = clock();
            PicardIterations(an);
            const clock_t tenda = clock();
            const REAL timea = REAL(REAL(tenda - tinia)/CLOCKS_PER_SEC);
            std::cout << "Time for Broyden: " << timea << std::endl;
            
        }
        else
        {
            const clock_t tinia = clock();
            NewtonIterations(an);
            const clock_t tenda = clock();
            const REAL timea = REAL(REAL(tenda - tinia)/CLOCKS_PER_SEC);
            std::cout << "Time for Newton: " << timea << std::endl;
            
        }
        i_sub_dt++;

        if (i_sub_dt == n_sub_dt) {
            tk = reporting_times[i_time];
            this->fSimulationData->SetTime(tk);
            std::cout << "Reported time " << tk/(86400.0) << "; dt = " << current_dt/(86400.0) << "; File number = " << i_time + 1 << std::endl;

            if (fSimulationData->GetGR())
            {
                std::cout << "Writing Saturations with reconstruction." << std::endl;
                fcmesh->LoadSolution(an->Solution());
                TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmesh);
            }
            std::cout << std::endl;
            
            this->PostProcessVTK(an);
            REAL day = 86400.0;
            IntegrateVelocities(velocities);
            current_v(i_time+1,0) = tk/day;
            current_v(i_time+1,1) += velocities[0]*current_dt*n_sub_dt;
            current_v(i_time+1,2) += velocities[1]*current_dt*n_sub_dt;
            current_v(i_time+1,3) += velocities[2]*current_dt*n_sub_dt;
            
            accumul_v(i_time+1,0) = tk/day;
            accumul_v(i_time+1,1) = velocities[0]*current_dt*n_sub_dt + accumul_v(i_time,1);
            accumul_v(i_time+1,2) = velocities[1]*current_dt*n_sub_dt + accumul_v(i_time,2);
            accumul_v(i_time+1,3) = velocities[2]*current_dt*n_sub_dt + accumul_v(i_time,3);
            

            // Computing the rates at reporting times
            
//            if (fSimulationData->IsTwoPhaseQ()) {
//                TPZManVector<REAL> l2_norm_s(1,0.0);
//                this->IntegrateL2SError(l2_norm_s);
//                fL2_norm_s[0] = sqrt(l2_norm_s[0]);
//            }
            
            if (i_time == reporting_times.size()-1) {
                break;
            }
            current_dt = (reporting_times[i_time+1] - reporting_times[i_time])/n_sub_dt;
            i_time++;
            i_sub_dt=0;


        }
        
    }
    
    TPZManVector<REAL> l2_norm_flux(1,0.0);
    TPZManVector<REAL> l2_norm(1,0.0);
    this->IntegrateFluxPError(l2_norm_flux,l2_norm);

    fl2_norm_flux[0] = sqrt(l2_norm_flux[0]);
    fL2_norm[0] = sqrt(l2_norm[0]);
    
    
    std::ofstream q_out("current_production.txt");
    std::ofstream qt_out("accumulated_production.txt");
    current_v.Print("q = ", q_out,EMathematicaInput);
    accumul_v.Print("qt = ", qt_out,EMathematicaInput);
    
    return;
    
}

void TPZDarcyAnalysis::IntegrateL2SError(TPZManVector<REAL> & l2_norm){
    
    int mat_id = 1;
    int int_order = 36;
    int int_typ = 0;
    l2_norm[0] = 0.0;
    TPZManVector<STATE> saturation;
    TPZManVector<STATE> s_exact;
    
    fcmesh->Reference()->ResetReference();
    fcmesh->LoadReferences();
    
    long n_elements = fcmesh->NElements();
    
    for (long iel = 0 ; iel < n_elements; iel++) {
        TPZCompEl * cel = fcmesh->Element(iel);
        if (!cel) {
            std::cout << "Computational element non-exist " << std::endl;
            DebugStop();
        }
        
        TPZGeoEl * gel = cel->Reference();
        if (!gel) {
            std::cout << "Geomtric element non-exist " << std::endl;
            DebugStop();
        }
        
        if (gel->Dimension() != 2) {
            continue;
        }
        
        
        if (gel->MaterialId() == mat_id && gel->NumInterfaces() == 0) {
            
            // Creating the integration rule
            int gel_side = gel->NSides() - 1;
            TPZIntPoints * NumericIntegral = gel->CreateSideIntegrationRule(gel_side, int_order);
            NumericIntegral->SetType(int_typ, int_order);
            
            int dimension   = NumericIntegral->Dimension();
            int npoints     = NumericIntegral->NPoints();
            
            if (dimension != gel->Dimension()) {
                std::cout << "Incompatible dimensions." << std::endl;
                DebugStop();
            }
            
            // compute the integrals
            TPZManVector<REAL,2> xi_eta_duplet(2,0.0);
            TPZManVector<REAL,3> x(3,0.0);
            
            REAL weight = 0.0;
            REAL s = 1.0;
            for (int it = 0 ; it < npoints; it++) {
                
                NumericIntegral->Point(it, xi_eta_duplet, weight);
                TPZFMatrix<REAL> jac;
                TPZFMatrix<REAL> axes;
                REAL detjac;
                TPZFMatrix<REAL> jacinv;
                gel->Jacobian(xi_eta_duplet, jac, axes, detjac, jacinv);
                gel->X(xi_eta_duplet, x);
                
                if (fSimulationData->IsAxisymmetricQ()) {
                    s = 2.0*M_PI*x[0];
                }
                
                cel->Solution(xi_eta_duplet, 2, saturation);
                cel->Solution(xi_eta_duplet, 10, s_exact);
//                l2_norm[0] += s*weight * detjac * (saturation[0]);
                l2_norm[0] += s*weight * detjac * (s_exact[0] - saturation[0])*(s_exact[0] - saturation[0]);
                
            }
        }
        
    }
    
    
}

void TPZDarcyAnalysis::IntegrateFluxPError(TPZManVector<REAL> & l2_norm_flux,TPZManVector<REAL> & l2_norm){
    
    int mat_id = 1;
    int int_order = 36;
    int int_typ = 0;
    l2_norm[0] = 0.0;
    l2_norm_flux[0] = 0.0;
    REAL s = 1.0;    
    TPZManVector<STATE> p;
    TPZManVector<STATE> p_exact;
    TPZManVector<STATE> u;
    TPZManVector<STATE> u_exact;
    TPZManVector<STATE> divu;
    TPZManVector<STATE> divu_exact;
    
    TPZFMatrix<REAL> normals;
    TPZManVector<REAL,3> n(3,0.0);
    
    TPZManVector<int> v_sides;
    
    fcmesh->Reference()->ResetReference();
    fcmesh->LoadReferences();
    
    long n_elements = fcmesh->NElements();
    
    for (long iel = 0 ; iel < n_elements; iel++) {
        TPZCompEl * cel = fcmesh->Element(iel);
        if (!cel) {
            std::cout << "Computational element non-exist " << std::endl;
            DebugStop();
        }
        
        TPZGeoEl * gel = cel->Reference();
        if (!gel) {
            std::cout << "Geomtric element non-exist " << std::endl;
            DebugStop();
        }
        
        if (gel->Dimension() != 2) {
            continue;
        }
        
        
        if (gel->MaterialId() == mat_id && gel->NumInterfaces() == 0) {
            
            // Creating the integration rule
            int gel_side = gel->NSides() - 1;
            TPZIntPoints * NumericIntegral = gel->CreateSideIntegrationRule(gel_side, int_order);
            NumericIntegral->SetType(int_typ, int_order);
            
            int dimension   = NumericIntegral->Dimension();
            int npoints     = NumericIntegral->NPoints();
            
            if (dimension != gel->Dimension()) {
                std::cout << "Incompatible dimensions." << std::endl;
                DebugStop();
            }
            
            // compute the integrals
            TPZManVector<REAL,2> xi_eta_duplet(2,0.0);
            TPZManVector<REAL,3> x(3,0.0);
            
            REAL weight = 0.0;
            REAL dot = 0.0;
            for (int it = 0 ; it < npoints; it++) {
                
                NumericIntegral->Point(it, xi_eta_duplet, weight);
                TPZFMatrix<REAL> jac;
                TPZFMatrix<REAL> axes;
                REAL detjac;
                TPZFMatrix<REAL> jacinv;
                gel->Jacobian(xi_eta_duplet, jac, axes, detjac, jacinv);
                gel->X(xi_eta_duplet, x);
                
                if (fSimulationData->IsAxisymmetricQ()) {
                    // Computing the radius
                    TPZFMatrix<REAL> x_spatial(3,1,0.0);
                    x_spatial(0,0) = x[0];
                    REAL r = Norm(x_spatial);
                    s = 2.0*M_PI*r;
                }
                
                cel->Solution(xi_eta_duplet, 0, p);
                cel->Solution(xi_eta_duplet, 10, p_exact);
                
                cel->Solution(xi_eta_duplet, 1, u);
                cel->Solution(xi_eta_duplet, 11, u_exact);
                
                cel->Solution(xi_eta_duplet, 9, divu);
                cel->Solution(xi_eta_duplet, 12, divu_exact);

                dot = (u[0]-u_exact[0])*(u[0]-u_exact[0]) + (u[1]-u_exact[1])*(u[1]-u_exact[1]);
                l2_norm[0] += s*weight * detjac * (p_exact[0] - p[0])*(p_exact[0] - p[0]);
                l2_norm_flux[0] += s*weight * detjac * (dot);
                
            }
            
        }
        
    }
    
    
}

void TPZDarcyAnalysis::IntegrateVelocities(TPZManVector<REAL> & velocities){
    
    int mat_id = 5;
    int int_order = 5;
    int int_typ = 0;
    velocities[0] = 0.0;
    velocities[1] = 0.0;
    velocities[2] = 0.0;
    TPZManVector<STATE> sol;
    TPZFMatrix<REAL> normals;
    TPZManVector<REAL,3> n(3,0.0);
    
    TPZManVector<int> v_sides;
    
    fcmesh->Reference()->ResetReference();
    fcmesh->LoadReferences();
    
    long n_elements = fcmesh->NElements();
    
    for (long iel = 0 ; iel < n_elements; iel++) {
        TPZCompEl * cel = fcmesh->Element(iel);
        if (!cel) {
            std::cout << "Computational element non-exist " << std::endl;
            DebugStop();
        }
        
        TPZGeoEl * gel = cel->Reference();
        if (!gel) {
            std::cout << "Geomtric element non-exist " << std::endl;
            DebugStop();
        }
        
        if (gel->Dimension() != 1) {
            continue;
        }
        
        
        if (gel->MaterialId() == mat_id && gel->NumInterfaces() == 0) {
            
            
            TPZGeoEl * gel_2D = GetVolElement(gel);
            TPZGeoElSide intermediate_side;
            TPZTransform afine_transformation = Transform_1D_To_2D(gel,gel_2D,intermediate_side);
            
            int itself_2d = gel_2D->NSides()-1;
            TPZGeoElSide gel_side_2D(gel_2D,itself_2d);
            TPZCompEl * cel_2D = gel_2D->Reference();
            if (!cel_2D) {
                DebugStop();
            }
            
            
            int gel_side = gel->NSides() - 1;
            TPZIntPoints * NumericIntegral = gel->CreateSideIntegrationRule(gel_side, int_order);
            NumericIntegral->SetType(int_typ, int_order);
            
            // Creating the integration rule
            int dimension   = NumericIntegral->Dimension();
            int npoints     = NumericIntegral->NPoints();
            
            if (dimension != gel->Dimension()) {
                std::cout << "Incompatible dimensions." << std::endl;
                DebugStop();
            }
            
            
            gel_2D->ComputeNormals(normals, v_sides);
            for (int i = 0 ; i < v_sides.size(); i++) {
                if(v_sides[i] ==  intermediate_side.Side()){
                    n[0] = normals(0,i);
                    n[1] = normals(1,i);
                    n[2] = normals(2,i);
                    break;
                }
            }
            
            // compute the integrals
            TPZManVector<REAL,1> xi_singlet(1,0.0);
            TPZManVector<REAL,2> xi_eta_duplet(2,0.0);
            REAL weight = 0.0;
            for (int it = 0 ; it < npoints; it++) {

                TPZFMatrix<REAL> jac;
                TPZFMatrix<REAL> axes;
                REAL detjac;
                TPZFMatrix<REAL> jacinv;
                gel->Jacobian(xi_singlet, jac, axes, detjac, jacinv);
                
                NumericIntegral->Point(it, xi_singlet, weight);
                afine_transformation.Apply(xi_singlet, xi_eta_duplet);
                

                REAL cross_area = (detjac*2.0)*(1.0);
                if (fSimulationData->IsAxisymmetricQ()) {
                    REAL rw = fLayers[0]->Layerrw();
                    cross_area *= 2.0*M_PI*rw;
                }
                cel_2D->Solution(xi_eta_duplet, 19, sol);
                velocities[0] += weight * detjac * (sol[0] * n[0] + sol[1] * n[1]);

                
                cel_2D->Solution(xi_eta_duplet, 20, sol);
                velocities[1] += weight * detjac * (sol[0] * n[0] + sol[1] * n[1]);
                
            }
        }
        
    }
    
    
}

TPZGeoEl * TPZDarcyAnalysis::GetVolElement(TPZGeoEl * gel){
    
    if (gel->Dimension() != 1) {
        DebugStop();
    }
    TPZGeoEl * gel_2D;
    TPZGeoElSide gel_side(gel,2);
    while (gel_side != gel_side.Neighbour()) {

        TPZGeoElSide gel_2d_side = gel_side.Neighbour();
        gel_2D =  gel_2d_side.Element();
        if(gel_2D->Dimension() ==2)
        {
            break;
        }
        gel_side = gel_2d_side;
    }
    

    
    return gel_2D;
}

TPZTransform  TPZDarcyAnalysis::Transform_1D_To_2D(TPZGeoEl * gel_o, TPZGeoEl * gel_d, TPZGeoElSide & intermediate_side){
    
    int itself_o = gel_o->NSides()-1;
    int itself_d = gel_d->NSides()-1;
    
    TPZGeoElSide gel_side_o(gel_o,itself_o);
    TPZGeoElSide gel_side_d(gel_d,itself_d);
    TPZGeoElSide neigh = gel_side_o.Neighbour();
    TPZGeoEl * gel_2D;
    while(neigh != neigh.Neighbour()){
        gel_2D =  neigh.Element();
        if(gel_2D->Dimension() ==2)
        {
            break;
        }
        neigh = neigh.Neighbour();
    }
    
    intermediate_side = neigh;
    TPZTransform t1 = gel_side_o.NeighbourSideTransform(neigh);
    TPZTransform t2 = neigh.SideToSideTransform(gel_side_d);
    TPZTransform t3 = t2.Multiply(t1);
    
    return t3;
}

void TPZDarcyAnalysis::CleanUpGradients(TPZAnalysis *an){
    
    long numofdof = fNonactiveEquations.size();
    TPZFMatrix<REAL> SolToLoad = an->Solution();
    for(long i=0; i < numofdof; i++)
    {
        SolToLoad(fNonactiveEquations[i],0) = 0.0;
    }
    an->LoadSolution(SolToLoad);
    
}

void TPZDarcyAnalysis::PrintSaturations(TPZAnalysis *an){
    
    long numofdof = fNonactiveEquations.size();
    TPZFMatrix<REAL> SolToLoad = an->Solution();
    TPZFMatrix<REAL> gradS(numofdof,1,0.0);
    for(long i=0; i < numofdof; i++)
    {
       gradS(i,0) = SolToLoad(fNonactiveEquations[i],0);
    }
    
    TPZManVector<long> AverageS;
    
    int ncon_sw = fmeshvec[2]->NConnects();
    int ncon = fcmesh->NConnects();
    
    // DOF Related with S constant
    for(int i = ncon-ncon_sw; i< ncon; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        int vs = AverageS.size();
        AverageS.Resize(vs+1);
        
        int ieq = blocksize-1;
        AverageS[vs] = pos+ieq;
    }
    
    long numofSdof = AverageS.size();
    TPZFMatrix<REAL> S(numofSdof,1,0.0);
    for(long i=0; i < numofSdof; i++)
    {
        S(i,0) = SolToLoad(AverageS[i],0);
    }

    std::cout << "S = " << S << std::endl;
    std::cout << "Gradients = " << gradS << std::endl;
}

void TPZDarcyAnalysis::SaturationReconstruction(TPZAnalysis *an)
{
    fcmesh->LoadSolution(an->Solution());
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmesh);
    TPZGradientReconstruction *gradreconst = new TPZGradientReconstruction(false,1.);

    
    fmeshvec[2]->Reference()->ResetReference();
    fmeshvec[2]->LoadReferences();
    gradreconst->ProjectionL2GradientReconstructed(fmeshvec[2], fSimulationData->fMatL2);
    
    TPZBuildMultiphysicsMesh::TransferFromMeshes(fmeshvec, fcmesh);
    an->LoadSolution(fcmesh->Solution());
    

}

void TPZDarcyAnalysis::NewtonIterations(TPZAnalysis *an)
{
    
    TPZFMatrix<STATE> Residual(an->Rhs().Rows(),1,0.0);
    Residual = fResidualAtn + fResidualAtnplusOne;
    
    TPZFMatrix<STATE> X = falphaAtn;
    TPZFMatrix<STATE> DeltaX = falphaAtn;
    
    STATE error     =   1.0;
    STATE normdx    =   1.0;//Norm(Residual);
    int iterations  =   0;
    int centinel    =   0;
    int fixed       =   fSimulationData->GetFixediterations();
    
    while (iterations <= fSimulationData->GetMaxiterations()) {
        
        an->Rhs() = Residual;
        an->Rhs() *= -1.0;
        
        this->PrintLS(an);
        
        an->Solve();
        
        DeltaX = an->Solution();
        normdx = Norm(DeltaX);
        X += DeltaX;
        
//        X.Print("X = ");
        
        falphaAtnplusOne=X;
        an->LoadSolution(X);
        
        if (fSimulationData->GetGR())
        {
            CleanUpGradients(an);
            this->SaturationReconstruction(an);
            falphaAtnplusOne = an->Solution();
        }
        
        
        if (((fixed+1) * (centinel) == iterations)) {
            
            this->AssembleNextStep(an);
            centinel++;
        }
        else{
            this->AssembleResNextStep(an);
        }
              
        fResidualAtnplusOne = an->Rhs();
        
        Residual = fResidualAtn + fResidualAtnplusOne;
        error = Norm(Residual);
        iterations++;
//        Residual.Print("Residual = ");
        
#ifdef PZDEBUG
#ifdef LOG4CXX
        if(logger->isDebugEnabled())
        {
            std::stringstream sout;
            DeltaX.Print("DeltaX = ", sout,EMathematicaInput);
            X.Print("X = ", sout,EMathematicaInput);
            Residual.Print("Residual = ", sout,EMathematicaInput);
            LOGPZ_DEBUG(logger,sout.str())
        }
#endif
#endif
        
        if(error < fSimulationData->GetToleranceRes() || normdx < fSimulationData->GetToleranceDX())
        {
            std::cout << "Converged; iterations:  " << iterations << "; error: " << error <<  "; dx: " << normdx << std::endl;
            break;
        }
        
        
        if (iterations == fSimulationData->GetMaxiterations()) {
            std::cout << "Out; iterations:  " << iterations << "; error: " << error <<  "; dx: " << normdx << std::endl;
            break;
        }
        
    }
    
    this->UpDateAlphaVec(X);
    
}

void TPZDarcyAnalysis::UpdateSaturations(TPZAnalysis *an)
{
    
    
}

void TPZDarcyAnalysis::ComputeSaturations(TPZAnalysis *an){
    
    REAL scale = 0.2 * 1.0 /(fSimulationData->GetDeltaT());
    SetLastState();
    an->AssembleResidual();
    TPZFMatrix<REAL> residual = an->Rhs();

    
    SetNextState();
}


void TPZDarcyAnalysis::PicardIterations(TPZAnalysis *an)
{
    
    TPZFMatrix<STATE> Residual(an->Rhs().Rows(),1,0.0);
    Residual = fResidualAtn + fResidualAtnplusOne;
    
    TPZFMatrix<STATE> X = falphaAtn;
    TPZFMatrix<STATE> DeltaX = falphaAtn;
    
    STATE error     =   1.0;
    STATE normdx    =   1.0;//Norm(Residual);
    int iterations  =   0;
    int centinel    =   0;
    int fixed       =   fSimulationData->GetFixediterations();
    
    while (iterations <= fSimulationData->GetMaxiterations()) {
        
        an->Rhs() = Residual;
        an->Rhs() *= -1.0;
        
        this->PrintLS(an);
        
        an->Solve();
        
        
        DeltaX = an->Solution();
        normdx = Norm(DeltaX);
        X += DeltaX;
        
//        DeltaX.Print("DeltaX = ", std::cout,EMathematicaInput);
        X.Print("X = ", std::cout,EMathematicaInput);
//        Residual.Print("Residual = ", std::cout,EMathematicaInput);
        
        // compute Saturations
        ComputeSaturations(an);
        
        // update Saturations
        UpdateSaturations(an);
        
        falphaAtnplusOne=X;
        
        if (fSimulationData->GetGR())
        {
            an->Solution() = falphaAtnplusOne;
            CleanUpGradients(an);
            this->SaturationReconstruction(an);
            fcmesh->LoadSolution(an->Solution());
        }
        
        
        if (((fixed+1) * (centinel) == iterations)) {
            
            this->AssembleNextStep(an);
            centinel++;
        }
        else{
            this->AssembleResNextStep(an);
        }
        
        fResidualAtnplusOne = an->Rhs();
        
        Residual = fResidualAtn + fResidualAtnplusOne;
        error = Norm(Residual);
        iterations++;
        
        
#ifdef PZDEBUG
#ifdef LOG4CXX
        if(logger->isDebugEnabled())
        {
            std::stringstream sout;
            DeltaX.Print("DeltaX = ", sout,EMathematicaInput);
            X.Print("X = ", sout,EMathematicaInput);
            Residual.Print("Residual = ", sout,EMathematicaInput);
            LOGPZ_DEBUG(logger,sout.str())
        }
#endif
#endif
        
        if(error < fSimulationData->GetToleranceRes() || normdx < fSimulationData->GetToleranceDX())
        {
            std::cout << "Converged; iterations:  " << iterations << "; error: " << error <<  "; dx: " << normdx << std::endl;
            break;
        }
        
        
        if (iterations == fSimulationData->GetMaxiterations()) {
            std::cout << "Out; iterations:  " << iterations << "; error: " << error <<  "; dx: " << normdx << std::endl;
            break;
        }
        
    }
    
    this->UpDateAlphaVec(X);
    
}


TPZFMatrix<STATE>  TPZDarcyAnalysis::TensorProduct(TPZFMatrix<STATE> &g, TPZFMatrix<STATE> &d)
{
    TPZFMatrix<STATE> dT=d;
    d.Transpose(&dT);
    TPZFMatrix<STATE> RankOne;
    g.Multiply(dT, RankOne);
    
    //#ifdef LOG4CXX
    //    if(logger->isDebugEnabled())
    //    {
    //        std::stringstream sout;
    //        g.Print("g = ", sout,EMathematicaInput);
    //        dT.Print("dT = ", sout,EMathematicaInput);
    //        d.Print("d = ", sout,EMathematicaInput);
    //        RankOne.Print("RankOne = ", sout,EMathematicaInput);
    //        LOGPZ_DEBUG(logger,sout.str())
    //    }
    //#endif
    
    return RankOne;
    
}

TPZCompMesh * TPZDarcyAnalysis::CmeshMixedInitial()
{
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    int dim = 2;
    int ilayer = 0;
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    
    
    // Material medio poroso
    TPZAxiSymmetricDarcyFlow * mat = new TPZAxiSymmetricDarcyFlow(RockId);
    mat->SetSimulationData(fSimulationData);
    mat->SetReservoirData(fLayers[ilayer]);
    mat->SetPetroPhysicsData(fRockPetroPhysic[ilayer]);
    mat->SetFluidAlpha(falpha_fluid);
    mat->SetFluidBeta(fbeta_fluid);
    mat->SetFluidGamma(fgamma_fluid);
    int nvars = 2 + fSimulationData->GetsystemType().size() - 1;
    mat->SetNvars(nvars);
    cmesh->InsertMaterialObject(mat);
    
    
    // Rigth hand side function
    TPZDummyFunction<STATE> *dum = new TPZDummyFunction<STATE>(Ffunction);
    TPZAutoPointer<TPZFunction<STATE> > forcef;
    dum->SetPolynomialOrder(20);
    forcef = dum;
//    mat->SetTimeDependentForcingFunction(forcef);
    
    // Setting up linear tracer solution
      TPZDummyFunction<STATE> *Ltracer = new TPZDummyFunction<STATE>(LinearTracer);
//    TPZDummyFunction<STATE> *Ltracer = new TPZDummyFunction<STATE>(BluckleyAndLeverett);
    TPZAutoPointer<TPZFunction<STATE> > fLTracer = Ltracer;
    mat->SetTimeDependentFunctionExact(fLTracer);
    
    TPZManVector<REAL,4> Bottom     = fSimulationData->GetBottomBCini();
    TPZManVector<REAL,4> Right      = fSimulationData->GetRightBCini();
    TPZManVector<REAL,4> Top        = fSimulationData->GetTopBCini();
    TPZManVector<REAL,4> Left       = fSimulationData->GetLeftBCini();
    

    TPZDummyFunction<STATE> * P_hydrostatic = new TPZDummyFunction<STATE>(P_Hydrostatic);
    TPZAutoPointer<TPZFunction<STATE> > P_hydrostatic_ptr;
    P_hydrostatic_ptr = P_hydrostatic;
    mat->SetTimedependentBCForcingFunction(P_hydrostatic_ptr);
    
    // Bc Bottom
    val2(0,0) = Bottom[1];
    val2(1,0) = Bottom[2];
    val2(2,0) = Bottom[3];
    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, int(Bottom[0]), val1, val2);

    // Bc Right
    val2(0,0) = Right[1];
    val2(1,0) = Right[2];
    val2(2,0) = Right[3];
    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, int(Right[0]), val1, val2);
    
    // Bc Top
    val2(0,0) = Top[1];
    val2(1,0) = Top[2];
    val2(2,0) = Top[3];
    TPZBndCond * bcTop = mat->CreateBC(mat, topId, int(Top[0]), val1, val2);
    
    // Bc Left
    val2(0,0) = Left[1];
    val2(1,0) = Left[2];
    val2(2,0) = Left[3];
    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, int(Left[0]), val1, val2);
    
    cmesh->InsertMaterialObject(bcBottom);
    cmesh->InsertMaterialObject(bcRight);
    cmesh->InsertMaterialObject(bcTop);
    cmesh->InsertMaterialObject(bcLeft);
    
    cmesh->SetDimModel(dim);
    cmesh->SetAllCreateFunctionsMultiphysicElem();
    
    cmesh->AutoBuild();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    
    return cmesh;
}

TPZCompMesh * TPZDarcyAnalysis::CmeshMixed()
{
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    int dim = 2;
    int ilayer = 0;
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    
    
    // Material medio poroso
    TPZAxiSymmetricDarcyFlow * mat = new TPZAxiSymmetricDarcyFlow(RockId);
    mat->SetSimulationData(fSimulationData);
    mat->SetReservoirData(fLayers[ilayer]);
    mat->SetPetroPhysicsData(fRockPetroPhysic[ilayer]);
    mat->SetFluidAlpha(falpha_fluid);
    mat->SetFluidBeta(fbeta_fluid);
    mat->SetFluidGamma(fgamma_fluid);
    int nvars = 2 + fSimulationData->GetsystemType().size() - 1;
    mat->SetNvars(nvars);
    cmesh->InsertMaterialObject(mat);
    
    
    // Rigth hand side function
    TPZDummyFunction<STATE> *dum = new TPZDummyFunction<STATE>(Ffunction);
    TPZAutoPointer<TPZFunction<STATE> > forcef;
    dum->SetPolynomialOrder(20);
    forcef = dum;
    mat->SetTimeDependentForcingFunction(forcef);
    
    // Setting up linear tracer solution
    TPZDummyFunction<STATE> *Load_function= new TPZDummyFunction<STATE>(Cylindrical_Elliptic);
//    TPZDummyFunction<STATE> *Load_function = new TPZDummyFunction<STATE>(Dupuit_Thiem);
//    TPZDummyFunction<STATE> *Load_function = new TPZDummyFunction<STATE>(LinearTracer);
//    TPZDummyFunction<STATE> *Load_function = new TPZDummyFunction<STATE>(BluckleyAndLeverett);
    TPZAutoPointer<TPZFunction<STATE> > fLoad_function = Load_function;
    mat->SetTimeDependentFunctionExact(fLoad_function);
    
    TPZManVector<REAL,4> Bottom     = fSimulationData->GetBottomBC();
    TPZManVector<REAL,4> Right      = fSimulationData->GetRightBC();
    TPZManVector<REAL,4> Top        = fSimulationData->GetTopBC();
    TPZManVector<REAL,4> Left       = fSimulationData->GetLeftBC();
    
    
    TPZDummyFunction<STATE> * P_hydrostatic = new TPZDummyFunction<STATE>(P_Hydrostatic);
    TPZAutoPointer<TPZFunction<STATE> > P_hydrostatic_ptr;
    P_hydrostatic_ptr = P_hydrostatic;
    mat->SetTimedependentBCForcingFunction(P_hydrostatic_ptr);
    
    // Bc Bottom
    val2(0,0) = Bottom[1];
    val2(1,0) = Bottom[2];
    val2(2,0) = Bottom[3];
    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, int(Bottom[0]), val1, val2);
    
    // Bc Right
    val2(0,0) = Right[1];
    val2(1,0) = Right[2];
    val2(2,0) = Right[3];
    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, int(Right[0]), val1, val2);
    
    // Bc Top
    val2(0,0) = Top[1];
    val2(1,0) = Top[2];
    val2(2,0) = Top[3];
    TPZBndCond * bcTop = mat->CreateBC(mat, topId, int(Top[0]), val1, val2);
    
    // Bc Left
    val2(0,0) = Left[1];
    val2(1,0) = Left[2];
    val2(2,0) = Left[3];
    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, int(Left[0]), val1, val2);
    
    cmesh->InsertMaterialObject(bcBottom);
    cmesh->InsertMaterialObject(bcRight);
    cmesh->InsertMaterialObject(bcTop);
    cmesh->InsertMaterialObject(bcLeft);
    
    cmesh->SetDimModel(dim);
    cmesh->SetAllCreateFunctionsMultiphysicElem();
    
    cmesh->AutoBuild();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    
    return cmesh;
}

void TPZDarcyAnalysis::ApplyStaticCondensation(){
    
    if (!fcmeshdarcy) {
        std::cout<< "No multiphysic computational mesh " << std::endl;
        DebugStop();
    }
    
    fcmeshinitialdarcy->Reference()->ResetReference();
    fcmeshinitialdarcy->LoadReferences();
    
    fcmeshinitialdarcy->ComputeNodElCon();
    // create condensed elements
    // increase the NumElConnected of one pressure connects in order to prevent condensation
    for (long icel=0; icel < fcmeshinitialdarcy->NElements(); icel++) {
        TPZCompEl  * cel = fcmeshinitialdarcy->Element(icel);
        if(!cel) continue;
        int nc = cel->NConnects();
        for (int ic=0; ic<nc; ic++) {
            TPZConnect &c = cel->Connect(ic);
            if (c.LagrangeMultiplier() > 0) {
                c.IncrementElConnected();
                break;
            }
        }
        new TPZCondensedCompEl(cel);
    }
    
    int DOF = fmeshvec[0]->NEquations() + fmeshvec[1]->NEquations();// + fmeshvec[2]->NEquations() + fmeshvec[3]->NEquations();
    REAL PercentCondensedDOF = 100.0*(1.0 - REAL(fcmeshinitialdarcy->NEquations())/REAL(DOF));
    std::cout << "Percent of condensed Degrees of freedom: " << PercentCondensedDOF << std::endl;
    std::cout << "Condensed degrees of freedom: " << fcmeshinitialdarcy->NEquations() << std::endl;
    
    fcmeshinitialdarcy->CleanUpUnconnectedNodes();
    fcmeshinitialdarcy->ExpandSolution();
    
}


TPZCompMesh * TPZDarcyAnalysis::CmeshFlux(int qorder)
{
    
    int dim = fgmesh->Dimension();
    const int typeFlux = 0, typePressure = 1;
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    int ilayer = 0;
    
    
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    TPZAxiSymmetricDarcyFlow * mat = new TPZAxiSymmetricDarcyFlow(RockId);
    cmesh->InsertMaterialObject(mat);
    
    // Bc Bottom
    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcBottom);
    
    // Bc Right
    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, typePressure, val1, val2);
    cmesh->InsertMaterialObject(bcRight);
    
    // Bc Top
    TPZBndCond * bcTop = mat->CreateBC(mat, topId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcTop);
    
    // Bc Left
    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcLeft);
    
    // Setando Hdiv
    cmesh->SetDimModel(dim);
    cmesh->SetDefaultOrder(qorder);
    cmesh->SetAllCreateFunctionsHDiv();
    
    
    cmesh->AutoBuild();
    
    
#ifdef PZDEBUG
    std::ofstream out("cmeshFlux.txt");
    cmesh->Print(out);
#endif
    
    return cmesh;
}

TPZCompMesh * TPZDarcyAnalysis::CmeshPressure(int porder)
{
    
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    int dim = fgmesh->Dimension();
    
    
    int ilayer = 0;
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
//    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
//    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
//    int topId = fLayers[ilayer]->GetMatIDs()[3];
//    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    
    
    TPZMatPoisson3d * mat = new TPZMatPoisson3d(RockId,dim);
    cmesh->InsertMaterialObject(mat);
    
    //    // Bc Bottom
    //    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, typeFlux, val1, val2);
    //    cmesh->InsertMaterialObject(bcBottom);
    //
    //    // Bc Right
    //    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, typePressure, val1, val2);
    //    cmesh->InsertMaterialObject(bcRight);
    //
    //    // Bc Top
    //    TPZBndCond * bcTop = mat->CreateBC(mat, topId, typeFlux, val1, val2);
    //    cmesh->InsertMaterialObject(bcTop);
    //
    //    // Bc Left
    //    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, typeFlux, val1, val2);
    //    cmesh->InsertMaterialObject(bcLeft);
    
    // Setando L2
    cmesh->SetDimModel(dim);
    cmesh->SetDefaultOrder(porder);
    
    cmesh->SetAllCreateFunctionsContinuous();
    cmesh->ApproxSpace().CreateDisconnectedElements(true);
    //    cmesh->SetAllCreateFunctionsDiscontinuous();
    cmesh->AutoBuild();
    
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        newnod.SetLagrangeMultiplier(1);
    }
    
#ifdef PZDEBUG
    std::ofstream out("cmeshPress.txt");
    cmesh->Print(out);
#endif
    
    return cmesh;
    
}

TPZCompMesh * TPZDarcyAnalysis::CmeshSw(int Sworder)
{
    
    int dim = fgmesh->Dimension();
    int ilayer = 0;
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    const int typeFlux = 0, typePressure = 1;
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    TPZMatPoisson3d * mat = new TPZMatPoisson3d(RockId,dim);
    cmesh->InsertMaterialObject(mat);
    
    // Bc Bottom
    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcBottom);
    
    // Bc Right
    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, typePressure, val1, val2);
    cmesh->InsertMaterialObject(bcRight);
    
    // Bc Top
    TPZBndCond * bcTop = mat->CreateBC(mat, topId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcTop);
    
    // Bc Left
    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcLeft);
    
    // Void material
    int matIdL2Proj = fSimulationData->fMatL2;
    TPZVec<STATE> sol(1,0.);
    TPZL2Projection *matl2proj = new TPZL2Projection(matIdL2Proj,dim,mat->NStateVariables(),sol);
    cmesh->InsertMaterialObject(matl2proj);
    
    // Setando L2
    cmesh->SetDimModel(dim);
    cmesh->SetDefaultOrder(Sworder);
    
    cmesh->SetAllCreateFunctionsDiscontinuous();
    cmesh->AutoBuild();
    
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    
#ifdef PZDEBUG
    std::ofstream out("cmeshSw.txt");
    cmesh->Print(out);
#endif
    
    return cmesh;
    
}

TPZCompMesh * TPZDarcyAnalysis::CmeshSo(int Soorder)
{
    
    int dim = fgmesh->Dimension();
    int ilayer = 0;
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    const int typeFlux = 0, typePressure = 1;
    TPZFMatrix<STATE> val1(3,2,0.), val2(3,1,0.);
    
    // Malha computacional
    TPZCompMesh *cmesh = new TPZCompMesh(fgmesh);
    
    TPZMatPoisson3d * mat = new TPZMatPoisson3d(RockId,dim);
    cmesh->InsertMaterialObject(mat);
    
    // Bc Bottom
    TPZBndCond * bcBottom = mat->CreateBC(mat, bottomId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcBottom);
    
    // Bc Right
    TPZBndCond * bcRight = mat->CreateBC(mat, rigthId, typePressure, val1, val2);
    cmesh->InsertMaterialObject(bcRight);
    
    // Bc Top
    TPZBndCond * bcTop = mat->CreateBC(mat, topId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcTop);
    
    // Bc Left
    TPZBndCond * bcLeft = mat->CreateBC(mat, leftId, typeFlux, val1, val2);
    cmesh->InsertMaterialObject(bcLeft);
    
    // Void material
    int matIdL2Proj = fSimulationData->fMatL2;
    TPZVec<STATE> sol(1,0.);
    TPZL2Projection *matl2proj = new TPZL2Projection(matIdL2Proj,dim,mat->NStateVariables(),sol);
    cmesh->InsertMaterialObject(matl2proj);
    
    // Setando L2
    cmesh->SetDimModel(dim);
    cmesh->SetDefaultOrder(Soorder);
    
    cmesh->SetAllCreateFunctionsDiscontinuous();
    cmesh->AutoBuild();
    
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    
#ifdef PZDEBUG
    std::ofstream out("cmeshSo.txt");
    cmesh->Print(out);
#endif
    
    return cmesh;
}


void TPZDarcyAnalysis::ReadGeoMesh(std::string GridFileName)
{
    TPZReadGIDGrid GeometryInfo;
    GeometryInfo.SetfDimensionlessL(1.0);
    fgmesh = GeometryInfo.GeometricGIDMesh(GridFileName);
    fgmesh->SetDimension(3);
}

void TPZDarcyAnalysis::CreatedGeoMesh()
{
    
    
    long Qnodes = 4;
    int ilayer = 0;
    
    TPZGeoMesh *gmesh= new TPZGeoMesh;
    
    gmesh->SetMaxNodeId(Qnodes-1);
    gmesh->NodeVec().Resize(Qnodes);
    TPZVec<TPZGeoNode> Node(Qnodes);
    
    TPZVec <long> TopolQuad(4);
    TPZVec <long> TopolLine(2);
    REAL r     = fLayers[ilayer]->Layerr();
    REAL rw    = fLayers[ilayer]->Layerrw();
    REAL h     = fLayers[ilayer]->Layerh();
    REAL top   = fLayers[ilayer]->LayerTop();
    
    int RockId = fLayers[ilayer]->GetMatIDs()[0];
    int bottomId = fLayers[ilayer]->GetMatIDs()[1];
    int rigthId = fLayers[ilayer]->GetMatIDs()[2];
    int topId = fLayers[ilayer]->GetMatIDs()[3];
    int leftId = fLayers[ilayer]->GetMatIDs()[4];
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    Node[id].SetCoord(0 ,  rw);         //coord r
    Node[id].SetCoord(1 , top - h);     //coord z
    gmesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    Node[id].SetCoord(0 ,  rw + r);         //coord r
    Node[id].SetCoord(1 , top - h);     //coord z
    gmesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    Node[id].SetCoord(0 ,  rw + r);         //coord r
    Node[id].SetCoord(1 ,  top);     //coord z
    gmesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    Node[id].SetCoord(0 , rw);         //coord r
    Node[id].SetCoord(1 , top);     //coord z
    gmesh->NodeVec()[id] = Node[id];
    id++;
    
    
    //  Geometric Elements
    int elid = 0;
    
    TopolLine[0] = 0;
    TopolLine[1] = 1;
    new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elid,TopolLine,bottomId,*gmesh);
    id++;
    
    TopolLine[0] = 1;
    TopolLine[1] = 2;
    new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elid,TopolLine,rigthId,*gmesh);
    id++;
    
    TopolLine[0] = 2;
    TopolLine[1] = 3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elid,TopolLine,topId,*gmesh);
    id++;
    
    TopolLine[0] = 0;
    TopolLine[1] = 3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elid,TopolLine,leftId,*gmesh);
    id++;
    
    
    TopolQuad[0] = 0;
    TopolQuad[1] = 1;
    TopolQuad[2] = 2;
    TopolQuad[3] = 3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (elid,TopolQuad,RockId,*gmesh);
    
    
    
    
    gmesh->BuildConnectivity();
    fgmesh = gmesh;
    
}

void TPZDarcyAnalysis::ParametricfunctionX(const TPZVec<STATE> &par, TPZVec<STATE> &X)
{
    X[0] = par[0];//cos(par[0]);
    X[1] = 0.0 * par[0];//sin(par[0]);
    X[2] = 0.0;
}

void TPZDarcyAnalysis::ParametricfunctionY(const TPZVec<STATE> &par, TPZVec<STATE> &X)
{
    X[0] = 0.0;//par[0];
    X[1] = par[0];
    X[2] = 0.0;
}

void TPZDarcyAnalysis::ApplyPG(TPZGeoMesh * geomesh){

    if (!geomesh) {
        DebugStop();
    }
    
    int n = 0;
    REAL dx = fSimulationData->GetLengthElementx();
    REAL ratio = fSimulationData->GetPGRatio();
    REAL l = REAL(fSimulationData->GetnElementsx())*dx;
    int n_nodes = geomesh->NNodes();
    REAL dr_0,dr_n;
    REAL r_powers = 0.0;
    for (int i=1; i <= n_nodes-1; i++) {
        r_powers += std::pow(ratio, REAL(i-1));
    }
    dr_0 = l/r_powers;
    TPZManVector<REAL,3> coor_n(3,0.0);
    TPZManVector<REAL,3> coor(3,0.0);
    for (int inode = 1; inode < n_nodes-1; inode++) {
        geomesh->NodeVec()[inode-1].GetCoordinates(coor);
        geomesh->NodeVec()[inode].GetCoordinates(coor_n);
        
        n = (n_nodes-2) - inode + 1;
        dr_n = dr_0*std::pow(ratio, REAL(n));
        coor_n[0] = dr_n + coor[0];
        geomesh->NodeVec()[inode].SetCoord(coor_n);
        
    }
    
}

void TPZDarcyAnalysis::Geometry2D(int nx, int ny)
{
    REAL t=0.0;
    REAL dt = fSimulationData->GetLengthElementx();
    int n = nx;
    
    // Creating a 0D element to be extruded
    TPZGeoMesh * GeoMesh1 = new TPZGeoMesh;
    GeoMesh1->NodeVec().Resize(1);
    TPZGeoNode Node;
    TPZVec<REAL> coors(3,0.0);
    coors[0] = fLayers[0]->Layerrw();
    Node.SetCoord(coors);
    Node.SetNodeId(0);
    
    
    GeoMesh1->NodeVec()[0]=Node;
    
    TPZVec<long> Topology(1,0);
    int elid=0;
    int matid=1;
    
    new TPZGeoElRefPattern < pzgeom::TPZGeoPoint >(elid,Topology,matid,*GeoMesh1);
    GeoMesh1->BuildConnectivity();
    GeoMesh1->SetDimension(0);
    GeoMesh1->SetMaxNodeId(0);
    GeoMesh1->SetMaxElementId(0);
    
    TPZHierarquicalGrid *CreateGridFrom = new TPZHierarquicalGrid(GeoMesh1);
    TPZAutoPointer<TPZFunction<STATE> > ParFunc = new TPZDummyFunction<STATE>(ParametricfunctionX);
    CreateGridFrom->SetParametricFunction(ParFunc);
    CreateGridFrom->SetFrontBackMatId(5,3);
    
    // Computing Mesh extruded along the parametric curve ParametricfunctionX
    TPZGeoMesh * GeoMesh2 = CreateGridFrom->ComputeExtrusion(t, dt, n);
    if (fSimulationData->IsMeshwithPGQ()) {
        ApplyPG(GeoMesh2);
    }

    TPZHierarquicalGrid * CreateGridFrom2 = new TPZHierarquicalGrid(GeoMesh2);
    TPZAutoPointer<TPZFunction<STATE> > ParFunc2 = new TPZDummyFunction<STATE>(ParametricfunctionY);
    CreateGridFrom2->SetParametricFunction(ParFunc2);
    CreateGridFrom2->SetFrontBackMatId(2,4);
    
    if (fSimulationData->IsTriangularMeshQ())
    {
        CreateGridFrom2->SetTriangleExtrusion();
    }
    
    dt = fSimulationData->GetLengthElementy();
    n = ny;
    
    // Computing Mesh extruded along the parametric curve ParametricfunctionY
    fgmesh = CreateGridFrom2->ComputeExtrusion(t, dt, n);
    
    
}

void TPZDarcyAnalysis::PrintGeoMesh()
{
    
//#ifdef PZDEBUG
    //  Print Geometrical Base Mesh
    std::ofstream argument("GeometicMesh.txt");
    fgmesh->Print(argument);
    std::ofstream Dummyfile("GeometricMesh.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(fgmesh,Dummyfile, true);
    
//#endif
}

void TPZDarcyAnalysis::RotateGeomesh(REAL CounterClockwiseAngle)
{
    REAL theta = CounterClockwiseAngle;
    // It represents a 3D rotation around the z axis.
    TPZFMatrix<STATE> RotationMatrix(3,3,0.0);
    RotationMatrix(0,0) =   +cos(theta);
    RotationMatrix(0,1) =   -sin(theta);
    RotationMatrix(1,0) =   +sin(theta);
    RotationMatrix(1,1) =   +cos(theta);
    RotationMatrix(2,2) = 1.0;
    TPZVec<STATE> iCoords(3,0.0);
    TPZVec<STATE> iCoordsRotated(3,0.0);
    
    RotationMatrix.Print("Rotation = ");
    
    int NumberofGeoNodes = fgmesh->NNodes();
    for (int inode = 0; inode < NumberofGeoNodes; inode++)
    {
        TPZGeoNode GeoNode = fgmesh->NodeVec()[inode];
        GeoNode.GetCoordinates(iCoords);
        // Apply rotation
        iCoordsRotated[0] = RotationMatrix(0,0)*iCoords[0]+RotationMatrix(0,1)*iCoords[1]+RotationMatrix(0,2)*iCoords[2];
        iCoordsRotated[1] = RotationMatrix(1,0)*iCoords[0]+RotationMatrix(1,1)*iCoords[1]+RotationMatrix(1,2)*iCoords[2];
        iCoordsRotated[2] = RotationMatrix(2,0)*iCoords[0]+RotationMatrix(2,1)*iCoords[1]+RotationMatrix(2,2)*iCoords[2];
        GeoNode.SetCoord(iCoordsRotated);
        fgmesh->NodeVec()[inode] = GeoNode;
    }
}

void TPZDarcyAnalysis::ApplyShear(REAL CounterClockwiseAngle)
{
    REAL theta = CounterClockwiseAngle;
    // It represents a 3D rotation around the z axis.
    TPZFMatrix<STATE> ShearMatrix(3,3,0.0);
    ShearMatrix(0,0) =   1.0;
    ShearMatrix(0,1) =   sin(theta);
    ShearMatrix(1,0) =   0.0;
    ShearMatrix(1,1) =   1.0;
    ShearMatrix(2,2) = 1.0;
    TPZVec<STATE> iCoords(3,0.0);
    TPZVec<STATE> iCoordsRotated(3,0.0);
    
    ShearMatrix.Print("ShearMatrix = ");
    
    int NumberofGeoNodes = fgmesh->NNodes();
    for (int inode = 0; inode < NumberofGeoNodes; inode++)
    {
        TPZGeoNode GeoNode = fgmesh->NodeVec()[inode];
        GeoNode.GetCoordinates(iCoords);
        // Apply rotation
        iCoordsRotated[0] = ShearMatrix(0,0)*iCoords[0]+ShearMatrix(0,1)*iCoords[1]+ShearMatrix(0,2)*iCoords[2];
        iCoordsRotated[1] = ShearMatrix(1,0)*iCoords[0]+ShearMatrix(1,1)*iCoords[1]+ShearMatrix(1,2)*iCoords[2];
        iCoordsRotated[2] = ShearMatrix(2,0)*iCoords[0]+ShearMatrix(2,1)*iCoords[1]+ShearMatrix(2,2)*iCoords[2];
        GeoNode.SetCoord(iCoordsRotated);
        fgmesh->NodeVec()[inode] = GeoNode;
    }
}

void TPZDarcyAnalysis::UniformRefinement(int nh)
{
    for ( int ref = 0; ref < nh; ref++ ){
        TPZVec<TPZGeoEl *> filhos;
        long n = fgmesh->NElements();
        for ( long i = 0; i < n; i++ ){
            TPZGeoEl * gel = fgmesh->ElementVec() [i];
            if (gel->Dimension() == 2 || gel->Dimension() == 1) gel->Divide (filhos);
        }//for i
    }//ref
    fgmesh->BuildConnectivity();
}

void TPZDarcyAnalysis::UniformRefinement(int nh, std::set<int> &MatToRef)
{
//    gRefDBase.InitializeUniformRefPattern(EOned);
//    gRefDBase.InitializeUniformRefPattern(ETriangle);
//    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
    
    for ( int ref = 0; ref < nh; ref++ ){
        TPZVec<TPZGeoEl *> filhos;
        long n = fgmesh->NElements();
        for ( long i = 0; i < n; i++ ){
            TPZGeoEl * gel = fgmesh->ElementVec() [i];
            if(!gel || gel->HasSubElement())
            {
                continue;
            }
//                int reflevel = gel->Level();
//                if (reflevel == ref + 1) {
//                    continue;
//                }
            TPZRefPatternTools::RefineDirectional(gel,MatToRef);
        }//for i
    }//ref
    fgmesh->BuildConnectivity();
}

void TPZDarcyAnalysis::UniformRefinement(int nh, int MatId)
{
    //    for ( int ref = 0; ref < nh; ref++ ){
    //        TPZVec<TPZGeoEl *> filhos;
    //        long n = fgmesh->NElements();
    //        for ( long i = 0; i < n; i++ ){
    //            TPZGeoEl * gel = fgmesh->ElementVec() [i];
    //            if(!gel){continue;}
    //            if (gel->Dimension() == 1){
    //                if (gel->MaterialId() == MatId) {
    //                    gel->Divide(filhos);
    //                }
    //
    //            }
    //        }//for i
    //    }//ref
    
    ///Refinamento
//    gRefDBase.InitializeUniformRefPattern(EOned);
//    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
    
    
    for (int idivide = 0; idivide < nh; idivide++){
        const int nels = fgmesh->NElements();
        TPZVec< TPZGeoEl * > allEls(nels);
        for(int iel = 0; iel < nels; iel++){
            allEls[iel] = fgmesh->ElementVec()[iel];
        }
        
        for(int iel = 0; iel < nels; iel++){
            TPZGeoEl * gel = allEls[iel];
            if(!gel) continue;
            if(gel->HasSubElement()) continue;
            int nnodes = gel->NNodes();
            int found = -1;
            for(int in = 0; in < nnodes; in++){
                if(gel->NodePtr(in)->Id() == MatId){
                    found = in;
                    break;
                }
            }///for in
            if(found == -1) continue;
            
            MElementType gelT = gel->Type();
            TPZAutoPointer<TPZRefPattern> uniform = gRefDBase.GetUniformRefPattern(gelT);
            if(!uniform){
                DebugStop();
            }
            gel->SetRefPattern(uniform);
            TPZVec<TPZGeoEl*> filhos;
            gel->Divide(filhos);
            
        }///for iel
    }//idivide
    
    fgmesh->BuildConnectivity();
    
}

////refinamento uniforme em direcao ao no
//void DirectionalRef(int nh, int MatId){
//
//    ///Refinamento
//    gRefDBase.InitializeUniformRefPattern(EOned);
//    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
//
//
//    for (int idivide = 0; idivide < nh; idivide++){
//        const int nels = fgmesh->NElements();
//        TPZVec< TPZGeoEl * > allEls(nels);
//        for(int iel = 0; iel < nels; iel++){
//            allEls[iel] = gmesh->ElementVec()[iel];
//        }
//
//        for(int iel = 0; iel < nels; iel++){
//            TPZGeoEl * gel = allEls[iel];
//            if(!gel) continue;
//            if(gel->HasSubElement()) continue;
//            int nnodes = gel->NNodes();
//            int found = -1;
//            for(int in = 0; in < nnodes; in++){
//                if(gel->NodePtr(in)->Id() == nodeAtOriginId){
//                    found = in;
//                    break;
//                }
//            }///for in
//            if(found == -1) continue;
//
//            MElementType gelT = gel->Type();
//            TPZAutoPointer<TPZRefPattern> uniform = gRefDBase.GetUniformRefPattern(gelT);
//            if(!uniform){
//                DebugStop();
//            }
//            gel->SetRefPattern(uniform);
//            TPZVec<TPZGeoEl*> filhos;
//            gel->Divide(filhos);
//
//        }///for iel
//    }//idivide
//
//    gmesh->BuildConnectivity();
//
//#ifdef LOG4CXX
//    if (logger->isDebugEnabled())
//    {
//        std::stringstream sout;
//        sout<<"gmesh depois de refinar direcionalmente\n";
//        gmesh->Print(sout);
//        LOGPZ_DEBUG(logger, sout.str());
//    }
//#endif
//
//}///void



void TPZDarcyAnalysis::PostProcessVTK(TPZAnalysis *an)
{
    const int dim = 2;
    int div = fSimulationData->GetHPostrefinement();
    TPZStack<std::string> scalnames, vecnames;
    std::string plotfile;
    plotfile = "2DMixed.vtk";
    
    scalnames.Push("P");
    vecnames.Push("u");
    scalnames.Push("Porosity");
    scalnames.Push("Rhs");
    scalnames.Push("div_u");
    
    scalnames.Push("Exact_S");
    vecnames.Push("Exact_GradS");
    
    
    if (fSimulationData->IsOnePhaseQ()) {
        scalnames.Push("Rho_alpha");
        an->DefineGraphMesh(dim, scalnames, vecnames, plotfile);
        an->PostProcess(div);
        return;
    }
    
    
    if (fSimulationData->IsTwoPhaseQ()) {
//        scalnames.Push("Rho_alpha");
//        scalnames.Push("Rho_beta");
        scalnames.Push("S_alpha");
        scalnames.Push("S_beta");
        scalnames.Push("f_alpha");
        scalnames.Push("f_beta");
//        scalnames.Push("P_alpha");
//        scalnames.Push("P_beta");
//        scalnames.Push("Pc_beta_alpha");
        vecnames.Push("u_alpha");
        vecnames.Push("u_beta");
        an->DefineGraphMesh(dim, scalnames, vecnames, plotfile);
        an->PostProcess(div);
        return;
    }
    
    
    if (fSimulationData->IsThreePhaseQ()) {
        scalnames.Push("S_alpha");
        scalnames.Push("S_beta");
        scalnames.Push("S_gamma");
        scalnames.Push("Rho_alpha");
        scalnames.Push("Rho_beta");
        scalnames.Push("Rho_gamma");
        an->DefineGraphMesh(dim, scalnames, vecnames, plotfile);
        an->PostProcess(div);
        return;
    }
    
    
}

void TPZDarcyAnalysis::BCDfunction(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &ff, TPZFMatrix<REAL> &Grad){
    
    REAL t = time;
    REAL pd;
    REAL x = pt[0];
//    REAL y = pt[1];
    
    if(time <= 0.0){
        t = 0.0001;
    }
    
    pd = exp(-x/t) + x;
    ff[0] = pd;
}

void TPZDarcyAnalysis::BCNfunction(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &ff, TPZFMatrix<REAL> &Grad){
    
    REAL t = time;
    REAL qn;
    REAL x = pt[0];
//    REAL y = pt[1];
    
    if(time <= 0.0){
        t = 0.0001;
    }
    
    qn = -1.*(1 - 1/(exp(x/t)*t))*(1 + (-0.1 + exp(-x/t) + x)/10.);
    ff[0] = -qn;
}

void TPZDarcyAnalysis::Ffunction(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &ff, TPZFMatrix<REAL> &Grad)
{
    REAL rwD = 0.0*0.127/100.0;
    REAL rD  = pt[0];
    REAL zD  = pt[1];
    REAL a = 1.0;
    REAL b = 0.1;
    
    REAL f  = ((std::pow(a,2.0) + std::pow(b,2.0))*std::pow(M_PI,2.0)*sin((M_PI*rD)/a)*sin((M_PI*zD)/b))/(std::pow(a,2.0)*std::pow(b,2.));
//    REAL f = (M_PI*(-(a*std::pow(b,2.0)*cos((M_PI*(rD - rwD))/a)) + (std::pow(a,2.0) + std::pow(b,2.0))*M_PI*rD*sin((M_PI*(rD - rwD))/a))*sin((M_PI*zD)/b))/(std::pow(a,2.0)*std::pow(b,2.0)*rD);
    
    ff[0] = f;
    return;
}

TPZFMatrix<STATE> * TPZDarcyAnalysis::ComputeInverse()
{
    int neq = fcmeshdarcy->NEquations();
    TPZFMatrix<STATE> * PreInverse =  new TPZFMatrix<STATE> (neq,neq,0.0);
    TPZFStructMatrix skyl(fcmeshdarcy);
    std::set<int> matids; // to be computed
    matids.insert(1);
    matids.insert(2);
    matids.insert(3);
    matids.insert(4);
    matids.insert(5);
    skyl.SetMaterialIds(matids);
    TPZFMatrix<STATE> rhsfrac;
    TPZFMatrix<STATE> Identity;
    TPZAutoPointer<TPZGuiInterface> gui = new TPZGuiInterface;
    TPZAutoPointer<TPZMatrix<STATE> > MatG = skyl.CreateAssemble(rhsfrac, gui);
    TPZFMatrix<STATE> oldmat = *MatG.operator->();
    oldmat.Inverse( * PreInverse);
    oldmat.Multiply(*PreInverse, Identity);
    
#ifdef PZDEBUG
#ifdef LOG4CXX
    if(logger->isDebugEnabled())
    {
        std::stringstream sout;
        sout << "Is decomposed=  " << MatG->IsDecomposed() << std::endl;
        oldmat.Print("oldmat = ", sout,EMathematicaInput);
        PreInverse->Print("PreInverse = ", sout,EMathematicaInput);
        Identity.Print("Identity = ", sout,EMathematicaInput);
        LOGPZ_DEBUG(logger,sout.str())
    }
#endif
#endif
    
    return PreInverse;
    
}

void TPZDarcyAnalysis::FilterSaturations(TPZManVector<long> &active, TPZManVector<long> &nonactive){
    
    int ncon_sw = fmeshvec[2]->NConnects();
    int ncon = fcmesh->NConnects();
    
    
    // DOF related with the Q-P system
    for(int i = 0; i < ncon-ncon_sw; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        
        int vs = active.size();
        active.Resize(vs+blocksize);
        for(int ieq = 0; ieq<blocksize; ieq++)
        {
            active[vs+ieq] = pos+ieq;
        }
    }
    
   
    // DOF Related with S
    for(int i = ncon-ncon_sw; i< ncon; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        int vs = nonactive.size();
        nonactive.Resize(vs+blocksize);
        for(int ieq = 0; ieq<blocksize; ieq++)
        {
            nonactive[vs+ieq] = pos+ieq;
        }
        
    }
    
    
}

void TPZDarcyAnalysis::FilterSaturationGradients(TPZManVector<long> &active, TPZManVector<long> &nonactive)
{

    int ncon_sw = fmeshvec[2]->NConnects();
    int ncon = fcmesh->NConnects();
    
    
    // DOF related with the Q-P system
    for(int i = 0; i < ncon-ncon_sw; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        
        int vs = active.size();
        active.Resize(vs+blocksize);
        for(int ieq = 0; ieq<blocksize; ieq++)
        {
            active[vs+ieq] = pos+ieq;
        }
    }
    
    // DOF Related with S constant
    for(int i = ncon-ncon_sw; i< ncon; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        int vs = active.size();
        active.Resize(vs+1);
        
        int ieq = blocksize-1;
        active[vs] = pos+ieq;
    }
    
    // DOF Related with S grandients
    for(int i = ncon-ncon_sw; i< ncon; i++)
    {
        TPZConnect &con = fcmesh->ConnectVec()[i];
        int seqnum = con.SequenceNumber();
        int pos = fcmesh->Block().Position(seqnum);
        int blocksize = fcmesh->Block().Size(seqnum);
        int vs = nonactive.size();
        nonactive.Resize(vs+blocksize-1);
        for(int ieq = 0; ieq<blocksize-1; ieq++)
        {
            nonactive[vs+ieq] = pos+ieq;
        }
        
    }
    
}

void TPZDarcyAnalysis::Cylindrical_Elliptic(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &Sol, TPZFMatrix<STATE> &GradSol){
    
    REAL rwD = 0.0*0.127/100.0;
    REAL rD  = pt[0];
    REAL zD  = pt[1];
    REAL a = 1.0;
    REAL b = 0.1;

    Sol[0] = sin(M_PI* (rD-rwD)/a)*sin(M_PI*zD/b);
    GradSol(0,0) = -((M_PI*cos((M_PI*(rD-rwD))/a)*sin((M_PI*zD)/b))/a);
    GradSol(1,0) = -((M_PI*cos((M_PI*zD)/b)*sin((M_PI*(rD-rwD))/a))/b);
    
}

void TPZDarcyAnalysis::Dupuit_Thiem(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &Sol, TPZFMatrix<STATE> &GradSol){
    
    REAL rstar = 1000.0;
    REAL r = pt[0]*rstar;
    REAL rw = 0.127;
    REAL h = 10.0;
    REAL re = 1000.0 + rw;

    REAL Pstar = 20.0*1.0e6;
    REAL day = 86400.0;
    
    REAL Q = 158.99/day;//158.99/day;
    REAL u = Q/(2.0*M_PI*rw*h);
    REAL rho = 1000.0;
    REAL mu = 0.001;
    REAL k = 1.0e-13;
    REAL muD = mu/mu;
    REAL rhoD = rho/rho;
    REAL kD = k/k;

    REAL m = rho * u ;
    REAL reD = re/rstar;
    REAL rwD = rw/rstar;
    REAL rD  = r/rstar;

    REAL mD = (m*mu*rstar)/(k*rho*Pstar);
    Sol[0] = 1 + mD * rwD * log(rD/reD);
    GradSol(0,0) = -((kD*rhoD*mD*rwD)/muD)/(rD);

}

void TPZDarcyAnalysis::Morris_Muskat(const TPZVec<REAL> &pt, REAL time, TPZVec<STATE> &Sol, TPZFMatrix<STATE> &GradSol){
    
    REAL rw = 0.127;
    REAL re = 1000.0 + rw;
    REAL reD = re/rw;
    REAL rD = pt[0];
    REAL rhoD = 1.0;
    REAL muD = 1.0;
    REAL kD = 1.0;
    Sol[0] = - log(rD/reD)/log(re);
    GradSol(0,0) =  (kD*rhoD/muD)/(rD*log(re));
    
    double x = 0.2;
    int p = 0;
    double J = boost::math::cyl_bessel_j(p,x);
    double Y = boost::math::cyl_neumann(p,x);
    std::cout << "x = " << x << ", J = " << J << ", Y = " << Y << std::endl;
    int p2 = 0;
}

void TPZDarcyAnalysis::LinearTracer(const TPZVec< REAL >& pt, REAL time, TPZVec< STATE >& Saturation, TPZFMatrix< STATE >& Grad)
{
    REAL x = pt[0];
    REAL v = 1.0;
    REAL Porosity = 0.2;
    REAL Sor            = 0.20;
    REAL Swr            = 0.20;
    REAL rho = 1.0;
    REAL xshock = (v*time/((1.0-Swr-Sor)*Porosity*rho));
    
    if(x < xshock)
    {
        Saturation[0] = 0.8;
    }
    else
    {
        Saturation[0] = 0.2;
    }
    
    return;
    
}


void TPZDarcyAnalysis::BluckleyAndLeverett(const TPZVec< REAL >& pt, REAL time, TPZVec< STATE >& Saturation, TPZFMatrix< STATE >& Grad)
{
    
    REAL Porosity = 0.2;
    REAL x = pt[0];
    
    REAL x_shock        = 0.0;
    REAL S_shock        = 0.0;
    REAL epsilon        = 1.0e-8;
    REAL u              = 0.1;
    REAL mu_alpha       = 1.0;
    REAL mu_beta        = 1.0;
    REAL rho_alpha      = 1.0;
    REAL rho_beta       = 1.0;
    REAL Sor            = 0.2;
    REAL Swr            = 0.2;
    S_shock = Swr + sqrt(mu_alpha*rho_beta*(mu_beta*rho_alpha + mu_alpha*rho_beta)*std::pow(-1.0 + Sor + Swr,2.0))/(mu_beta*rho_alpha + mu_alpha*rho_beta);
    x_shock  = (u*time)/(Porosity*rho_alpha)*dfdsw(S_shock, Swr, Sor, mu_alpha, mu_beta, rho_alpha, rho_beta);
  
    if(x < x_shock)
    {
        REAL Sw = S_Newton(x, time, u, Swr, Sor, Porosity, S_shock, mu_alpha, mu_beta, rho_alpha, rho_beta, epsilon);
        Saturation[0] = Sw;
        
    }
    else
    {
        Saturation[0] = Swr;
    }

    return;
    
}


REAL TPZDarcyAnalysis::S_Newton(REAL x, REAL t, REAL u, REAL Swr, REAL Sor, REAL phi, REAL s_shok, REAL mu_alpha, REAL mu_beta, REAL rho_alpha, REAL rho_beta, REAL epsilon)
{
    REAL S_trial = ((1.0-Sor) + s_shok)/2.0;
    REAL jac    = 0.0;
    REAL r      = 1.0;
    REAL delta_S;
    REAL ds;
    REAL S_k = S_trial;
    int max = 20;
    int it = 0;
    
    
    
    while ( fabs(r) > epsilon && it < max){

        ds = dfdsw(S_k, Swr, Sor, mu_alpha, mu_beta, rho_alpha, rho_beta);
        r = x - (u * t * ds)/(phi * rho_alpha);

        jac = - (u * t * df2dsw(S_k, Swr, Sor, mu_alpha, mu_beta, rho_alpha, rho_beta))/(phi * rho_alpha);
        
        delta_S = -r/jac;
        S_k += delta_S;
        
        ds = dfdsw(S_k, Swr, Sor, mu_alpha, mu_beta, rho_alpha, rho_beta);
        r = x - (u * t * ds)/(phi * rho_alpha);
        it++;
        
    }
    
    return S_k;
}

REAL TPZDarcyAnalysis::dfdsw(REAL Sw, REAL Swr, REAL Sor, REAL mu_alpha, REAL mu_beta, REAL rho_alpha, REAL rho_beta)
{
    REAL dfwdSwS;
    
    
    dfwdSwS = (2.0*mu_alpha*mu_beta*rho_alpha*rho_beta*(-1.0 + Sor + Sw)*(Sw - Swr)*(-1.0 + Sor + Swr))/std::pow(mu_alpha*rho_beta*std::pow(-1.0 + Sor + Sw,2.0) + mu_beta*rho_alpha*std::pow(Sw - Swr,2.0),2.0);
    
    return (dfwdSwS);
}
REAL TPZDarcyAnalysis::df2dsw(REAL Sw, REAL Swr, REAL Sor, REAL mu_alpha, REAL mu_beta, REAL rho_alpha, REAL rho_beta)
{
    REAL dfw2dSwS2;

  
    dfw2dSwS2 = (2.0*mu_alpha*mu_beta*rho_alpha*rho_beta*(-1.0 + Sor + Swr)*(-(mu_beta*rho_alpha*std::pow(Sw - Swr,2.0)*(-3.0 + 3.0*Sor + 2.0*Sw + Swr)) + mu_alpha*rho_beta*std::pow(-1.0 + Sor + Sw,2.0)*(-1.0 + Sor - 2.0*Sw + 3.0*Swr)))/
    std::pow(mu_alpha*rho_beta*std::pow(-1.0 + Sor + Sw,2.0) + mu_beta*rho_alpha*std::pow(Sw - Swr,2.0),3.0);

    return dfw2dSwS2;
}


// L2 projection

void TPZDarcyAnalysis::SolveProjection(TPZAnalysis *an, TPZCompMesh *Cmesh)
{
    TPZSkylineStructMatrix full(Cmesh);
    an->SetStructuralMatrix(full);
    TPZStepSolver<STATE> step;
    step.SetDirect(ELDLt);
    //step.SetDirect(ELU);
    an->SetSolver(step);
    an->Run();
    
}


TPZCompMesh * TPZDarcyAnalysis::L2ProjectionCmesh(TPZVec<STATE> &solini)
{
    /// criar materiais
    int dim = 2;
    TPZL2Projection *material;
    material = new TPZL2Projection(1, dim, 1, solini, fSimulationData->Getsorder());
    
    TPZCompMesh * cmesh = new TPZCompMesh(this->fgmesh);
    cmesh->SetDimModel(dim);
    TPZMaterial * mat(material);
    cmesh->InsertMaterialObject(mat);
    TPZAutoPointer<TPZFunction<STATE> > forcef = new TPZDummyFunction<STATE>(InitialS_alpha);
    material->SetForcingFunction(forcef);
    cmesh->SetAllCreateFunctionsDiscontinuous();
    cmesh->SetDefaultOrder(fSimulationData->Getsorder());
    cmesh->SetDimModel(dim);
    cmesh->AutoBuild();
    
    return cmesh;
    
}

void TPZDarcyAnalysis::InitialS_alpha(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    
//    REAL x = pt[0];
    REAL y = pt[1];
    REAL S_wett_nc = 0.2;
    REAL S_nwett_ir = 0.0;
    disp[0] = S_wett_nc;

//    bool inside_x = ( x >= 0.3 ) && (x <= 0.7);
//    bool inside_y = ( y >= 0.3 ) && (y <= 0.7);
    
//    if (y<=0.5) {
//         disp[0] = 0.0;
//    }
//
//    if ( y >= 50.0 ) {
//         disp[0] = 1.0;
//    }
    
}

/**
 * Computes computational mesh for L2 projection
 */
void TPZDarcyAnalysis::P_Hydrostatic(const TPZVec< REAL >& pt, REAL time, TPZVec< STATE >& P_Hydro, TPZFMatrix< STATE >& GradP_Hydro){
    
//    REAL x = pt[0];
    REAL y = pt[1];
    
//    REAL Kstr           = 1.0e-13;
//    REAL Pstr           = 1.0e7;
//    REAL Tstr           = 355.37;
//    REAL Tres           = 355.37;
//    REAL Lstr           = 100.0;
//    REAL Mustr          = 0.001;
//    REAL Rhostr         = 1000.0;

    REAL rho_beta = 1000.0;
    REAL P_at_datum = 1.5*1.0e7;
    REAL g = -10.0;
    P_Hydro[0] = (rho_beta * g * y)+P_at_datum;
    
}


/**
 * Computes convergence rate for an element
 */
void TPZDarcyAnalysis::CheckElementConvergence(int wichelement)
{
    
    TPZFMatrix<STATE> UAtn          = falphaAtn;
    TPZFMatrix<STATE> UAtnPlusOne   = fcmeshdarcy->Solution();
    
    std::ofstream outsol("Solutions.txt");
    UAtn.Print("UAtn =",outsol,EMathematicaInput);
    UAtnPlusOne.Print("UAtnPlusOne =",outsol,EMathematicaInput);
    outsol.flush();
    
    
    long neq = fcmeshdarcy->NEquations();
    TPZElementMatrix Jacobian(fcmeshdarcy, TPZElementMatrix::EK),Residual(fcmeshdarcy, TPZElementMatrix::EF);
    
    int nsteps = 4;
    REAL du=0.0001;
    
    std::ofstream outfile("CheckElementConvergence.txt");
    
    TPZCompEl * icel = fcmeshdarcy->ElementVec()[wichelement];
    if (!icel) {
        std::cout << "There's no element with index = " << wichelement << std::endl;
        DebugStop();
    }
    icel->Print(outfile);
    
    this->SetLastState();
    fcmeshdarcy->LoadSolution(UAtn);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    icel->CalcStiff(Jacobian,Residual);
    TPZFNMatrix<9,REAL> ResidualAtUn = Residual.fMat;
    
    
    this->SetNextState();
    fcmeshdarcy->LoadSolution(UAtnPlusOne);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    icel->CalcStiff(Jacobian,Residual);
    TPZFNMatrix<9,REAL> JacobianAtUnPlusOne = Jacobian.fMat;
    TPZFNMatrix<9,REAL> ResidualAtUnPlusOne = Residual.fMat;
    
    int SizeOfElMat = JacobianAtUnPlusOne.Rows();
    TPZFNMatrix<9,STATE> DeltaU(SizeOfElMat,1,du),JacobianDeltaU(SizeOfElMat,1,0.0);
    TPZFNMatrix<9,STATE> ResidualU(SizeOfElMat,1,0.0),Alpha_ResidualAtUnPlusOne_DeltaU(SizeOfElMat,1,0.0);
    TPZFNMatrix<9,STATE> ResidualUPerturbed(SizeOfElMat,1,0.0);
    
    ResidualU = ResidualAtUnPlusOne + ResidualAtUn;
    
    std::ofstream outRoot("Root.txt");
    std::ofstream outPerturbedRoot("PerturbedRoot.txt");
    
    JacobianAtUnPlusOne.Print("JacobianAtUnPlusOne = ",outRoot,EMathematicaInput);
    ResidualAtUn.Print("ResidualAtUn = ",outRoot,EMathematicaInput);
    ResidualAtUnPlusOne.Print("ResidualAtUnPlusOne = ",outRoot,EMathematicaInput);
    outRoot.flush();
    
    REAL alpha = 0;
    TPZFNMatrix<4,REAL> alphas(nsteps,1,0.0),ElConvergenceOrder(nsteps-1,1,0.0);
    TPZFNMatrix<9,REAL> res(nsteps,1,0.0);
    TPZFMatrix<REAL> DeltaUGlobal(neq,1,du);
    
    for(int j = 0; j < nsteps; j++)
    {
        
        alpha = (1.0*j+1.0)/100.0;
        JacobianAtUnPlusOne.Multiply(alpha*DeltaU,Alpha_ResidualAtUnPlusOne_DeltaU);
        alphas(j,0) = log10(alpha);
        
        fcmeshdarcy->LoadSolution(UAtnPlusOne+alpha*DeltaUGlobal);
        TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
        
        this->SetNextState();
        icel->CalcStiff(Jacobian,Residual);
        
        TPZFNMatrix<9,REAL> ResidualAtUnPlusOne_AlphaDeltaU = Residual.fMat;
        ResidualAtUnPlusOne_AlphaDeltaU.Print("ResidualAtUnPlusOne_AlphaDeltaU =",outPerturbedRoot,EMathematicaInput);
        ResidualUPerturbed = ResidualAtUnPlusOne_AlphaDeltaU + ResidualAtUn;
        STATE NormValue = Norm(ResidualUPerturbed-(ResidualU+Alpha_ResidualAtUnPlusOne_DeltaU));
        res(j) = log10(NormValue);
        
    }
    
    for(int j = 1; j < nsteps ; j++){
        ElConvergenceOrder(j-1,0)=(res(j,0)-res(j-1,0))/(alphas(j,0)-alphas(j-1,0));
    }
    
    ElConvergenceOrder.Print("CheckConv = ",outfile,EMathematicaInput);
    outfile.flush();
    
}




void TPZDarcyAnalysis::CheckGlobalConvergence(TPZAnalysis * an)
{
    
    long neq = fcmeshdarcy->NEquations();
    int nsteps = 5;
    REAL du=1.0;
    REAL alpha = 0;
    
    TPZFMatrix<STATE> U0(neq,1,0.5);//(rand()/double(RAND_MAX))
    TPZFMatrix<STATE> UPlusAlphaDU   = fcmeshdarcy->Solution();
    
    std::ofstream outfile("CheckConvergence.txt");
    
    fcmeshdarcy->LoadSolution(U0);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    this->SetLastState();
    an->AssembleResidual();
    TPZFNMatrix<9,REAL> ResidualAtU0n = an->Rhs();
    
    fcmeshdarcy->LoadSolution(U0);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    this->SetNextState();
    an->Assemble();
    TPZAutoPointer< TPZMatrix<REAL> > Jacobian;
    Jacobian =   an->Solver().Matrix();
    TPZFMatrix<REAL> JacobianAtU0 = *Jacobian.operator->();
    TPZFNMatrix<9,REAL> ResidualAtU0 = an->Rhs();
    
    
    int SizeOfGlobalMat = JacobianAtU0.Rows();
    TPZFNMatrix<9,STATE> DeltaU(SizeOfGlobalMat,1,du),JacobianDeltaU(SizeOfGlobalMat,1,0.0);
    TPZFNMatrix<9,STATE> ResidualU0(SizeOfGlobalMat,1,0.0),Alpha_JacAtU0_DeltaU(SizeOfGlobalMat,1,0.0);
    TPZFNMatrix<9,STATE> ResidualUPerturbed(SizeOfGlobalMat,1,0.0);
    
    ResidualU0 = ResidualAtU0 + ResidualAtU0n;
    
    //    JacobianAtU0.Print("JacobianAtU0 = ",outfile,EMathematicaInput);
    //    ResidualAtU0n.Print("ResidualAtU0n = ",outfile,EMathematicaInput);
    //    ResidualAtU0.Print("ResidualAtU0 = ",outfile,EMathematicaInput);
    //    ResidualU0.Print("ResidualU0 = ",outfile,EMathematicaInput);
    //    U0.Print("U0 = ",outfile,EMathematicaInput);
    
    TPZFNMatrix<4,REAL> alphas(nsteps,1,0.0),ElConvergenceOrder(nsteps-1,1,0.0);
    TPZFNMatrix<9,REAL> res(nsteps,1,0.0);
    this->SetNextState();
    for(int j = 0; j < nsteps; j++)
    {
        
        alpha = (j+1.0)/1000.0;
        
        JacobianAtU0.Multiply(alpha*DeltaU,Alpha_JacAtU0_DeltaU);
        
        fcmeshdarcy->LoadSolution(U0+alpha*DeltaU);
        TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
        
        
        an->AssembleResidual();
        TPZFNMatrix<9,REAL> ResidualAtU0PlusAlphaDU = an->Rhs();
        ResidualUPerturbed = ResidualAtU0PlusAlphaDU + ResidualAtU0n;
        TPZFNMatrix<9,REAL> error = ResidualUPerturbed-(ResidualU0+Alpha_JacAtU0_DeltaU);
        
        //        outfile << alpha <<std::endl;
        //        error.Print("error = ",outfile,EMathematicaInput);
        
        REAL NormValue = Norm(error);
        res(j) = log(NormValue);
        alphas(j,0) = log(alpha);
        
    }
    
    for(int j = 1; j < nsteps ; j++){
        ElConvergenceOrder(j-1,0)=(res(j,0)-res(j-1,0))/(alphas(j,0)-alphas(j-1,0));
    }
    
    ElConvergenceOrder.Print("CheckConv = ",outfile,EMathematicaInput);
    outfile.flush();
    
}

void TPZDarcyAnalysis::CheckGlobalJacobian(TPZAnalysis * an)
{
    
    long neq = fcmeshdarcy->NEquations();
    REAL du=1.0;
    
    TPZFMatrix<STATE> U(neq,1,0.0);//(rand()/double(RAND_MAX))
    
    U(0,0) = -0.5 ;
    U(1,0) = -0.5 ;
    U(2,0) = 0.5;
    U(3,0) = 0.5;
    
    U(12,0) = 10.29367992;
    U(13,0) = 0.1;
    
    U(14,0) = 0.1;
    U(15,0) = 10.29367992;
    
    U(16,0) = 1.0;
    U(17,0) = 0.0;
    
    TPZFMatrix<STATE> DeltaU(neq,1,du);
    DeltaU = du*U;
    TPZFMatrix<STATE> Uk = U + DeltaU;
    
    
    Uk(16,0) = 1.0 -  0.0 * du;
    Uk(17,0) = 0.0 +  0.0 * du;
    Uk.Print("Uk = ");
    std::ofstream outfile("CheckJacobian.txt");
    
    fcmeshdarcy->LoadSolution(U);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    this->SetLastState();
    an->AssembleResidual();
    TPZFNMatrix<9,REAL> ResidualAtUn = an->Rhs();
    
    fcmeshdarcy->LoadSolution(U);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    this->SetNextState();
    an->Assemble();
    TPZAutoPointer< TPZMatrix<REAL> > Jacobian;
    Jacobian =   an->Solver().Matrix();
    TPZFMatrix<REAL> JacobianAtU = *Jacobian.operator->();
    TPZFNMatrix<9,REAL> ResidualAtU = an->Rhs();
    
    
    int SizeOfGlobalMat = JacobianAtU.Rows();
    TPZFNMatrix<9,STATE> JacobianDeltaU(SizeOfGlobalMat,1,0.0);
    TPZFNMatrix<9,STATE> ResidualU(SizeOfGlobalMat,1,0.0),Jac_DeltaU(SizeOfGlobalMat,1,0.0);
    TPZFNMatrix<9,STATE> Residual(SizeOfGlobalMat,1,0.0);
    
    ResidualU = ResidualAtU + ResidualAtUn;
    
    fcmeshdarcy->LoadSolution(Uk);
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(fmeshvec, fcmeshdarcy);
    
    an->AssembleResidual();
    TPZFNMatrix<9,REAL> ResidualAtUk = an->Rhs() + ResidualAtUn;
    JacobianAtU.Multiply(Uk-U,Jac_DeltaU);
    Residual = ResidualAtUk - (ResidualU + JacobianDeltaU);
    
    JacobianAtU.Print("JacobianAtU = ",outfile,EMathematicaInput);
    JacobianDeltaU.Print("JacobianDeltaU = ",outfile,EMathematicaInput);
    ResidualU.Print("ResidualU = ",outfile,EMathematicaInput);
    ResidualAtUk.Print("ResidualAtUk = ",outfile,EMathematicaInput);
    Residual.Print("Residual = ",outfile,EMathematicaInput);
    //    U.Print("U = ",outfile,EMathematicaInput);
    //    Uk.Print("Uk = ",outfile,EMathematicaInput);
    outfile << std::endl;
    outfile << "Norm DeltaU " << Norm(DeltaU) << std::endl;
    outfile << "Norm Residual " << Norm(Residual) << std::endl;
    outfile.flush();
    
}