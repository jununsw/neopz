#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pzvec.h"
#include "pzstack.h"
#include "pzfmatrix.h"
#include "pzfstrmatrix.h"
#include "pzlog.h"

#include "pzgmesh.h"
#include "pzcmesh.h"
#include "pzcompel.h"
#include "TPZInterfaceEl.h"
#include "pzgeoelside.h"
#include "TPZGeoLinear.h"
#include "pzgeopoint.h"

#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"
#include "tpzcompmeshreferred.h"
#include "tpzautopointer.h"
#include "pzbndcond.h"
#include "pzanalysis.h"

#include "TPZParSkylineStructMatrix.h"
#include "pzstepsolver.h"
#include "pzstrmatrix.h"
#include "pzfstrmatrix.h"
#include "TPZFrontNonSym.h"
#include "TPZFrontSym.h"
#include "TPBSpStructMatrix.h"
#include "TPZSpStructMatrix.h"
#include "pzbstrmatrix.h"

#include "pzpoisson3d.h"
//#include "pzhybridpoisson.h"
#include "pzpoisson3dreferred.h"
#include "mixedpoisson.h"
#include "pzelasmat.h"
#include "pzelasthybrid.h"
#include "pzmat1dlin.h"
#include "TPZVecL2.h"
#include "TPZMatLaplacianLagrange.h"

#include "pzbuildmultiphysicsmesh.h"
#include "pzelementgroup.h"
#include "TPZCompMeshTools.h"
#include "pzcondensedcompel.h"
#include "pzfunction.h"
#include "pzgraphmesh.h"
#include "pzfmatrix.h"

#include "pzlog.h"

#include "TPZVTKGeoMesh.h"
#include "pzgengrid.h"
#include "TPZExtendGridDimension.h"
#include "pzcheckgeom.h"
#include "tpzhierarquicalgrid.h"
#include "TPZMHMeshControl.h"

#include <iostream>
#include <string>

#include <math.h>
#include <set>



using namespace std;

TPZGeoMesh *MalhaGeom2(REAL Lx, REAL Ly);
TPZGeoMesh * CreateGeometricBoxMesh(int nref, TPZManVector<REAL,2> dx, TPZManVector<REAL,2> dy, TPZManVector<REAL,2> dz);
TPZGeoMesh * CreateGeometricBoxMesh2D(int nref, TPZManVector<REAL,2> dx, TPZManVector<REAL,2> dy);
void ParametricfunctionX(const TPZVec<STATE> &par, TPZVec<STATE> &X);
void ParametricfunctionY(const TPZVec<STATE> &par, TPZVec<STATE> &X);
void ParametricfunctionZ(const TPZVec<STATE> &par, TPZVec<STATE> &X);

/// malha geometrica de grande porte
TPZGeoMesh * MalhaGeomBig(REAL Lx, REAL Ly, REAL Lz, TPZVec<int> &nblocks, int nref);

TPZCompMesh *MalhaCompTemporaria(TPZAutoPointer<TPZGeoMesh>  gmesh);
TPZCompMesh *MalhaComp2(TPZAutoPointer<TPZGeoMesh>  gmesh,int pOrder,std::set<long> coarseindex);
TPZGeoMesh *GMeshSteklov(bool triang_elements);

void RefinamentoSingular(TPZGeoMesh *gmesh,int nref);

void RefinamentoSingular(TPZAutoPointer<TPZGeoMesh> gmesh,int nref);

void RefinamentoUniforme(TPZAutoPointer<TPZGeoMesh> gmesh, int nref,TPZVec<int> dims);
void RefinamentoUniforme(TPZGeoMesh *gmesh, int nref,TPZVec<int> dims);

void RefinamentoAdaptado(TPZAutoPointer<TPZGeoMesh> gmesh, TPZStack<TPZManVector<REAL,3> > coordcentro);

TPZCompMesh *SkeletonCoarseCompMesh (TPZCompMesh *cmesh, int matId);

void InsertMaterialObjects(TPZCompMesh &cmesh);

TPZCompMesh * CreateHDivMHMMesh(TPZGeoMesh * gmesh, int porder);
TPZCompMesh * CreatePressureMHMMesh(TPZGeoMesh * gmesh, int porder);
TPZCompMesh * CreateHDivPressureMHMMesh(TPZVec<TPZCompMesh * > &cmesh);
void DuplicateNeighbouringConnects(TPZCompMesh * HDivMesh);

void HideTheElements(TPZCompMesh * Multiphysics, bool KeepOneLagrangian);


void ChangeIndex(TPZGeoMesh * gmesh, int matcoarse1D);

void GetElIndexCoarseMesh(TPZGeoMesh *  gmesh, std::set<long> &coarseindex);

void InterfaceToCoarse(TPZCompMesh *cmesh, int matvolume, int matskeleton, int matinterface);

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.mainskeleton"));
#endif


const int matInterno = 1;
const int matCoarse = 2;
const int matInterface = 3;

const int dirichlet = 0;
const int neumann = 1;
const int mixed = 2;

int const bc1=-1;
int const bc2=-2;
int const bc3=-3;
int const bc4=-4;
int const bc5=-5;

void SolExataSteklov(const TPZVec<REAL> &loc, TPZVec<STATE> &u, TPZFMatrix<STATE> &du);
static void Dirichlet(const TPZVec<REAL> &loc, TPZVec<STATE> &result){
    TPZFMatrix<STATE> du(2,1);
    SolExataSteklov(loc,result,du);
}

static void DirichletValidacao(const TPZVec<REAL> &loc, TPZVec<STATE> &result){
    result[0] = loc[0]*loc[0] + loc[1]*loc[1];
}

//sol suave
void InsertMaterialObjectsSuave(TPZCompMesh &cmesh);
void SolSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &u, TPZFMatrix<STATE> &du);
void ForceSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &force);
void DirichletSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &result);
bool problemasuave = true;

