/**
 * @file TPZGeoTriangle
 * @brief Contains the TPZGeoTriangle class which implements the geometry of a triangle element.
 */

#ifndef TPZGEOTRIANGLEH
#define TPZGEOTRIANGLEH

#include "pzvec.h"
#include "pzeltype.h"
#include "pznoderep.h"
#include "tpztriangle.h"

#include <string>
#include <map>

template<class TVar>
class TPZFMatrix;
class TPZGeoEl;
class TPZGeoMesh;


namespace pzgeom {
	
	/**
	 * @ingroup geometry
	 * @brief Implements the geometry of a triangle element. \ref geometry "Geometry"
	 */
	class TPZGeoTriangle : public TPZNodeRep<3, pztopology::TPZTriangle> 
	{
	public:

		/** @brief Number of corner nodes */
		enum {NNodes = 3};
		
		/** @brief Constructor with list of nodes */
		TPZGeoTriangle(TPZVec<long> &nodeindexes) : TPZNodeRep<NNodes,pztopology::TPZTriangle>(nodeindexes)
		{
		}
		
		/** @brief Empty constructor */
		TPZGeoTriangle() : TPZNodeRep<NNodes,pztopology::TPZTriangle>()
		{
		}
		
		/** @brief Constructor with node map */
		TPZGeoTriangle(const TPZGeoTriangle &cp,
					   std::map<long,long> & gl2lcNdMap) : TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp,gl2lcNdMap)
		{
		}
		
		/** @brief Copy constructor */
		TPZGeoTriangle(const TPZGeoTriangle &cp) : TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp)
		{
		}
		
		/** @brief Copy constructor */
		TPZGeoTriangle(const TPZGeoTriangle &cp, TPZGeoMesh &) : TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp)
		{
		}
        
        static bool IsLinearMapping(int side)
        {
            return true;
        }
		
		/** @brief Returns the type name of the element */
		static std::string TypeName() { return "Triangle";}
		
        /** @brief Compute the shape being used to construct the x mapping from local parametric coordinates  */
        static void Shape(TPZVec<REAL> &loc,TPZFMatrix<REAL> &phi,TPZFMatrix<REAL> &dphi){
            TShape(loc, phi, dphi);
        }
        
        static void CorrectFact (TPZVec<REAL> &ksi, int &EdgeSides ,REAL &cf)
        {
            if (EdgeSides == 3)
            {
                cf = (1 - ksi[1])*(1 - ksi[1]);
            }
            else if (EdgeSides == 4)
            {
                cf = (ksi[0] + ksi[1])*(ksi[0] + ksi[1]);
            }
            else if (EdgeSides == 5)
            {
                cf = (1 - ksi[0])*(1 - ksi[0]);
            }
        }
        
        /** @brief Returns the equivalent ksi from a triangle element to a linear element */
        
        static void LinearKsi(int &EdgeSides, TPZVec<REAL> &ksi, TPZVec<REAL> &s)
        {
            
            if (EdgeSides == 3) {
                if(1-ksi[1]<=1e-6) {
                    s[0] = -1;
                }
                else {
                    s[0] = 2*ksi[0]/(1-ksi[1]) - 1;
                }
                
            }
            else if (EdgeSides == 4) {
                if((ksi[0] && ksi[1] <= 1e-6) || ksi[0] == ksi[1]) {
                    s[0] = 0;
                }
                else {
                    s[0] = (ksi[1] - ksi[0])/(ksi[0] + ksi[1]);
                }
            }
            else if (EdgeSides == 5) {
                if(1-ksi[0]<=1e-6) {
                    s[0] = 1;
                }
                else {
                    s[0] = - 2*ksi[1]/(1-ksi[0]) + 1;
                }
            }
        }

        //Needs implementation
        template<class T>
        void XLinearMapping(const TPZGeoEl &gel,TPZVec<T> &ksi, TPZVec<T> &result) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            X(coord,ksi,result);
        }
        
        /* @brief Compute x mapping from local parametric coordinates */
        template<class T>
        void X(const TPZGeoEl &gel,TPZVec<T> &loc,TPZVec<T> &x) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            X(coord,loc,x);
        }
        
        /** @brief Compute gradient of x mapping from local parametric coordinates */
        template<class T>
        void GradX(const TPZGeoEl &gel, TPZVec<T> &loc, TPZFMatrix<T> &gradx) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            int nrow = coord.Rows();
            int ncol = coord.Cols();
            TPZFMatrix<T> nodes(nrow,ncol);
            for(int i = 0; i < nrow; i++)
            {
                for(int j = 0; j < ncol; j++)
                {
                    nodes(i,j) = coord(i,j);
                }
            }
            
            GradX(nodes,loc,gradx);
        }
        
        /** @brief Compute x mapping from element nodes and local parametric coordinates */
        template<class T>
        static void X(const TPZFMatrix<REAL> &nodes,TPZVec<T> &loc,TPZVec<T> &x);
        
        /** @brief Compute gradient of x mapping from element nodes and local parametric coordinates */
        template<class T>
        static void GradX(const TPZFMatrix<T> &nodes,TPZVec<T> &loc, TPZFMatrix<T> &gradx);
        
        /** @brief Compute the shape being used to construct the x mapping from local parametric coordinates  */
        template<class T>
        static void TShape(TPZVec<T> &loc,TPZFMatrix<T> &phi,TPZFMatrix<T> &dphi);
        
