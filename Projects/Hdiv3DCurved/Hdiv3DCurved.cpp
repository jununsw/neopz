//
//  Hdiv3DCurved.cpp
//  Publication about 3D Curved piola mapping for mixed formulation
//
//  Created by omar on 01/07/2016.
//
//

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>

#include "pzlog.h"
#include "tpzautopointer.h"
#include "TPZRefPatternTools.h"


#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"
#include "TPZRefPatternDataBase.h"
#include "TPZRefPatternTools.h"
#include "pzgeopoint.h"
#include "TPZGeoLinear.h"
#include "TPZGeoCube.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"
#include "pzgeoelside.h"
#include "tpzgeoblend.h"
#include "tpzarc3d.h"
#include "pzgeotetrahedra.h"
#include "pzgeoelrefless.h"
#include "tpzquadraticquad.h"
#include "tpzquadraticline.h"
#include "TPZQuadSphere.h"
#include "TPZTriangleSphere.h"

#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"
#include "pzfunction.h"
#include "tpzchangeel.h"

#include "pzpoisson3d.h"
#include "mixedpoisson.h"

#include "TPZPrimalPoisson.h"
#include "TPZDualPoisson.h"
#include "pzbndcond.h"
#include "pzbuildmultiphysicsmesh.h"

#include "TPZCompMeshTools.h"

#include "pzanalysis.h"
#include "pzstepsolver.h"
#include "pzskylstrmatrix.h"
#include "TPZParFrontStructMatrix.h"
#include "TPZSSpStructMatrix.h"

#include "pzgmesh.h"
#include "TPZVTKGeoMesh.h"

#ifdef USING_BOOST
#include "boost/date_time/posix_time/posix_time.hpp"
#endif


struct SimulationCase {
    bool            IsHdivQ;
    bool            UsePardisoQ;
    bool            UseFrontalQ;
    int             n_h_levels;
    int             n_p_levels;
    int             n_acc_terms;
    int             int_order;
    int             n_threads;
    std::string     mesh_type;
    std::string     domain_type;
    std::string     conv_summary;
    std::string     dump_folder;
    TPZStack<int>   omega_ids;
    TPZStack<int>   gamma_ids;
    
    SimulationCase() : IsHdivQ(false), UsePardisoQ(true), UseFrontalQ(false), n_h_levels(0), n_p_levels(1), n_acc_terms(1), int_order(1), n_threads(0), mesh_type(""),
    domain_type(""),conv_summary(""),dump_folder(""),omega_ids(),gamma_ids()
    {
        
    }
    
    SimulationCase(const SimulationCase &copy) : IsHdivQ(copy.IsHdivQ), UsePardisoQ(copy.UsePardisoQ), UseFrontalQ(copy.UseFrontalQ),
        n_h_levels(copy.n_h_levels), n_p_levels(copy.n_p_levels), n_acc_terms(copy.n_acc_terms), int_order(copy.int_order),
        n_threads(copy.n_threads), mesh_type(copy.mesh_type), domain_type(copy.domain_type), conv_summary(copy.conv_summary),
        dump_folder(copy.dump_folder), omega_ids(copy.omega_ids), gamma_ids(copy.gamma_ids)
    {
        
    }
    
    SimulationCase &operator=(const SimulationCase &copy)
    {
        IsHdivQ = copy.IsHdivQ;
        UsePardisoQ = copy.UsePardisoQ;
        UseFrontalQ = copy.UseFrontalQ;
        n_h_levels = copy.n_h_levels;
        n_p_levels = copy.n_p_levels;
        n_acc_terms = copy.n_acc_terms;
        int_order = copy.int_order;
        n_threads = copy.n_threads;
        mesh_type = copy.mesh_type;
        domain_type = copy.domain_type;
        conv_summary = copy.conv_summary;
        dump_folder = copy.dump_folder;
        omega_ids = copy.omega_ids;
        gamma_ids = copy.gamma_ids;
        return *this;
    }
};

//#define Solution1
#define Solution6




static void Analytic(const TPZVec<REAL> &x, TPZVec<STATE> &u,TPZFMatrix<STATE> &gradu);
static void Solution(const TPZVec<REAL> &x, TPZVec<STATE> &f);
static void f(const TPZVec<REAL> &p, TPZVec<STATE> &f, TPZFMatrix<STATE> &gradf);

TPZGeoMesh * GeomtricMesh(int ndiv, SimulationCase sim_data);

TPZGeoMesh * MakeSphereFromLinearQuadrilateralFaces(int ndiv, SimulationCase sim_data);
TPZGeoMesh * MakeSphereFromQuadrilateralFaces(int ndiv, SimulationCase sim_data);

TPZManVector<STATE,3> ParametricSphere(REAL radius,REAL theta,REAL phi);
void TransformToQuadratic(TPZGeoMesh *gmesh);
void RotateGeomesh(TPZGeoMesh *gmesh, REAL CounterClockwiseAngle, int &Axis);

TPZCompMesh *DualMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data, TPZVec<TPZCompMesh *> &meshvec);

TPZCompMesh * ComputationalMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data, long &ndof, TPZVec<TPZCompMesh *> &meshvec);

TPZCompMesh * PrimalMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data); //  Primal approximation
TPZCompMesh * qMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data); // Hdiv space
TPZCompMesh * pMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data); // L2 space

/// adjust the polynomial orders of the hdiv elements such that the internal order is higher than the sideorders
void AdjustFluxPolynomialOrders(TPZCompMesh *fluxmesh, int hdivplusplus);

/// set the pressure order acording to the order of internal connect of the elements of the fluxmesh
void SetPressureOrders(TPZCompMesh *fluxmesh, TPZCompMesh *pressuremesh);


TPZAnalysis * CreateAnalysis(TPZCompMesh * cmesh, SimulationCase sim_data);
void PosProcess(TPZCompMesh* cmesh, TPZAnalysis * an, std::string file, SimulationCase sim_data);

void ComputeCases(TPZStack<SimulationCase> cases);
void ComputeApproximation(SimulationCase sim_data);
void ComputeConvergenceRates(TPZVec<STATE> &error, TPZVec<STATE> &convergence);

STATE IntegrateVolume(TPZGeoMesh * geometry, SimulationCase sim_data);
STATE IntegrateSolution(TPZCompMesh * cmesh,  SimulationCase sim_data);

void ErrorH1(TPZCompMesh *cmesh, REAL &error_primal , REAL & error_dual, REAL & error_h1);
void ErrorHdiv(TPZCompMesh *cmesh, REAL &error_primal , REAL & error_dual, REAL & error_hdiv);

int main()
{

  HDivPiola = 1;
    
  gRefDBase.InitializeAllUniformRefPatterns();
    //	gRefDBase.InitializeRefPatterns();
    
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    
    TPZStack<SimulationCase> simulations;
    
    // Primal Formulation over the solid sphere
    struct SimulationCase common;
    common.UsePardisoQ = true;
    common.UseFrontalQ = false;
    common.n_h_levels = 1;
    common.n_p_levels = 1;
    common.int_order  = 10;
    common.n_threads  = 8;
    common.domain_type = "sphere";
    common.conv_summary = "convergence_summary";
    common.omega_ids.Push(1);     // Domain
    common.gamma_ids.Push(-1);    // Gamma_D outer surface
    common.gamma_ids.Push(-2);    // Gamma_D inner surface
    
    // Primal Formulation over the solid sphere
//    struct SimulationCase H1Case_1 = common;
//    H1Case_1.IsHdivQ = false;
//    H1Case_1.mesh_type = "linear";
//    H1Case_1.dump_folder = "H1_sphere";
//    simulations.Push(H1Case_1);
    
    // Primal Formulation over the solid sphere
    struct SimulationCase H1Case_2 = common;
    H1Case_2.IsHdivQ = false;
    H1Case_2.mesh_type = "quadratic";
    H1Case_2.dump_folder = "H1_sphere";
    simulations.Push(H1Case_2);

//    // Primal Formulation over the solid sphere
//    struct SimulationCase H1Case_3 = common;
//    H1Case_3.IsHdivQ = false;
//    H1Case_3.mesh_type = "blended";
//    H1Case_3.dump_folder = "H1_sphere";
//    simulations.Push(H1Case_3);
//
    
//    // Dual Formulation over the solid sphere
//    struct SimulationCase HdivCase_1 = common;
//    HdivCase_1.IsHdivQ = true;
//    HdivCase_1.mesh_type = "linear";
//    HdivCase_1.dump_folder = "Hdiv_sphere";
//    simulations.Push(HdivCase_1);
    
//    // Dual Formulation over the solid sphere
//    struct SimulationCase HdivCase_2 = common;
//    HdivCase_2.IsHdivQ = true;
//    HdivCase_2.mesh_type = "quadratic";
//    HdivCase_2.dump_folder = "Hdiv_sphere";
//    simulations.Push(HdivCase_2);
    
//    // Dual Formulation over the solid sphere
//    struct SimulationCase HdivCase_3 = common;
//    HdivCase_3.IsHdivQ = true;
//    HdivCase_3.mesh_type = "blended";
//    HdivCase_3.dump_folder = "Hdiv_sphere";
//    simulations.Push(HdivCase_3);

    ComputeCases(simulations);
    
    return 0;
}