//problema arctan
void SolArcTan(const TPZVec<REAL> &pt, TPZVec<STATE> &p, TPZFMatrix<STATE> &flux);
void ForcingTang(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
REAL feps= 1000.;
REAL flambda = 50.;
bool problemaarctan=false;


int main(int argc, char *argv[])
{
    HDivPiola = 1;
#ifdef LOG4CXX
    InitializePZLOG();
#endif
//    gRefDBase.InitializeUniformRefPattern(EOned);
//    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
//    gRefDBase.InitializeUniformRefPattern(ETriangle);
//    gRefDBase.InitializeUniformRefPattern(ECube);
    TPZGeoMesh * gmesh;
    bool UseGenGridQ = false;
    REAL Lx = 1000.,Ly = 100., Lz = 10;
    int nref = 1;
    
    if(UseGenGridQ){
        TPZManVector<int> nblocks(4,4);
        //    nblocks[1] = 30;
        gmesh = MalhaGeomBig(Lx, Ly, Lz, nblocks, nref);
    }
    else{
        int nel_x = 2;
        int nel_y = 1;
        int nel_z = 1;
        
        TPZManVector<REAL,2> dx(2,nel_x), dy(2,nel_y), dz(2,nel_z);
        dx[0] = Lx/REAL(nel_x);
        dy[0] = Ly/REAL(nel_y);
        dz[0] = Lz/REAL(nel_z);
        gmesh = CreateGeometricBoxMesh2D(nref, dx, dy);
//        gmesh = CreateGeometricBoxMesh(nref,dx, dy, dz);
    }


    int porder = 1;
    std::cout << "Geometric mesh created\n";
    TPZManVector<TPZCompMesh *,2 > cmeshes(2);
    cmeshes[0] = CreateHDivMHMMesh(gmesh, porder);
    DuplicateNeighbouringConnects(cmeshes[0]);
    cmeshes[1] = CreatePressureMHMMesh(gmesh, porder);

    std::cout << "Computational meshes created\n";
    
    TPZCompMesh * CHDivPressureMesh = CreateHDivPressureMHMMesh(cmeshes);

    std::cout << "Number of equations " << CHDivPressureMesh->NEquations() << std::endl;
    
    
    bool KeepOneLagrangian = true;
    HideTheElements(CHDivPressureMesh,KeepOneLagrangian);

    std::cout << "Reduced number of equations " << CHDivPressureMesh->NEquations() << std::endl;
    
    //calculo solution
    TPZAnalysis an(CHDivPressureMesh);
    TPZSkylineStructMatrix skyl(CHDivPressureMesh);
    skyl.SetNumThreads(16);
    
#ifdef PZDEBUG
    std::ofstream out_flux("MHM_hdiv.txt");
    an.Mesh()->Print(out_flux);
#endif
    
    an.SetStructuralMatrix(skyl);
    TPZStepSolver<STATE> step;
    step.SetDirect(ELDLt);
    an.SetSolver(step);
    
    std::cout << "Assembling\n";
    an.Assemble();
    
#ifdef PZDEBUG
    {
        std::ofstream out_k_f("system.txt");
        an.Solver().Matrix()->Print("k = ",out_k_f,EMathematicaInput);
        an.Rhs().Print("f = ",out_k_f,EMathematicaInput);
    }
#endif
    
    std::cout << "Solving\n";
    an.Solve();
    std::cout << "Finished\n";
    
#ifdef PZDEBUG
    {
        std::ofstream out("MHM_hdiv_sol.txt");
        an.Mesh()->Print(out);
    }
#endif
    an.LoadSolution(); // compute internal dofs
//    an.Solution().Print("sol = ");
    
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(cmeshes, an.Mesh());
//    TPZBuildMultiphysicsMesh::TransferFromMeshes(cmeshes, an.Mesh());
//    cmeshes[0]->Solution().Print("solq = ");
//    cmeshes[1]->Solution().Print("solp = ");
    std::string plotfile("mixed_solution.vtk");
    TPZStack<std::string> scalnames,vecnames;
    scalnames.Push("Pressure");
    vecnames.Push("Flux");
    an.DefineGraphMesh(CHDivPressureMesh->Dimension(), scalnames, vecnames, plotfile);
    an.PostProcess(0,CHDivPressureMesh->Dimension());

    return 0;
}


int main_not_used(int argc, char *argv[])
{
    HDivPiola = 1;
    InitializePZLOG();
    gRefDBase.InitializeUniformRefPattern(EOned);
    gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
    gRefDBase.InitializeUniformRefPattern(ETriangle);

    
    TPZGeoMesh * gmesh = new TPZGeoMesh;
    if(problemasuave || problemaarctan){
        gmesh= MalhaGeom2(1, 1);}
    else
    {
        gmesh = GMeshSteklov(false);
    }
    

    {
        ofstream arg1("gmesh1.txt");
        gmesh->Print(arg1);
    }

    
    //-------- construindo malha coarse ----------
    
    //1 refinamento uniforme
    TPZVec<int> dims(2,0);
    dims[0]=1; dims[1]=2;
    int nref = 1;
    RefinamentoUniforme(gmesh, nref, dims);
    
    {
        ofstream arg1("gmesh1.txt");
        gmesh->Print(arg1);
    }
    
    if(!problemasuave){
        nref = 2;
        RefinamentoSingular(gmesh, nref);
    }
    
    std::ofstream Dummyfile("GeometricMesh.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh,Dummyfile, true);
    
    {
        ofstream arg1("gmesh1.txt");
        gmesh->Print(arg1);
    }
    //index dos elementos da malha coarse
    std::set<long> coarseindex;
    GetElIndexCoarseMesh(gmesh, coarseindex);
    
    
    TPZGeoMesh * gmesh2 = new TPZGeoMesh(*gmesh);
    
    dims.Resize(1, 0);
    dims[0]=2;
    nref = 0;
    RefinamentoUniforme(gmesh2, nref, dims);
    
    std::ofstream Dummyfile2("GeometricMesh2.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh2,Dummyfile2, true);
    
    TPZMHMeshControl mhm(gmesh2,coarseindex);
    bool uselagrange = false;
    mhm.SetLagrangeAveragePressure(uselagrange);
    mhm.SetInternalPOrder(3);
    mhm.SetSkeletonPOrder(1);
    mhm.CreateCoarseInterfaces(matCoarse);
//    if(problemasuave || problemaarctan){
//        InsertMaterialObjectsSuave(mhm.CMesh());
//    }else {
        InsertMaterialObjects(mhm.CMesh());
//    }
    mhm.BuildComputationalMesh(true);
    {
        ofstream arq("gmeshmhm.txt");
        mhm.CMesh()->Reference()->Print(arq);
    }
#ifdef LOG4CXX
    if (logger->isDebugEnabled()) {
        std::stringstream sout;
        mhm.CMesh()->Print(sout);
        LOGPZ_DEBUG(logger, sout.str())
    }
#endif
    
#ifdef LOG4CXX
    if (logger->isDebugEnabled()) {
        std::stringstream sout;
        //        mhm.PrintDiagnostics(std::cout);
        LOGPZ_DEBUG(logger, sout.str())
    }
#endif
    
    
#ifdef LOG4CXX2
    if (logger->isDebugEnabled()) {
        std::stringstream sout;
        stiff->Print("Stiffness = ",sout,EMathematicaInput);
        rhs.Print("Rhs = ",sout);
        LOGPZ_DEBUG(logger, sout.str())
    }
#endif
    
    //calculo solution
    TPZAnalysis an(mhm.CMesh());
    TPZSkylineStructMatrix skyl(mhm.CMesh());
    an.SetStructuralMatrix(skyl);
    TPZStepSolver<STATE> step;
    step.SetDirect(ELDLt);
    an.SetSolver(step);
    an.Assemble();
    an.Solve();
    an.Solution().Print("solution = ");
    long neq = an.Solution().Rows();
    long numeq = MIN(10, neq);
    TPZManVector<long> equationindices(numeq);
    for (int i=0; i<numeq; i++) {
        equationindices[i] = i;
    }
    an.ShowShape("Shapes.vtk", equationindices);
    an.SetStep(0);
    
    if(problemasuave || problemaarctan){
        std::string plotfile("result.vtk");
        TPZStack<std::string> scalnames,vecnames;
        scalnames.Push("Solution");
        scalnames.Push("ExactSolution");
        scalnames.Push("PressureConstante");
        an.DefineGraphMesh(mhm.CMesh()->Dimension(), scalnames, vecnames, plotfile);
        an.PostProcess(0,2);
    }
    
//    an.SetExact(*SolExataSteklov);
//    TPZVec<REAL> erros(3);
//    an.PostProcessError(erros);
    
    //    //construir elementos 1D de interface
    //    TPZCompMesh * cmesh1 = MalhaCompTemporaria(gmesh);
    //    gmesh->ResetReference();
    //
    //    //mudar matId dos elementos 1D de interface
    //    ChangeIndex(gmesh, matCoarse);
    //    ofstream arg3("gmesh3.txt");
    //	gmesh->Print(arg3);
    //
    //    ofstream file1("malhageometricaCoarse.vtk");
    //    TPZVTKGeoMesh::PrintGMeshVTK(gmesh.operator->(), file1, true);
    //
    //
    //    if(coarseindex.find(6) != coarseindex.end())
    //    {
    //        std::cout << "\n\n\nNAO ACHEI O NUMERO DESEJADO\n\n\n";
    //    }
    //
    //
    //    std::set<long>::iterator it;
    //    std::cout << "coarse index: \n";
    //    for (it=coarseindex.begin(); it!=coarseindex.end(); ++it)
    //        std::cout << ' ' << *it;
    //    std::cout << '\n';
    //
    ////-------- malha mais fina -------
    //    //refinamento uniforme dos elementos 2D
    //    dims.Resize(1, 0);
    //    dims[0]=2;
    //    RefinamentoUniforme(gmesh, 1, dims);
    //    ofstream arg4("gmesh4.txt");
    //	gmesh->Print(arg4);
    //
    //    ofstream file2("malhageometricaFina.vtk");
    //    TPZVTKGeoMesh::PrintGMeshVTK(gmesh.operator->(), file2, true);
    //
    ////malha computacional
    //    TPZCompMesh * cmesh = MalhaComp2(gmesh,1,coarseindex);
    //    ofstream arg5("cmesh.txt");
    //	cmesh->Print(arg5);
    //
    //    InterfaceToCoarse(cmesh, matInterno, matCoarse, matInterface);
    //
    //    ofstream arg6("cmesh2.txt");
    //	cmesh->Print(arg6);
    
    return EXIT_SUCCESS;
}

TPZGeoMesh *MalhaGeom2(REAL Lx, REAL Ly)
{
    int Qnodes = 4;
	long dim = 2;
    
	TPZGeoMesh * gmesh = new TPZGeoMesh;
    gmesh->SetDimension(dim);
	gmesh->SetMaxNodeId(Qnodes-1);
	gmesh->NodeVec().Resize(Qnodes);
	TPZVec<TPZGeoNode> Node(Qnodes);
	
	TPZVec <long> TopolQuad(4);
	TPZVec <long> TopolLine(2);
	
	//indice dos nos
	long id = 0;
	REAL valx;
	for(int xi = 0; xi < Qnodes/2; xi++)
	{
		valx = xi*Lx;
		Node[id].SetNodeId(id);
		Node[id].SetCoord(0 ,valx );//coord X
		Node[id].SetCoord(1 ,0. );//coord Y
		gmesh->NodeVec()[id] = Node[id];
		id++;
	}
	
	for(int xi = 0; xi < Qnodes/2; xi++)
	{
		valx = Lx - xi*Lx;
		Node[id].SetNodeId(id);
		Node[id].SetCoord(0 ,valx );//coord X
		Node[id].SetCoord(1 ,Ly);//coord Y
		gmesh->NodeVec()[id] = Node[id];
		id++;
	}
    
	//indice dos elementos
	id = 0;
    
    //elementos internos
    TopolQuad[0] = 0;
	TopolQuad[1] = 1;
	TopolQuad[2] = 2;
	TopolQuad[3] = 3;
	new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,matInterno,*gmesh);
	id++;
    
    //elementos de contorno
	TopolLine[0] = 0;
	TopolLine[1] = 1;
	new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc1,*gmesh);
	id++;
	
	TopolLine[0] = 1;
	TopolLine[1] = 2;
	new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc2,*gmesh);
	id++;
	
	TopolLine[0] = 2;
	TopolLine[1] = 3;
	new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc3,*gmesh);
	id++;
	
	TopolLine[0] = 3;
	TopolLine[1] = 0;
	new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc4,*gmesh);
	id++;
    
    //construir a malha
	gmesh->BuildConnectivity();
	
	return gmesh;
}


