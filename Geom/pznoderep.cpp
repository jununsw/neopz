/**
 * @file
 * @brief Creates TPZNodeRep classes for several master elements. 
 */

#ifndef BORLAND
#include "pznoderep.h.h"
#endif

#ifdef BORLAND
#include "pznoderep.h"
#endif

#include "tpzpoint.h"
#include "tpzline.h"
#include "tpztriangle.h"
#include "tpzquadrilateral.h"
#include "tpzpyramid.h"
#include "tpztetrahedron.h"
#include "tpzcube.h"
#include "tpzprism.h"

using namespace pztopology;
using namespace pzgeom;

template<int N, class Topology>
bool TPZNodeRep<N,Topology>::IsLinearMapping() const
{
    return true;
}


template class pzgeom::TPZNodeRep<1,TPZPoint>;
template class pzgeom::TPZNodeRep<2,TPZLine>;
template class pzgeom::TPZNodeRep<3,TPZTriangle>;
template class pzgeom::TPZNodeRep<4,TPZQuadrilateral>;
template class pzgeom::TPZNodeRep<5,TPZPyramid>;
template class pzgeom::TPZNodeRep<4,TPZTetrahedron>;
template class pzgeom::TPZNodeRep<6,TPZPrism>;
template class pzgeom::TPZNodeRep<8,TPZCube>;

