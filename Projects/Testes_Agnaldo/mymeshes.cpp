//
//  mymeshes.cpp
//  PZ
//
//  Created by Agnaldo Farias on 29/04/13.
//
//

#include "mymeshes.h"

#include "pzgeoelbc.h"
#include "TPZCompElDisc.h"
#include "pzpoisson3d.h"
#include "pzelasmat.h"
#include "pzgeoelbc.h"


DadosMalhas::DadosMalhas(){
    
    //dados do material
    fEyoung = 0.;
    fpoisson= 0.;
    falpha = 0.;
    fSe = 0.;
    fperm = 0.;
    fvisc = 0.;
    ffx = 0.;
    ffy = 0.;
    fsign = 0.;
    
    fpref= 0.;
    fLref = 0.;
    fkovervisc = 0.;
    
    fvalsourceterm = 0.;
    
    //dados da malha geometrica
    fmatId =1;
    fbcBottom = -1;
    fbcRight = -2;
    fbcTop = -3;
    fbcLeft = -4;
    fbcSourceTerm = 2;
    
    
    fdirichlet =0;
    fneumann = 1;
    fneumdir=10;
    fdirfreey_neum=300;
    fdirneum = 1;
    fmixedneum = 21;
}

DadosMalhas::~DadosMalhas(){
    
}

DadosMalhas::DadosMalhas(const DadosMalhas &copy){
    
    this->operator=(copy);
}

DadosMalhas & DadosMalhas::operator=(const DadosMalhas &copy){
    
    fEyoung  = copy. fEyoung;
	fpoisson = copy.fpoisson;
	falpha  = copy.falpha;
    fSe  = copy.fSe;
    fperm = copy.fperm ;
    fvisc = copy.fvisc;
    ffx = copy.ffx;
    ffy = copy.ffy;
    fsign = copy.fsign;
    
    
    fmatId = copy.fmatId;
    fbcBottom = copy.fbcBottom;
    fbcRight = copy.fbcRight;
    fbcTop = copy.fbcTop;
    fbcLeft = copy.fbcLeft;
    fbcSourceTerm = copy.fbcSourceTerm;
    
    fdirichlet = copy.fdirichlet;
    fneumann = copy.fneumann;
    fneumdir = copy.fneumdir;
    fdirfreey_neum = copy.fdirfreey_neum;
    fdirneum = copy.fdirneum;
    fmixedneum = copy.fmixedneum;
    
    fpref= copy.fpref;
    fLref = copy.fLref;
    fkovervisc = copy.fkovervisc;
    
    fvalsourceterm = copy.fvalsourceterm;
    
	return *this;
}


TPZGeoMesh *DadosMalhas::GMesh(bool triang_elements, REAL L, REAL w){
    
    int Qnodes = 4;
	
	TPZGeoMesh * gmesh = new TPZGeoMesh;
	gmesh->SetMaxNodeId(Qnodes-1);
	gmesh->NodeVec().Resize(Qnodes);
	TPZVec<TPZGeoNode> Node(Qnodes);
	
	TPZVec <int> TopolQuad(4);
    TPZVec <int> TopolTriang(3);
	TPZVec <int> TopolLine(2);
    TPZVec <int> TopolPoint(1);
	
	//indice dos nos
	int id = 0;
	REAL valx;
	for(int xi = 0; xi < Qnodes/2; xi++)
	{
		valx = xi*L;
		Node[id].SetNodeId(id);
		Node[id].SetCoord(0 ,valx );//coord X
		Node[id].SetCoord(1 ,0. );//coord Y
		gmesh->NodeVec()[id] = Node[id];
		id++;
	}
	
	for(int xi = 0; xi < Qnodes/2; xi++)
	{
		valx = L - xi*L;
		Node[id].SetNodeId(id);
		Node[id].SetCoord(0 ,valx );//coord X
		Node[id].SetCoord(1 ,w);//coord Y
		gmesh->NodeVec()[id] = Node[id];
		id++;
	}
	
	//indice dos elementos
	id = 0;
    
    if(triang_elements==true)
    {
        TopolTriang[0] = 0;
        TopolTriang[1] = 1;
        TopolTriang[2] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle> (id,TopolTriang,fmatId,*gmesh);
        id++;
        
        TopolTriang[0] = 2;
        TopolTriang[1] = 1;
        TopolTriang[2] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle> (id,TopolTriang,fmatId,*gmesh);
        id++;
        
        TopolLine[0] = 0;
        TopolLine[1] = 1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcBottom,*gmesh);
        id++;
        
        TopolLine[0] = 2;
        TopolLine[1] = 1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcRight,*gmesh);
        id++;
        
        TopolLine[0] = 3;
        TopolLine[1] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcTop,*gmesh);
        id++;
        
        TopolLine[0] = 3;
        TopolLine[1] = 0;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcLeft,*gmesh);
