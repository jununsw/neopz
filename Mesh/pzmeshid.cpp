//
// C++ Implementation: pzmeshid
//
// Description: 
//
//
// Author: Philippe R. B. Devloo <phil@corona>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pzmeshid.h"
#include "TPZGeoElement.h"
#include "pzgnode.h"
#include "TPZGeoCube.h"
#include "pzshapecube.h"
#include "TPZRefCube.h"
#include "pzshapelinear.h"
#include "TPZGeoLinear.h"
#include "TPZRefLinear.h"
#include "pzrefquad.h"
#include "pzshapequad.h"
#include "pzgeoquad.h"
#include "pzshapetriang.h"
#include "pzreftriangle.h"
#include "pzgeotriangle.h"
#include "pzshapeprism.h"
#include "pzrefprism.h"
#include "pzgeoprism.h"
#include "pzshapetetra.h"
#include "pzreftetrahedra.h"
#include "pzgeotetrahedra.h"
#include "pzshapepiram.h"
#include "pzrefpyram.h"
#include "pzgeopyramid.h"
#include "pzrefpoint.h"
#include "pzgeopoint.h"
#include "pzshapepoint.h"
#include "pzquad.h"

#include "pzgeoelbc.h"
#include "pzgmesh.h"
#include "TPZAgglomerateEl.h"
#include "TPZInterfaceEl.h"
#include "pzelctemp.h"
#include "pzcmesh.h"
#include "pzflowcmesh.h"
#include "pzsubcmesh.h"




void RegisterMeshClasses() {
  TPZSaveable::Register(TPZGEONODEID,Restore<TPZGeoNode>);
  TPZSaveable::Register(TPZFGEOELEMENTPOINTID,Restore<TPZGeoElement<TPZShapePoint,TPZGeoPoint,TPZRefPoint> >);
  TPZSaveable::Register(TPZFGEOELEMENTLINEARID,Restore<TPZGeoElement<TPZShapeLinear,TPZGeoLinear,TPZRefLinear> >);
  TPZSaveable::Register(TPZFGEOELEMENTRIANGLEID,Restore<TPZGeoElement<TPZShapeTriang,TPZGeoTriangle,TPZRefTriangle> >);
  TPZSaveable::Register(TPZFGEOELEMENTQUADID,Restore<TPZGeoElement<TPZShapeQuad,TPZGeoQuad,TPZRefQuad> >);
  TPZSaveable::Register(TPZFGEOELEMENTCUBEID,Restore<TPZGeoElement<TPZShapeCube,TPZGeoCube,TPZRefCube> >);
  TPZSaveable::Register(TPZFGEOELEMENTPRISMID,Restore<TPZGeoElement<TPZShapePrism,TPZGeoPrism,TPZRefPrism> >);
  TPZSaveable::Register(TPZFGEOELEMENTTETRAID,Restore<TPZGeoElement<TPZShapeTetra,TPZGeoTetrahedra,TPZRefTetrahedra> >);
  TPZSaveable::Register(TPZFGEOELEMENTPYRAMID,Restore<TPZGeoElement<TPZShapePiram,TPZGeoPyramid,TPZRefPyramid> >);
  TPZSaveable::Register(TPZGEOELBCID,Restore<TPZGeoElBC>);
  TPZSaveable::Register(TPZGEOMESHID,Restore<TPZGeoMesh>);
  
  TPZSaveable::Register(TPZCOMPELDISCID,Restore<TPZCompElDisc>);
  TPZSaveable::Register(TPZAGGLOMERATEELID,Restore<TPZAgglomerateElement>);
  
  TPZSaveable::Register(TPZINTERFACEELEMENTID,Restore<TPZInterfaceElement>);
  TPZSaveable::Register(TPZINTELPOINTID,Restore<TPZIntelGen<TPZGeoPoint,TPZShapePoint> >);
  TPZSaveable::Register(TPZINTELLINEARID,Restore<TPZIntelGen<TPZGeoLinear, TPZShapeLinear> >);
  TPZSaveable::Register(TPZINTELQUADID,Restore<TPZIntelGen<TPZGeoQuad,TPZShapeQuad> >);
  TPZSaveable::Register(TPZINTELTRIANGLEID,Restore<TPZIntelGen<TPZGeoTriangle,TPZShapeTriang> >);
  TPZSaveable::Register(TPZINTELCUBEID,Restore<TPZIntelGen<TPZGeoCube,TPZShapeCube> >);
  TPZSaveable::Register(TPZINTELTETRAID,Restore<TPZIntelGen<TPZGeoTetrahedra,TPZShapeTetra> >);
  TPZSaveable::Register(TPZINTELPRISMID,Restore<TPZIntelGen<TPZGeoPrism,TPZShapePrism> >);
  TPZSaveable::Register(TPZINTELPYRAMID,Restore<TPZIntelGen<TPZGeoPyramid,TPZShapePiram> >);
  TPZSaveable::Register(TPZSUBCOMPMESHID,Restore<TPZSubCompMesh>);
  TPZSaveable::Register(TPZCOMPMESHID,Restore<TPZCompMesh>);

  TPZSaveable::Register(TPZFLOWCOMPMESHID,Restore<TPZFlowCompMesh>);
}
