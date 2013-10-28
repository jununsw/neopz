/**
 * @file
 * @brief Tutorial program showing a method to reconstruction gradient for a solution precomputed
 */

#include <iostream>

#include "pzgmesh.h"
#include "pzcmesh.h"
#include "tpzcompmeshreferred.h"
#include "pzreferredcompel.h"
#include "pzcompel.h"
#include "pzbndcond.h"
#include "TPZInterfaceEl.h"
#include "pzinterpolationspace.h"
#include "pzpoisson3d.h"
#include "pzmat1dlin.h"

#include "pzgengrid.h"
#include "pzgradient.h"
#include "pzl2projection.h"

#include "tpzgeoelrefpattern.h"
#include "TPZGeoLinear.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"
#include "pzgeopoint.h"

#include "pzgeoelbc.h"

#include "pzanalysis.h"
#include "pzskylstrmatrix.h"
#include "pzstrmatrix.h"
#include "pzbstrmatrix.h"
#include "pzstepsolver.h"
#include "pzfstrmatrix.h"
#include "pzlog.h"

#include "TPZVTKGeoMesh.h"


#include <iostream>
#include <math.h>
using namespace std;

// Functions to fill coordinates for several points to test the adjusting by ellipse
void FillingPoints2D(TPZManVector<REAL> &Points);
void FillingPoints3D(TPZManVector<REAL> &Points);

// Least Squares Method to compute a ellipse nearest for a points in vector
// The obtained ellipse has the axes parallels to rectangular axes
// Format (x-x0)^2/a^2 + (y-y0)^2/b^2 = 1.
bool AdjustingWithSimpleEllipse(int dim,TPZManVector<REAL> &points);

// Least Squares Method to compute a ellipse nearest for a points in vector
// The ellipse is a conic with second order equation as
// y^2 = A*x^2 + B*xy + C*x + D*y + E
// Then their axes could be rotated and translated
bool AdjustingWithGeneral2DEllipse(int dim,TPZManVector<REAL> &points);

int main(int argc, char *argv[]) {

#ifdef LOG4CXX
    InitializePZLOG();
#endif

	int dim = 2;
	TPZManVector<REAL> Points;
	if(dim==2)
		FillingPoints2D(Points);
	else
		FillingPoints3D(Points);

	// Finding a ellipse nearest for all points
	AdjustingWithSimpleEllipse(dim,Points);
	
	return 0;
}

bool LeastSquaresToGetSimpleEllipse(int dim,TPZManVector<REAL> &points,TPZFMatrix<REAL> &Coefficients) {
	if(dim < 2 || dim > 3) return false;
	long npoints = points.NElements()/dim;
	int nincog, i;
	if(dim == 2) nincog = 4;
	else if(dim == 3) nincog = 6;

	if(npoints<nincog) return false;

	// Dimensioning vector of coefficients
	Coefficients.Redim(nincog,1);
	Coefficients.Zero();

	// Will be solved y^2 = p*x^2 + q*x + r*y + s
	// Constructing matrix H and Transpose of H to compute by least squares method
	TPZFMatrix<REAL> DeltaH;
	TPZFMatrix<REAL> DeltaHTranspose;
	TPZFMatrix<REAL> DifSol;
	TPZFMatrix<REAL> A;

	// Redimensioning
	A.Redim(nincog,nincog);
    DeltaH.Redim(npoints,nincog);
    DeltaHTranspose.Redim(nincog,npoints);
    DifSol.Redim(npoints,1);
//	DifSol.Zero();

	if(dim == 2) {
		// Filling y^2 into Coefficients
		for(i=0;i<npoints;i++)
			DifSol.PutVal(i,0,points[2*i+1]*points[2*i+1]);  // fill y*y

		// Filling elements for H matrix
		for(int i=0;i<npoints;i++) {
			DeltaH.PutVal(i,0,points[2*i]*points[2*i]);      // fill x*x
			DeltaH.PutVal(i,1,points[2*i]);                  // fill x
			DeltaH.PutVal(i,2,points[2*i+1]);                // fill y
			DeltaH.PutVal(i,3,1.);                           // fill 1.
		}
	}
	else if(dim == 3) {
		// Filling y^2 into Coefficients
		for(i=0;i<npoints;i++)
			DifSol.PutVal(i,0,points[3*i+2]*points[3*i+2]);   // fill z*z

		// Filling elements for H matrix
		for(int i=0;i<npoints;i++) {
			DeltaH.PutVal(i,0,points[3*i]*points[3*i]);       // fill x*x
			DeltaH.PutVal(i,1,points[3*i]);                   // fill x
			DeltaH.PutVal(i,2,points[3*i+1]*points[3*i+1]);   // fill y*y
			DeltaH.PutVal(i,3,points[3*i+1]);                 // fill y
			DeltaH.PutVal(i,4,points[3*i+2]);                 // fill z
			DeltaH.PutVal(i,5,1.);                            // fill 1.
		}
	}
	else 
		return false;

	DeltaH.Print(std::cout);

    // Solving by least squares using product of matrix: DeltaH_t * DifSol = DeltaH_t * DeltaH * Coeffs(u)
    A.Zero();
    DeltaH.Transpose(&DeltaHTranspose);
    A = DeltaHTranspose*DeltaH;
	Coefficients = DeltaHTranspose*DifSol;
	A.SolveDirect(Coefficients,ELU);
	Coefficients.Print(std::cout);
}