//        
//        TopolPoint[0] = 1;
//        new TPZGeoElRefPattern< pzgeom::TPZGeoPoint > (id,TopolLine,fmatId+1,*gmesh);
    }
    else{
        TopolQuad[0] = 0;
        TopolQuad[1] = 1;
        TopolQuad[2] = 2;
        TopolQuad[3] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,fmatId,*gmesh);
        id++;
        
        TopolLine[0] = 0;
        TopolLine[1] = 1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcBottom,*gmesh);
        id++;
        
        TopolLine[0] = 1;
        TopolLine[1] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcRight,*gmesh);
        id++;
        
        TopolLine[0] = 2;
        TopolLine[1] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcTop,*gmesh);
        id++;
        
        TopolLine[0] = 3;
        TopolLine[1] = 0;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcLeft,*gmesh);
    }
    
	gmesh->BuildConnectivity();
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Geometrica Inicial\n ";
    //        gmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
	return gmesh;
}

TPZGeoMesh * DadosMalhas::GMesh2(REAL L, REAL w){
    
    int Qnodes = 16;
	
	TPZGeoMesh * gmesh = new TPZGeoMesh;
	gmesh->SetMaxNodeId(Qnodes-1);
	gmesh->NodeVec().Resize(Qnodes);
	TPZVec<TPZGeoNode> Node(Qnodes);
	
	TPZVec <int> TopolQuad(4);
	TPZVec <int> TopolLine(2);
	
	//indice dos nos
	int id = 0;
	REAL valx;
    REAL valy;
    
	for(int xi = 0; xi < 4; xi++)
	{
        valy = 0.;
        if(xi < 2) valx = xi*L/2;
        else valx = xi*L/4 + L/4;
        
        Node[id].SetNodeId(id);
        Node[id].SetCoord(0 ,valx);//coord X
        Node[id].SetCoord(1 ,valy);//coord Y
        gmesh->NodeVec()[id] = Node[id];
        id++;
	}
    
    for(int xi = 0; xi < 4; xi++)
	{
        valy = w/4;
        if(xi < 2) valx = xi*L/2;
        else valx = xi*L/4 + L/4;
        
        Node[id].SetNodeId(id);
        Node[id].SetCoord(0 ,valx);//coord X
        Node[id].SetCoord(1 ,valy);//coord Y
        gmesh->NodeVec()[id] = Node[id];
        id++;
	}
    
    for(int xi = 0; xi < 4; xi++)
	{
        valy = w/2;
        if(xi < 2) valx = xi*L/2;
        else valx = xi*L/4 + L/4;
        
        Node[id].SetNodeId(id);
        Node[id].SetCoord(0 ,valx);//coord X
        Node[id].SetCoord(1 ,valy);//coord Y
        gmesh->NodeVec()[id] = Node[id];
        id++;
	}
    
    for(int xi = 0; xi < 4; xi++)
	{
        valy = w;
        if(xi < 2) valx = xi*L/2;
        else valx = xi*L/4 + L/4;
        
        Node[id].SetNodeId(id);
        Node[id].SetCoord(0 ,valx);//coord X
        Node[id].SetCoord(1 ,valy);//coord Y
        gmesh->NodeVec()[id] = Node[id];
        id++;
	}
	
	
	//indice dos elementos
	id = 0;
    for(int iel = 0; iel<3; iel ++){
        TopolQuad[0] = iel;
        TopolQuad[1] = iel+1;
        TopolQuad[2] = iel+5;
        TopolQuad[3] = iel+4;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,fmatId,*gmesh);
        id++;
    }
    
    for(int iel = 0; iel<3; iel ++){
        TopolQuad[0] = iel+4;
        TopolQuad[1] = iel+5;
        TopolQuad[2] = iel+9;
        TopolQuad[3] = iel+8;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,fmatId,*gmesh);
        id++;
    }
    
    for(int iel = 0; iel<3; iel ++){
        TopolQuad[0] = iel+8;
        TopolQuad[1] = iel+9;
        TopolQuad[2] = iel+13;
        TopolQuad[3] = iel+12;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,fmatId,*gmesh);
        id++;
    }
     
    for(int iel = 0; iel<3; iel ++){
        TopolLine[0] = iel;
        TopolLine[1] = iel+1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcBottom,*gmesh);
        id++;
    }
    
    for(int iel = 0; iel<3; iel ++){
        TopolLine[0] = iel*4+3;
        TopolLine[1] = iel*4+7;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcRight,*gmesh);
        id++;
    }
    
    for(int iel = 0; iel<3; iel ++){
        TopolLine[0] = 15-iel;
        TopolLine[1] = 14-iel;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcTop,*gmesh);
        id++;
    }
    
    for(int iel = 0; iel<3; iel ++){
        TopolLine[0] = 12-iel*4;
        TopolLine[1] = 8-iel*4;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,fbcLeft,*gmesh);
        id++;
    }
    
    
	gmesh->BuildConnectivity();
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Geometrica Inicial\n ";
    //        gmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    return gmesh;
}


