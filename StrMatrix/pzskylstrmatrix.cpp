/* Generated by Together */

#include "pzskylstrmatrix.h"
#include "pzskylmat.h"
#include "pzcmesh.h"
#include "pzvec.h"

TPZStructMatrix * TPZSkylineStructMatrix::Clone(){
    return new TPZSkylineStructMatrix(fMesh);
}
TPZMatrix * TPZSkylineStructMatrix::CreateAssemble(TPZFMatrix &rhs){
    int neq = fMesh->NEquations();
    if(HasRange()) neq = fMaxEq-fMinEq;
    TPZMatrix *stiff = Create();
    rhs.Redim(neq,1);
    Assemble(*stiff,rhs);
    return stiff;
}
TPZMatrix * TPZSkylineStructMatrix::Create(){
    int neq = fMesh->NEquations();
    TPZVec<int> skyline;
    fMesh->Skyline(skyline);
    if(HasRange())
    {
      neq = fMaxEq-fMinEq;
      FilterSkyline(skyline);
    }
    return new TPZSkylMatrix(neq,skyline);
}
TPZSkylineStructMatrix::TPZSkylineStructMatrix(TPZCompMesh *mesh) : TPZStructMatrix(mesh)
{}


/*!
    \fn TPZSkylineStructMatrix::FilterSkyline()
 */
void TPZSkylineStructMatrix::FilterSkyline(TPZVec<int> &skyline)
{
  if(!HasRange()) return;
//  int neq = skyline.NElements();
  int ieq;
  for(ieq = fMinEq; ieq < fMaxEq; ieq++)
  {
    skyline[ieq-fMinEq] = skyline[ieq]-fMinEq;
  }
  skyline.Resize(fMaxEq-fMinEq);
}