void ComputeCases(TPZStack<SimulationCase> cases){
    
    int n_cases = cases.size();
    for (int i = 0; i < n_cases; i++) {
        ComputeApproximation(cases[i]);
    }
}

void ComputeApproximation(SimulationCase sim_data){
    
#ifdef USING_BOOST
    boost::posix_time::ptime int_case_t1 = boost::posix_time::microsec_clock::local_time();
#endif
    
    // Creating the directory
    std::string command = "mkdir " + sim_data.dump_folder;
    system(command.c_str());
    
    std::stringstream summary;
    summary   << sim_data.dump_folder << "/" "conv" << "_" << sim_data.mesh_type << "_" << sim_data.domain_type << ".txt";
    std::ofstream convergence(summary.str(),std::ios::app);
    
    TPZManVector<STATE,10> p_error(sim_data.n_h_levels+1,1.0);
    TPZManVector<STATE,10> d_error(sim_data.n_h_levels+1,1.0);
    TPZManVector<STATE,10> h_error(sim_data.n_h_levels+1,1.0);
    
    TPZManVector<STATE,10> p_conv(sim_data.n_h_levels,0.0);
    TPZManVector<STATE,10> d_conv(sim_data.n_h_levels,0.0);
    TPZManVector<STATE,10> h_conv(sim_data.n_h_levels,0.0);
    
    int n_h_levels = sim_data.n_h_levels;
    int n_p_levels = sim_data.n_p_levels;
    

    using namespace std;
    
    for (int p = 1; p <= n_p_levels; p++) {
        
        convergence << std::endl;        
        convergence << " Polynomial order  =  " << p << std::endl;
        convergence << setw(5)  << " h" << setw(25) << " ndof" << setw(25) << " ndof_cond" << setw(25) << " assemble_time (msec)" << setw(25) << " solving_time (msec)" << setw(25) << " error_time (msec)" << setw(25) << " Primal l2 error" << setw(25) << " Dual l2 error"  << setw(25) << " H error (H1 or Hdiv)" << endl;
        
        int h_base = 0;
        for (int h = 0; h <= n_h_levels; h++) {
            
            // Compute the geometry
            h_base = h + 1;
            TPZGeoMesh * gmesh = GeomtricMesh(h_base, sim_data);
     
#ifdef PZDEBUG
            
#ifdef USING_BOOST
            boost::posix_time::ptime int_t1 = boost::posix_time::microsec_clock::local_time();
#endif
            
            STATE volume = IntegrateVolume(gmesh, sim_data);
            
#ifdef USING_BOOST
            boost::posix_time::ptime int_t2 = boost::posix_time::microsec_clock::local_time();
#endif
            
            std::cout << "Domain volume = " << volume << "; Time for integration = " << int_t2-int_t1 <<std::endl;
            

            
            std::stringstream text_name;
            std::stringstream vtk_name;
            text_name   << sim_data.dump_folder << "/" "geo" << "_" << sim_data.mesh_type << "_" << sim_data.domain_type << "_" << "p" << p << "h" <<  h << ".txt";
            vtk_name    << sim_data.dump_folder << "/" "geo" << "_" << sim_data.mesh_type << "_" << sim_data.domain_type << "_" << "p" << p << "h" <<  h << ".vtk";
            ofstream textfile(text_name.str());
            gmesh->Print(textfile);
            
            std::ofstream vtkfile(vtk_name.str());
            TPZVTKGeoMesh::PrintGMeshVTK(gmesh, vtkfile, true);
        
#endif
            
            // Compute the geometry
            long ndof, ndof_cond;
            TPZManVector<TPZCompMesh *,5> meshvec;
            TPZCompMesh * cmesh = ComputationalMesh(gmesh, p, sim_data, ndof, meshvec);

//#ifdef PZDEBUG
//            
//#ifdef USING_BOOST
//            boost::posix_time::ptime int_p_t1 = boost::posix_time::microsec_clock::local_time();
//#endif
//            
//            STATE p_integral = 1.0;//IntegrateSolution(cmesh, sim_data);
//            
//#ifdef USING_BOOST
//            boost::posix_time::ptime int_p_t2 = boost::posix_time::microsec_clock::local_time();
//#endif
//            
//            std::cout << "Analytic pressure integral = " << p_integral << "; Time for integration = " << int_p_t2-int_p_t1 <<std::endl;
//
//#endif
            
            // Create Analysis
            TPZAnalysis * analysis = CreateAnalysis(cmesh,sim_data);
            
#ifdef USING_BOOST
            boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
#endif
            
            analysis->Assemble();
            analysis->Rhs() *= -1.0;
            ndof_cond = analysis->Rhs().Rows();
#ifdef USING_BOOST
            boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
#endif
            
            analysis->Solve();
            
#ifdef USING_BOOST
            boost::posix_time::ptime t3 = boost::posix_time::microsec_clock::local_time();
            STATE assemble_time = boost::numeric_cast<double>((t2-t1).total_milliseconds());
            STATE solving_time  = boost::numeric_cast<double>((t3-t2).total_milliseconds());
#endif
            
            // PostProccessing
            std::stringstream sol_vtk_name;
            sol_vtk_name    << sim_data.dump_folder << "/" "sol" << "_" << sim_data.mesh_type << "_" << sim_data.domain_type << "_" << "p" << p << "h" <<  h << ".vtk";
            std::string file(sol_vtk_name.str());
            PosProcess(cmesh, analysis, file, sim_data);
            
            // compute the error
#ifdef USING_BOOST
            boost::posix_time::ptime error_t1 = boost::posix_time::microsec_clock::local_time();
#endif
            
            if (sim_data.IsHdivQ) {
                ErrorHdiv(cmesh, p_error[h], d_error[h], h_error[h]);
            }
            else{
                ErrorH1(cmesh, p_error[h], d_error[h], h_error[h]);
            }
            
#ifdef USING_BOOST
            boost::posix_time::ptime error_t2 = boost::posix_time::microsec_clock::local_time();
#endif
            STATE error_time = boost::numeric_cast<double>((error_t2 - error_t1).total_milliseconds());
            
            // current summary
            convergence << setw(5) << h << setw(25) << ndof << setw(25) << ndof_cond << setw(25) << assemble_time << setw(25) << solving_time << setw(25) << error_time << setw(25) << p_error[h] << setw(25) << d_error[h]  << setw(25) << h_error[h] << endl;
            
            delete cmesh;
            for (int i=0; i<meshvec.size(); i++) {
                delete meshvec[i];
            }
            delete gmesh;
        }
        
        // compute rates
        ComputeConvergenceRates(p_error,p_conv);
        ComputeConvergenceRates(d_error,d_conv);
        ComputeConvergenceRates(h_error,h_conv);
        
        
        // print convergence summary
        convergence << std::endl;
        convergence << " Convergence rates summary " << std::endl;
        convergence << " Polynomial order  =  " << p << std::endl;
        convergence << " Primal convergence rates = " << setw(5) << p_conv << std::endl;
        convergence << " Dual convergence rates = " << setw(5) << d_conv << std::endl;
        convergence << " H1 or Hdiv convergence rates = " << setw(5) << h_conv << std::endl;
        convergence << std::endl;
        convergence << " ------------------------------------------------------------------ " << std::endl;
        convergence.flush();
        
    }
    
#ifdef USING_BOOST
    boost::posix_time::ptime int_case_t2 = boost::posix_time::microsec_clock::local_time();
#endif
    
#ifdef USING_BOOST
    STATE case_solving_time = boost::numeric_cast<double>((int_case_t2-int_case_t1).total_milliseconds());
#endif
    
    convergence <<  "Case closed in :" << setw(10) <<  case_solving_time/1000.0 << setw(5)   << " seconds." << std::endl;
    std::cout << "Case closed in :" << setw(10) <<  case_solving_time/1000.0 << setw(5)   << " seconds." << std::endl;
    
}