TPZGeoMesh *DadosMalhas::GMesh3(REAL L, REAL w){
    
    
    TPZGeoMesh *gmesh = new TPZGeoMesh();
	REAL co[9][2] = {{0.,0.},{L/2,0.},{L,0.},{0.,w/2},{L/2,w/2},{L,w/2},{0.,w},{L/2,w},{L,w}};
	int indices[1][9] = {{0,1,2,3,4,5,6,7,8}};
	
	int nnode = 9;
	const int nelem = 5;
	TPZGeoEl *elvec[nelem];
	int nod;
	for ( nod=0; nod<nnode; nod++ )
	{
		int nodind = gmesh->NodeVec().AllocateNewElement();
		TPZVec<REAL> coord ( 2 );
		coord[0] = co[nod][0];
		coord[1] = co[nod][1];
		gmesh->NodeVec() [nodind].Initialize ( nod,coord,*gmesh );
	}

    int el;
	for ( el=0; el<nelem; el++ )
	{
        if(el!=1 && el!=2){
            TPZVec<int> nodind(4);
            nodind[0] = el;
            nodind[1] = el + 1;
            nodind[2] = el + 4;
            nodind[3] = el + 3;
            elvec[el] = gmesh->CreateGeoElement(EQuadrilateral,nodind,fmatId,el);
        }
        else if (el==1){
            TPZVec<int> nodind(3);
            nodind[0] = 1;
            nodind[1] = 2;
            nodind[2] = 4;
            elvec[el] = gmesh->CreateGeoElement(ETriangle,nodind,fmatId,el);
        }else{
            TPZVec<int> nodind(3);
            nodind[0] = 5;
            nodind[1] = 2;
            nodind[2] = 4;
            elvec[el] = gmesh->CreateGeoElement(ETriangle,nodind,fmatId,el);
        }
	}
    
    gmesh->BuildConnectivity();
    
    //Cricao das condicoes de contorno
    TPZGeoElBC gbc1(elvec[0], 4,fbcBottom);
    TPZGeoElBC gbc2(elvec[1], 3,fbcBottom);
    TPZGeoElBC gbc3(elvec[2], 3,fbcRight);
    TPZGeoElBC gbc4(elvec[4], 5,fbcRight);
    TPZGeoElBC gbc5(elvec[4], 6,fbcTop);
    TPZGeoElBC gbc6(elvec[3], 6,fbcTop);
    TPZGeoElBC gbc7(elvec[3], 7,fbcLeft);
    TPZGeoElBC gbc8(elvec[0], 7,fbcLeft);
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Geometrica Inicial\n ";
    //        gmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
	return gmesh;
}

void DadosMalhas::UniformRefine(TPZGeoMesh* gmesh, int nDiv)
{
    for(int D = 0; D < nDiv; D++)
    {
        int nels = gmesh->NElements();
        for(int elem = 0; elem < nels; elem++)
        {
            TPZVec< TPZGeoEl * > filhos;
            TPZGeoEl * gel = gmesh->ElementVec()[elem];
            gel->Divide(filhos);
        }
    }
    //	gmesh->BuildConnectivity();
}


TPZCompMesh* DadosMalhas:: MalhaCompElast(TPZGeoMesh * gmesh,int pOrder)
{
    /// criar material
	int dim = 2;
	TPZVec<REAL> force(dim,0.);
    REAL E = 0;
	REAL poisson = 0;
    int planestress = -1;
	TPZElasticityMaterial *material;
	material = new TPZElasticityMaterial(fmatId, E, poisson, force[0], force[1], planestress);
	TPZMaterial * mat(material);
    
    ///criar malha computacional
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(pOrder);
	cmesh->SetDimModel(dim);
    cmesh->InsertMaterialObject(mat);
    
	///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(2,2,0.), val2(2,1,0.);
    TPZMaterial * BCond1 = material->CreateBC(mat, fbcBottom,fdirichlet, val1, val2);
    TPZMaterial * BCond2 = material->CreateBC(mat, fbcLeft,fdirichlet, val1, val2);
    TPZMaterial * BCond3 = material->CreateBC(mat, fbcTop,fdirichlet, val1, val2);
    TPZMaterial * BCond4 = material->CreateBC(mat, fbcRight,fdirichlet, val1, val2);
    
    cmesh->SetAllCreateFunctionsContinuous();
	cmesh->InsertMaterialObject(mat);
    cmesh->InsertMaterialObject(BCond1);
	cmesh->InsertMaterialObject(BCond2);
	cmesh->InsertMaterialObject(BCond3);
	cmesh->InsertMaterialObject(BCond4);
	
	
	//Ajuste da estrutura de dados computacional
	cmesh->AutoBuild();
	cmesh->AdjustBoundaryElements();
	cmesh->CleanUpUnconnectedNodes();
    
	return cmesh;
}

