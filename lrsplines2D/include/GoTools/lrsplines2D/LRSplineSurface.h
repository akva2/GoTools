#ifndef LR_SPLINESURFACE_H
#define LR_SPLINESURFACE_H

#include <array>
#include <functional>
#include <set>
#include <map>
#include <vector>
#include <unordered_map>
#include <iostream> // @@ debug

#include "GoTools/geometry/SplineSurface.h"
#include "GoTools/lrsplines2D/Mesh2D.h"
#include "GoTools/lrsplines2D/LRBSpline2D.h"
#include "GoTools/lrsplines2D/Element2D.h"

namespace Go
{
  // =============================================================================
  class LRSplineSurface : public ParamSurface
// =============================================================================
{
   public:
  //
  // SEARCH STRUCTURES
  //
  // Structure representing a refinement to carry out.  This structure is used as an
  // argument to the LRSplineSurface::refine() methods below.  It is particularly useful
  // when passing along a whole batch of refinements. 
  struct Refinement2D {
    double kval;      // value of the fixed parameter of the meshrectangle to insert 
    double start;     // start value of the meshrectangle's non-fixed parameter
    double end;       // end value of the meshrectangle's non-fixed parameter
    Direction2D d;    // direction of the meshrectangle (XFIXED or YFIXED)
    int multiplicity; // multiplicity of the meshrectangle 
  };

  // 'BSKey' defines the key for storing/looking-up individual B-spline basis 
  // functions. 'u_min','v_min', 'u_max' and 'v_max' designate the support corners
  // in the parametric domain. 'u_mult' and 'v_mult' designate the knot multiplicities
  // at the lower left corner.  Taken together, these six values uniquely determine
  // a particular B-spline function of a given bi-degree in a given mesh.
  //
  // NB: Note the oddity of using a structure with 'double' values as a key to 
  // a STL map.  This works as long as the values are _never_ separately computed, 
  // but  _always_ copied from the _same_ source before insertion or lookup.  
  // This can safely be assumed here though, as the doubles will all be taken
  // (copied) directly from the mesh (which never does any arithmetic on knot 
  // values, only stores them).  The reason for choosing to store 'double' values
  // within the key rather than just indices to an external knotvector is that
  // a later mesh refinement will change indexing of the knot vectors, whereas 
  // keys in an STL map are not allowed to change (for good reason).  The parameter
  // values themselves do not change, though, which make them suitable for use
  // in the key.
  struct BSKey 
  {
    double u_min, v_min, u_max, v_max;
    int u_mult1, v_mult1, u_mult2, v_mult2;
    bool operator<(const BSKey& rhs) const; // needed for sorting when used in an STL map
  };

  // these maps could be redefined as hash tables later, as this is likely to improve
  // performance (at the expense of having to specify hash functions for these types of keys).
  typedef std::map<BSKey, shared_ptr<LRBSpline2D> > BSplineMap; // storage of basis functions


  struct double_pair_hash {
    size_t operator()(const std::pair<double, double>& dp) const {
      // Two hashes for independent variables, combined using XOR.  It is expected
      // that the resulting hash is probably as good as the input hashes.
      return std::hash<double>()(dp.first) ^ std::hash<double>()(dp.second);
    }
  };


  // Function for generating the key to use when storing B-spline function 'b'.  (This is an 
  // implementation detail that should not worry users).
  static BSKey generate_key(const LRBSpline2D& b, const Mesh2D& m);
  static BSKey generate_key(const LRBSpline2D& b);

  // The ElementMap will be used to keep track over which BasisFunctions are covering each
  // element.  An element is represented by its lower-left coordinates (doubles). 
  // Note: the same comment regarding the use of 'double' as a key in a map applies here 
  // (c.f. comment above on BSKey).
   struct ElemKey
  {
    double u_min, v_min;
    bool operator<(const ElemKey& rhs) const; // needed for sorting when used in an STL map
  };
    
  // these maps could be redefined as hash tables later, as this is likely to improve
  // performance (at the expense of having to specify hash functions for these types of keys).
  typedef std::map<ElemKey, shared_ptr<Element2D> > ElementMap; // storage of basis functions
  // Function for generating the key to use when storing elemen 'elem'.  (This is an 
  // implementation detail that should not worry users).

  static ElemKey generate_key(const double&, const double&);

  // ----------------------------------------------------
  // ---- CONSTRUCTORS, COPY, SWAP AND ASSIGNMENT -------
  // ----------------------------------------------------
  // Construct a LR-spline tensor-product surface with k-multiple 
  // knots at endpoints.
  // The coefficients are assumed to be stored sequentially, 
  // with the shortest stride in the u-direction.
  template<typename KnotIterator, typename CoefIterator>
  LRSplineSurface(int deg_u,
	   int deg_v,
	   int coefs_u,
	   int coefs_v,
	   int dimension,
	   KnotIterator knotvals_u_start,
	   KnotIterator knotvals_v_start,
	   CoefIterator coefs_start,
	   double knot_tol = 1.0e-8);

  // Construct a LR-spline tensor-product surface with k-multiple
  // knots at endpoints, and with all coefficients set to the origin
  // in the appropriate dimension.
  template<typename KnotIterator>
  LRSplineSurface(int deg_u,
	   int deg_v,
	   int coefs_u,
	   int coefs_v,
	   int dimension,
	   KnotIterator knotvals_u_start,
	   KnotIterator knotvals_v_start,
	   double knot_tol = 1.0e-8);

  // Construct a LRSplineSurface based on a spline surface
  LRSplineSurface(SplineSurface *surf, double knot_tol);

  // construct empty, invalid spline
  LRSplineSurface() {} 

  // Copy constructor
  LRSplineSurface(const LRSplineSurface& rhs);

  // Constructor reading from an input stream
  LRSplineSurface(std::istream& is) { read(is);}
  
  // Swap operator (swap contents of two LRSplineSurfaces).
  void swap(LRSplineSurface& rhs);

  // ----------------------------------------------------
  // ------- READ AND WRITE FUNCTIONALITY ---------------
  // ----------------------------------------------------
  virtual void  read(std::istream& is);       
  virtual void write(std::ostream& os) const; 

  // ----------------------------------------------------
  // Inherited from GeomObject
  // ----------------------------------------------------
    virtual ClassType instanceType() const;

    static ClassType classType()
    { return Class_LRSplineSurface; }

    virtual LRSplineSurface* clone() const
    { return new LRSplineSurface(*this); }

    /// Return the spline surface represented by this surface, if any
    virtual SplineSurface* asSplineSurface() 
    {
      return NULL;
    }

  // inherited from GeomObject
  virtual BoundingBox boundingBox() const;

  // inherited from GeomObject
  // Dimension of function codomain (e.g. dimension of geometry space)
  virtual int dimension() const
  {
    return bsplines_.size() > 0 ? 
      bsplines_.begin()->second->dimension() - (int)rational_ : 0;
  }
    
   // ----------------------------------------------------
  // Inherited from ParamSurface
  // ----------------------------------------------------
    virtual const RectDomain& parameterDomain() const;

    virtual RectDomain containingDomain() const;

    /// Check if a parameter pair lies inside the domain of this surface
    virtual bool inDomain(double u, double v) const;

    virtual Point closestInDomain(double u, double v) const;

    // inherited from ParamSurface
    virtual CurveLoop outerBoundaryLoop(double degenerate_epsilon
					  = DEFAULT_SPACE_EPSILON) const;
    // inherited from ParamSurface
    virtual std::vector<CurveLoop> allBoundaryLoops(double degenerate_epsilon
						      = DEFAULT_SPACE_EPSILON) const;

    // inherited from ParamSurface
    virtual void point(Point& pt, double upar, double vpar) const;

    // Output: Partial derivatives up to order derivs (pts[0]=S(u,v),
    // pts[1]=dS/du=S_u, pts[2]=S_v, pts[3]=S_uu, pts[4]=S_uv, pts[5]=S_vv, ...)
    // inherited from ParamSurface
    virtual void point(std::vector<Point>& pts, 
		       double upar, double vpar,
		       int derivs,
		       bool u_from_right = true,
		       bool v_from_right = true,
		       double resolution = 1.0e-12) const;

    /// Get the start value for the u-parameter
    /// \return the start value for the u-parameter
    virtual double startparam_u() const;

    /// Get the start value for the v-parameter
    /// \return the start value for the v-parameter
    virtual double startparam_v() const;

    /// Get the end value for the u-parameter
    /// \return the end value for the u-parameter
    virtual double endparam_u() const;

    /// Get the end value for the v-parameter
    /// \return the end value for the v-parameter
    virtual double endparam_v() const;

     // inherited from ParamSurface
    virtual void normal(Point& n, double upar, double vpar) const;

    /// Function that calls normalCone(NormalConeMethod) with method =
    /// SederbergMeyers. Needed because normalCone() is virtual! 
    /// (Inherited from ParamSurface).
    /// \return a DirectionCone (not necessarily the smallest) containing all normals 
    ///         to this surface.
    virtual DirectionCone normalCone() const;

    // inherited from ParamSurface
    virtual DirectionCone tangentCone(bool pardir_is_u) const;

    // Creates a composite box enclosing the surface. The composite box
    // consists of an inner and an edge box. The inner box is
    // made from the interior control points of the surface, while the
    // edge box is made from the boundary control points.
    // Inherited from ParamSurface
    virtual CompositeBox compositeBox() const;

   // inherited from ParamSurface
    virtual std::vector<shared_ptr<ParamSurface> >
    subSurfaces(double from_upar, double from_vpar,
		double to_upar, double to_vpar,
		double fuzzy = DEFAULT_PARAMETER_EPSILON) const;

    /// Mirror a surface around a specified plane
    virtual LRSplineSurface* mirrorSurface(const Point& pos, const Point& norm) const;
    // inherited from ParamSurface
    virtual void closestBoundaryPoint(const Point& pt,
				      double&        clo_u,
				      double&        clo_v, 
				      Point&         clo_pt,
				      double&        clo_dist,
				      double         epsilon,
				      const RectDomain* rd = NULL,
				      double *seed = 0) const;
    // inherited from ParamSurface
    virtual void getBoundaryInfo(Point& pt1, Point& pt2, 
				 double epsilon, SplineCurve*& cv,
				 SplineCurve*& crosscv, double knot_tol = 1e-05) const;

    // inherited from ParamSurface
    virtual void turnOrientation();

    // inherited from ParamSurface
    virtual void swapParameterDirection();

    // inherited from ParamSurface
    virtual void reverseParameterDirection(bool direction_is_u);

    /// Compute the total area of this surface up to some tolerance
    /// \param tol the relative tolerance when approximating the area, i.e.
    ///            stop iteration when error becomes smaller than
    ///            tol/(surface area)
    /// \return the area calculated
    virtual double area(double tol) const;

    /// Return surface corners, geometric and parametric points
    /// in that sequence
    virtual void 
      getCornerPoints(std::vector<std::pair<Point,Point> >& corners) const;

    // inherited from ParamSurface
    virtual std::vector< shared_ptr<ParamCurve> >
    constParamCurves(double parameter, bool pardir_is_u) const;

    bool isDegenerate(bool& b, bool& r,
		      bool& t, bool& l, double tolerance) const;

    /// Check for paralell and anti paralell partial derivatives in surface corners
    virtual void getDegenerateCorners(std::vector<Point>& deg_corners, double tol) const;

        // inherited from ParamSurface
    virtual double nextSegmentVal(int dir, double par, bool forward, double tol) const;

    // ----------------------------------------------------
  // --------------- QUERY FUNCTIONS --------------------
  // ----------------------------------------------------
  // point evaluation
  Point operator()(double u, double v, int u_deriv = 0, int v_deriv = 0) const; // evaluation

  /* virtual void point(Point &pt, double u, double v, int iEl=-1) const; */
  /* virtual void point(Point &pt, double u, double v, int iEl, bool u_from_right, bool v_from_right) const; */
  /* virtual void point(std::vector<Point> &pts, double upar, double vpar,  */
  /* 		     int derivs, int iEl=-1) const; */

  // Query parametric domain (along first (x) parameter: d = XFIXED; along second (y) parameter: YFIXED)
  double paramMin(Direction2D d) const;
  double paramMax(Direction2D d) const;

  // Query spline polynomial degree
  int degree(Direction2D d) const;

  // get a reference to the box partition (the underlying mesh)
  const Mesh2D& mesh() const {return mesh_;} 
  
  // Total number of separate basis functions defined over the box partition
  int numBasisFunctions() const {return (int)bsplines_.size();}

  int numElements() const {return (int)emap_.size();}

  // @@@ VSK. This functionality interface is fetched from the Trondheim code
  // We need a storage for last element evaluated. Index or reference?
  // Should the element be identified by index or reference?
  // How should the set of elements be traversed? Iterator?
  void computeBasis (double param_u, double param_v, BasisPtsSf     & result, int iEl=-1 ) const;
  void computeBasis (double param_u, double param_v, BasisDerivsSf  & result, int iEl=-1 ) const;
  void computeBasis (double param_u, double param_v, BasisDerivsSf2 & result, int iEl=-1 ) const;
  void computeBasis (double param_u,
		     double param_v,
		     std::vector<std::vector<double> >& result,
		     int derivs=0,
		     int iEl=-1 ) const;
  int getElementContaining(double u, double v) const;

  // Returns a pair.  
  // The first element of this pair is a pair of doubles representing the lower-left
  // corner of the element in which the point at (u, v) is located.  
  // The second element of this pair is a vector of pointers to the LRBSpline2Ds that cover
  // this element. (Ownership of the pointed-to LRBSpline2Ds is retained by the LRSplineSurface).
  const ElementMap::value_type& coveringElement(double u, double v) const;

  // Returns pointers to all basis functions whose support covers the parametric point (u, v). 
  // (NB: ownership of the pointed-to LRBSpline2Ds is retained by the LRSplineSurface.)
  std::vector<LRBSpline2D*> basisFunctionsWithSupportAt(double u, double v) const;

  // Return an iterator to the beginning/end of the map storing the LRBSpline2Ds
  BSplineMap::const_iterator basisFunctionsBegin() const {return bsplines_.begin();}
  BSplineMap::const_iterator basisFunctionsEnd()   const {return bsplines_.end();}

  // The following function returns 'true' if the underlying mesh is a regular grid, i.e. 
  // the surface is a tensor product spline surface.
  bool isFullTensorProduct() const;

  // ----------------------------------------------------
  // --------------- EDIT FUNCTIONS ---------------------
  // ----------------------------------------------------
  
  // Insert a single refinement in the mesh.  
  // If 'absolute' is set to 'false' (default), the refinement will _increment_ multiplicty
  // of the involved meshrectangles by 'mult'.  If set to 'true', the refinement 
  // will _set_ the multiplicity of the involved meshrectangles to 'mult' (however, if this results in
  // a _decrease_ of multiplicity for any involved meshrectangle, the method will throw an error instead).
  // The method will also throw an error if the resulting multiplicity for any meshrectangle would
  // end up being higher than degree+1.
  void refine(Direction2D d, double fixed_val, double start, double end, int mult = 1, bool absolute=false);

  // Same function as previous, but information about the refinement is passed along in a 'Refinement2D' structure
  // (defined above).  The 'absolute' argument works as in the previous refine() method.
  void refine(const Refinement2D& ref, bool absolute=false);

  // Insert a batch of refinements simultaneously.  The 'absolute' argument works as in the two 
  // preceding refine() methods.
  void refine(const std::vector<Refinement2D>& refs, bool absolute=false);

  // @@@ VSK. Index or iterator? Must define how the elements or bsplines 
  // are refined and call one of the other functions (refine one or refine
  // many). Is there a limit where one should be chosen before the other?
  // Refinement of one element really doesn't make sense, but it could b
  // a part of a larger strategy
  void refineBasisFunction(int index);
  void refineBasisFunction(const std::vector<int> &indices);
  void refineElement(int index);
  void refineElement(const std::vector<int> &indices);
  
  // Set the coefficient of the LRBSpline2D pointed to by 'target' to 'value'.
  // Conditions for calling this function are:
  // 1) 'target' should be a LRBSpline2D that belongs to the LRSplineSurface.
  // 2) The dimension of 'value' should be equal to the dimension of the LRSplineSurface image (e.g.
  //    the value returned by LRSplineSurface::dimension().
  void setCoef(const Point& value, const LRBSpline2D* target);

  // Set the coefficient of the LRBSpline2D with support as specified by the knots
  // with indices 'umin_ix', 'vmin_ix', 'umax_ix' and 'vmax_ix' in the mesh, and whose
  // knot multiplicities at the lower-left corner are indicated by u_mult and v_mult respectively.  
  // Throws if no such LRBSpline2D exists, or if the dimension of 'value' is not 
  // equal to LRSplineSurface::dimension().
  void setCoef(const Point& value, int umin_ix, int vmin_ix, int umax_ix, int vmax_ix, 
	       int u_mult = 1, int v_mult = 1);
  
  // Convert the LRSplineSurface to its full tensor product spline representation (NB: not reversible!)
  void expandToFullTensorProduct(); 

  // Convert a 1-D LRSplineSurface ("function") to a 3-D spline, by using the Greville points as x-
  // and y-coordinates, and the LRSplineSurface function value as z-coordinate.  
  // Requires that the LRSplineSurface is 1-D, and that the degree is > 0.
  void to3D();

  // ----------------------------------------------------
  // --------------- DEBUG FUNCTIONS --------------------
  // ----------------------------------------------------

  /* // Produce an ASCII image of the underlying mesh.  */
  /* // NB: requires a wide character stream (wostream). */
  /* void plotMesh(std::wostream& os) const; */

  /* // Produce a series of ASCII images showing the support of */
  /* // each basis function and its relation to the mesh. */
  /* // NB: requires a wide character stream (wostream). */
  /* void plotBasisFunctionSupports(std::wostream& os) const; */

  ElementMap::const_iterator elementsBegin() const { return emap_.begin();}
  ElementMap::const_iterator elementsEnd()   const { return emap_.end();}
  
 private:

  // ----------------------------------------------------
  // ----------------- PRIVATE DATA ---------------------
  // ----------------------------------------------------

  double knot_tol_;       // Tolerance for when to consider two knot values distinct rather than 
                          // a single one of higher multiplicity.

  bool rational_;

  Mesh2D mesh_;           // Represents mesh topology, multiplicites, as well as knot values.

  BSplineMap bsplines_;   // Map of individual b-spline basis functions.  

  ElementMap emap_;       // Map of individual elements

  void expand_to_full_(Direction2D d); // used by expandToFullTensorProduct()

  // Locate all elements in a mesh
  static ElementMap construct_element_map_(const Mesh2D&, const BSplineMap&);

}; // end class LRSplineSurface



}; // end namespace Go

#include "LRSplineSurface_impl.h"


#endif