void ComputeConvergenceRates(TPZVec<STATE> &error, TPZVec<STATE> &convergence){
    
    int ndata = error.size();
    STATE log0p5 = log(0.5);
    for (int i = 1; i < ndata; i++) {
        STATE logerror = log(error[i-1]);
        STATE logerrori = log(error[i]);
        convergence[i-1] = (logerrori - logerror)/log0p5;
    }
}

void Analytic(const TPZVec<REAL> &p, TPZVec<STATE> &u,TPZFMatrix<STATE> &gradu){
    
    gradu.Resize(4,1);
    
    STATE x,y,z;
    x = p[0];
    y = p[1];
    z = p[2];
    
    STATE r = sqrt(x*x+y*y+z*z);
    STATE theta = acos(z/r);
    STATE phi = atan2(y,x);
    
    STATE costheta = cos(theta);
    STATE cosphi = cos(phi);
    STATE sintheta = sin(theta);
    STATE sinphi = sin(phi);
    
    // Gradient computations
    
    STATE Radialunitx = sintheta*cosphi;
    STATE Radialunity = sintheta*sinphi;
    STATE Radialunitz = costheta;
    
    STATE Thetaunitx = cosphi*costheta;
    STATE Thetaunity = costheta*sinphi;
    STATE Thetaunitz = -sintheta;
    
    STATE Phiunitx = -sinphi;
    STATE Phiunity = cosphi;
    STATE Phiunitz = 0.0;
    
#ifdef Solution1
    
    STATE dfdr       = 2.0*r;
    STATE dfdTheta   = 0.0;
    STATE dfdPhi     = 0.0;
    
    u[0] = r*r;
    
    gradu(0,0) = -1.0*(dfdr * Radialunitx + dfdTheta * Thetaunitx + dfdPhi * Phiunitx);
    gradu(1,0) = -1.0*(dfdr * Radialunity + dfdTheta * Thetaunity + dfdPhi * Phiunity);
    gradu(2,0) = -1.0*(dfdr * Radialunitz + dfdTheta * Thetaunitz + dfdPhi * Phiunitz);
    
    gradu(3,0) = -6.0;
    
#endif
    
#ifdef Solution6
    
    REAL a = +5.0/4.0;
    REAL b = -1.0/4.0;
    REAL c = -1.0/4.0;
    
    REAL d = 5.0;
    
    REAL xma = x-a;
    REAL ymb = y-b;
    REAL zmc = z-c;
    REAL piover2 = M_PI/2.0;
    REAL piover3 = M_PI/3.0;
    REAL rad = xma*xma + ymb*ymb+ zmc*zmc;
    REAL sqrt_rad = sqrt(rad);
    REAL artan_arg = atan(d*(sqrt_rad - piover3));
    
    
    REAL denomfactor1 = -9.0 + d*d*(-9.0*rad+M_PI*(-M_PI+6.0*sqrt_rad));
    REAL numfactro1   = 18.0*d*(-9.0+d*d*M_PI*(-M_PI+3.0*sqrt_rad));
    
    u[0] = piover2 - artan_arg;
    
    gradu(0,0) = -1.0*((-d*xma)/( (1.0+d*d*(sqrt_rad - piover3)*(sqrt_rad - piover3)) * (sqrt_rad) ));
    gradu(1,0) = -1.0*((-d*ymb)/( (1.0+d*d*(sqrt_rad - piover3)*(sqrt_rad - piover3)) * (sqrt_rad) ));
    gradu(2,0) = -1.0*((-d*zmc)/( (1.0+d*d*(sqrt_rad - piover3)*(sqrt_rad - piover3)) * (sqrt_rad) ));
    
    gradu(3,0) = -1.0*(numfactro1)/(denomfactor1*denomfactor1*sqrt_rad);
    
#endif
    
}

void Solution(const TPZVec<REAL> &p, TPZVec<STATE> &f){

    REAL x,y,z;
    x = p[0];
    y = p[1];
    z = p[2];
    
    REAL r = sqrt(x*x+y*y+z*z);
    REAL theta = acos(z/r);
    REAL phi = atan2(y,x);
    
#ifdef Solution1
    
    f[0] = r*r;
    
#endif
    
#ifdef Solution6
    
    REAL a = +5.0/4.0;
    REAL b = -1.0/4.0;
    REAL c = -1.0/4.0;
    
    REAL d = 5.0;
    
    REAL xma = x-a;
    REAL ymb = y-b;
    REAL zmc = z-c;
    REAL piover2 = M_PI/2.0;
    REAL piover3 = M_PI/3.0;
    REAL rad = xma*xma + ymb*ymb+ zmc*zmc;
    REAL sqrt_rad = sqrt(rad);
    REAL artan_arg = atan(d*(sqrt_rad - piover3));
    
    
    REAL denomfactor1 = -9.0 + d*d*(-9.0*rad+M_PI*(-M_PI+6.0*sqrt_rad));
    REAL numfactro1   = 18.0*d*(-9.0+d*d*M_PI*(-M_PI+3.0*sqrt_rad));
    
    f[0] = piover2 - artan_arg;
    
#endif
    
    
}

void f(const TPZVec<REAL> &p, TPZVec<STATE> &f, TPZFMatrix<STATE> &gradf){
    REAL x,y,z;
    x = p[0];
    y = p[1];
    z = p[2];
    
    REAL r = sqrt(x*x+y*y+z*z);
    REAL theta = acos(z/r);
    REAL phi = atan2(y,x);
    
#ifdef Solution1
    
    f[0] = -6.0;
    
#endif
    
#ifdef Solution6
    
    REAL a = +5.0/4.0;
    REAL b = -1.0/4.0;
    REAL c = -1.0/4.0;
    
    REAL d = 5.0;
    
    REAL xma = x-a;
    REAL ymb = y-b;
    REAL zmc = z-c;
    REAL piover2 = M_PI/2.0;
    REAL piover3 = M_PI/3.0;
    REAL rad = xma*xma + ymb*ymb+ zmc*zmc;
    REAL sqrt_rad = sqrt(rad);
    REAL artan_arg = atan(d*(sqrt_rad - piover3));
    
    
    REAL denomfactor1 = -9.0 + d*d*(-9.0*rad+M_PI*(-M_PI+6.0*sqrt_rad));
    REAL numfactro1   = 18.0*d*(-9.0+d*d*M_PI*(-M_PI+3.0*sqrt_rad));
    
    f[0] = -1.0*(numfactro1)/(denomfactor1*denomfactor1*sqrt_rad);
    
#endif
    
}