TPZGeoMesh *GMeshSteklov(bool triang_elements)
{
    TPZManVector<int,2> nx(2,2);
    REAL extent = 1;
    nx[1] =1;
    TPZManVector<REAL,3> x0(3,0.),x1(3,extent);
    x0[0] = -extent;
    TPZGenGrid gengrid(nx,x0,x1);
    TPZGeoMesh *gmesh = new TPZGeoMesh;
    if(triang_elements)
    {
        gengrid.SetElementType(ETriangle);
    }
    gengrid.Read(gmesh);
    
    //elementos de contorno
    TPZManVector<REAL,3> firstpoint(3,0.),secondpoint(3,0.);
    firstpoint[0] = extent;
    secondpoint[0] = extent;
    secondpoint[1] = extent;
    gengrid.SetBC(gmesh,firstpoint,secondpoint,bc2);
    gengrid.SetBC(gmesh,6,bc3);
    gengrid.SetBC(gmesh,7,bc4);
    firstpoint[0] = -extent;
    secondpoint[0] = 0.;
    secondpoint[1] = 0.;
    gengrid.SetBC(gmesh,firstpoint,secondpoint,bc5);
    firstpoint = secondpoint;
    secondpoint[0] = extent;
    gengrid.SetBC(gmesh,firstpoint,secondpoint,bc1);
    
    return gmesh;
}

TPZCompMesh* MalhaCompTemporaria(TPZAutoPointer<TPZGeoMesh>  gmesh)
{
	/// criar materiais
	int dim = 2;
    TPZMatPoisson3d *material = new TPZMatPoisson3d(matInterno,dim);
	TPZMaterial * mat1(material);
	material->NStateVariables();
    
	TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
	cmesh->SetDimModel(dim);
	cmesh->InsertMaterialObject(mat1);
	
	///Inserir condicao de contorno
	TPZFMatrix<STATE> val1(2,2,0.), val2(2,1,0.);
	
    TPZMaterial * BCondD1 = material->CreateBC(mat1, bc2,dirichlet, val1, val2);
	cmesh->InsertMaterialObject(BCondD1);
	
	TPZMaterial * BCondD2 = material->CreateBC(mat1, bc4,dirichlet, val1, val2);
	cmesh->InsertMaterialObject(BCondD2);
    
	TPZMaterial * BCondN1 = material->CreateBC(mat1, bc1,dirichlet, val1, val2);
	cmesh->InsertMaterialObject(BCondN1);
    
    TPZMaterial * BCondN2 = material->CreateBC(mat1, bc3,dirichlet, val1, val2);
    cmesh->InsertMaterialObject(BCondN2);
    
    cmesh->SetAllCreateFunctionsDiscontinuous();
    
    cmesh->AutoBuild();
    
    TPZCreateApproximationSpace::CreateInterfaces(*cmesh);
    
    return cmesh;
}

void InsertMaterialObjects(TPZCompMesh &cmesh)
{
	/// criar materiais
	int dim = cmesh.Dimension();
    TPZMatLaplacianLagrange *material1 = new TPZMatLaplacianLagrange(matInterno,dim);
    
	TPZMaterial * mat1(material1);
    
    TPZMat1dLin *materialCoarse = new TPZMat1dLin(matCoarse);
    TPZFNMatrix<1,STATE> xk(1,1,0.),xb(1,1,0.),xc(1,1,0.),xf(1,1,0.);
    materialCoarse->SetMaterial(xk, xc, xb, xf);
    
    cmesh.InsertMaterialObject(materialCoarse);
    
    //    REAL diff = -1.;
    //	REAL conv = 0.;
    //	TPZVec<REAL> convdir(3,0.);
    //	REAL flux = 8.;
    //
    //	material1->SetParameters(diff, conv, convdir);
    //	material1->SetInternalFlux( flux);
    
	cmesh.InsertMaterialObject(mat1);
	
	///Inserir condicao de contorno
	TPZFMatrix<STATE> val1(2,2,1.), val2(2,1,0.);
	
    //BC -1
    TPZMaterial * BCondD1 = material1->CreateBC(mat1, bc1,neumann, val1, val2);
    TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet1 = new TPZDummyFunction<REAL>(Dirichlet);
    BCondD1->SetForcingFunction(bcmatDirichlet1);
    cmesh.InsertMaterialObject(BCondD1);
    
    //BC -2
	TPZMaterial * BCondD2 = material1->CreateBC(mat1, bc2,neumann, val1, val2);
    TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet2 = new TPZDummyFunction<REAL>(Dirichlet);
    BCondD2->SetForcingFunction(bcmatDirichlet2);
    cmesh.InsertMaterialObject(BCondD2);
    
    //BC -3
	TPZMaterial * BCondD3 = material1->CreateBC(mat1, bc3,neumann, val1, val2);
    TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet3 = new TPZDummyFunction<REAL>(Dirichlet);
    BCondD3->SetForcingFunction(bcmatDirichlet3);
    cmesh.InsertMaterialObject(BCondD3);
    
    //BC -4
	TPZMaterial * BCondD4 = material1->CreateBC(mat1, bc4,neumann, val1, val2);
    TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet4 = new TPZDummyFunction<REAL>(Dirichlet);
    BCondD4->SetForcingFunction(bcmatDirichlet4);
    cmesh.InsertMaterialObject(BCondD4);
    
    //BC -5: dirichlet nulo
    TPZMaterial * BCondD5 = material1->CreateBC(mat1, bc5,neumann, val1, val2);
    cmesh.InsertMaterialObject(BCondD5);
}

void InsertMaterialObjectsSuave(TPZCompMesh &cmesh)
{
	/// criar materiais
	int dim = cmesh.Dimension();
    TPZMatLaplacianLagrange *materialFiner = new TPZMatLaplacianLagrange(matInterno,dim);
    
    TPZAutoPointer<TPZFunction<REAL> > forcef;
    TPZAutoPointer<TPZFunction<STATE> > solExata;
    
    if(problemaarctan)
    {
        forcef = new TPZDummyFunction<REAL>(ForcingTang);
        solExata = new TPZDummyFunction<STATE>(SolArcTan);
    }else
    {
        forcef = new TPZDummyFunction<REAL>(ForceSuave);
        solExata = new TPZDummyFunction<STATE>(SolSuave);
    }
    
    materialFiner->SetForcingFunction(forcef);
    materialFiner->SetForcingFunctionExact(solExata);
    
	TPZMaterial * mat1(materialFiner);
    
    TPZMat1dLin *materialSkeleton = new TPZMat1dLin(matCoarse);
    TPZFNMatrix<1,STATE> xk(1,1,0.),xb(1,1,0.),xc(1,1,0.),xf(1,1,0.);
    materialSkeleton->SetMaterial(xk, xc, xb, xf);
    
	cmesh.InsertMaterialObject(mat1);
    cmesh.InsertMaterialObject(materialSkeleton);
	
	///Inserir condicao de contorno
	TPZFMatrix<STATE> val1(2,2,1.), val2(2,1,0.);
    TPZMaterial * BCondD1;
    TPZMaterial * BCondD2;
    TPZMaterial * BCondD3;
    TPZMaterial * BCondD4;
    
    if(problemaarctan)
    {
        //u=0 no contorno de Omega
        BCondD1 = materialFiner->CreateBC(mat1, bc1,neumann, val1, val2);
        BCondD2 = materialFiner->CreateBC(mat1, bc2,neumann, val1, val2);
        BCondD3 = materialFiner->CreateBC(mat1, bc3,neumann, val1, val2);
        BCondD4 = materialFiner->CreateBC(mat1, bc4,neumann, val1, val2);
    }
    else
    {
        //BC -1
        BCondD1 = materialFiner->CreateBC(mat1, bc1,neumann, val1, val2);
        TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet1 = new TPZDummyFunction<REAL>(DirichletSuave);
        BCondD1->SetForcingFunction(bcmatDirichlet1);
        
        //BC -2
        BCondD2 = materialFiner->CreateBC(mat1, bc2,neumann, val1, val2);
        TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet2 = new TPZDummyFunction<REAL>(DirichletSuave);
        BCondD2->SetForcingFunction(bcmatDirichlet2);
        
        //BC -3
        BCondD3 = materialFiner->CreateBC(mat1, bc3,neumann, val1, val2);
        TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet3 = new TPZDummyFunction<REAL>(DirichletSuave);
        BCondD3->SetForcingFunction(bcmatDirichlet3);
        
        //BC -4
        BCondD4 = materialFiner->CreateBC(mat1, bc4,neumann, val1, val2);
        TPZAutoPointer<TPZFunction<REAL> > bcmatDirichlet4 = new TPZDummyFunction<REAL>(DirichletSuave);
        BCondD4->SetForcingFunction(bcmatDirichlet4);
    }
    
    cmesh.InsertMaterialObject(BCondD1);
    cmesh.InsertMaterialObject(BCondD2);
    cmesh.InsertMaterialObject(BCondD3);
    cmesh.InsertMaterialObject(BCondD4);
}