TPZCompMesh *DadosMalhas::CMeshFlux(TPZGeoMesh *gmesh, int pOrder)
{
    /// criar materiais
	int dim = 2;
	TPZMatPoisson3d *material;
	material = new TPZMatPoisson3d(fmatId,dim);
	TPZMaterial * mat(material);
	material->NStateVariables();
	
	TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
	cmesh->SetDefaultOrder(pOrder);
    cmesh->SetDimModel(dim);
    cmesh->InsertMaterialObject(mat);
    
    ///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(2,2,0.), val2(2,1,0.);
	TPZMaterial * BCond1 = material->CreateBC(mat, fbcBottom,fdirichlet, val1, val2);
    TPZMaterial * BCond2 = material->CreateBC(mat, fbcRight,fdirichlet, val1, val2);
    TPZMaterial * BCond3 = material->CreateBC(mat, fbcTop,fdirichlet, val1, val2);
    TPZMaterial * BCond4 = material->CreateBC(mat, fbcLeft,fdirichlet, val1, val2);
    
	cmesh->SetAllCreateFunctionsHDiv();
    cmesh->InsertMaterialObject(BCond1);
    cmesh->InsertMaterialObject(BCond2);
    cmesh->InsertMaterialObject(BCond3);
    cmesh->InsertMaterialObject(BCond4);
    
	//Ajuste da estrutura de dados computacional
	cmesh->AutoBuild();
    cmesh->AdjustBoundaryElements();
	cmesh->CleanUpUnconnectedNodes();
    
	return cmesh;
}

TPZCompMesh *DadosMalhas::CMeshPressure(TPZGeoMesh *gmesh, int pOrder, bool triang)
{
    /// criar materiais
	int dim = 2;
	TPZMatPoisson3d *material;
	material = new TPZMatPoisson3d(fmatId,dim);
	material->NStateVariables();
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDimModel(dim);
    TPZMaterial * mat(material);
    cmesh->InsertMaterialObject(mat);
    
    ///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(2,2,0.), val2(2,1,0.);
	TPZMaterial * BCond1 = material->CreateBC(mat, fbcBottom,fdirichlet, val1, val2);
    TPZMaterial * BCond2 = material->CreateBC(mat, fbcRight,fdirichlet, val1, val2);
    TPZMaterial * BCond3 = material->CreateBC(mat, fbcTop,fdirichlet, val1, val2);
    TPZMaterial * BCond4 = material->CreateBC(mat, fbcLeft,fdirichlet, val1, val2);
    
	cmesh->SetAllCreateFunctionsDiscontinuous();
    
    cmesh->InsertMaterialObject(BCond1);
    cmesh->InsertMaterialObject(BCond2);
    cmesh->InsertMaterialObject(BCond3);
    cmesh->InsertMaterialObject(BCond4);
    
	cmesh->SetDefaultOrder(pOrder);
    cmesh->SetDimModel(dim);
	
	//Ajuste da estrutura de dados computacional
	cmesh->AutoBuild();
    
    ///inserir connect da pressao
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        newnod.SetPressure(true);
    }
    
    ///set order total da shape
    int nel = cmesh->NElements();
    for(int i=0; i<nel; i++){
        TPZCompEl *cel = cmesh->ElementVec()[i];
        TPZCompElDisc *celdisc = dynamic_cast<TPZCompElDisc *>(cel);
        celdisc->SetConstC(1.);
        celdisc->SetCenterPoint(0, 0.);
        celdisc->SetCenterPoint(1, 0.);
        celdisc->SetCenterPoint(2, 0.);
        celdisc->SetTrueUseQsiEta();
        if(celdisc && celdisc->Reference()->Dimension() == cmesh->Dimension())
        {
            if(triang==true || celdisc->Reference()->Type()==ETriangle) celdisc->SetTotalOrderShape();
            else celdisc->SetTensorialShape();
        }
    }
    
    
#ifdef DEBUG
    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }
#endif
    
    cmesh->AdjustBoundaryElements();
	cmesh->CleanUpUnconnectedNodes();
	return cmesh;
}