STATE IntegrateVolume(TPZGeoMesh * geometry, SimulationCase sim_data){

    int order = sim_data.int_order;
    int nel = geometry->NElements();
    STATE domain_volume = 0.0;
    
    for(int iel  = 0; iel < nel; iel++)
    {
    
        TPZGeoEl * gel = geometry->Element(iel);

#ifdef PZDEBUG
        if (!gel) {
            DebugStop();
        }
#endif
        if (gel->Dimension() !=3) {
            continue;
        }
        
        if (gel->HasSubElement()){
            continue;
        }
        
        int gel_volume_side = gel->NSides() - 1;
        TPZIntPoints * int_rule = gel->CreateSideIntegrationRule(gel_volume_side, order);
        int npoints = int_rule->NPoints();
        
        TPZManVector<STATE,3> triplet(3,0.0);
        STATE w;
        
        TPZFMatrix<REAL> jac;
        TPZFMatrix<REAL> axes;
        TPZFMatrix<REAL> jacinv;
        REAL detjac;
        
        STATE el_volume = 0.0;
        for (int i = 0; i < npoints ; i++) {
            int_rule->Point(i, triplet, w);
            gel->Jacobian(triplet, jac, axes, detjac, jacinv);
            el_volume += w * detjac;
        }
        
        domain_volume += el_volume;
    }
    return domain_volume;
}

STATE IntegrateSolution(TPZCompMesh * cmesh, SimulationCase sim_data){
    
    int order = sim_data.int_order;
    STATE p_integral = 0.0;
    
    long nel = cmesh->NElements();
    int dim = cmesh->Dimension();
    TPZManVector<STATE,10> globalerror(3,0.   );
    for (long iel = 0; iel < nel; iel++) {
        TPZCompEl *cel = cmesh->ElementVec()[iel];
        
#ifdef PZDEBUG
        if (!cel) {
            DebugStop();
        }
#endif
        
        if(cel->Reference()->Dimension()!=dim) {
            continue;
        }
        
        TPZGeoEl * gel = cel->Reference();
        
#ifdef PZDEBUG
        if (!gel) {
            DebugStop();
        }
#endif
        
        int gel_volume_side = gel->NSides() - 1;
        TPZIntPoints * int_rule = gel->CreateSideIntegrationRule(gel_volume_side, order);
        int npoints = int_rule->NPoints();
        
        TPZManVector<STATE,3> triplet(3,0.0);
        STATE w;
        
        TPZFMatrix<REAL> jac;
        TPZFMatrix<REAL> axes;
        TPZFMatrix<REAL> jacinv;
        REAL detjac;
        TPZManVector<STATE,1> p;
        STATE el_interal = 0.0;
        for (int i = 0; i < npoints ; i++) {
            int_rule->Point(i, triplet, w);
            gel->Jacobian(triplet, jac, axes, detjac, jacinv);
            cel->Solution(triplet, 4, p);
            el_interal += w * detjac * p[0];
        }
        
        p_integral += el_interal;
    }
    
    return p_integral;
    
}

TPZGeoMesh * GeomtricMesh(int ndiv, SimulationCase sim_data){
    
    TPZGeoMesh * geometry = NULL;
    
    if (sim_data.domain_type == "sphere") {
        
        if (sim_data.mesh_type == "linear") {
            geometry = MakeSphereFromLinearQuadrilateralFaces(ndiv, sim_data);
            return geometry;
        }
        
        if (sim_data.mesh_type == "quadratic") {
            geometry = MakeSphereFromQuadrilateralFaces(ndiv, sim_data);
            TransformToQuadratic(geometry);
            return geometry;
        }
        
        if (sim_data.mesh_type == "blended") {
            geometry = MakeSphereFromQuadrilateralFaces(ndiv, sim_data);
            return geometry;
        }
        
        std::cout << "Error:: unable to compute geometry for the given case = " << &sim_data << std::endl;
        DebugStop();
        
    }
    
    std::cout << "Error:: unable to compute geometry for the given case = " << &sim_data << std::endl;
    DebugStop();
    return geometry;
}

TPZCompMesh * ComputationalMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data, long &ndof, TPZVec<TPZCompMesh *> &meshvec){
    
    TPZCompMesh * mesh = NULL;
    
    if (sim_data.IsHdivQ) {
        mesh = DualMesh(geometry, p, sim_data, meshvec);
        ndof = mesh->NEquations();
        return mesh;
    }
    else
    {
        mesh = PrimalMesh(geometry, p, sim_data);
        ndof = mesh->NEquations();
        return mesh;        
    }
    
    std::cout << "Error:: unable to compute the given case = " << &sim_data << std::endl;
    DebugStop();
    return mesh;
}

TPZAnalysis * CreateAnalysis(TPZCompMesh * cmesh, SimulationCase sim_data){
    
    TPZAnalysis * analysis = new TPZAnalysis(cmesh, true);
    
    if (sim_data.UsePardisoQ) {
        
        TPZSymetricSpStructMatrix matrix(cmesh);
        matrix.SetNumThreads(sim_data.n_threads);
        analysis->SetStructuralMatrix(matrix);
        TPZStepSolver<STATE> step;
        step.SetDirect(ELDLt);
        analysis->SetSolver(step);
        
        return analysis;
    }
    
    if (sim_data.UseFrontalQ) {

        TPZParFrontStructMatrix<TPZFrontSym<STATE> > matrix(cmesh);
        matrix.SetDecomposeType(ELDLt);
        matrix.SetNumThreads(sim_data.n_threads);
        
        analysis->SetStructuralMatrix(matrix);
        TPZStepSolver<STATE> step;
        step.SetDirect(ELDLt);
        analysis->SetSolver(step);
        
        return analysis;
    }
    else{
        
        TPZSkylineStructMatrix matrix(cmesh);
        matrix.SetNumThreads(sim_data.n_threads);        
        TPZStepSolver<STATE> step;
        step.SetDirect(ECholesky);
        analysis->SetSolver(step);
        analysis->SetStructuralMatrix(matrix);
        return analysis;
    }
    

    
   return analysis;
    
}

void PosProcess(TPZCompMesh* cmesh, TPZAnalysis  * an, std::string file, SimulationCase sim_data)
{
    int dim = 3;
//    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(meshvec, cmesh);
    TPZStack<std::string,10> scalnames, vecnames;
    
    if (sim_data.IsHdivQ) {
        vecnames.Push("q");
        vecnames.Push("q_exact");
        scalnames.Push("p");
        scalnames.Push("p_exact");
        scalnames.Push("f_exact");
        scalnames.Push("div_q");
    }
    else{
        vecnames.Push("q");
        vecnames.Push("q_exact");
        scalnames.Push("p");
        scalnames.Push("p_exact");
        scalnames.Push("f_exact");
    }

    int div = 0;
    an->DefineGraphMesh(dim,scalnames,vecnames,file);
    an->PostProcess(div,dim);
    
}

TPZCompMesh * PrimalMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data){
    
    int dimension = 3;
    int dirichlet = 0;
    int nvolumes = sim_data.omega_ids.size();
    int nboundaries = sim_data.gamma_ids.size();
    
#ifdef PZDEBUG
    if (nvolumes != 1) {
    std::cout << "Error:: unable to compute the given case = " << &sim_data << std::endl;
        DebugStop();
    }
#endif
    
    TPZCompMesh *cmesh = new TPZCompMesh(geometry);
        
    TPZFMatrix<STATE> val1(dimension,dimension,0.0),val2(dimension,1,0.0);
    for (int iv = 0; iv < nvolumes ; iv++) {
        
        TPZMaterial * volume = new TPZPrimalPoisson(sim_data.omega_ids[iv]);
        
        TPZDummyFunction<STATE> * rhs_exact = new TPZDummyFunction<STATE>(f);
        rhs_exact->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > rhs = rhs_exact;
        volume->SetForcingFunction(rhs);
        
        
        TPZDummyFunction<STATE> * analytic = new TPZDummyFunction<STATE>(Analytic);
        analytic->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > analytic_full = analytic;
        volume->SetForcingFunctionExact(analytic_full);
        
        cmesh->InsertMaterialObject(volume);
        
        for (int ib = 0; ib < nboundaries; ib++) {
            TPZDummyFunction<STATE> * analytic_bc = new TPZDummyFunction<STATE>(Solution);
            analytic_bc->SetPolynomialOrder(sim_data.int_order);
            TPZAutoPointer< TPZFunction<STATE> > solution = analytic_bc;
            
            TPZMaterial * face = volume->CreateBC(volume,sim_data.gamma_ids[ib],dirichlet,val1,val2);
            face->SetfBCForcingFunction(solution);
            cmesh->InsertMaterialObject(face);
        }
        
    }
    cmesh->SetDimModel(dimension);
    cmesh->SetDefaultOrder(p);
    cmesh->SetAllCreateFunctionsContinuous();
    
    cmesh->AutoBuild();
    cmesh->ExpandSolution();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
#ifdef PZDEBUG
    std::stringstream file_name;
    file_name   << sim_data.dump_folder << "/" << "Primal_cmesh" << ".txt";
    std::ofstream sout(file_name.str());
    cmesh->Print(sout);
#endif
    
    return cmesh;
    
}