TPZCompMesh* MalhaComp2(TPZAutoPointer<TPZGeoMesh> gmesh, int pOrder,std::set<long> coarseindex)
{
	/// criar materiais
	int dim = 2;
    TPZCompEl::SetgOrder(pOrder);
	TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
	cmesh->SetDimModel(dim);
    
    InsertMaterialObjects(*cmesh);
    TPZMatPoisson3d *material2 = new TPZMatPoisson3d(matCoarse,dim);
    cmesh->InsertMaterialObject(material2);
    
    //    REAL diff = -1.;
    //	REAL conv = 0.;
    //	TPZVec<REAL> convdir(3,0.);
    //	REAL flux = 8.;
    //
    //	material1->SetParameters(diff, conv, convdir);
    //	material1->SetInternalFlux( flux);
    
    cmesh->SetDefaultOrder(pOrder);
    
    cmesh->SetAllCreateFunctionsContinuous();
    
    //Criar elementos computacionais malha MHM
    int nel = gmesh->NElements();
    int matid, eldim;
    long index;
    int hassubel, nsubels;
    int iel, is;
    
    TPZGeoEl *gel = NULL;
    TPZGeoEl *gsubel = NULL;
    
    for(iel = 0; iel<nel; iel++)
    {
        gel = gmesh->ElementVec()[iel];
        if(!gel) DebugStop();
        
        eldim = gel->Dimension();
        matid = gel->MaterialId();
        
        //elementos de dimensao = dim (malha fina)
        if(eldim==dim)
        {
            index = gel->Index();
            if(coarseindex.find(index) != coarseindex.end())
            {
                nsubels = gel->NSubElements();
                for (is=0; is<nsubels; is++)
                {
                    gsubel = gel->SubElement(is);
                    hassubel = gsubel->HasSubElement();
                    if(!hassubel){
                        cmesh->CreateCompEl(gsubel,index);
                    }
                }
                for (is=0; is<nsubels; is++)
                {
                    gsubel = gel->SubElement(is);
                    hassubel = gsubel->HasSubElement();
                    if(!hassubel){
                        gsubel->ResetReference();
                    }
                }
            }
            continue;
        }
        
        //elementos de dimensao = dim-1
        
        //malha coarse
        if(matid==matCoarse)
        {
            cmesh->CreateCompEl(gel, index);
            gel->ResetReference();
            
            continue;
        }
        
        //elementos de contorno
        hassubel =gel->HasSubElement();
        if(!hassubel)
        {
            cmesh->CreateCompEl(gel, index);
            gel->ResetReference();
        }
    }
    
    cmesh->LoadReferences();
    cmesh->ExpandSolution();
    
    return cmesh;
}



void RefinamentoUniforme(TPZAutoPointer<TPZGeoMesh> gmesh, int nref,TPZVec<int> dims)
{
    RefinamentoUniforme(gmesh.operator->(), nref, dims);
}



void RefinamentoUniforme(TPZGeoMesh *gmesh, int nref,TPZVec<int> dims)
{
    
    int ir, iel, k;
    int nel=0, dim=0;
    int ndims = dims.size();
	for(ir = 0; ir < nref; ir++ )
    {
		TPZVec<TPZGeoEl *> filhos;
        nel = gmesh->NElements();
        
		for (iel = 0; iel < nel; iel++ )
        {
			TPZGeoEl * gel = gmesh->ElementVec()[iel];
            if(!gel) DebugStop();
            
            dim = gel->Dimension();
            
            for(k = 0; k<ndims; k++)
            {
                if(dim == dims[k])
                {
                    gel->Divide (filhos);
                    break;
                }
            }
		}
	}
    
}

void RefinamentoAdaptado(TPZAutoPointer<TPZGeoMesh> gmesh, TPZStack<TPZManVector<REAL,3> > coordcentro)
{
    int size = coordcentro.NElements();
    std::set<TPZGeoEl *> setgeo;
    
    TPZVec<REAL> qsi(3,0.);
    long iniEl = 0;
    
    TPZStack<TPZGeoEl *> vecgel;
    vecgel.Resize(0);
    int eldim = 2;
    for(int ix = 0; ix < size; ix++)
    {
        TPZGeoEl * gel = NULL;
        
        if(coordcentro[ix][0]== 0. || coordcentro[ix][1]==0. || coordcentro[ix][0]== 1. || coordcentro[ix][1]==1.)
        {
            eldim = 1;
            gel = gmesh->FindElement(coordcentro[ix], qsi, iniEl, eldim);
            eldim = 2;
        }else gel = gmesh->FindElement(coordcentro[ix], qsi, iniEl, eldim);
        if(!gel) DebugStop();
        setgeo.insert(gel);
    }
    
    
    int nel = gmesh->NElements();
    TPZVec<TPZGeoEl *> filhos;
    for(int iel = 0; iel<nel; iel++)
    {
        TPZGeoEl * gel = gmesh->ElementVec()[iel];
        std::set<TPZGeoEl *>::iterator found = setgeo.find(gel);
        
        if(gel==(*found)){
            continue;
        }
        gel->Divide (filhos);
    }
}

void GetElIndexCoarseMesh(TPZGeoMesh *  gmesh, std::set<long> &coarseindex)
{
    int nel = gmesh->NElements();
    int iel;
    int hassubel=0;
    int dim = gmesh->Dimension();
    int eldim;
    for(iel = 0; iel<nel; iel++)
    {
        TPZGeoEl * gel = gmesh->ElementVec()[iel];
        if(!gel) DebugStop();
        
        hassubel = gel->HasSubElement();
        eldim = gel->Dimension();
        if(!hassubel && eldim ==dim)
        {
            coarseindex.insert(gel->Index());
        }
    }
    
}

void ChangeIndex(TPZAutoPointer<TPZGeoMesh> gmesh, int matcoarse1D)
{
    int nel = gmesh->NElements();
    int iel;
    int hassubel, ninterf;
    
    for(iel = 0; iel<nel; iel++)
    {
        TPZGeoEl * gel = gmesh->ElementVec()[iel];
        if(!gel) DebugStop();
        
        hassubel = gel->HasSubElement();
        if(!hassubel)
        {
            ninterf = gel->NumInterfaces();
            if(ninterf > 1) DebugStop();
            if (ninterf==1)
            {
                gel->SetMaterialId(matcoarse1D);
                gel->DecrementNumInterfaces();
            }
        }
    }
    
}

