/*
 * @file
 * @author Denise de Siqueira
 * @since 6/9/11.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "pzgengrid.h"
#include "pzgmesh.h"
#include "pzgeoelbc.h"
#include "pzcmesh.h"
#include "tpzcompmeshreferred.h"
#include "pzcompel.h"
#include "pzpoisson3d.h"
#include "pzbndcond.h"
#include "pzanalysiserror.h"
#include "pzanalysis.h"
#include "pzcmesh.h"
#include "pzstepsolver.h"
#include "TPZParFrontStructMatrix.h"
#include "pzmatrix.h"
#include "TPZCompElDisc.h"
#include "pzfstrmatrix.h"
#include "pzinterpolationspace.h"
#include "pzsubcmesh.h"
#include "pzlog.h"
#include "pzelctemp.h"
#include "pzelchdiv.h"
#include "pzshapequad.h"
#include "pzshapetriang.h"
#include "pzgeoquad.h"
#include "pzgeotriangle.h"
#include "pzfstrmatrix.h"
#include "pzgengrid.h"
#include "pzbndcond.h"
#include "pzmaterial.h"
#include "tpzquadrilateral.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "pzlog.h"
#include <cmath>
#include "Tools.h"

#ifdef LOG4CXX

static LoggerPtr logger(Logger::getLogger("HpAdaptivity.main"));

#endif

using namespace std;

/** Resolver o problema do tipo 
 * -Laplac(u) = 0
 * du/dn = lambda u em todo contorno
 */


using namespace std;
int main()
{
	
#ifdef LOG4CXX
	{
		InitializePZLOG();
		std::stringstream sout;
		sout<< "Testando p adaptatividade"<<endl;
		LOGPZ_DEBUG(logger, sout.str().c_str());
	}
#endif
	std::ofstream erro("TaxaProbModelo.txt");
	TPZVec<REAL> calcErro;
	for (int porder=2; porder<3; porder++) {
		
		erro<<"ordem "<<porder <<std::endl;
			for(int h=2;h<3;h++){
			erro<<std::endl;
			erro<< "\n NRefinamento "<<h<<std::endl;
			//1. Criacao da malha geom. e computacional
					bool hrefine=false;
					bool prefine=false;
                TPZGeoMesh *gmesh = MalhaGeo(h, hrefine);
              //  RefiningNearCircunference(2,gmesh,h,1);
                //MalhaGeoT(h, hrefine);
					gmesh->Print();
                

    std::ofstream file("TestedeMalha.vtk");
		PrintGMeshVTK( gmesh, file);

			
			TPZCompMesh *cmesh = CompMeshPAdap(*gmesh,porder,prefine);

            
			//2. Resolve o problema
		
		 TPZAnalysis analysis(cmesh);
		 cmesh->SaddlePermute();
		 SolveLU ( analysis );	
			
			
			
			//3. Calcula o erro 
					
			TPZVec<REAL> calcErro;
			analysis.SetExact(*SolArcTan);
			analysis.PostProcess(calcErro,erro);
			
			//4. visualizacao grafica usando vtk
			 TPZVec<std::string> scalnames(3), vecnames(2);
			 
			// scalnames[0] = "Divergence";
			 scalnames[1] = "Pressure";
			 scalnames[0] = "ExactPressure";
			 scalnames[2] = "ExactDiv";
			 
			 
			 vecnames[0] = "ExactFlux";
			 vecnames[1] = "Flux";
			 //scalnames[2] = "Divergence";
			 
			 
			 //vecnames[0] = "Derivate";
			 
			 
			 std::string plotfile("GraficHpAdaptivity.vtk");
			 const int dim = 2;
			 int div = 2;
			 analysis.DefineGraphMesh(dim,scalnames,vecnames,plotfile);
			 analysis.PostProcess(div);
             
			 
			 
			
		}}
	
	
	return 0;
}