TPZCompMesh *DualMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data, TPZVec<TPZCompMesh *> &meshvec)
{
    
    int dimension = 3;
    int dirichlet = 0;
    int nvolumes = sim_data.omega_ids.size();
    int nboundaries = sim_data.gamma_ids.size();
    
#ifdef PZDEBUG
    if (nvolumes != 1) {
        std::cout << "Error:: unable to compute the given case = " << &sim_data << std::endl;
        DebugStop();
    }
#endif
    
    TPZCompMesh *cmesh = new TPZCompMesh(geometry);
    
    
    TPZFMatrix<STATE> val1(dimension,dimension,0.0),val2(dimension,1,0.0);
    for (int iv = 0; iv < nvolumes ; iv++) {
        
        TPZMaterial * volume = new TPZDualPoisson(sim_data.omega_ids[iv]);
        
        TPZDummyFunction<STATE> * rhs_exact = new TPZDummyFunction<STATE>(f);
        rhs_exact->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > rhs = rhs_exact;
        volume->SetForcingFunction(rhs);
        
        
        TPZDummyFunction<STATE> * analytic = new TPZDummyFunction<STATE>(Analytic);
        analytic->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > analytic_full = analytic;
        volume->SetForcingFunctionExact(analytic_full);
        
        cmesh->InsertMaterialObject(volume);
        
        for (int ib = 0; ib < nboundaries; ib++) {
            
            TPZDummyFunction<STATE> * analytic_bc = new TPZDummyFunction<STATE>(Solution);
            analytic_bc->SetPolynomialOrder(sim_data.int_order);
            TPZAutoPointer< TPZFunction<STATE> > solution = analytic_bc;
            
            TPZMaterial * face = volume->CreateBC(volume,sim_data.gamma_ids[ib],dirichlet,val1,val2);
            face->SetfBCForcingFunction(solution);
            cmesh->InsertMaterialObject(face);
        }
        
    }
    cmesh->SetDimModel(dimension);
    cmesh->SetDefaultOrder(p);
    cmesh->SetAllCreateFunctionsMultiphysicElem();
    
    cmesh->AutoBuild();
    cmesh->ExpandSolution();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    
    TPZManVector<TPZCompMesh * ,2> meshvector(2);
    meshvector[0] = qMesh(geometry, p, sim_data);
    meshvector[1] = pMesh(geometry, p, sim_data);
    
//    AdjustFluxPolynomialOrders(meshvector[0], sim_data.n_acc_terms);
//    SetPressureOrders(meshvector[0], meshvector[1]);
    
    
    // Transferindo para a multifisica
    TPZBuildMultiphysicsMesh::AddElements(meshvector, cmesh);
    TPZBuildMultiphysicsMesh::AddConnects(meshvector, cmesh);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvector, cmesh);
    
//    TPZCompMeshTools::GroupElements(cmesh);
//    TPZCompMeshTools::CreatedCondensedElements(cmesh, true);

    
#ifdef PZDEBUG
    std::stringstream file_name;
    file_name   << sim_data.dump_folder << "/" << "Dual_cmesh" << ".txt";
    std::ofstream sout(file_name.str());
    cmesh->Print(sout);
#endif
    
    meshvec = meshvector;
    
    return cmesh;
    

}

/// adjust the polynomial orders of the hdiv elements such that the internal order is higher than the sideorders
void AdjustFluxPolynomialOrders(TPZCompMesh *fluxmesh, int hdivplusplus)
{
    int dim = fluxmesh->Dimension();
    /// loop over all the elements
    long nel = fluxmesh->NElements();
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = fluxmesh->Element(el);
        TPZInterpolatedElement *intel = dynamic_cast<TPZInterpolatedElement *>(cel);
        if (!intel) {
            continue;
        }
        TPZGeoEl *gel = intel->Reference();
        if (gel->Dimension() != dim) {
            continue;
        }
        // compute the maxorder
        int maxorder = -1;
        int ncon = intel->NConnects();
        for (int i=0; i<ncon-1; i++) {
            int conorder = intel->Connect(i).Order();
            maxorder = maxorder < conorder ? conorder : maxorder;
        }
        int nsides = gel->NSides();
        int nconside = intel->NSideConnects(nsides-1);
        // tive que tirar para rodar H1
        //        if (nconside != 1 || maxorder == -1) {
        //            DebugStop();
        //        }
        long cindex = intel->SideConnectIndex(nconside-1, nsides-1);
        TPZConnect &c = fluxmesh->ConnectVec()[cindex];
        if (c.NElConnected() != 1) {
            DebugStop();
        }
        if (c.Order()+hdivplusplus != maxorder) {
            //            std::cout << "Changing the order of the central connect " << cindex << " from " << c.Order() << " to " << maxorder+hdivplusplus << std::endl;
            // change the internal connect order to be equal do maxorder
            intel->SetSideOrder(nsides-1, maxorder+hdivplusplus);
        }
    }
    fluxmesh->ExpandSolution();
}

void SetPressureOrders(TPZCompMesh *fluxmesh, TPZCompMesh *pressuremesh)
{
    // build a vector with the required order of each element in the pressuremesh
    // if an element of the mesh dimension of the fluxmesh does not have a corresponding element in the pressuremesh DebugStop is called
    int meshdim = fluxmesh->Dimension();
    pressuremesh->Reference()->ResetReference();
    pressuremesh->LoadReferences();
    TPZManVector<long> pressorder(pressuremesh->NElements(),-1);
    long nel = fluxmesh->NElements();
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = fluxmesh->Element(el);
        TPZInterpolatedElement *intel = dynamic_cast<TPZInterpolatedElement *>(cel);
        if (!intel) {
            continue;
        }
        TPZGeoEl *gel = intel->Reference();
        if (gel->Dimension() != meshdim) {
            continue;
        }
        int nsides = gel->NSides();
        long cindex = intel->SideConnectIndex(0, nsides-1);
        TPZConnect &c = fluxmesh->ConnectVec()[cindex];
        int order = c.Order();
        TPZCompEl *pressureel = gel->Reference();
        TPZInterpolatedElement *pintel = dynamic_cast<TPZInterpolatedElement *>(pressureel);
        if (!pintel) {
            DebugStop();
        }
        pressorder[pintel->Index()] = order;
    }
    pressuremesh->Reference()->ResetReference();
    nel = pressorder.size();
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = pressuremesh->Element(el);
        TPZInterpolatedElement *pintel = dynamic_cast<TPZInterpolatedElement *>(cel);
        if (!pintel) {
            continue;
        }
        if (pressorder[el] == -1) {
            continue;
        }
        pintel->PRefine(pressorder[el]);
    }
    
    pressuremesh->ExpandSolution();
}

