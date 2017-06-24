/**
 * @file
 * @brief Contains the implementation of the TPZHCurlNedFTriEl methods.
 */

#include "TPZHCurlNedFTriEl.h"
#include "pzaxestools.h"
#include "pzmaterial.h"
#include "pzshapetriang.h"
#include "pzvec_extras.h"
#include "tpzgraphelt2dmapped.h"

using namespace pzshape;

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.mesh.TPZHCurlNedFTriEl"));
#endif

TPZHCurlNedFTriEl::TPZHCurlNedFTriEl(TPZCompMesh &mesh, TPZGeoEl *geoEl,
                                     long &index)
    : TPZInterpolatedElement(mesh, geoEl, index),
      fConnectIndexes(TPZShapeTriang::NFaces + 1, -1),
      fSideOrient(TPZShapeTriang::NFaces, 1) {
    geoEl->SetReference(this);

    TPZStack<int> facesides;

    TPZShapeTriang::LowerDimensionSides(TPZShapeTriang::NSides - 1, facesides,
                                        TPZShapeTriang::Dimension -
                                            1); // inserts 3 edges
    facesides.Push(TPZShapeTriang::NSides - 1); // inserts triangle face
    for (int i = 0; i < fConnectIndexes.size(); i++) {
        int side = facesides[i];
        fConnectIndexes[i] = CreateMidSideConnect(side);
        mesh.ConnectVec()[fConnectIndexes[i]].IncrementElConnected();
        this->IdentifySideOrder(side);
    }

    AdjustIntegrationRule();

    const int firstside = TPZShapeTriang::NSides - TPZShapeTriang::NFaces - 1;
    for (int side = firstside; side < TPZShapeTriang::NSides - 1; side++) {
        fSideOrient[side - firstside] =
            this->Reference()->NormalOrientation(side);
    }
}

TPZHCurlNedFTriEl::TPZHCurlNedFTriEl(TPZCompMesh &mesh,
                                     const TPZHCurlNedFTriEl &copy)
    : TPZInterpolatedElement(mesh, copy), fConnectIndexes(copy.fConnectIndexes),
      fSideOrient(copy.fSideOrient) {
    fIntRule = copy.fIntRule;
    fPreferredOrder = copy.fPreferredOrder;
}

TPZHCurlNedFTriEl::TPZHCurlNedFTriEl(TPZCompMesh &mesh,
                                     const TPZHCurlNedFTriEl &copy,
                                     std::map<long, long> &gl2lcElMap)
    : TPZInterpolatedElement(mesh, copy, gl2lcElMap),
      fConnectIndexes(copy.fConnectIndexes), fSideOrient(copy.fSideOrient) {
    fIntRule = copy.fIntRule;
    fPreferredOrder = copy.fPreferredOrder;
}

TPZHCurlNedFTriEl::TPZHCurlNedFTriEl()
    : TPZInterpolatedElement(), fConnectIndexes(TPZShapeTriang::NSides - 1) {
    fPreferredOrder = -1;
    for (int i = 0; i < fConnectIndexes.size(); i++) {
        fConnectIndexes[i] = -1;
    }
}

TPZHCurlNedFTriEl::~TPZHCurlNedFTriEl() {}

TPZHCurlNedFTriEl *TPZHCurlNedFTriEl::Clone(TPZCompMesh &mesh) const {
    DebugStop();
}

TPZHCurlNedFTriEl *
TPZHCurlNedFTriEl::ClonePatchEl(TPZCompMesh &mesh,
                                std::map<long, long> &gl2lcConMap,
                                std::map<long, long> &gl2lcElMap) const {
    DebugStop();
}

int TPZHCurlNedFTriEl::Dimension() const { return TPZShapeTriang::Dimension; }

int TPZHCurlNedFTriEl::NCornerConnects() const { return 0; }

int TPZHCurlNedFTriEl::NConnects() const { return fConnectIndexes.size(); }

long TPZHCurlNedFTriEl::ConnectIndex(int iCon) const {
#ifndef NODEBUG
    if (iCon < 0 || iCon >= this->NConnects()) {
        std::cout << "TPZHCurlNedFTriEl::ConnectIndex wrong parameter connect "
                  << iCon << " NConnects " << this->NConnects() << std::endl;
        DebugStop();
        return -1;
    }
#endif

    return this->fConnectIndexes[iCon];
}

