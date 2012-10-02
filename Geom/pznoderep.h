/**
 * @file
 * @brief Contains the TPZNodeRep class which implements ...
 */

#ifndef PZNODEREPH
#define PZNODEREPH

#include "pzvec.h"

#include <map>
#include <memory.h>

#include "pzmanvector.h"
#include "pztrnsform.h"

class TPZGeoMesh;
class TPZGeoEl;
class TPZGeoElSide;
#include "pzgeoel.h"

#include "pzlog.h"
#ifdef LOG4CXX
static LoggerPtr lognoderep(Logger::getLogger("pz.geom.tpznoderep"));
#endif

namespace pzgeom {
	
	/**
	 * @ingroup topology
	 * @brief Initializing tolerance to TPZNodeRep.
	 */
	const double pzgeom_TPZNodeRep_tol = 1.E-6;
	
	/**
	 * @ingroup geometry
	 * @ingroup topology
	 * @brief Implements ... \ref geometry "Geometry" \ref topology "Topology"
	 */
	template<int N, class Topology>
	class TPZNodeRep : public Topology
	{
		
	private:
		
		/** @brief Verifies if pt (in parametric domain of the side) is within boundaries */
		bool IsInSideParametricDomain(int side, TPZVec<REAL> &pt, REAL tol);
		
	public:
		
		virtual void SetNeighbourInfo(int side, TPZGeoElSide &neigh, TPZTransform &trans) {
			std::cout << "Element that is NOT TPZGeoBlend trying to Set Neighbour Information on Geometric Mesh!\n";
			std::cout << "See TPZGeoElRefLess::SetNeighbourInfo() Method!\n";
			DebugStop();
		}
		
		bool IsLinearMapping() const { return true; }
		bool IsGeoBlendEl() const 
		{ 
			return false; 
		}
		
		static const int NNodes=N;
		/** @brief Node indexes of the element */
		int fNodeIndexes[N];
		/** @brief Constructor with list of nodes */
		TPZNodeRep(TPZVec<int> &nodeindexes)
		{
			int nn = nodeindexes.NElements() < N ? nodeindexes.NElements() : N;
			memcpy(fNodeIndexes,&nodeindexes[0],nn*sizeof(int));
			int i;
			for(i=nn; i<N; i++) fNodeIndexes[i]=-1;
		}
		
		/** @brief Empty constructor */
		TPZNodeRep()
		{
			int i;
			for(i=0; i<N; i++) fNodeIndexes[i]=-1;
		}
		
		/** @brief Constructor with node map */
		TPZNodeRep(const TPZNodeRep &cp,
				   std::map<int,int> & gl2lcNdMap);
		
		/** @brief Copy constructor */
		TPZNodeRep(const TPZNodeRep<N,Topology> &cp)
		{
			memcpy(fNodeIndexes,cp.fNodeIndexes,N*sizeof(int));
		}
        
        void Read(TPZStream &buf, void *context)
        {
            buf.Read(fNodeIndexes,NNodes);
        }
        
        void Write(TPZStream &buf)
        {
            buf.Write(fNodeIndexes,NNodes);
		}
        
		void Initialize(TPZVec<int> &nodeindexes)
		{
			int nn = nodeindexes.NElements() < N ? nodeindexes.NElements() : N;
#ifndef NODEBUG
			if(nodeindexes.NElements() != N)
			{
				std::stringstream sout;
				sout << __PRETTY_FUNCTION__ << " Nodeindexes have wrong size " << nodeindexes.NElements() << " but should be " << N;
#ifdef LOG4CXX
				LOGPZ_ERROR(lognoderep,sout.str().c_str());
#else
				std::cout << sout.str().c_str() << std::endl;
#endif
			}
#endif
			memcpy(fNodeIndexes,&nodeindexes[0],nn*sizeof(int));
			int i;
			for(i=nn; i<N; i++) fNodeIndexes[i]=-1;
			
		}
		void Initialize(TPZGeoEl *refel)
		{
		}
        