TPZCompMesh * qMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data){
    
    int dimension = 3;
    int dirichlet = 0;
    int nvolumes = sim_data.omega_ids.size();
    int nboundaries = sim_data.gamma_ids.size();
    
#ifdef PZDEBUG
    if (nvolumes != 1) {
        std::cout << "Error:: unable to compute the given case = " << &sim_data << std::endl;
        DebugStop();
    }
#endif
    
    TPZCompMesh *cmesh = new TPZCompMesh(geometry);
    
    TPZMaterial * volume;
    TPZMaterial * face;
    
    TPZFMatrix<STATE> val1(dimension,dimension,0.0),val2(dimension,1,0.0);
    for (int iv = 0; iv < nvolumes ; iv++) {
        
        volume = new TPZDualPoisson(sim_data.omega_ids[iv]);
        
        TPZDummyFunction<STATE> * rhs_exact = new TPZDummyFunction<STATE>(f);
        rhs_exact->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > rhs = rhs_exact;
        volume->SetForcingFunction(rhs);
        
        TPZDummyFunction<STATE> * analytic_bc = new TPZDummyFunction<STATE>(Solution);
        analytic_bc->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > solution = analytic_bc;
        volume->SetfBCForcingFunction(solution);
        
        
        TPZDummyFunction<STATE> * analytic = new TPZDummyFunction<STATE>(Analytic);
        analytic->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > analytic_full = analytic;
        volume->SetForcingFunctionExact(analytic_full);
        
        cmesh->InsertMaterialObject(volume);
        
        for (int ib = 0; ib < nboundaries; ib++) {
            face = volume->CreateBC(volume,sim_data.gamma_ids[ib],dirichlet,val1,val2);
            cmesh->InsertMaterialObject(face);
        }
        
    }
    cmesh->SetDimModel(dimension);
    cmesh->SetDefaultOrder(p);
    cmesh->SetAllCreateFunctionsHDiv();
    
    cmesh->AutoBuild();
    cmesh->ExpandSolution();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
#ifdef PZDEBUG
    std::stringstream file_name;
    file_name   << sim_data.dump_folder << "/" << "q_cmesh" << ".txt";
    std::ofstream sout(file_name.str());
    cmesh->Print(sout);
#endif
    
    return cmesh;
    
}
TPZCompMesh * pMesh(TPZGeoMesh * geometry, int p, SimulationCase sim_data){
    
    int dimension = 3;
    int nvolumes = sim_data.omega_ids.size();
    
#ifdef PZDEBUG
    if (nvolumes != 1) {
        std::cout << "Error:: unable to compute the given case = " << &sim_data << std::endl;
        DebugStop();
    }
#endif
    
    TPZCompMesh *cmesh = new TPZCompMesh(geometry);
    
    TPZMaterial * volume;
    
    TPZFMatrix<STATE> val1(dimension,dimension,0.0),val2(dimension,1,0.0);
    for (int iv = 0; iv < nvolumes ; iv++) {
        
        volume = new TPZMatPoisson3d(sim_data.omega_ids[iv]);
        
        TPZDummyFunction<STATE> * rhs_exact = new TPZDummyFunction<STATE>(f);
        rhs_exact->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > rhs = rhs_exact;
        volume->SetForcingFunction(rhs);
        
        TPZDummyFunction<STATE> * analytic_bc = new TPZDummyFunction<STATE>(Solution);
        analytic_bc->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > solution = analytic_bc;
        volume->SetfBCForcingFunction(solution);
        
        
        TPZDummyFunction<STATE> * analytic = new TPZDummyFunction<STATE>(Analytic);
        analytic->SetPolynomialOrder(sim_data.int_order);
        TPZAutoPointer<TPZFunction<STATE> > analytic_full = analytic;
        volume->SetForcingFunctionExact(analytic_full);
        
        cmesh->InsertMaterialObject(volume);
        
    }
    cmesh->SetDimModel(dimension);
    cmesh->SetDefaultOrder(p);
    cmesh->SetAllCreateFunctionsContinuous();
    cmesh->ApproxSpace().CreateDisconnectedElements(true);
    
    cmesh->AutoBuild();
    cmesh->ExpandSolution();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        newnod.SetLagrangeMultiplier(1);
    }
    
#ifdef PZDEBUG
    std::stringstream file_name;
    file_name   << sim_data.dump_folder << "/" << "p_cmesh" << ".txt";
    std::ofstream sout(file_name.str());
    cmesh->Print(sout);
#endif
    
    return cmesh;
    
}

TPZGeoMesh * MakeSphereFromLinearQuadrilateralFaces(int ndiv, SimulationCase sim_data){
    
#ifdef PZDEBUG
    if (sim_data.omega_ids.size() != 1 || sim_data.gamma_ids.size() != 2) {
        std::cout << "Sphere:: Please pass materials ids for volume (1) and boundary domains = { (-1 -> inner surface ),  (-2 -> outer surface) }" << std::endl;
        DebugStop();
    }
#endif
    
    TPZGeoMesh * geomesh = new TPZGeoMesh;
    geomesh->SetDimension(3);
    int nl = 2;// Let it fixed
    int basenodes = 8;
    int nodes =  basenodes * (nl);
    REAL radius_o = 1.0;
    REAL radius_i = 0.25;
    
    REAL dr = (radius_o- radius_i)/REAL(nl-1);
    geomesh->SetMaxNodeId(nodes-1);
    geomesh->NodeVec().Resize(nodes);
    TPZManVector<TPZGeoNode,4> Node(nodes);
    
    TPZManVector<long,4> TopolQuad(4);
    TPZManVector<long,8> TopolCube(8);
    TPZManVector<REAL,3> coord(3,0.);
    TPZVec<REAL> xc(3,0.);
    REAL cphi = atan(sqrt(2.0));
    
    int nodeindex = 0;
    long id = 0;
    int matid = sim_data.omega_ids[0];
    
    TPZManVector< TPZManVector<REAL,3> , 8 > points(nodes,0.);
    for (int il = 0; il < nl; il++) {
        
        if (il==0) {
            matid = sim_data.gamma_ids[0];
        }
        
        if (il==nl-1) {
            matid = sim_data.gamma_ids[1];
        }
        
        REAL radius = radius_o - REAL(il)*dr;
        points[0].Resize(3, 0.0);
        points[0][0]=radius;
        points[0][1]=cphi;
        points[0][2]=M_PI/4.0;
        
        points[1].Resize(3, 0.0);
        points[1][0]=radius;
        points[1][1]=cphi;
        points[1][2]=-M_PI/4.0;
        
        points[2].Resize(3, 0.0);
        points[2][0]=radius;
        points[2][1]=cphi;
        points[2][2]=-3.0*M_PI/4.0;
        
        points[3].Resize(3, 0.0);
        points[3][0]=radius;
        points[3][1]=cphi;
        points[3][2]=3.0*M_PI/4.0;
        
        points[4].Resize(3, 0.0);
        points[4][0]=radius;
        points[4][1]=M_PI-cphi;
        points[4][2]=M_PI/4.0;
        
        points[5].Resize(3, 0.0);
        points[5][0]=radius;
        points[5][1]=M_PI-cphi;
        points[5][2]=-M_PI/4.0;
        
        points[6].Resize(3, 0.0);
        points[6][0]=radius;
        points[6][1]=M_PI-cphi;
        points[6][2]=-3.0*M_PI/4.0;
        
        points[7].Resize(3, 0.0);
        points[7][0]=radius;
        points[7][1]=M_PI-cphi;
        points[7][2]=3.0*M_PI/4.0;
        
        for (int i = 0; i < basenodes; i++) {
            coord = ParametricSphere(points[i][0],points[i][1],points[i][2]);
            geomesh->NodeVec()[nodeindex].SetCoord(coord);
            geomesh->NodeVec()[nodeindex].SetNodeId(nodeindex);
            nodeindex++;
        }
        
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 1+il*basenodes;
        TopolQuad[2] = 2+il*basenodes;
        TopolQuad[3] = 3+il*basenodes;
        new TPZGeoElRefPattern<  pzgeom::TPZGeoQuad  > (id,TopolQuad,matid,*geomesh);
        id++;
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 3+il*basenodes;
        TopolQuad[2] = 7+il*basenodes;
        TopolQuad[3] = 4+il*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (id,TopolQuad,matid,*geomesh);
        id++;
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 1+il*basenodes;
        TopolQuad[2] = 5+il*basenodes;
        TopolQuad[3] = 4+il*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (id,TopolQuad,matid,*geomesh);
        id++;
        
        TopolQuad[0] = 3+il*basenodes;
        TopolQuad[1] = 2+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 7+il*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad  > (id,TopolQuad,matid,*geomesh);
        id++;
        
        TopolQuad[0] = 1+il*basenodes;
        TopolQuad[1] = 2+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 5+il*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (id,TopolQuad,matid,*geomesh);
        id++;
        
        TopolQuad[0] = 4+il*basenodes;
        TopolQuad[1] = 5+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 7+il*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (id,TopolQuad,matid,*geomesh);
        id++;
    }
    
    matid = sim_data.omega_ids[0];
    
    for (int il = 0; il < nl - 1 ; il++) {
        //      Inserting blend elements
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 1+il*basenodes;
        TopolCube[2] = 2+il*basenodes;
        TopolCube[3] = 3+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 1+(il+1)*basenodes;
        TopolCube[6] = 2+(il+1)*basenodes;
        TopolCube[7] = 3+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 3+il*basenodes;
        TopolCube[2] = 7+il*basenodes;
        TopolCube[3] = 4+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 3+(il+1)*basenodes;
        TopolCube[6] = 7+(il+1)*basenodes;
        TopolCube[7] = 4+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 1+il*basenodes;
        TopolCube[2] = 5+il*basenodes;
        TopolCube[3] = 4+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 1+(il+1)*basenodes;
        TopolCube[6] = 5+(il+1)*basenodes;
        TopolCube[7] = 4+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
        TopolCube[0] = 3+il*basenodes;
        TopolCube[1] = 2+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 7+il*basenodes;
        TopolCube[4] = 3+(il+1)*basenodes;
        TopolCube[5] = 2+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 7+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
        TopolCube[0] = 1+il*basenodes;
        TopolCube[1] = 2+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 5+il*basenodes;
        TopolCube[4] = 1+(il+1)*basenodes;
        TopolCube[5] = 2+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 5+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
        TopolCube[0] = 4+il*basenodes;
        TopolCube[1] = 5+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 7+il*basenodes;
        TopolCube[4] = 4+(il+1)*basenodes;
        TopolCube[5] = 5+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 7+(il+1)*basenodes;
        new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (id,TopolCube,matid,*geomesh);
        id++;
        
    }
    
    geomesh->BuildConnectivity();
    
    TPZVec<TPZGeoEl *> sons;
    const int nref = ndiv;
    for (int iref = 0; iref < nref; iref++) {
        int nel = geomesh->NElements();
        for (int iel = 0; iel < nel; iel++) {
            TPZGeoEl *gel = geomesh->ElementVec()[iel];
            if(!gel->HasSubElement())
            {
                gel->Divide(sons);
            }
        }
    }
    
    int axis = 1;
    REAL angle = 0.0;//-45.0;
    RotateGeomesh(geomesh, angle, axis);
    return geomesh;
}