void TPZHCurlNedFTriEl::SetConnectIndex(int i, long connectindex) {
#ifdef PZDEBUG
    if (i < 0 || i >= this->NConnects()) {
        std::cout << "TPZHCurlNedFTriEl::SetConnectIndex index " << i
                  << " out of range\n";
        DebugStop();
    }
#endif
    this->fConnectIndexes[i] = connectindex;
#ifdef LOG4CXX
    if (logger->isDebugEnabled()) {
        std::stringstream sout;
        sout << std::endl
             << "Setting Connect : " << i << " to connectindex " << connectindex
             << std::endl;
        LOGPZ_DEBUG(logger, sout.str())
    }
#endif
}

int TPZHCurlNedFTriEl::NSideConnects(int side) const {
    if (TPZShapeTriang::SideDimension(side) <= Dimension() - 2)
        return 0;
    if (TPZShapeTriang::SideDimension(side) == Dimension() - 1)
        return 1; // side connects
    if (TPZShapeTriang::SideDimension(side) == Dimension())
        return 1; // internal connects
    return -1;
} // TODO: TRANSFER TO LINEAR EL

int TPZHCurlNedFTriEl::SideConnectLocId(int con, int side) const {
#ifdef PZDEBUG
    if (TPZShapeTriang::SideDimension(side) <= TPZShapeTriang::Dimension - 2 ||
        con >= NSideConnects(side)) {
        PZError << "TPZHCurlNedFTriEl::SideConnectLocId no connect associate "
                << std::endl;
        return -1;
    }
#endif
    return side - (TPZShapeTriang::NSides -
                   TPZShapeTriang::NumSides(TPZShapeTriang::Dimension - 1) - 1);
}

int TPZHCurlNedFTriEl::NConnectShapeF(int icon, int order) const {
    const int side = icon + TPZShapeTriang::NSides -
                     TPZShapeTriang::NumSides(TPZShapeTriang::Dimension - 1) -
                     1;
#ifdef PZDEBUG
    if (side <= TPZShapeTriang::NumSides(Dimension()) - 2 ||
        side >= TPZShapeTriang::NSides) {
        DebugStop();
    }
#endif
    if (TPZShapeTriang::SideDimension(side) == Dimension() - 1) {
        if (order > EffectiveSideOrder(side) || order == 0)
            return 0;
        return 1 + NConnectShapeF(icon, order - 1);
    }
    if (TPZShapeTriang::SideDimension(side) == Dimension()) {
        if (order < 2)
            return 0;

        return 2 * order + 1 - 3 + NConnectShapeF(icon, order - 1);
    }
    return -1;
}

void TPZHCurlNedFTriEl::SideShapeFunction(int side, TPZVec<REAL> &point,
                                          TPZFMatrix<REAL> &phi,
                                          TPZFMatrix<REAL> &dphi) {
    DebugStop();
}

void TPZHCurlNedFTriEl::SetIntegrationRule(int ord) {
    TPZManVector<int, 3> order(TPZShapeTriang::Dimension, ord);
    fIntRule.SetOrder(order);
}
const TPZIntPoints &TPZHCurlNedFTriEl::GetIntegrationRule() const {
    if (this->fIntegrationRule) {
        return *fIntegrationRule;
    } else {
        return fIntRule;
    }
}

TPZIntPoints &TPZHCurlNedFTriEl::GetIntegrationRule() {
    if (fIntegrationRule) {
        return *fIntegrationRule;
    } else {
        return fIntRule;
    }
}

void TPZHCurlNedFTriEl::SetPreferredOrder(int order) {
    fPreferredOrder = order;
}

void TPZHCurlNedFTriEl::GetInterpolationOrder(TPZVec<int> &ord) { DebugStop(); }

int TPZHCurlNedFTriEl::PreferredSideOrder(int side) {
    if (TPZShapeTriang::SideDimension(side) < Dimension() - 1) {
        DebugStop();
    }
    int connect = SideConnectLocId(0, side);
    if (connect < 0 || connect > NConnects()) {
#ifdef LOG4CXX
        {
            std::stringstream sout;
            sout << "Connect index out of range connect " << connect
                 << " nconnects " << NConnects();
            LOGPZ_DEBUG(logger, sout.str())
        }
#endif
        return -1;
    }
    if (connect < NConnects()) {
        int order = this->fPreferredOrder;
        return order;
    }
    PZError << "TPZHCurlNedFTriEl::PreferredSideOrder called for connect = "
            << connect << "\n";
    return 0;
}
int TPZHCurlNedFTriEl::ConnectOrder(int connect) const {
    if (connect < 0 || connect >= this->NConnects()) {
#ifdef LOG4CXX
        {
            std::stringstream sout;
            sout << "Connect index out of range connect " << connect
                 << " nconnects " << NConnects();
            LOGPZ_DEBUG(logger, sout.str())
        }
#endif
        DebugStop();
        return -1;
    }

    if (this->fConnectIndexes[connect] == -1) {
        std::stringstream sout;
        sout << __PRETTY_FUNCTION__ << " connect " << connect
             << " is not initialized" << std::endl;
#ifdef LOG4CXX
        LOGPZ_ERROR(logger, sout.str());
#else
        std::cout << sout.str() << std::endl;
#endif
        DebugStop();
        return 0;
    }

    TPZConnect &c = this->Connect(connect);
    return c.Order();
}