//TPZCompMesh *SkeletonCoarseCompMesh (TPZCompMesh *cmesh)
//{
//
//    //ponteiro para a malha geometrica de mesh
//    TPZGeoMesh *gmesh = cmesh->Reference();
//    if(!gmesh)
//    {
//        DebugStop();
//    }
//
//    //Reseta as referencias do ponteiro para a malha geometrica criada
//    //e criar uma nova malha computacional baseada nesta malha geometrica
//    gmesh->ResetReference();
//    cmesh->LoadReferences();
//    //TPZCompMesh *newcmesh = new TPZCompMesh(gmesh);
//
//    int dim = cmesh->Dimension();
//    //newcmesh->SetDimModel(dim);
//
//    TPZStack<TPZCompElSide> neighequal,neighsmaller;
//    TPZCompElSide neighbigger;
//    int nneighs=0;
//
//    int nel = cmesh->ElementVec().NElements();
//    for(long el = 0; el < nel; el++)
//	{
//		TPZCompEl * compEl = cmesh->ElementVec()[el];
//		if(!compEl) continue;
//
//        int eldim = compEl->Reference()->Dimension();
//        int elmatId = compEl->Reference()->MaterialId();
//
//        //convencao PZ: elemento de contorno tem sempre Id negativo
//		if(elmatId < 0)
//		{
//			compEl->Reference()->ResetReference();
//		}
//        else if(eldim == dim)
//        {
//            compEl->Reference()->ResetReference();
//
//            int nsides = compEl->Reference()->NSides();
//            int ncorn = compEl->Reference()->NCornerNodes();
//            for(int side = ncorn; side < nsides; side++)
//            {
//                neighequal.Resize(0);
//                neighsmaller.Resize(0);
//
//                TPZCompElSide celside(compEl,side);
//                celside.EqualLevelElementList(neighequal, 0, 0);
//                neighbigger = celside.LowerLevelElementList(0);
//
//                if(neighbigger){
//                    neighequal.Push(neighbigger);
//                }
//                nneighs = neighequal.NElements();
//
//                //                celside.HigherLevelElementList(neighsmaller, 1, 1);
//                //                int nneighsmaller = neighsmaller.size();
//                //                if(nneighs && nneighsmaller)
//                //                {
//                //                    DebugStop();
//                //                }
//
//                if(nneighs != 0)
//                {
//                    //Loop on neighboring elements greater or equal level
//                    for(int i =0; i<nneighs; i++)
//                    {
//                        TPZGeoEl *geoside = neighequal[i].Reference().Element();
//
//                        //Do not assume neighbors by node
//                        if(neighequal[i].Side() < geoside->NCornerNodes()) continue;
//
//                        //verificando se eh elemento 1d
//                        if(geoside->Dimension() == dim-1 && geoside->MaterialId() > 0) continue;
//
//                        long index;
//                        TPZInterpolatedElement *newcompel;
//                        newcompel = dynamic_cast<TPZInterpolatedElement *> (cmesh->CreateCompEl(geoside, index));
//                        newcompel->LoadElementReference();
//                    }
//                }
//            }
//        }
//        else continue;
//	}
//
//    return cmesh;
//}

TPZCompMesh *SkeletonCoarseCompMesh (TPZCompMesh *cmesh, int matId)
{
    
    //ponteiro para a malha geometrica de mesh
    TPZGeoMesh *gmesh = cmesh->Reference();
    if(!gmesh)
    {
        DebugStop();
    }
    
    //Resetar as referencias do ponteiro para a malha geometrica criada
    //e criar uma nova malha computacional baseada nesta malha geometrica
    //gmesh->ResetReference();
    
    //    TPZCompMesh *newcmesh = new TPZCompMesh(gmesh);
    //    gmesh->ResetReference();
    //    newcmesh->LoadReferences();
    
    int dim = cmesh->Dimension();
    //newcmesh->SetDimModel(dim);
    
    TPZStack<TPZCompElSide> neighequal,neighsmaller;
    TPZCompElSide neighbigger;
    int nneighs=0;
    
    int nel = gmesh->ElementVec().NElements();
    for(long el = 0; el < nel; el++)
	{
		TPZGeoEl * gel = gmesh->ElementVec()[el];
		if(!gel) continue;
        if(gel->HasSubElement()) continue;
        
        int eldim = gel->Dimension();
        //int elmatId = gel->MaterialId();
        
        //        //convencao PZ: elemento de contorno tem sempre Id negativo
        //		if(elmatId < 0)
        //		{
        //			geo->Reference()->ResetReference();
        //		}
        if(eldim == dim)
        {
            //compEl->Reference()->ResetReference();
            
            int nsides = gel->NSides();
            int ncorn = gel->NCornerNodes();
            for(int side = ncorn; side < nsides; side++)
            {
                neighequal.Resize(0);
                neighsmaller.Resize(0);
                
                TPZGeoElSide gelside(gel,side);
                gelside.EqualLevelCompElementList(neighequal, 0, 0);
                neighbigger = gelside.LowerLevelCompElementList2(0);
                
                if(neighbigger){
                    neighequal.Push(neighbigger);
                }
                nneighs = neighequal.NElements();
                
                //                gelside.HigherLevelCompElementList2(neighsmaller, 1, 1);
                //                int nneighsmaller = neighsmaller.size();
                //                if(nneighs && nneighsmaller)
                //                {
                //                    DebugStop();
                //                }
                
                if(nneighs != 0)
                {
                    //Loop on neighboring elements greater or equal level
                    for(int i =0; i<nneighs; i++)
                    {
                        TPZGeoEl *geoside = neighequal[i].Reference().Element();
                        
                        //Do not assume neighbors by node
                        if(neighequal[i].Side() < geoside->NCornerNodes()) continue;
                        
                        //verificando se eh elemento 1d
                        if(geoside->Dimension() == dim-1 && geoside->MaterialId() > 0) continue;
                        
                        long index;
                        TPZInterpolatedElement *newcompel;
                        geoside->ResetReference();
                        newcompel = dynamic_cast<TPZInterpolatedElement *> (cmesh->CreateCompEl(geoside, index));
                        newcompel->Reference()->SetMaterialId(matId);
                        newcompel->LoadElementReference();
                        newcompel->Print();
                        cout<<"\n";
                        newcompel->Reference()->Print();
                    }
                }
            }
            gel->ResetReference();
        }
        else continue;
	}
    //cmesh->LoadReferences();
    cmesh->InitializeBlock();
    return cmesh;
}

void InterfaceToCoarse(TPZCompMesh *cmesh, int matvolume, int matskeleton, int matinterface)
{
    int nelem = cmesh->NElements();
    for (int el=0; el<nelem; el++) {
        TPZCompEl *cel = cmesh->ElementVec()[el];
        if (!cel) {
            continue;
        }
        int matid = cel->Reference()->MaterialId();
        if (matid != matskeleton) {
            continue;
        }
        // the element cel is a skeleton element
        TPZGeoEl *gel = cel->Reference();
        std::set<long> celindices;
        TPZGeoElSide gelside(gel,gel->NSides()-1);
        TPZCompElSide celskeleton = gelside.Reference();
        TPZGeoElSide neighbour = gelside.Neighbour();
        while (neighbour != gelside) {
            TPZStack<TPZCompElSide> celstack;
            gelside.HigherLevelCompElementList2(celstack, 1, 0);
            long nstack = celstack.NElements();
            for (long i=0; i<nstack; i++) {
                TPZCompElSide celside = celstack[i];
                long celindex = celside.Element()->Index();
                if (celindices.find(celindex) != celindices.end()) {
                    continue;
                }
                celindices.insert(celindex);
                int side = gel->NSides()-1;
                long index;
                TPZGeoEl *gelnew = gel->CreateBCGeoEl(side, matinterface);
                new TPZInterfaceElement(*cmesh, gelnew , index, celside, celskeleton);
                
            }
            neighbour = neighbour.Neighbour();
        }
    }
}


void SolExataSteklov(const TPZVec<REAL> &loc, TPZVec<STATE> &u, TPZFMatrix<STATE> &du){
    
    const REAL n = 0;
    
    const REAL x = loc[0];
    const REAL y = loc[1];
    const REAL r = sqrt(x*x+y*y);
    const REAL t = atan2(y,x);
    const REAL sol = pow((REAL)2,0.25 + n/2.)*pow(r,0.5 + n)*cos((0.5 + n)*t);
    u[0] = sol;
    
    du(0,0) = pow((REAL)2,-0.75 + n/2.)*(1 + 2*n)*pow(pow(x,2) + pow(y,2),-0.75 + n/2.)*(x*cos((0.5 + n)*atan2(y,x)) + y*sin((0.5 + n)*atan2(y,x)));
    du(1,0) = pow((REAL)2,-0.75 + n/2.)*(1 + 2*n)*pow(pow(x,2) + pow(y,2),-0.75 + n/2.)*(y*cos((0.5 + n)*atan2(y,x)) - x*sin((0.5 + n)*atan2(y,x)));
    
}

void RefinamentoSingular(TPZAutoPointer<TPZGeoMesh> gmesh,int nref)
{
    RefinamentoSingular(gmesh.operator->(), nref);
}

