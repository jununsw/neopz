 //
//  File.cpp
//  PZ
//
//  Created by Agnaldo Farias on 7/31/12.
//  Copyright (c) 2012 LabMec-Unicamp. All rights reserved.
//


#include "pzfstrmatrix.h"
#include "toolstransienttime.h"

#include "TPZVTKGeoMesh.h"
#include "TPZRefPatternDataBase.h"
#include "CohesiveTests.h"

//#ifdef LOG4CXX
//static LoggerPtr logger(Logger::getLogger("pz.reducedspace.data"));
//#endif


void CohesiveTest();
int mainFrac(int argc, char *argv[]);


int main(int argc, char *argv[])
{	
	if(1){
		mainFrac(argc,argv);
	}
	else {
		CohesiveTest();
	}
}
int mainFrac(int argc, char *argv[])
{
  //Propagation criterion
  REAL Lx = 15.;
  REAL Ly = 5.;
  REAL Lf = 0.25;
  REAL Hf = 1.;
  REAL Lmax_edge = 0.25/4.;
  REAL Young1 = 25.e9;
  REAL Poiss1 = 0.2;
  
  
  REAL Young2 = 3.9E5;
  REAL Poiss2 = 0.25;
  REAL Xinterface = Lx + 10; // BECAUSE I WILL NOT USE INTERFACE
  
  REAL Fx = 0.;
  REAL Fy = 0.;
  REAL preStressXX = -50.e6;
  REAL preStressXY = 0.;
  REAL preStressYY = -25.e6; //(negativo : estado de compressao)
  
  int NStripes = 1;
  REAL Visc = 5.e-2;
  
  REAL SigN = 1.;
  
  /**
   * Lembre-se que a divisao por 2 (1 asa) e por Hf (na secao de 1 asa) eh feita no kernel.
   * Aqui vai Qinj total mesmo (no poco)!!!
   */
  REAL QinjTot  = -0.0001;
  
  REAL Ttot = 10.; /** em segundos */
  REAL maxDeltaT = 4.; /** em segundos */
  int nTimes = 1; /** quantidade de divisao do maxDeltaT para definir minDeltaT (minDeltaT = maxDeltaT/nTimes) */
  
  REAL Cl = 0.005;
  REAL Pe = 10.;
  REAL SigmaConf = -preStressYY;
  REAL Pref = 60000.;
  REAL vsp = 0.001;
  REAL KIc = 25.;
  REAL Jradius = 0.5;
  
  int p = 2;
  
  globFractInputData.SetData(Lx, Ly, Lf, Hf, Lmax_edge, Young1, Poiss1, Young2, Poiss2, Xinterface,
                             Fx, Fy, preStressXX, preStressXY, preStressYY, NStripes, Visc, SigN,
                             QinjTot, Ttot, maxDeltaT, nTimes, Cl, Pe, SigmaConf, Pref, vsp, KIc, Jradius);
  ToolsTransient ToolTrans(p);
  
  std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
  std::cout << "****\n";
  std::cout << "*****\n";
  std::cout << "******\n";
  std::cout << "*******\n";
  std::cout << "********\n"; 
  std::cout << "**********\n";
  std::cout << "*************\n";
  std::cout << "*******************\n";
  std::cout << "*************************\n";
  std::cout << "*************************************\n";
  std::cout << "*************************************************\n";
  std::cout << "*******************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "**********************************************************************\n";
  std::cout << "Lembre-se de deixar os minT, maxT, actT etc como inteiros!!!\n";
  std::cout << "*******************************************************\n";
  std::cout << "*******************************************************\n";
  std::cout << "*******************************************************\n";
  std::cout << "*******************************************************\n";
  std::cout << "*******************************************************\n";
  //    LEIA ACIMA!!!
	
	ToolTrans.Run();
  
  return 0;
}

void CohesiveTest()
{
	TestContinuousDiscontinuous();
}