TPZGeoMesh * MakeSphereFromQuadrilateralFaces(int ndiv, SimulationCase sim_data)
{
    
#ifdef PZDEBUG
    if (sim_data.omega_ids.size() != 1 || sim_data.gamma_ids.size() != 2) {
        std::cout << "Sphere:: Please pass materials ids for volume (1) and boundary domains = { (-1 -> inner surface ),  (-2 -> outer surface) }" << std::endl;
        DebugStop();
    }
#endif
    
    TPZGeoMesh * geomesh = new TPZGeoMesh;
    geomesh->SetDimension(3);
    int nl = 2;// Let it fixed
    int basenodes = 8;
    int nodes =  basenodes * (nl);
    REAL radius_o = 1.0;
    REAL radius_i = 0.25;
    
    REAL dr = (radius_o - radius_i)/REAL(nl-1);
    geomesh->SetMaxNodeId(nodes-1);
    geomesh->NodeVec().Resize(nodes);
    TPZManVector<TPZGeoNode,4> Node(nodes);
    
    TPZManVector<long,4> TopolQuad(4);
    TPZManVector<long,8> TopolCube(8);
    TPZManVector<REAL,3> coord(3,0.);
    TPZVec<REAL> xc(3,0.);
    REAL cphi = atan(sqrt(2.0));
    
    int nodeindex = 0;
    long id = 0;
    int matid = sim_data.omega_ids[0];
    
    TPZManVector< TPZManVector<REAL,3> , 8 > points(nodes,0.);
    for (int il = 0; il < nl; il++) {
        
        if (il==0) {
            matid = sim_data.gamma_ids[0];
        }
        
        if (il==nl-1) {
            matid = sim_data.gamma_ids[1];
        }
        
        REAL radius = radius_o - REAL(il)*dr;
        points[0].Resize(3, 0.0);
        points[0][0]=radius;
        points[0][1]=cphi;
        points[0][2]=M_PI/4.0;
        
        points[1].Resize(3, 0.0);
        points[1][0]=radius;
        points[1][1]=cphi;
        points[1][2]=-M_PI/4.0;
        
        points[2].Resize(3, 0.0);
        points[2][0]=radius;
        points[2][1]=cphi;
        points[2][2]=-3.0*M_PI/4.0;
        
        points[3].Resize(3, 0.0);
        points[3][0]=radius;
        points[3][1]=cphi;
        points[3][2]=3.0*M_PI/4.0;
        
        points[4].Resize(3, 0.0);
        points[4][0]=radius;
        points[4][1]=M_PI-cphi;
        points[4][2]=M_PI/4.0;
        
        points[5].Resize(3, 0.0);
        points[5][0]=radius;
        points[5][1]=M_PI-cphi;
        points[5][2]=-M_PI/4.0;
        
        points[6].Resize(3, 0.0);
        points[6][0]=radius;
        points[6][1]=M_PI-cphi;
        points[6][2]=-3.0*M_PI/4.0;
        
        points[7].Resize(3, 0.0);
        points[7][0]=radius;
        points[7][1]=M_PI-cphi;
        points[7][2]=3.0*M_PI/4.0;
        
        
        for (int i = 0; i < basenodes; i++) {
            coord = ParametricSphere(points[i][0],points[i][1],points[i][2]);
            geomesh->NodeVec()[nodeindex].SetCoord(coord);
            geomesh->NodeVec()[nodeindex].SetNodeId(nodeindex);
            nodeindex++;
        }
        
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 1+il*basenodes;
        TopolQuad[2] = 2+il*basenodes;
        TopolQuad[3] = 3+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad1 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad1->Geom().SetData(radius, xc);
        id++;
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 3+il*basenodes;
        TopolQuad[2] = 7+il*basenodes;
        TopolQuad[3] = 4+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad2 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad2->Geom().SetData(radius, xc);
        id++;
        
        TopolQuad[0] = 0+il*basenodes;
        TopolQuad[1] = 1+il*basenodes;
        TopolQuad[2] = 5+il*basenodes;
        TopolQuad[3] = 4+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad3 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad3->Geom().SetData(radius, xc);
        id++;
        
        TopolQuad[0] = 3+il*basenodes;
        TopolQuad[1] = 2+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 7+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad4 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad4->Geom().SetData(radius, xc);
        id++;
        
        TopolQuad[0] = 1+il*basenodes;
        TopolQuad[1] = 2+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 5+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad5 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad5->Geom().SetData(radius, xc);
        id++;
        
        TopolQuad[0] = 4+il*basenodes;
        TopolQuad[1] = 5+il*basenodes;
        TopolQuad[2] = 6+il*basenodes;
        TopolQuad[3] = 7+il*basenodes;
        TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > * quad6 = new TPZGeoElRefPattern< pzgeom::TPZQuadSphere< pzgeom::TPZGeoQuad > > (id,TopolQuad,matid,*geomesh);
        quad6->Geom().SetData(radius, xc);
        id++;
    }
    
    matid = sim_data.omega_ids[0];
    
    for (int il = 0; il < nl - 1 ; il++) {
        //      Inserting blend elements
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 1+il*basenodes;
        TopolCube[2] = 2+il*basenodes;
        TopolCube[3] = 3+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 1+(il+1)*basenodes;
        TopolCube[6] = 2+(il+1)*basenodes;
        TopolCube[7] = 3+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 3+il*basenodes;
        TopolCube[2] = 7+il*basenodes;
        TopolCube[3] = 4+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 3+(il+1)*basenodes;
        TopolCube[6] = 7+(il+1)*basenodes;
        TopolCube[7] = 4+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
        TopolCube[0] = 0+il*basenodes;
        TopolCube[1] = 1+il*basenodes;
        TopolCube[2] = 5+il*basenodes;
        TopolCube[3] = 4+il*basenodes;
        TopolCube[4] = 0+(il+1)*basenodes;
        TopolCube[5] = 1+(il+1)*basenodes;
        TopolCube[6] = 5+(il+1)*basenodes;
        TopolCube[7] = 4+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
        TopolCube[0] = 3+il*basenodes;
        TopolCube[1] = 2+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 7+il*basenodes;
        TopolCube[4] = 3+(il+1)*basenodes;
        TopolCube[5] = 2+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 7+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
        TopolCube[0] = 1+il*basenodes;
        TopolCube[1] = 2+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 5+il*basenodes;
        TopolCube[4] = 1+(il+1)*basenodes;
        TopolCube[5] = 2+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 5+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
        TopolCube[0] = 4+il*basenodes;
        TopolCube[1] = 5+il*basenodes;
        TopolCube[2] = 6+il*basenodes;
        TopolCube[3] = 7+il*basenodes;
        TopolCube[4] = 4+(il+1)*basenodes;
        TopolCube[5] = 5+(il+1)*basenodes;
        TopolCube[6] = 6+(il+1)*basenodes;
        TopolCube[7] = 7+(il+1)*basenodes;
        geomesh->CreateGeoBlendElement(ECube, TopolCube, matid, id);
        id++;
        
    }
    
    geomesh->BuildConnectivity();
    
    TPZVec<TPZGeoEl *> sons;
    const int nref = ndiv;
    for (int iref = 0; iref < nref; iref++) {
        int nel = geomesh->NElements();
        for (int iel = 0; iel < nel; iel++) {
            TPZGeoEl *gel = geomesh->ElementVec()[iel];
            if(!gel->HasSubElement())
            {
                gel->Divide(sons);
            }
        }
    }
    
    int axis = 1;
    REAL angle = 0.0;//-45.0;
    RotateGeomesh(geomesh, angle, axis);

    return geomesh;
}