		/** @brief Gets the corner node coordinates in coord */
        void CornerCoordinates(const TPZGeoEl &gel, TPZFMatrix<REAL> &coord) const
        {
            TPZGeoNode *np;
            int i,j;
            for(i=0;i<NNodes;i++) {
                np = gel.NodePtr(i);
                for(j=0;j<3;j++) {
                    coord(j,i) = np->Coord(j);
                }
            }
        }
		
		/** 
		 * @brief Projects point pt (in parametric coordinate system) in the element parametric domain.
		 * @return Returns the side where the point was projected.
		 * @note Observe that if the point is already in the parametric domain, the method will return
		 * \f$ NSides() - 1 \f$
		 */
		int ProjectInParametricDomain(TPZVec<REAL> &pt, TPZVec<REAL> &ptInDomain){
			const int nsides = Topology::NSides;
            const int dim = Topology::Dimension;
            
            ptInDomain.Resize(dim);
            
			if(this->IsInParametricDomain(pt,0.))///it is already in the domain
            {
				ptInDomain = pt;
				return nsides-1;
			}
			
            REAL tol = 1.e-10;
            
            ///first, will be made a project to center direction
            TPZVec<REAL> OutPt(pt), InnPt(dim);
            Topology::CenterPoint(nsides-1,InnPt);
            
            REAL dist = 0.;
            for(int c = 0; c < dim; c++)
            {
                dist += (InnPt[c] - OutPt[c])*(InnPt[c] - OutPt[c]);
            }
            dist = sqrt(dist);
            
            while(dist > tol)
            {
                for(int c = 0; c < dim; c++)
                {
                    ptInDomain[c] = (InnPt[c] + OutPt[c])/2.;
                }
                if(this->IsInParametricDomain(ptInDomain,0.))
                {
                    InnPt = ptInDomain;
                }
                else
                {
                    OutPt = ptInDomain;
                }
                dist = 0.;
                for(int c = 0; c < dim; c++)
                {
                    dist += (InnPt[c] - OutPt[c])*(InnPt[c] - OutPt[c]);
                }
                dist = sqrt(dist);
            }
            
            ///found in witch side the projection belongs
   			int winnerSide = -1;
			TPZManVector<REAL,3> pt1(dim), pt2(dim);
			for(int is = 0; is < nsides-1; is++)
            {
				///Go orthogonally from \f$ NSides-1 \f$ to side is
				TPZTransform T1 = Topology::SideToSideTransform(nsides-1, is);
				T1.Apply(ptInDomain,pt1);
				
				///Come back from side is to \f$ NSides-1 \f$
				TPZTransform T2 = Topology::SideToSideTransform(is,nsides-1);
				T2.Apply(pt1,pt2);
				
				///Compare ptInDomain to transformed point
				dist = 0.;
				for(int c = 0; c < dim; c++)
                {
					dist += (ptInDomain[c]-pt2[c]) * (ptInDomain[c]-pt2[c]);
				}//i
				dist = sqrt(dist);
				
				///Closest side
				if(dist < tol)
                {
					winnerSide = is;
					ptInDomain = pt2;
                    break;
				}
			}//for is
			
			return winnerSide;
			
		}//method
		
		void Print(std::ostream &out) const
		{
			int nn;
			out << "Nodeindices :";
			for(nn=0; nn<N; nn++)
			{
				out << fNodeIndexes[nn] << ' ';
			}
			out << std::endl;
		}
		
    protected:
		/**
		 * @brief This method is redefined in TPZGeoTriangle, TPZGeoPrism, TPZGeoTetrahedra, TPZGeoPyramid \n
		 * to fix singularity problems when using MapToSide() method!
		 */
		static void FixSingularity(int side, TPZVec<REAL>& OriginalPoint, TPZVec<REAL>& ChangedPoint)
		{
			ChangedPoint.Resize(OriginalPoint.NElements(),0.);
			ChangedPoint = OriginalPoint;
		}
	};
};

#include "pznoderep.h.h"

#endif