int TPZHCurlNedFTriEl::EffectiveSideOrder(int side) const {
    if (!NSideConnects(side))
        return -1;
    int firstSideCon = SideConnectLocId(0, side);
    int connectOrder = ConnectOrder(firstSideCon);

    if (firstSideCon >= 0 || firstSideCon <= NConnects())
        return connectOrder;

    DebugStop();
    return -1;
}

void TPZHCurlNedFTriEl::SetSideOrder(int side, int order) {
    int connectaux = SideConnectLocId(0, side);
    if (connectaux < 0 || connectaux > this->NConnects()) {
#ifdef LOG4CXX
        std::stringstream sout;
        sout << __PRETTY_FUNCTION__ << " Bad side or order " << side
             << " order " << order;
        LOGPZ_DEBUG(logger, sout.str())
#endif
        DebugStop();
        return;
    }
    TPZConnect &c = this->Connect(connectaux);
    c.SetOrder(order, this->fConnectIndexes[connectaux]);
    long seqnum = c.SequenceNumber();
    int nvar = 1;
    TPZMaterial *mat = this->Material();
    if (mat)
        nvar = mat->NStateVariables();
    c.SetNState(nvar);
    int nshape = this->NConnectShapeF(connectaux, order);
    c.SetNShape(nshape);
    this->Mesh()->Block().Set(seqnum, nshape * nvar);
}

TPZTransform TPZHCurlNedFTriEl::TransformSideToElement(int side) {
    DebugStop();
}

void TPZHCurlNedFTriEl::ComputeShape(TPZVec<REAL> &intpoint, TPZVec<REAL> &X,
                                     TPZFMatrix<REAL> &jacobian,
                                     TPZFMatrix<REAL> &axes, REAL &detjac,
                                     TPZFMatrix<REAL> &jacinv,
                                     TPZFMatrix<REAL> &phi,
                                     TPZFMatrix<REAL> &curlPhiHat,
                                     TPZFMatrix<REAL> &curlPhi) {
    TPZGeoEl *ref = this->Reference();
#ifdef PZDEBUG
    if (!ref) {
        PZError << "\nERROR AT " << __PRETTY_FUNCTION__
                << " - this->Reference() == NULL\n";
        return;
    } // if
#endif
    TPZFMatrix<REAL> phiHat;

    ref->Jacobian(intpoint, jacobian, axes, detjac, jacinv);
    this->Shape(intpoint, phiHat, curlPhiHat);
    this->ShapeTransform(phiHat, jacinv, phi);
    this->CurlTransform(curlPhiHat, jacinv, curlPhi);
}

void TPZHCurlNedFTriEl::ShapeTransform(const TPZFMatrix<REAL> &phiHat,
                                       const TPZFMatrix<REAL> &jacinv,
                                       TPZFMatrix<REAL> &phi) {
    int nshape = phiHat.Rows();
#ifdef PZDEBUG
    if (phiHat.Cols() != 2)
        DebugStop();
#endif
    phi.Redim(phiHat.Rows(), phiHat.Cols());

    for (int iPhi = 0; iPhi < nshape; iPhi++) {
        phi(iPhi, 0) = jacinv.GetVal(0, 0) * phiHat.GetVal(iPhi, 0) +
                       jacinv.GetVal(1, 0) * phiHat.GetVal(iPhi, 1);
        phi(iPhi, 1) = jacinv.GetVal(0, 1) * phiHat.GetVal(iPhi, 0) +
                       jacinv.GetVal(1, 1) * phiHat.GetVal(iPhi, 1);

        phi(iPhi, 0) *= 2;
        phi(iPhi, 1) *= 2; // this is the scale factor
                           // present in TPZTriangle::ComputeDirections
    }
}

void TPZHCurlNedFTriEl::CurlTransform(const TPZFMatrix<REAL> &curlPhiHat,
                                      const TPZFMatrix<REAL> &jacinv,
                                      TPZFMatrix<REAL> &curlPhi) {
    int nshape = curlPhiHat.Cols();
#ifdef PZDEBUG
    if (curlPhiHat.Rows() != 1)
        DebugStop();
#endif
    curlPhi.Redim(curlPhiHat.Rows(), curlPhiHat.Cols());

    REAL detJacInv = jacinv.GetVal(0, 0) * jacinv.GetVal(1, 1) -
                     jacinv.GetVal(1, 0) * jacinv.GetVal(0, 1);
    detJacInv *= 2; // this is the scale factor present
                    // in TPZTriangle::ComputeDirections

    int ieq;
    for (ieq = 0; ieq < nshape; ieq++) {
        curlPhi(0, ieq) = detJacInv * curlPhiHat.GetVal(0, ieq);
    }
}