void RefinamentoSingular(TPZGeoMesh *gmesh,int nref)
{
    long nnodes = gmesh->NNodes();
    long in;
    for (in=0; in<nnodes; in++) {
        TPZGeoNode *gno = &gmesh->NodeVec()[in];
        if (abs(gno->Coord(0))< 1.e-6 && abs(gno->Coord(1)) < 1.e-6) {
            break;
        }
    }
    if (in == nnodes) {
        DebugStop();
    }
    TPZGeoElSide gelside;
    long nelem = gmesh->NElements();
    for (long el = 0; el<nelem; el++) {
        TPZGeoEl *gel = gmesh->ElementVec()[el];
        int ncorner = gel->NCornerNodes();
        for (int ic=0; ic<ncorner; ic++) {
            long nodeindex = gel->NodeIndex(ic);
            if (nodeindex == in) {
                gelside = TPZGeoElSide(gel, ic);
                break;
            }
        }
        if (gelside.Element()) {
            break;
        }
    }
    if (!gelside.Element()) {
        DebugStop();
    }
    for (int iref = 0; iref <nref; iref++) {
        TPZStack<TPZGeoElSide> gelstack;
        gelstack.Push(gelside);
        TPZGeoElSide neighbour = gelside.Neighbour();
        while (neighbour != gelside) {
            gelstack.Push(neighbour);
            neighbour = neighbour.Neighbour();
        }
        long nstack = gelstack.size();
        for (long ist=0; ist < nstack; ist++) {
            if (!gelstack[ist].Element()->HasSubElement()) {
                TPZVec<TPZGeoEl *> subel;
                gelstack[ist].Element()->Divide(subel);
            }
        }
    }
}

void SolSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &u, TPZFMatrix<STATE> &du){
    
    const REAL x = loc[0];
    const REAL y = loc[1];
    const REAL sol = sin(M_PI*x)*sin(M_PI*y);
    u[0] = sol;
    du.Resize(2, 1);
    du(0,0) = M_PI*cos(M_PI*x)*sin(M_PI*y);
    du(1,0) = M_PI*cos(M_PI*y)*sin(M_PI*x);
}

void ForceSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &force){
    const REAL x = loc[0];
    const REAL y = loc[1];
    const REAL f = 2.*M_PI*M_PI*sin(M_PI*x)*sin(M_PI*y);
    force[0] = f;
}

void DirichletSuave(const TPZVec<REAL> &loc, TPZVec<STATE> &result){
    TPZFMatrix<STATE> du(2,1);
    SolSuave(loc,result,du);
}

#define Power pow
#define ArcTan atan
#define Sqrt sqrt

void SolArcTan(const TPZVec<REAL> &pt, TPZVec<STATE> &p, TPZFMatrix<STATE> &flux){
    REAL x = pt[0];
    REAL y = pt[1];
    
    p[0]=0;
    flux(0,0)=0;
    flux(1,0)=0;
    
    REAL eps = feps;
    REAL lambda = flambda;//frequencia
    
    
    REAL temp1 = (x-0.5)*(x-0.5) + (y-0.5)*(y-0.5);
    REAL temp2 = 1. + (2./M_PI)*ArcTan(Sqrt(eps)*1./16. - sqrt(eps)*temp1);
    
    p[0] = 5.*x*(x - 1.)*y*(y - 1.)*(0.1*cos(lambda*M_PI*x)*cos(lambda*M_PI*y) + temp2);
    
    
    //px
    flux(0,0)=5*(-1 + y)*y*(-0.6366197723675814*(-1. + x)*(-1.5707963267948966 + 1.*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) - 0.15707963267948966*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) - 0.6366197723675814*x*(-1.5707963267948966 + 1.*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) - 0.15707963267948966*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) + (-1 + x)*x*((Sqrt(eps)*(0.6366197723675814 - 1.2732395447351628*x))/(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)) - 0.3141592653589793*lambda*cos(lambda*M_PI*y)*sin(lambda*M_PI*x)));
    
    
    //py
    flux(1,0)= 5*(-1 + x)*x*(-0.6366197723675814*(-1. + y)*(-1.5707963267948966 + 1.*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) - 0.15707963267948966*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) - 0.6366197723675814*y*(-1.5707963267948966 + 1.*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) - 0.15707963267948966*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) + (-1 + y)*y*((Sqrt(eps)*(0.6366197723675814 - 1.2732395447351628*y))/(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)) - 0.3141592653589793*lambda*cos(lambda*M_PI*x)*sin(lambda*M_PI*y)));
}


void ForcingTang(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    
    double x = pt[0];
    double y = pt[1];
    
    disp[0] = 0.;
    REAL eps = feps;
    REAL lambda = flambda;
    
    disp[0]= 6.366197723675814*(-1. + y)*y*(-1.5707963267948966 + 1.*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) - 0.15707963267948966*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) - 5*(-1 + x)*x*(-1 + y)*y* ((4*Sqrt(eps)*(-1 + 4.*eps*Power(-0.5 + x,2)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2)) - 1.*eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)))/(M_PI*Power(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2),2)) - 0.9869604401089358*Power(lambda,2)*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) - 2*(-5 + 10*x)*(-1 + y)*y*((Sqrt(eps)*(0.6366197723675814 - 1.2732395447351628*x))/(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)) - 0.3141592653589793*lambda*cos(lambda*M_PI*y)*sin(lambda*M_PI*x)) - 5*(-1 + x)*x*(2. - 1.2732395447351628*ArcTan(1.*Sqrt(eps)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2))) + 0.19999999999999998*cos(lambda*M_PI*x)*cos(lambda*M_PI*y) + (-1 + y)*y* ((4*Sqrt(eps)*(-1 + 4.*eps*Power(-0.5 + y,2)*(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2)) - 1.*eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)))/(M_PI*Power(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2),2)) - 0.9869604401089358*Power(lambda,2)*cos(lambda*M_PI*x)*cos(lambda*M_PI*y)) + 2*(-1 + 2*y)*((Sqrt(eps)*(0.6366197723675814 - 1.2732395447351628*y))/(1 + eps*Power(0.4375 - 1.*x + Power(x,2) - 1.*y + Power(y,2),2)) - 0.3141592653589793*lambda*cos(lambda*M_PI*x)*sin(lambda*M_PI*y)));
}

/// insert face elements between elements of level 0
static void InsertInterfaceElements(TPZGeoMesh * gmesh)
{
    long nel = gmesh->NElements();
    int dim = gmesh->Dimension();
    for (long el = 0; el<nel; el++) {
        TPZGeoEl *gel = gmesh->Element(el);
        if (!gel || gel->Level() != 0 || gel->Dimension() != dim) {
            continue;
        }
        int nsides = gel->NSides();
        for (int is = gel->NCornerNodes(); is<nsides; is++) {
            if (gel->SideDimension(is) != dim -1 ) {
                continue;
            }
            TPZGeoElSide gelside(gel,is);
            TPZGeoElSide neighbour = gelside.Neighbour();
            while (neighbour != gelside) {
                if (neighbour.Element()->Dimension() == dim - 1) {
                    break;
                }
                neighbour = neighbour.Neighbour();
            }
            if (neighbour == gelside) {
                TPZGeoElBC(gelside, 2);
            }
        }
    }
}