TPZCompMesh *DadosMalhas::MalhaCompMultphysics(TPZGeoMesh * gmesh, TPZVec<TPZCompMesh *> meshvec, TPZPoroElasticMF2d * &mymaterial,TPZAutoPointer<TPZFunction<STATE> > solExata){
    
    //Creating computational mesh for multiphysic elements
	gmesh->ResetReference();
	TPZCompMesh *mphysics = new TPZCompMesh(gmesh);
    
    int MatId=fmatId;
    int dim =2;
    
    //	criar material
    int planestress = 0; // This is a Plain strain problem
    mymaterial = new TPZPoroElasticMF2d(MatId,dim);
    mymaterial->SetfPlaneProblem(planestress);
    
    mymaterial->SetParameters(fperm, fvisc);
    mymaterial->SetElasticityParameters(fEyoung, fpoisson, falpha, fSe, ffx, ffy);
    
    TPZMaterial *mat(mymaterial);
    mphysics->InsertMaterialObject(mat);
    
	mymaterial->SetForcingFunctionExact(solExata);
    
    ///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(3,2,0.), val2(3,1,0.);
    
    REAL sig0 = fsign;
    REAL ptop = 0.;
    val2(0,0)= 0.;
    val2(1,0)= sig0;
    val2(2,0)= ptop;
	TPZMaterial * BCond1 = mymaterial->CreateBC(mat, fbcTop,fneumdir, val1, val2);
    
    val2.Redim(3,1);
    REAL big = mymaterial->gBigNumber;
    val1(0,0) = big;
    TPZMaterial * BCond2 = mymaterial->CreateBC(mat,fbcRight, fmixedneum, val1, val2);
    TPZMaterial * BCond4 = mymaterial->CreateBC(mat,fbcLeft, fmixedneum, val1, val2);
    
    val1.Redim(3,2);
    val2.Redim(3,1);
    //val2(2,0)= piniD;
    TPZMaterial * BCond3 = mymaterial->CreateBC(mat,fbcBottom,fdirneum, val1, val2);
    
    mphysics->SetAllCreateFunctionsMultiphysicElem();
    mphysics->InsertMaterialObject(BCond1);
    mphysics->InsertMaterialObject(BCond2);
    mphysics->InsertMaterialObject(BCond3);
    mphysics->InsertMaterialObject(BCond4);
    
    mphysics->AutoBuild();
	mphysics->AdjustBoundaryElements();
	mphysics->CleanUpUnconnectedNodes();
    
    // Creating multiphysic elements into mphysics computational mesh
	TPZBuildMultiphysicsMesh::AddElements(meshvec, mphysics);
	TPZBuildMultiphysicsMesh::AddConnects(meshvec,mphysics);
	TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvec, mphysics);
    
    return mphysics;
}

TPZCompMesh *DadosMalhas::MalhaCompTerzaghi(TPZGeoMesh * gmesh, TPZVec<TPZCompMesh *> meshvec, TPZPoroElasticMF2d * &mymaterial,TPZAutoPointer<TPZFunction<STATE> > solExata){
    
    //Creating computational mesh for multiphysic elements
	gmesh->ResetReference();
	TPZCompMesh *mphysics = new TPZCompMesh(gmesh);
    
    int MatId=fmatId;
    int dim =2;
    
    int planestress = 0; // This is a Plain strain problem
    
    
    mymaterial = new TPZPoroElasticMF2d(MatId,dim);
    
    mymaterial->SetfPlaneProblem(planestress);
    mymaterial->SetParameters(fperm, fvisc);
    mymaterial->SetElasticityParameters(fEyoung, fpoisson, falpha, fSe, ffx, ffy);
    
    TPZMaterial *mat(mymaterial);
    mphysics->InsertMaterialObject(mat);
    
	mymaterial->SetForcingFunctionExact(solExata);
    
    ///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(3,2,0.), val2(3,1,0.);
    
    REAL sig0 = fsign;
    REAL ptop = 0.;
    val2(0,0)= 0.;
    val2(1,0)= sig0;
    val2(2,0)= ptop;
	TPZMaterial * BCond1 = mymaterial->CreateBC(mat, fbcTop,fneumdir, val1, val2);
    
    val2.Redim(3,1);
    REAL big = mymaterial->gBigNumber;
    val1(0,0) = big;
    TPZMaterial * BCond2 = mymaterial->CreateBC(mat,fbcRight, fmixedneum, val1, val2);
    TPZMaterial * BCond4 = mymaterial->CreateBC(mat,fbcLeft, fmixedneum, val1, val2);
    
    val1.Redim(3,2);
    val2.Redim(3,1);
    TPZMaterial * BCond3 = mymaterial->CreateBC(mat,fbcBottom,fdirneum, val1, val2);
    
    mphysics->SetAllCreateFunctionsMultiphysicElem();
    mphysics->InsertMaterialObject(BCond1);
    mphysics->InsertMaterialObject(BCond2);
    mphysics->InsertMaterialObject(BCond3);
    mphysics->InsertMaterialObject(BCond4);
    
    mphysics->AutoBuild();
	mphysics->AdjustBoundaryElements();
	mphysics->CleanUpUnconnectedNodes();
    
    // Creating multiphysic elements into mphysics computational mesh
	TPZBuildMultiphysicsMesh::AddElements(meshvec, mphysics);
	TPZBuildMultiphysicsMesh::AddConnects(meshvec,mphysics);
	TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvec, mphysics);
    
    return mphysics;
}