void TransformToQuadratic(TPZGeoMesh *gmesh)
{
    long nel = gmesh->NElements();
    for (long el=0; el<nel; el++)
    {
        TPZGeoEl *gel = gmesh->Element(el);
        if (!gel || gel->HasSubElement()) {
            continue;
        }
        TPZGeoEl *father = gel->Father();
        int whichsubel = gel->WhichSubel();
        gel = TPZChangeEl::ChangeToQuadratic(gmesh, el);
        if (whichsubel != -1) {
            father->SetSubElement(whichsubel, gel);
        }
    }
}


TPZManVector<STATE,3> ParametricSphere(REAL radius, REAL theta, REAL phi)
{
    TPZManVector<STATE,3> xcoor(3,0.0);
    xcoor[0] = radius * sin(theta) * cos(phi) ;
    xcoor[1] = radius * sin(theta) * sin(phi) ;
    xcoor[2] = radius * cos(theta) ;
    return xcoor;
}

void RotateGeomesh(TPZGeoMesh *gmesh, REAL CounterClockwiseAngle, int &Axis)
{
    REAL theta =  (M_PI/180.0)*CounterClockwiseAngle;
    // It represents a 3D rotation around the axis -> i.
    TPZFMatrix<STATE> RotationMatrix(3,3,0.0);
    
    switch (Axis) {
        case 1:
        {
            RotationMatrix(0,0) = 1.0;
            RotationMatrix(1,1) =   +cos(theta);
            RotationMatrix(1,2) =   -sin(theta);
            RotationMatrix(2,1) =   +sin(theta);
            RotationMatrix(2,2) =   +cos(theta);
        }
            break;
        case 2:
        {
            RotationMatrix(0,0) =   +cos(theta);
            RotationMatrix(0,2) =   +sin(theta);
            RotationMatrix(1,1) = 1.0;
            RotationMatrix(2,0) =   -sin(theta);
            RotationMatrix(2,2) =   +cos(theta);
        }
            break;
        case 3:
        {
            RotationMatrix(0,0) =   +cos(theta);
            RotationMatrix(0,1) =   -sin(theta);
            RotationMatrix(1,0) =   +sin(theta);
            RotationMatrix(1,1) =   +cos(theta);
            RotationMatrix(2,2) = 1.0;
        }
            break;
        default:
        {
            RotationMatrix(0,0) =   +cos(theta);
            RotationMatrix(0,1) =   -sin(theta);
            RotationMatrix(1,0) =   +sin(theta);
            RotationMatrix(1,1) =   +cos(theta);
            RotationMatrix(2,2) = 1.0;
        }
            break;
    }
    
    TPZVec<STATE> iCoords(3,0.0);
    TPZVec<STATE> iCoordsRotated(3,0.0);
    
    //RotationMatrix.Print("Rotation = ");
    
    int NumberofGeoNodes = gmesh->NNodes();
    for (int inode = 0; inode < NumberofGeoNodes; inode++)
    {
        TPZGeoNode GeoNode = gmesh->NodeVec()[inode];
        GeoNode.GetCoordinates(iCoords);
        // Apply rotation
        iCoordsRotated[0] = RotationMatrix(0,0)*iCoords[0]+RotationMatrix(0,1)*iCoords[1]+RotationMatrix(0,2)*iCoords[2];
        iCoordsRotated[1] = RotationMatrix(1,0)*iCoords[0]+RotationMatrix(1,1)*iCoords[1]+RotationMatrix(1,2)*iCoords[2];
        iCoordsRotated[2] = RotationMatrix(2,0)*iCoords[0]+RotationMatrix(2,1)*iCoords[1]+RotationMatrix(2,2)*iCoords[2];
        GeoNode.SetCoord(iCoordsRotated);
        gmesh->NodeVec()[inode] = GeoNode;
    }
    
}

void ErrorH1(TPZCompMesh *cmesh, REAL &error_primal , REAL & error_dual, REAL & error_h1)
{
    
    long nel = cmesh->NElements();
    int dim = cmesh->Dimension();
    TPZManVector<STATE,10> globalerror(3,0.   );
    for (long iel = 0; iel < nel; iel++) {
        TPZCompEl *cel = cmesh->ElementVec()[iel];

        if(cel->Reference()->Dimension()!=dim) {
            continue;
        }
        
        TPZManVector<STATE,10> elerror(3,0.);
        elerror.Fill(0.);
        cel->EvaluateError(Analytic, elerror, NULL);
        int nerr = elerror.size();
        for (int i=0; i<nerr; i++) {
            globalerror[i] += elerror[i]*elerror[i];
            
        }
    }
    
    error_primal    = sqrt(globalerror[0]);
    error_dual      = sqrt(globalerror[1]);
    error_h1        = sqrt(globalerror[2]);
    
    
}

void ErrorHdiv(TPZCompMesh *cmesh, REAL &error_primal , REAL & error_dual, REAL & error_hdiv){
    
    long nel = cmesh->NElements();
    int dim = cmesh->Dimension();
    TPZManVector<STATE,10> globalerror(3,0.   );
    for (long iel = 0; iel < nel; iel++) {
        TPZCompEl *cel = cmesh->ElementVec()[iel];
        
        if(cel->Reference()->Dimension()!=dim) {
            continue;
        }
        
        TPZManVector<STATE,10> elerror(3,0.);
        elerror.Fill(0.);
        cel->EvaluateError(Analytic, elerror, NULL);
        int nerr = elerror.size();
        for (int i=0; i<nerr; i++) {
            globalerror[i] += elerror[i]*elerror[i];
            
        }
    }
    
    error_primal    = sqrt(globalerror[0]);
    error_dual      = sqrt(globalerror[1]);
    error_hdiv      = sqrt(globalerror[1] + globalerror[2]);
    
}