/// malha geometrica de grande porte
TPZGeoMesh * MalhaGeomBig(REAL Lx, REAL Ly, REAL Lz, TPZVec<int> &nblocks, int nref)
{
    TPZManVector<REAL,3> x0(3,0.),x1(3,0.);
    x1[0] = Lx;
    x1[1] = Ly;
    x1[2] = 0.;
    TPZManVector<int,2> nx(nblocks);
    TPZGenGrid gengrid(nx,x0,x1);
    TPZGeoMesh * meshresult2d = new TPZGeoMesh;
    gengrid.Read(meshresult2d);
    
    gengrid.SetBC(meshresult2d, 4, -1);
    gengrid.SetBC(meshresult2d, 5, -1);
    gengrid.SetBC(meshresult2d, 6, -1);
    gengrid.SetBC(meshresult2d, 7, -1);
    meshresult2d->SetDimension(2);
    TPZExtendGridDimension extend(meshresult2d,Lz);
    TPZGeoMesh * res3d = extend.ExtendedMesh(1,-2,-2);
    TPZGeoMesh * meshresult3d(res3d);
    meshresult3d->SetDimension(3);
#ifdef PZDEBUG
    {
        TPZCheckGeom check(res3d);
        check.UniformRefine(nref);
        if(check.PerformCheck() != 0){
            DebugStop();
        }
    }
#endif
    
//    InsertInterfaceElements(meshresult3d);
    
    std::ofstream vtkfile("geometry.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(meshresult3d, vtkfile);
    
#ifdef PZDEBUG
    {
        std::ofstream gfile("geometry.txt");
        meshresult3d->Print(gfile);
    }
#endif
    
    return meshresult3d;
}

TPZGeoMesh * CreateGeometricBoxMesh(int nref ,TPZManVector<REAL,2> dx, TPZManVector<REAL,2> dy, TPZManVector<REAL,2> dz){
    
    REAL t=0.0;
    REAL dt;
    int n;
    bool IsTetrahedronMeshQ = false;
    
    int rock =  1;
    
    int bc_W =  -1;
    int bc_E =  -1;
    int bc_S =  -1;
    int bc_N =  -1;
    int bc_B =  -2;
    int bc_T =  -2;
    
    // Creating a 0D element to be extruded
    TPZGeoMesh * GeoMesh0D = new TPZGeoMesh;
    GeoMesh0D->NodeVec().Resize(1);
    TPZGeoNode Node;
    TPZVec<REAL> coors(3,0.0);
    Node.SetCoord(coors);
    Node.SetNodeId(0);
    GeoMesh0D->NodeVec()[0]=Node;
    
    TPZVec<long> Topology(1,0);
    int elid=0;
    
    new TPZGeoElRefPattern < pzgeom::TPZGeoPoint >(elid,Topology,rock,*GeoMesh0D);
    GeoMesh0D->BuildConnectivity();
    GeoMesh0D->SetDimension(0);
    
    TPZHierarquicalGrid CreateGridFrom0D(GeoMesh0D);
    TPZAutoPointer<TPZFunction<STATE> > ParFuncX = new TPZDummyFunction<STATE>(ParametricfunctionX);
    CreateGridFrom0D.SetParametricFunction(ParFuncX);
    CreateGridFrom0D.SetFrontBackMatId(bc_W,bc_E);
    
    dt = dx[0];
    n = int(dx[1]);
    // Computing Mesh extruded along the parametric curve Parametricfunction
    TPZGeoMesh * GeoMesh1D = CreateGridFrom0D.ComputeExtrusion(t, dt, n);
    
    TPZHierarquicalGrid CreateGridFrom1D(GeoMesh1D);
    TPZAutoPointer<TPZFunction<STATE> > ParFuncY = new TPZDummyFunction<STATE>(ParametricfunctionY);
    CreateGridFrom1D.SetParametricFunction(ParFuncY);
    CreateGridFrom1D.SetFrontBackMatId(bc_S,bc_N);
    if(IsTetrahedronMeshQ){
        CreateGridFrom1D.SetTriangleExtrusion();
    }
    
    
    dt = dy[0];
    n = int(dy[1]);
    // Computing Mesh extruded along the parametric curve Parametricfunction2
    TPZGeoMesh * GeoMesh2D = CreateGridFrom1D.ComputeExtrusion(t, dt, n);
    
    TPZHierarquicalGrid CreateGridFrom2D(GeoMesh2D);
    TPZAutoPointer<TPZFunction<STATE> > ParFuncZ = new TPZDummyFunction<STATE>(ParametricfunctionZ);
    CreateGridFrom2D.SetParametricFunction(ParFuncZ);
    CreateGridFrom2D.SetFrontBackMatId(bc_B,bc_T);
    if(IsTetrahedronMeshQ){
        CreateGridFrom2D.SetTriangleExtrusion();
        CreateGridFrom2D.SetTetrahedonExtrusion();
    }
    
    
    dt = dz[0];
    n = int(dz[1]);
    // Computing Mesh extruded along the parametric curve Parametricfunction2
    TPZGeoMesh * gmesh = CreateGridFrom2D.ComputeExtrusion(t, dt, n);
    
    long last_node = gmesh->NNodes() - 1;
    long last_element = gmesh->NElements() - 1;
    long node_id = gmesh->NodeVec()[last_node].Id();
    long element_id = gmesh->Element(last_element)->Id();
    const std::string name("Reservoir box");
    gmesh->SetName(name);
    gmesh->SetMaxNodeId(node_id);
    gmesh->SetMaxElementId(element_id);
    gmesh->SetDimension(3);

    TPZCheckGeom check(gmesh);
    check.UniformRefine(nref);
    
#ifdef PZDEBUG
    {
        if(check.PerformCheck() != 0){
            DebugStop();
        }
    }
#endif
    
    InsertInterfaceElements(gmesh);
    
    std::ofstream vtkfile("geometry.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, vtkfile);
    
#ifdef PZDEBUG
    {
        std::ofstream gfile("geometry.txt");
        gmesh->Print(gfile);
    }
#endif
    
    return gmesh;
}


/** @brief Create a reservoir-box geometry */
TPZGeoMesh * CreateGeometricBoxMesh2D(int nref, TPZManVector<REAL,2> dx, TPZManVector<REAL,2> dy){
    
    REAL t=0.0;
    REAL dt;
    int n;
    bool IsTetrahedronMeshQ = false;
    
    int rock =  1;
    
    int bc_W =  -1;
    int bc_E =  -1;
    int bc_S =  -1;
    int bc_N =  -1;
    
    // Creating a 0D element to be extruded
    TPZGeoMesh * GeoMesh0D = new TPZGeoMesh;
    GeoMesh0D->NodeVec().Resize(1);
    TPZGeoNode Node;
    TPZVec<REAL> coors(3,0.0);
    Node.SetCoord(coors);
    Node.SetNodeId(0);
    GeoMesh0D->NodeVec()[0]=Node;
    
    TPZVec<long> Topology(1,0);
    int elid=0;
    
    new TPZGeoElRefPattern < pzgeom::TPZGeoPoint >(elid,Topology,rock,*GeoMesh0D);
    GeoMesh0D->BuildConnectivity();
    GeoMesh0D->SetDimension(0);
    
    TPZHierarquicalGrid CreateGridFrom0D(GeoMesh0D);
    TPZAutoPointer<TPZFunction<STATE> > ParFuncX = new TPZDummyFunction<STATE>(ParametricfunctionX);
    CreateGridFrom0D.SetParametricFunction(ParFuncX);
    CreateGridFrom0D.SetFrontBackMatId(bc_W,bc_E);
    
    dt = dx[0];
    n = int(dx[1]);
    // Computing Mesh extruded along the parametric curve Parametricfunction
    TPZGeoMesh * GeoMesh1D = CreateGridFrom0D.ComputeExtrusion(t, dt, n);
    
    TPZHierarquicalGrid CreateGridFrom1D(GeoMesh1D);
    TPZAutoPointer<TPZFunction<STATE> > ParFuncY = new TPZDummyFunction<STATE>(ParametricfunctionY);
    CreateGridFrom1D.SetParametricFunction(ParFuncY);
    CreateGridFrom1D.SetFrontBackMatId(bc_S,bc_N);
    if(IsTetrahedronMeshQ){
        CreateGridFrom1D.SetTriangleExtrusion();
    }
    
    
    dt = dy[0];
    n = int(dy[1]);
    // Computing Mesh extruded along the parametric curve Parametricfunction2
    TPZGeoMesh * gmesh = CreateGridFrom1D.ComputeExtrusion(t, dt, n);
    
    
    long last_node = gmesh->NNodes() - 1;
    long last_element = gmesh->NElements() - 1;
    long node_id = gmesh->NodeVec()[last_node].Id();
    long element_id = gmesh->Element(last_element)->Id();
    const std::string name("Reservoir box 2D");
    gmesh->SetName(name);
    gmesh->SetMaxNodeId(node_id);
    gmesh->SetMaxElementId(element_id);
    gmesh->SetDimension(2);
    
    TPZCheckGeom check(gmesh);
    check.UniformRefine(nref);
    
#ifdef PZDEBUG
    {
        if(check.PerformCheck() != 0){
            DebugStop();
        }
    }
#endif
    
    InsertInterfaceElements(gmesh);
    
    std::ofstream vtkfile("geometry.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, vtkfile);
    
#ifdef PZDEBUG
    {
        std::ofstream gfile("geometry.txt");
        gmesh->Print(gfile);
    }
#endif
    
    return gmesh;
    
}

void ParametricfunctionX(const TPZVec<STATE> &par, TPZVec<STATE> &X)
{
    X[0] = par[0];
    X[1] = 0.0;
    X[2] = 0.0;
}

void ParametricfunctionY(const TPZVec<STATE> &par, TPZVec<STATE> &X)
{
    X[0] = 0.0;
    X[1] = par[0];
    X[2] = 0.0;
}

void ParametricfunctionZ(const TPZVec<STATE> &par, TPZVec<STATE> &X)
{
    X[0] = 0.0;
    X[1] = 0.0;
    X[2] = par[0];
}

TPZCompMesh * CreateHDivMHMMesh(TPZGeoMesh * gmesh, int porder)
{
    int dim = gmesh->Dimension();
    TPZCompMesh * cmeshHDiv = new TPZCompMesh(gmesh);
    cmeshHDiv->SetDimModel(dim);
    cmeshHDiv->SetAllCreateFunctionsHDiv();
    
    TPZVecL2 *matl2 = new TPZVecL2(1);
    cmeshHDiv->InsertMaterialObject(matl2);
    matl2 = new TPZVecL2(2);
    cmeshHDiv->InsertMaterialObject(matl2);
    
    TPZFNMatrix<1,STATE> val1(1,1,0.),val2(1,1,0.);
    
    TPZBndCond *bc = matl2->CreateBC(matl2, -1, 0, val1, val2);
    cmeshHDiv->InsertMaterialObject(bc);
    
    bc = matl2->CreateBC(matl2, -2, 0, val1, val2);
    cmeshHDiv->InsertMaterialObject(bc);
    
    cmeshHDiv->SetDefaultOrder(porder);
    cmeshHDiv->AutoBuild();
    
//    cmeshHDiv->SetDefaultOrder(porder);
//    std::set<int> mat_skeleton;
//    mat_skeleton.insert(2);
//    cmeshHDiv->AutoBuild(mat_skeleton);
//    
//    cmeshHDiv->SetDefaultOrder(porder);
//    std::set<int> mat_vol;
//    mat_vol.insert(1);
//    mat_vol.insert(-1);
//    mat_vol.insert(-2);
//    cmeshHDiv->AutoBuild(mat_vol);
    

#ifdef PZDEBUG
    {
        std::ofstream outmesh("CmeshFlux.txt");
        cmeshHDiv->Print(outmesh);
    }
#endif
    return cmeshHDiv;
}

void DuplicateNeighbouringConnects(TPZCompMesh * HDivMesh)
{
    TPZGeoMesh *gmesh = HDivMesh->Reference();
    int dim = gmesh->Dimension();
    gmesh->ResetReference();
    HDivMesh->LoadReferences();
    HDivMesh->ComputeNodElCon();
    long nel = HDivMesh->NElements();
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = HDivMesh->Element(el);
        TPZGeoEl *gel = cel->Reference();
        if (!gel || gel->Dimension() != dim) {
            continue;
        }
        int nc = cel->NConnects();
        for (int ic =0; ic<nc; ic++) {
            TPZConnect &c = cel->Connect(ic);
            if (c.HasDependency() && c.NElConnected() == 2)
            {
                // duplicate the connect
                long cindex = HDivMesh->AllocateNewConnect(c);
                TPZConnect &newc = HDivMesh->ConnectVec()[cindex];
                newc = c;
                c.DecrementElConnected();
                newc.DecrementElConnected();
                cel->SetConnectIndex(ic, cindex);
            }
        }
    }
    HDivMesh->ExpandSolution();
}

TPZCompMesh * CreatePressureMHMMesh(TPZGeoMesh * gmesh, int porder)
{
    int dim = gmesh->Dimension();
    TPZCompMesh * cmeshPressure = new TPZCompMesh(gmesh);
    cmeshPressure->SetDimModel(dim);
    cmeshPressure->ApproxSpace().SetAllCreateFunctionsContinuous();
    cmeshPressure->ApproxSpace().CreateDisconnectedElements(true);
    cmeshPressure->SetDefaultOrder(porder);
    TPZMatLaplacian *matl2 = new TPZMatLaplacian(1);
    matl2->SetDimension(dim);
    cmeshPressure->InsertMaterialObject(matl2);
    cmeshPressure->AutoBuild();
    long nc = cmeshPressure->NConnects();
    for (long ic=0; ic<nc; ic++) {
        cmeshPressure->ConnectVec()[ic].SetLagrangeMultiplier(1);
    }
    
#ifdef PZDEBUG
    {
        std::ofstream outmesh("CmeshPressure.txt");
        cmeshPressure->Print(outmesh);
    }
#endif
    
    return cmeshPressure;
}

TPZCompMesh * CreateHDivPressureMHMMesh(TPZVec<TPZCompMesh * > & cmeshes)
{
    TPZGeoMesh *gmesh = cmeshes[0]->Reference();
    if(!gmesh)
    {
        std::cout<< "Geometric mesh doesn't exist" << std::endl;
        DebugStop();
    }
    int dim = gmesh->Dimension();
    
    const int typeFlux = 1, typePressure = 0;
    TPZFMatrix<STATE> val1(1,1,0.), val2Flux(1,1,0.), val2Pressure(1,1,0.);
    
    // Malha computacional
    TPZCompMesh * MixedFluxPressureCmesh = new TPZCompMesh(gmesh);
    
    // Material medio poroso
    TPZMixedPoisson * mat = new TPZMixedPoisson(1,dim);
//    mat->SetSymmetric();
    //    mat->SetForcingFunction(One);
    MixedFluxPressureCmesh->InsertMaterialObject(mat);
    
    // Bc N
    TPZBndCond * bcN = mat->CreateBC(mat, -1, typePressure, val1, val2Pressure);
    TPZAutoPointer<TPZFunction<STATE> > force = new TPZDummyFunction<STATE>(DirichletValidacao);
    bcN->SetBCForcingFunction(0, force);
    MixedFluxPressureCmesh->InsertMaterialObject(bcN);
    
    // Bc S
    TPZBndCond * bcS = mat->CreateBC(mat, -2, typeFlux, val1, val2Flux);
//    bcN->SetBCForcingFunction(0, force);
    MixedFluxPressureCmesh->InsertMaterialObject(bcS);
    
    MixedFluxPressureCmesh->SetDimModel(dim);
    MixedFluxPressureCmesh->SetAllCreateFunctionsMultiphysicElem();
    MixedFluxPressureCmesh->AutoBuild();
    
    TPZManVector<TPZCompMesh * ,2> meshvector(2);
    
    meshvector[0] = cmeshes[0];
    meshvector[1] = cmeshes[1];
    
    // Transferindo para a multifisica
    TPZBuildMultiphysicsMesh::AddElements(meshvector, MixedFluxPressureCmesh);
    TPZBuildMultiphysicsMesh::AddConnects(meshvector, MixedFluxPressureCmesh);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvector, MixedFluxPressureCmesh);
    
#ifdef PZDEBUG
    {
        std::ofstream outmesh("CmeshMixed.txt");
        MixedFluxPressureCmesh->Print(outmesh);
    }
#endif
    
    return MixedFluxPressureCmesh;

}

void HideTheElements(TPZCompMesh * Multiphysics, bool KeepOneLagrangian)
{
    typedef std::set<long> TCompIndexes;
    std::map<long, TCompIndexes> ElementGroups;
    TPZGeoMesh *gmesh = Multiphysics->Reference();
    int dim = gmesh->Dimension();
    gmesh->ResetReference();
    Multiphysics->LoadReferences();
    long nelg = gmesh->NElements();
    for (long el=0; el<nelg; el++) {
        TPZGeoEl *gel = gmesh->Element(el);
        if (gel->Father() != NULL) {
            continue;
        }
        if (gel->Dimension() == dim - 1 && gel->MaterialId() > 0) {
            continue;
        }
        long mapindex = gel->Index();
        if (gel->Dimension() == dim - 1) {
            TPZGeoElSide neighbour = gel->Neighbour(gel->NSides()-1);
            if (neighbour.Element()->Dimension() != dim) {
                DebugStop();
            }
            mapindex= neighbour.Element()->Index();
        }
        TPZStack<TPZCompElSide> highlevel;
        TPZGeoElSide gelside(gel,gel->NSides()-1);
        gelside.HigherLevelCompElementList3(highlevel, 0, 0);
        long nelst = highlevel.size();
        for (long elst=0; elst<nelst; elst++) {
            ElementGroups[mapindex].insert(highlevel[elst].Element()->Index());
        }
        if (gel->Reference()) {
            if (nelst) {
                DebugStop();
            }
            ElementGroups[mapindex].insert(gel->Reference()->Index());
        }
    }
    std::cout << "Number of element groups " << ElementGroups.size() << std::endl;
//    std::map<long,TCompIndexes>::iterator it;
//    for (it=ElementGroups.begin(); it != ElementGroups.end(); it++) {
//        std::cout << "Group " << it->first << " group size " << it->second.size() << std::endl;
//        std::cout << " elements ";
//        std::set<long>::iterator its;
//        for (its = it->second.begin(); its != it->second.end(); its++) {
//            std::cout << *its << " ";
//        }
//        std::cout << std::endl;
//    }
    
    std::set<long> submeshindices;
    TPZCompMeshTools::PutinSubmeshes(Multiphysics, ElementGroups, submeshindices, KeepOneLagrangian);
    std::cout << "After putting in substructures\n";
    
    Multiphysics->ComputeNodElCon();
    Multiphysics->CleanUpUnconnectedNodes();
    for (std::set<long>::iterator it=submeshindices.begin(); it != submeshindices.end(); it++) {
        TPZCompEl *cel = Multiphysics->Element(*it);
        TPZSubCompMesh *subcmesh = dynamic_cast<TPZSubCompMesh *>(cel);
        if (!subcmesh) {
            DebugStop();
        }
        TPZCompMeshTools::GroupElements(subcmesh);
        subcmesh->ComputeNodElCon();
        TPZCompMeshTools::CreatedCondensedElements(subcmesh, KeepOneLagrangian);
        subcmesh->SetAnalysisSkyline(16, 0, 0);
    }
    Multiphysics->ComputeNodElCon();
    Multiphysics->CleanUpUnconnectedNodes();
    std::cout << "Finished substructuring\n";
}
