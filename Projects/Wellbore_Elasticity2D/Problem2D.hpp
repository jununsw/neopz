//
//  Problem2D.hpp
//  PZ
//
//  Created by Nathalia Batalha on 9/8/17.
//
//

#ifndef Problem2D_hpp
#define Problem2D_hpp

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "pzvec.h"
#include "pzadmchunk.h"
#include "pzcmesh.h"
#include "pzvec_extras.h"
#include "pzdebug.h"
#include "pzcheckgeom.h"

#include "pzgeoel.h"
#include "pzgnode.h"
#include "pzgeoelside.h"
#include "pzgeoelbc.h"

#include "pzintel.h"
#include "pzcompel.h"

#include "pzmatrix.h"

#include "pzanalysis.h"
#include "pzfstrmatrix.h"
#include "pzskylstrmatrix.h"
#include "pzstepsolver.h"

#include "pzmaterial.h"
#include "pzbndcond.h"
#include "pzelasmat.h"
#include "pzplaca.h"
#include "pzmat2dlin.h"
#include "pzmathyperelastic.h"
#include "pzmattest3d.h"
#include "pzmatplaca2.h"

#include "pzfunction.h"
#include "TPZVTKGeoMesh.h"

#include <time.h>
#include <stdio.h>
#include <fstream>

#include "pzlog.h"
#include "pzgmesh.h"
#include "TPZMatElasticity2D.h"
#include <iostream>
#include <fstream>
#include <string>
#include "TPZVTKGeoMesh.h"
#include "pzanalysis.h"
#include "pzbndcond.h"

#include "pzstepsolver.h"
#include "math.h"
#include "TPZSkylineNSymStructMatrix.h"
#include "TPZReadGIDGrid.h"
#include "TPZParFrontStructMatrix.h"

#include <cmath>
#include <set>

#include "pzelast3d.h"
#include <pzgengrid.h>

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.elasticity"));
#endif

#include <time.h>
#include <stdio.h>
#include <fstream>

#include "pzrandomfield.h"
#include "GeometricMesh.hpp"
#include "ComputationalMesh.hpp"

// No. de divisoes do pos-processamento
#define NDIV 2

// Executa a simulação para malha 2D
int Problem2D(REAL rw, REAL rext, int ncircle, int nradial, int projection,
              int inclinedwellbore, int analytic, REAL SigmaV, REAL Sigmah,
              REAL SigmaH, REAL Pwb, REAL drdcirc, REAL direction, REAL inclination,
              bool isStochastic, std::ofstream &solutionfile,int &icase);

#endif /* Problem2D_hpp */