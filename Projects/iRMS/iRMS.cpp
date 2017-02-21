#include "pzlog.h"
#include "tpzautopointer.h"
#include "TPZIntQuadQuarterPoint.h"
#include <time.h>
#include "pzgmesh.h"
#include "TPZRefPatternTools.h"
#include "pzmaterial.h"

#include "TRMRawData.h"
#include "TRMSimworxMeshGenerator.h"
#include "TRMOrchestra.h"
#include "TRMSpaceOdissey.h"

#include "TPZGeoLinear.h"
#include "tpzgeoelrefpattern.h"


// iRMS (i rise for / innovatory Reservoir Muli-scale Simulator)

#ifdef LOG4CXX
static LoggerPtr logdata(Logger::getLogger("pz.iRMS"));
#endif

#ifdef USING_BOOST
#include "boost/date_time/posix_time/posix_time.hpp"
#endif

void LinearTracerPrimal();
void LinearTracerDual();
void MultiScaleSimulation();
void CheckQuarterPoint();
void BuildGeometry(TRMOrchestra  * SymphonyX);
void CreateExampleRawData(TRMRawData &data);

int main()
{
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    // This code use normalized piola contravariant mapping for nonlinear mappings
    HDivPiola = 1;
    TPZMaterial::gBigNumber = 1.0e14;
    
#ifdef USING_BOOST
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
#endif
    
    // Running dual problem on box shape
    MultiScaleSimulation();
    
#ifdef USING_BOOST
    boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
#endif
    
#ifdef USING_BOOST
    std::cout  << "iMRS:: Overal execution time = " << (t2-t1) << std::endl;
#endif
    
    std::cout << "iMRS:: Process complete normally." << std::endl;
    return 0;
}

void LinearTracerPrimal()
{

    TRMOrchestra  * SymphonyX = new TRMOrchestra;
    SymphonyX->CreateAnalysisPrimal();
    std::cout << "Primal complete normally." << std::endl;
    
}

void LinearTracerDual()
{
    
    TRMOrchestra  * SymphonyX = new TRMOrchestra;
    SymphonyX->CreateAnalysisDual();
    std::cout << "Dual complete normally." << std::endl;       
    
}

void MultiScaleSimulation()
{
    // Materials ids and boundary settings
    TPZAutoPointer<TRMRawData> RawData  = new TRMRawData;
    
    //  Dimension on gmsh reservoir    
    bool Is3DGeometry = true;
    
    bool IsSinglePhaseQ = false;
    if(IsSinglePhaseQ){
        RawData->SinglePhaseReservoir(Is3DGeometry); // Single-phase flow
    }
    else{
        RawData->TwoPhaseWaterOilReservoir(Is3DGeometry); // Two-phase flow
    }
    
    
    TRMSimulationData * SimData = new TRMSimulationData;
    SimData->SetRawData(RawData);
    TRMOrchestra  * SymphonyX           = new TRMOrchestra;
    SymphonyX->SetSimulationData(SimData);
    
    
    SymphonyX->SetSegregatedQ(true);
    SymphonyX->CreateAnalysisDualonBox(true); //  Static Solution
    SymphonyX->RunStaticProblem();
    SymphonyX->CreateAnalysisDualonBox(false);  // Evolutionary Solution
    SymphonyX->RunEvolutionaryProblem();

    std::cout << "Dual complete normally." << std::endl;
    
}

void CheckQuarterPoint()
{
    TPZIntQuadQuarterPoint qt(10);
    qt.SetCorner(1);
//    TPZIntQuad qt(60);
    int np = qt.NPoints();
    TPZManVector<REAL,2> pt(2,0.);
    std::cout << "Numpoints = " << np << std::endl;
    REAL weight;
    REAL integral = 0.;
    for (int ip = 0; ip<np; ip++) {
        qt.Point(ip, pt, weight);
        std::cout << "ip " << ip << " pt " << pt << " weight " << weight << std::endl;
        REAL r = sqrt((pt[0]-1)*(pt[0]-1)+(pt[1]+1)*(pt[1]+1));
        integral += weight/r;
    }
    std::cout << "Integral " << integral << std::endl;
    
}

