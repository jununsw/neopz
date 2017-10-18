/**
 * @file
 * @brief Contains the TPZQuadraticCube class which defines a cube geometric element with quadratic map.
 */
 
#ifndef TPZQUADRATICCUBE_H
#define TPZQUADRATICCUBE_H


#include "TPZGeoCube.h"
#include "pzgeoel.h"
#include "pznoderep.h"

#include <iostream>

/**
 * @author Paulo Cesar de Alvarenga Lucci (Caju)
 * @since 2011
 * @brief Defines a cube geometric element with quadratic map. \ref geometry "Geometry"
 * @ingroup geometry
 */

class TPZQuadraticCube : public pzgeom::TPZNodeRep<20,pztopology::TPZCube> {
	
public:
	/** @brief Number of nodes (3 by edge) */
	enum {NNodes = 20};
    
    //virtual void ParametricDomainNodeCoord(int node, TPZVec<REAL> &nodeCoord);
    

	/** @brief Constructor from node indexes */
	TPZQuadraticCube(TPZVec<long> &nodeindexes) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZCube>(nodeindexes)
	{
	}
	/** @brief Default constructor */
	TPZQuadraticCube() : pzgeom::TPZNodeRep<NNodes,pztopology::TPZCube>()
	{
	}
	/** @brief Copy constructor from node map */
	TPZQuadraticCube(const TPZQuadraticCube &cp,std::map<long,long> & gl2lcNdMap) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZCube>(cp,gl2lcNdMap)
	{
	}
	/** @brief Copy constructor */
	TPZQuadraticCube(const TPZQuadraticCube &cp) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZCube>(cp)
	{
	}
	/** @brief Copy constructor */
	TPZQuadraticCube(const TPZQuadraticCube &cp, TPZGeoMesh &) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZCube>(cp)
	{
	}
	
	/** @brief Returns the type name of the element */
	static std::string TypeName() { return "Hexa";}
    
    static bool IsLinearMapping(int side)
    {
        return false;
    }
    
    /** @brief Compute the shape being used to construct the X mapping from local parametric coordinates  */
    static void Shape(TPZVec<REAL> &loc,TPZFMatrix<REAL> &phi,TPZFMatrix<REAL> &dphi){
        TShape(loc, phi, dphi);
    }
	
    //Needs Implementation
    template<class T>
    void XLinearMapping(const TPZGeoEl &gel,TPZVec<T> &ksi, TPZVec<T> &result) const
    {
        TPZFNMatrix<3*NNodes> coord(3,NNodes);
        CornerCoordinates(gel, coord);
        pzgeom::TPZGeoCube::X(coord,ksi,result);
    }
    
    
    /* brief compute the coordinate of a point given in parameter space */
    template<class T>
    void X(const TPZGeoEl &gel,TPZVec<T> &loc,TPZVec<T> &result) const
    {
        TPZFNMatrix<3*NNodes> coord(3,NNodes);
        CornerCoordinates(gel, coord);
        X(coord,loc,result);
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
    
    template<class T>
    static void TShape(TPZVec<T> &x,TPZFMatrix<T> &phi,TPZFMatrix<T> &dphi);

    /** @brief Compute X mapping from element nodes and local parametric coordinates */    
    template<class T>
    static void X(TPZFMatrix<REAL> &coord, TPZVec<T> &par, TPZVec<T> &result);
    
    /** @brief Compute gradient of X mapping from element nodes and local parametric coordinates */
    template<class T>
    static void GradX(TPZFMatrix<T> &nodes,TPZVec<T> &loc, TPZFMatrix<T> &gradx);

	/** @brief Creates a geometric element according to the type of the father element */
	static TPZGeoEl *CreateGeoElement(TPZGeoMesh &mesh, MElementType type,
									  TPZVec<long>& nodeindexes,
									  int matid, long& index);
	
    static void InsertExampleElement(TPZGeoMesh &gmesh, int matid, TPZVec<REAL> &lowercorner, TPZVec<REAL> &size);

	TPZGeoEl *CreateBCGeoEl(TPZGeoEl *orig,int side,int bc);	
};

#endif