void TPZHCurlNedFTriEl::CreateGraphicalElement(TPZGraphMesh &grafgrid,
                                               int dimension) {
    if (dimension == TPZShapeTriang::Dimension && this->Material()->Id() > 0) {
        new TPZShapeTriang::GraphElType(this, &grafgrid);
    }
}

void TPZHCurlNedFTriEl::ComputeSolution(TPZVec<REAL> &qsi,
                                        TPZMaterialData &data) {

    ComputeSolution(qsi, data.phi, data.dphix, data.axes, data.sol, data.dsol);
}

void TPZHCurlNedFTriEl::ComputeSolution(TPZVec<REAL> &qsi,
                                        TPZFMatrix<REAL> &phi,
                                        TPZFMatrix<REAL> &dphix,
                                        const TPZFMatrix<REAL> &axes,
                                        TPZSolVec &sol, TPZGradSolVec &dsol) {
    const int dim = axes.Cols();
    const int nVar = this->Material()->NStateVariables();
    const int nCon = this->NConnects();

    TPZFMatrix<STATE> &meshSol = this->Mesh()->Solution();

    long numberSol = meshSol.Cols();
#ifdef PZDEBUG
    if (numberSol != 1) {
        DebugStop();
    }
#endif

    sol.Resize(numberSol);
    dsol.Resize(numberSol);

    for (long iSol = 0; iSol < numberSol; iSol++) {
        sol[iSol].Resize(dim, nVar);
        sol[iSol].Fill(0);
        dsol[iSol].Redim(dim, nVar);
        dsol[iSol].Zero();
    }

    // calculate shape functions
    TPZFMatrix<REAL> phiHat, phiHCurlAxes, phiHCurl;
    TPZFMatrix<REAL> curlPhiHat, curlPhiDAxes, curlPhi;

    TPZFMatrix<REAL> jacobian, jacinv, axesDummy;
    REAL detjac;

    this->Reference()->Jacobian(qsi, jacobian, axesDummy, detjac, jacinv);
    this->Shape(qsi, phiHat, curlPhiHat);
    this->ShapeTransform(phiHat, jacinv, phiHCurlAxes);
    this->CurlTransform(curlPhiHat, jacinv, curlPhiDAxes);

    TPZAxesTools<REAL>::Axes2XYZ(phiHCurlAxes, phiHCurl, axes, false);

    TPZManVector<REAL, 3> ax1(3), ax2(3), elNormal(3);
    for (int i = 0; i < 3; i++) {
        ax1[i] = axes.GetVal(0, i);
        ax2[i] = axes.GetVal(1, i);
    }
    Cross(ax1, ax2, elNormal);
    TPZFNMatrix<3, REAL> normalVec(1, 3);
    normalVec(0, 0) = elNormal[0];
    normalVec(0, 1) = elNormal[1];
    normalVec(0, 2) = elNormal[2];

    TPZAxesTools<REAL>::Axes2XYZ(curlPhiDAxes, curlPhi, normalVec);

    TPZBlock<STATE> &block = this->Mesh()->Block();
    int ishape = 0;
    for (int iCon = 0; iCon < nCon; iCon++) {
        TPZConnect *con = &this->Connect(iCon);
        long conSeqN = con->SequenceNumber();
        int nShapeCon = block.Size(conSeqN);
        long pos = block.Position(conSeqN);

        for (int jShape = 0; jShape < nShapeCon; jShape++) {

            for (long iSol = 0; iSol < numberSol; iSol++) {
                for (int coord = 0; coord < dim; coord++) {
                    sol[iSol][coord] +=
                        (STATE)meshSol(pos + jShape, iSol) * phiHCurl(ishape, coord);
                    dsol[iSol](coord, nVar - 1) +=
                        (STATE)meshSol(pos + jShape, iSol) * curlPhi(coord, ishape);
                }
                ishape++;
            }
        }
    }
}

void TPZHCurlNedFTriEl::InitMaterialData(TPZMaterialData &data){
	data.fShapeType = TPZMaterialData::EVecShape;
}

TPZCompEl *CreateHCurlNedFTriEl(TPZGeoEl *gel, TPZCompMesh &mesh, long &index) {
    return new TPZHCurlNedFTriEl(mesh, gel, index);
}
