//$Id: pzelmat.cpp,v 1.3 2004-04-05 14:09:35 phil Exp $

#include "pzelmat.h"
#include "pzfmatrix.h"
#include "pzcmesh.h"

void TPZElementMatrix::SetMatrixSize(short NumBli, short NumBlj,
				     short BlSizei, short BlSizej) {


  if(fMat.Rows() != NumBli*BlSizei || fMat.Cols() != NumBlj*BlSizej) {
    fMat.Redim(NumBli*BlSizei,NumBlj*BlSizej);
  }
}

void TPZElementMatrix::SetMatrixMinSize(short NumBli, short NumBlj, 
					short BlSizei, short BlSizej) {


  if(fMat.Rows() < NumBli*BlSizei || fMat.Cols() < NumBlj*BlSizej) {
    fMat.Redim(NumBli*BlSizei,NumBlj*BlSizej);
  }
}

void TPZElementMatrix::Print(TPZCompMesh &mesh, ostream &out){
  fMat.Print("Unconstrained matrix",out);
   int ncon = fConnect.NElements();
   int ic;
   for(ic=0; ic<ncon; ic++) {
   	out << "Connect index " << fConnect[ic] << endl;
   	mesh.ConnectVec()[fConnect[ic]].Print(mesh,out);
   }
   fConstrMat.Print("Constrained matrix",out);
   ncon = fConstrConnect.NElements();
   for(ic=0; ic<ncon; ic++) {
   	out << "Connect index " << fConstrConnect[ic] << endl;
   	mesh.ConnectVec()[fConstrConnect[ic]].Print(mesh,out);
   }

}


/*
  bandmat & bandmat::operator+=(elmat & ek) {

  for (int i = 0; i<ek.numnod(); i++) {
  for (int j = 0; j<ek.numnod(); j++) {
  addsub( (ek.node(i))->eq_number(), 
  (ek.node(j))->eq_number(), ek.mat->extract(i,j) );
  }
  }
  return *this;
  }

  void matrix::operator+=(elmat & ek) {

  if(ek.mat->colblocks() == ek.mat->rowblocks() ) {
  for (int i = 0; i<ek.numnod(); i++) {
  for (int j = 0; j<ek.numnod(); j++) {
  addsub( (ek.node(i))->eq_number(), 
  (ek.node(j))->eq_number(), ek.mat->extract(i,j) );
  }
  }
  } else if (ek.mat->colblocks() == 1) {
  for (int i = 0; i<ek.numnod(); i++) {
  addsub( (ek.node(i))->eq_number(), 
  0, ek.mat->extract(i,0) );
  }
  } else {
  pzerror << "matrix.+= doesn t know how to handle the\n"
  "assembly process\n";
  pzerror.show();
  }
  return;
  }
*/

