#ifndef TPZCHANGEEL_H
#define TPZCHANGEEL_H

#include "pzgeotriangle.h"
#include "tpzgeoblend.h"
#include "pzgeoel.h"
#include "pznoderep.h"

#include <iostream>

  /**
  / Class made by Paulo Cesar de Alvarenga Lucci (Caju)
  / LabMeC - FEC - UNICAMP
  / 2007
 */

class TPZChangeEl {
public:

    TPZChangeEl();
   ~TPZChangeEl();

    /** Turns an linear triangle or quadrilateral to quadratic */
    static TPZGeoEl * ChangeToQuadratic(TPZGeoMesh *Mesh, int ElemIndex);

    /** Turns a regular element into a geoblend */
    static TPZGeoEl * ChangeToGeoBlend(TPZGeoMesh *Mesh, int ElemIndex);

    /** Turns an quadratic triangle or quadrilateral to linear */
    static TPZGeoEl * ChangeToLinear(TPZGeoMesh *Mesh, int ElemIndex);

    /** Slide correct nodes of an triangle or quadrilateral to the quarterpoint with respect to a given side */
    static TPZGeoEl * QuarterPoints(TPZGeoMesh *Mesh, int ElemIndex, int side);

private:
    static void AdjustNeighbourhood(TPZGeoEl* OldElem, TPZGeoEl*NewElem);

};

#endif