TPZCompMesh *DadosMalhas::MalhaCompBarryMercer(TPZGeoMesh * gmesh, TPZVec<TPZCompMesh *> meshvec, TPZPoroElasticMF2d * &mymaterial,TPZAutoPointer<TPZFunction<STATE> > solExata){
    
    //Creating computational mesh for multiphysic elements
	gmesh->ResetReference();
	TPZCompMesh *mphysics = new TPZCompMesh(gmesh);
    
    int MatId=fmatId;
    int dim =2;
    
    int planestress = 0; // This is a Plain strain problem
    
    mymaterial = new TPZPoroElasticMF2d(MatId,dim);
    
    mymaterial->SetfPlaneProblem(planestress);
    mymaterial->SetParameters(fperm, fvisc);
    mymaterial->SetElasticityParameters(fEyoung, fpoisson, falpha, fSe, ffx, ffy);
    mymaterial->SetSourceTerm(fvalsourceterm);
    
    TPZMaterial *mat(mymaterial);
    mphysics->InsertMaterialObject(mat);
    
	mymaterial->SetForcingFunctionExact(solExata);
    
    ///Inserir condicao de contorno
	TPZFMatrix<REAL> val1(3,2,0.), val2(3,1,0.);
    
    TPZMaterial * BCond5 = mymaterial->CreateBC(mat,fbcSourceTerm,3, val1, val2);
    
    mphysics->SetAllCreateFunctionsMultiphysicElem();
    
    mphysics->InsertMaterialObject(BCond5);
    
    mphysics->AutoBuild();
	mphysics->AdjustBoundaryElements();
	mphysics->CleanUpUnconnectedNodes();
    
    // Creating multiphysic elements into mphysics computational mesh
	TPZBuildMultiphysicsMesh::AddElements(meshvec, mphysics);
	TPZBuildMultiphysicsMesh::AddConnects(meshvec,mphysics);
	TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvec, mphysics);
    
    return mphysics;
}


#include "pzbstrmatrix.h"

void DadosMalhas::SolveSist(TPZAnalysis &an, TPZCompMesh *fCmesh)
{
	TPZBandStructMatrix full(fCmesh);
	//TPZSkylineStructMatrix full(fCmesh); //caso simetrico
	an.SetStructuralMatrix(full);
	TPZStepSolver<REAL> step;
    //	step.SetDirect(ELDLt); //caso simetrico
    step.SetDirect(ELU);
	an.SetSolver(step);
	an.Run();
}

TPZAutoPointer <TPZMatrix<REAL> > DadosMalhas::MassMatrix(TPZPoroElasticMF2d * mymaterial, TPZCompMesh* mphysics){
    
    mymaterial->SetLastState();
    //TPZSkylineStructMatrix matsp(mphysics);
	TPZSpStructMatrix matsp(mphysics);
    
	std::set< int > materialid;
	int matid = mymaterial->MatId();
	materialid.insert(matid);
	matsp.SetMaterialIds (materialid);
	TPZAutoPointer<TPZGuiInterface> guiInterface;
	TPZFMatrix<REAL> Un;
    //TPZMatrix<REAL> *matK2 = matsp.CreateAssemble(Un,guiInterface);
    
    TPZAutoPointer <TPZMatrix<REAL> > matK2 = matsp.CreateAssemble(Un,guiInterface);
    
    return matK2;
}

void DadosMalhas::StiffMatrixLoadVec(TPZPoroElasticMF2d *mymaterial, TPZCompMesh* mphysics, TPZAnalysis &an, TPZFMatrix<REAL> &matK1, TPZFMatrix<REAL> &fvec){
    
	mymaterial->SetCurrentState();
    //TPZFStructMatrix matsk(mphysics);
    TPZSkylineStructMatrix matsk(mphysics);
	an.SetStructuralMatrix(matsk);
	TPZStepSolver<REAL> step;
	step.SetDirect(ELDLt);
	//step.SetDirect(ELU);
	an.SetSolver(step);
	an.Run();
	
	matK1 = an.StructMatrix();
	fvec = an.Rhs();
    
    
    
    //    TPZBandStructMatrix full(cmesh);
    //    an.SetStructuralMatrix(full);
    //    TPZStepSolver step;
    //    step.SetDirect(ELU);
    //    an.SetSolver(step);
}