bool StandardFormatForSimpleEllipse(TPZFMatrix<REAL> &Coeffs,TPZManVector<REAL> &Center,TPZManVector<REAL> &Ratios) {
	int dim = Center.NElements();
	int ncoeffs = Coeffs.Rows();
	REAL temp;
	if(ncoeffs != 2*dim || Coeffs.Cols()!=1)
		return false;
	if(dim ==2) {
		Center[0] = -(Coeffs(1,0)/(2.*Coeffs(0,0)));
		Center[1] = 0.5*Coeffs(2,0);
		// Computing Ratios[1] in temp
		temp = Coeffs(3,0)-(Center[0]*Center[0]*Coeffs(0,0))+(Center[1]*Center[1]);
		// Computing Ratios[0] in Ratios[1]
		Ratios[1] = -temp/Coeffs(0,0);
		if(temp < 0. || Ratios[1] < 0.)
			return false;
		Ratios[0] = sqrt(Ratios[1]);
		Ratios[1] = sqrt(temp);
	}
	else {
		Center[0] = -(Coeffs(1,0)/(2.*Coeffs(0,0)));
		Center[1] = -(Coeffs(3,0)/(2.*Coeffs(2,0)));
		Center[2] = 0.5*Coeffs(4,0);
		// Computing Ratios[2]
		temp = Coeffs(5,0)+(Center[2]*Center[2])-(Center[0]*Center[0]*Coeffs(0,0))-(Center[1]*Center[1]*Coeffs(2,0));
		if(temp < 0.) return false;
		Ratios[2] = sqrt(temp);
		// Computing Ratios[0] in Ratios[1]
		temp = -(Ratios[2]*Ratios[2]/Coeffs(2,0));
		if(temp < 0.) return false;
		Ratios[1] = sqrt(temp);
		temp = -(Ratios[2]*Ratios[2]/Coeffs(0,0));
		if(temp < 0.) return false;
		Ratios[0] = sqrt(temp);
	}
	return true;
}

void PrintingAsSimpleEquation(TPZFMatrix<REAL> &Coeffs,TPZManVector<REAL> &Center,TPZManVector<REAL> &Ratios) {
	int dim = Center.NElements();
	if(dim == 2) {
		std::cout << std::endl << "y*y = " << Coeffs(0,0) << "x*x + " << Coeffs(1,0) << "x + " << Coeffs(2,0) << "y + " << Coeffs(3,0) << "\n";
		std::cout << "\nElipse: (x - " << Center[0] << ")^2/" << Ratios[0]*Ratios[0] << " + (y - " << Center[1] << ")^2/" << Ratios[1]*Ratios[1] << " = 1.\n" << std::endl;
	}
	else {
		std::cout << std::endl << "z*z = " << Coeffs(0,0) << "x*x + " << Coeffs(1,0) << "x + " << Coeffs(2,0) << "y*y + " << Coeffs(3,0) << "y +";
		std::cout << Coeffs(4,0) << "z + " << Coeffs(5,0) << std::endl;
		std::cout << "\nElipse: (x - " << Center[0] << ")^2/" << Ratios[0]*Ratios[0] << " + (y - " << Center[1] << ")^2/" << Ratios[1]*Ratios[1];
		std::cout << " + (z - " << Center[2] << ")^2/" << Ratios[2]*Ratios[2] << " = 1.\n" << std::endl;
	}
}