//        /** @brief Compute the jacoabina associated to the x mapping from local parametric coordinates  */
//        static void Jacobian(const TPZFMatrix<REAL> &nodes,TPZVec<REAL> &param,TPZFMatrix<REAL> &jacobian,
//                             TPZFMatrix<REAL> &axes,REAL &detjac,TPZFMatrix<REAL> &jacinv);
        
		/**
		 * @brief Method which creates a geometric boundary condition 
		 * element based on the current geometric element, \n
		 * a side and a boundary condition number
		 */
		static  TPZGeoEl * CreateBCGeoEl(TPZGeoEl *orig,int side,int bc);
        
        /** @brief Implementation of Hdiv space*/
        static	void ComputeNormal(TPZVec<REAL> &p1, TPZVec<REAL> &p2,TPZVec<REAL> &p3,TPZVec<REAL> &result);
        
        static	void VectorialProduct(TPZVec<REAL> &v1, TPZVec<REAL> &v2,TPZVec<REAL> &result);
        
        static void VecHdiv(TPZFMatrix<REAL> & coord, TPZFMatrix<REAL> & fNormalVec,TPZVec<int> &sidevector);
        
        /* brief compute the vectors for defining an HDiv approximation space */
        void VecHdiv(const TPZGeoEl &gel,TPZFMatrix<REAL> &NormalVec,TPZVec<int> & VectorSide) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            VecHdiv(coord,NormalVec,VectorSide);
        }
		
	protected:
		/**
		 * @brief This method apply an infinitesimal displacement in some points
		 * to fix singularity problems when using MapToSide() method! \n
		 * This points are CornerNodes, when projected in the opposing side
		 */
		static void FixSingularity(int side, TPZVec<REAL>& OriginalPoint, TPZVec<REAL>& ChangedPoint);
		
	public:
        
        /// create an example element based on the topology
        /* @param gmesh mesh in which the element should be inserted
         @param matid material id of the element
         @param lowercorner (in/out) on input lower corner o the cube where the element should be created, on exit position of the next cube
         @param size (in) size of space where the element should be created
         */
        static void InsertExampleElement(TPZGeoMesh &gmesh, int matid, TPZVec<REAL> &lowercorner, TPZVec<REAL> &size);

		/** @brief Creates a geometric element according to the type of the father element */
		static TPZGeoEl *CreateGeoElement(TPZGeoMesh &mesh, MElementType type,
										  TPZVec<long>& nodeindexes, int matid, long& index);

	};
    
    template<class T>
    inline void TPZGeoTriangle::TShape(TPZVec<T> &loc,TPZFMatrix<T> &phi,TPZFMatrix<T> &dphi) {        
        T qsi = loc[0], eta = loc[1];
        phi(0,0) = 1.0-qsi-eta;
        phi(1,0) = qsi;
        phi(2,0) = eta;
        dphi(0,0) = dphi(1,0) = -1.0;
        dphi(0,1) = dphi(1,2) =  1.0;
        dphi(1,1) = dphi(0,2) =  0.0;
    }
    
    template<class T>
    inline void TPZGeoTriangle::X(const TPZFMatrix<REAL> &nodes,TPZVec<T> &loc,TPZVec<T> &x){
        
        TPZFNMatrix<3,T> phi(3,1);
        TPZFNMatrix<6,T> dphi(2,3);
        TShape(loc,phi,dphi);
        int space = nodes.Rows();
        
        for(int i = 0; i < space; i++) {
            x[i] = 0.0;
            for(int j = 0; j < 3; j++) {
                x[i] += phi(j,0)*nodes.GetVal(i,j);
            }
        }
    }
    
    template<class T>
    inline void TPZGeoTriangle::GradX(const TPZFMatrix<T> &nodes,TPZVec<T> &loc, TPZFMatrix<T> &gradx){
        
        gradx.Resize(3,2);
        gradx.Zero();
        int nrow = nodes.Rows();
        int ncol = nodes.Cols();
#ifdef PZDEBUG
        if(nrow != 3 || ncol  != 3){
            std::cout << "Objects of incompatible lengths, gradient cannot be computed." << std::endl;
            std::cout << "nodes matrix must be 3x3." << std::endl;
            DebugStop();
        }
        
#endif
        TPZFNMatrix<3,T> phi(NNodes,1);
        TPZFNMatrix<6,T> dphi(2,NNodes);
        TShape(loc,phi,dphi);
        for(int i = 0; i < 3; i++)
        {
            for(int j = 0; j < NNodes; j++)
            {
                gradx(j,0) += nodes.GetVal(j,i)*dphi(0,i);
                gradx(j,1) += nodes.GetVal(j,i)*dphi(1,i);
            }
        }
        
    }
	
};

#endif 