void DadosMalhas::PosProcessMultphysics(TPZVec<TPZCompMesh *> meshvec, TPZCompMesh* mphysics, TPZAnalysis &an, std::string plotfile)
{
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(meshvec, mphysics);
    
	TPZManVector<std::string,10> scalnames(9), vecnames(4);
	scalnames[0] = "DisplacementX";
	scalnames[1] = "DisplacementY";
    scalnames[2] = "SigmaX";
	scalnames[3] = "SigmaY";
	scalnames[4] = "PressureStress";
	scalnames[5] = "PorePressure";
    
	vecnames[0]  = "Displacement";
	vecnames[1]  = "Fluxo";
	vecnames[2]  = "MinusKMuGradP";
    
    scalnames[6] = "ExactPressure";
    scalnames[7] = "ExactDisplacementY";
    scalnames[8] = "ExactSigmaY";
    vecnames[3]  = "ExactFluxo";
	
	const int dim = 2;
	int div =0;
	an.DefineGraphMesh(dim,scalnames,vecnames,plotfile);
	an.PostProcess(div,dim);
	std::ofstream out("malha.txt");
	an.Print("nothing",out);
}

void DadosMalhas::SolveSistTransient(REAL deltaT,REAL maxTime, TPZPoroElasticMF2d * &mymaterial ,TPZVec<TPZCompMesh *> meshvec, TPZCompMesh* mphysics, int ntimestep, REAL &timeatual){
	
    
    TPZAnalysis an(mphysics);
	TPZFMatrix<REAL> Initialsolution = an.Solution();
    
    std::string outputfile;
	outputfile = "TransientSolution";
    
    std::stringstream outputfiletemp;
    outputfiletemp << outputfile << ".vtk";
    std::string plotfile = outputfiletemp.str();
    PosProcessMultphysics(meshvec,mphysics,an,plotfile);
    
    //Criando matriz de massa (matM)
    TPZAutoPointer <TPZMatrix<REAL> > matM = MassMatrix(mymaterial, mphysics);
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //            std::stringstream sout;
    //        	matM->Print("matM = ", sout,EMathematicaInput);
    //        	LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
    //Criando matriz de rigidez (matK) e vetor de carga
	TPZFMatrix<REAL> matK;
	TPZFMatrix<REAL> fvec;
    StiffMatrixLoadVec(mymaterial, mphysics, an, matK, fvec);
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //
    //        std::stringstream sout;
    //        matK.Print("matK = ", sout,EMathematicaInput);
    //        fvec.Print("fvec = ", sout,EMathematicaInput);
    //        //Print the temporal solution
    //        Initialsolution.Print("Intial conditions = ", sout,EMathematicaInput);
    //        TPZFMatrix<REAL> Temp;
    //        TPZFMatrix<REAL> Temp2;
    //        matM->Multiply(Initialsolution,Temp);
    //        Temp.Print("Temp matM = ", sout,EMathematicaInput);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
    
	int nrows;
	nrows = matM->Rows();
	TPZFMatrix<REAL> TotalRhs(nrows,1,0.0);
	TPZFMatrix<REAL> TotalRhstemp(nrows,1,0.0);
	TPZFMatrix<REAL> Lastsolution = Initialsolution;
	
	REAL TimeValue = 0.0;
	int cent = 1;
	TimeValue = cent*deltaT;
	while (TimeValue <= maxTime)
	{
        timeatual  = TimeValue;
		// This time solution i for Transient Analytic Solution
		mymaterial->SetTimeValue(TimeValue);
		matM->Multiply(Lastsolution,TotalRhstemp);
        
        //#ifdef LOG4CXX
        //        if(logdata->isDebugEnabled())
        //        {
        //            std::stringstream sout;
        //            sout<< " tempo = " << cent;
        //            Lastsolution.Print("\nIntial conditions = ", sout,EMathematicaInput);
        //            TotalRhstemp.Print("Mat Mass x Last solution = ", sout,EMathematicaInput);
        //            LOGPZ_DEBUG(logdata,sout.str())
        //        }
        //#endif
        
		TotalRhs = fvec + TotalRhstemp;
		an.Rhs() = TotalRhs;
		an.Solve();
		Lastsolution = an.Solution();
		
        if(cent%ntimestep==0){
            std::stringstream outputfiletemp;
            outputfiletemp << outputfile << ".vtk";
            std::string plotfile = outputfiletemp.str();
            PosProcessMultphysics(meshvec,mphysics,an,plotfile);
        }
		
        cent++;
		TimeValue = cent*deltaT;
        
        if(cent == 100){
            deltaT = 1000*deltaT;
            mymaterial->SetTimeStep(deltaT);
            mphysics->Solution().Zero();
            matM = MassMatrix(mymaterial, mphysics);
            StiffMatrixLoadVec(mymaterial, mphysics, an, matK, fvec);
            
        }
	}
}