bool AdjustingWithSimpleEllipse(int dim,TPZManVector<REAL> &Points) {

	TPZFMatrix<REAL> Coeffs;
	TPZManVector<REAL> Center(dim,0.);
	TPZManVector<REAL> Ratios(dim,0.);

	// Applying least squares for these five points
	LeastSquaresToGetSimpleEllipse(dim,Points,Coeffs);
	std::cout << "\n\nSolution:";

	// Making zero depending on Tolerance
	float Tol;
	ZeroTolerance(Tol);
	for(int i=0;i<Coeffs.Rows();i++)
		if(fabs(Coeffs(i)) < 100*Tol)
			Coeffs.PutVal(i,0,0.);

	// Getting the values of the center and ratios for the ellipse from Coeffs values
	if(!StandardFormatForSimpleEllipse(Coeffs,Center,Ratios))
		return false;
	PrintingAsSimpleEquation(Coeffs,Center,Ratios);
	return true;
}

bool AdjustingWithGeneral2DEllipse(int dim,TPZManVector<REAL> &Points) {

	TPZFMatrix<REAL> Coeffs;
	// Applying least squares for these five points
	if(!LeastSquaresToGetSimpleEllipse(dim,Points,Coeffs))
		return false;
	return true;
}

void FillingPoints2D(TPZManVector<REAL> &Points) {
	int dim = 2;
	int npoints = 8;
	if(0) {
		Points.Resize(dim*npoints);

		// Filling points coordinates
		Points[0] = 1.;
		Points[1] = 3;
		Points[2] = 1.;
		Points[3] = -3;
		Points[4] = -1.;
		Points[5] = 0.;
		Points[6] = 3.;
		Points[7] = 0.;
		Points[8] = 0.;
		Points[9] = (3./2.)*sqrt(3.);
		Points[10] = 0.;
		Points[11] = -(3./2.)*sqrt(3.);
		Points[12] = 2.97203;
		Points[13] = 0.5;
		Points[14] = 2.49071;
		Points[15] = -2.;
	}
	else if(0) {
		// Consideremos a elipse (1./9.)*(x-2)^2 + (1./18.)*(y-1)^2 = 1
		// Podemos incrementar qualquer numero de pontos nas coordenadas x pertencente [-1.,5.]
		int i;
		double x[] = {-0.5, 0., 0.5, 1., 2., 3., 4., 4.5};
		double temp;
		Points.Resize(2*npoints*dim);
		for(i=0;i<npoints;i++) {
			Points[dim*i] = x[i];
			temp = sqrt(18 - 2*(x[i] - 2.)*(x[i] - 2.));
			Points[dim*i+1] = 1.+temp;
			Points[2*npoints+dim*i] = x[i];
			Points[2*npoints+dim*i+1] = 1.-temp;
		}
	}
	else {
		// Consideremos a elipse (1./9.)*(x-2)^2 + (1./18.)*(y-1)^2 = 1
		// Podemos incrementar qualquer numero de pontos nas coordenadas x pertencente [-1.,5.]
		// Verificando com valores deslocados muito pouco
		int i;
		double x[] = {-0.88, -0.5, 0.5, 1., 2., 3., 4., 4.5};
		double temp;
		Points.Resize(2*(npoints+1)*dim);
		for(i=0;i<npoints;i++) {
			Points[dim*i] = x[i];
			temp = sqrt(18 - 2*(x[i] - 2.)*(x[i] - 2.));
			Points[dim*i+1] = 1.+temp;
			Points[2*(npoints+1)+dim*i] = x[i];
			Points[2*(npoints+1)+dim*i+1] = 1.-temp;
		}
		Points[dim*i] = -1.1;
		Points[dim*i+1] = 1.03;
		Points[2*(npoints+1)+dim*i] = 2.;
		Points[2*(npoints+1)+dim*i+1] = 1.+sqrt(18.);
	}
}
void FillingPoints3D(TPZManVector<REAL> &Points) {
	Points.Resize(18);

	// Filling points coordinates
	Points[0] = 1.;
	Points[1] = 0.;
	Points[2] = 1.;

	Points[3] = 1.;
	Points[4] = 0.;
	Points[5] = -3.;

	Points[6] = -1.;
	Points[7] = 0.;
	Points[8] = -1.;

	Points[9] = 3.;
	Points[10] = 0.;
	Points[11] = -1.;

	Points[12] = 1.;
	Points[13] = 3.;
	Points[14] = -1.;

	Points[15] = 1.;
	Points[16] = -3.;
	Points[17] = -1.;
}