#include "pzl2projection.h"
TPZCompMesh *DadosMalhas::CMeshPressureL2(TPZGeoMesh *gmesh, int pOrder, TPZVec<STATE> &solini, bool triang)
{
    /// criar materiais
	int dim = 2;
	TPZL2Projection *material;
	material = new TPZL2Projection(fmatId, dim, 1, solini, pOrder);
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDimModel(dim);
    TPZMaterial * mat(material);
    cmesh->InsertMaterialObject(mat);
    
	cmesh->SetAllCreateFunctionsDiscontinuous();
    
	cmesh->SetDefaultOrder(pOrder);
    cmesh->SetDimModel(dim);
	
	//Ajuste da estrutura de dados computacional
	cmesh->AutoBuild();
    
    ///inserir connect da pressao
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        newnod.SetPressure(true);
    }
    
    ///set order total da shape
    int nel = cmesh->NElements();
    for(int i=0; i<nel; i++){
        TPZCompEl *cel = cmesh->ElementVec()[i];
        TPZCompElDisc *celdisc = dynamic_cast<TPZCompElDisc *>(cel);
//        celdisc->SetConstC(1.);
//        celdisc->SetCenterPoint(0, 0.);
//        celdisc->SetCenterPoint(1, 0.);
//        celdisc->SetCenterPoint(2, 0.);
        if(celdisc && celdisc->Reference()->Dimension() == cmesh->Dimension())
        {
            if(triang==true || celdisc->Reference()->Type()==ETriangle) celdisc->SetTotalOrderShape();
            else celdisc->SetTensorialShape();
        }
    }
    
    
#ifdef DEBUG
    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }
#endif
    
	return cmesh;
}

void DadosMalhas::RefiningNearCircunference(int dim,TPZGeoMesh *gmesh,int nref,int ntyperefs) {
	
	int i;
	
	// Refinando no local desejado
	TPZVec<REAL> point(3);
    //	point[0] = point[1] = point[2] = -0.25;
    //	if(dim==1) point[1] = point[2] = 0.0;
    //	else if(dim==2) point[2] = 0.0;
    
    point[0] = 0.;
    point[1] = 1.;
    point[2] = 0.;
	REAL r = sqrt(1.);
	
	if(ntyperefs==2) {
		REAL radius = 0.2;
		for(i=0;i<nref;i+=2) {
			// To refine elements with center near to points than radius
			RefineGeoElements(dim,gmesh,point,r,radius);
			RefineGeoElements(dim,gmesh,point,r,radius);
			if(nref < 5) radius *= 0.35;
			else if(nref < 7) radius *= 0.2;
			else radius *= 0.1;
		}
		if(i==nref) {
			RefineGeoElements(dim,gmesh,point,r,radius);
		}
	}
	else {
		REAL radius = 0.5;
		for(i=0;i<nref+1;i++) {
			// To refine elements with center near to points than radius
			RefineGeoElements(dim,gmesh,point,r,radius);
			radius *= 0.2;
		}
	}
	// Constructing connectivities
	gmesh->ResetConnectivities();
	gmesh->BuildConnectivity();
}

void DadosMalhas::RefiningNearLine(int dim,TPZGeoMesh *gmesh,int nref) {
	
	int i;
	
	// Refinando no local desejado
	TPZManVector<REAL> point(3);
	point[0] = 0.5; point[1] =  1.0; point[2] = 0.0;
	REAL r = 0.0;
	
	REAL radius = 1.;
	for(i=0;i<nref;i++) {
		// To refine elements with center near to points than radius
		RefineGeoElements(dim,gmesh,point,r,radius);
		radius *= 0.8;
	}
	// Constructing connectivities
	gmesh->ResetConnectivities();
	gmesh->BuildConnectivity();
}


void DadosMalhas::RefineGeoElements(int dim,TPZGeoMesh *gmesh,TPZVec<REAL> &point,REAL r,REAL &distance) {
	TPZManVector<REAL> centerpsi(3), center(3);
	// Refinamento de elementos selecionados
	TPZGeoEl *gel;
	TPZVec<TPZGeoEl *> sub;
	
	int nelem = 0;
	int ngelem=gmesh->NElements();
    
	while(nelem<ngelem) {
		gel = gmesh->ElementVec()[nelem++];
		if(gel->Dimension()!=dim || gel->HasSubElement()) continue;
		gel->CenterPoint(gel->NSides()-1,centerpsi);
		gel->X(centerpsi,center);
		REAL centerdist = TPZGeoEl::Distance(center,point);
		if(fabs(r-centerdist) < distance) {
			gel->Divide(sub);
		}
	}
}