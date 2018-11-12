/** This file is part of the Pinocchio automatic rigging library.
 *  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
 *
 *  This library is an implementation of Automatic Rigging and Animation of 3D Characters, 
 *	Ilya Baran∗ Jovan Popovic´†, 2007.
 *  
 */

#ifndef VECUTILS_H_INCLUDED
#define VECUTILS_H_INCLUDED

#include "vector.h"

template<class Real>
void getBasis(const PVector<Real, 3> &n, PVector<Real, 3> &v1, PVector<Real, 3> &v2)
{
    if(n.lengthsq() < Real(1e-16)) {
        v1 = PVector<Real, 3>(1., 0., 0.);
        v2 = PVector<Real, 3>(0., 1., 0.);
        return;
    }
    if(fabs(n[0]) <= fabs(n[1]) && fabs(n[0]) <= fabs(n[2]))
        v1 = PVector<Real, 3>(1., 0., 0.);
    else if(fabs(n[1]) <= fabs(n[2]))
		v1 = PVector<Real, 3>(0., 1., 0.);
    else
		v1 = PVector<Real, 3>(0., 0., 1.);
    
    v2 = (n % v1).normalize(); //first basis vector
    v1 = (v2 % n).normalize(); //second basis vector
}

template<class Real, int Dim>
Real distsqToLine(const PVector<Real, Dim> &v, const PVector<Real, Dim> &p0, const PVector<Real, Dim> &dir)
{
  return max(Real(), (v - p0).lengthsq() - SQR((v - p0) * dir) / dir.lengthsq());
}

template<class Real, int Dim>
PVector<Real, Dim> projToLine(const PVector<Real, Dim> &v, const PVector<Real, Dim> &p0, const PVector<Real, Dim> &dir)
{
  return p0 + (((v - p0) * dir) / dir.lengthsq()) * dir;
}

template<class Real, int Dim>
Real distsqToSeg(const PVector<Real, Dim> &v, const PVector<Real, Dim> &p1, const PVector<Real, Dim> &p2)
{
  typedef PVector<Real, Dim> Vec;
  
  Vec dir = p2 - p1;
  Vec difp2 = p2 - v;
  
  if(difp2 * dir < Real())
    return difp2.lengthsq();
  
  Vec difp1 = v - p1;
  Real dot = difp1 * dir;

  if(dot <= Real())
    return difp1.lengthsq();

  return max(Real(), difp1.lengthsq() - SQR(dot) / dir.lengthsq());
}

template<class Real, int Dim>
PVector<Real, Dim> projToSeg(const PVector<Real, Dim> &v, const PVector<Real, Dim> &p1, const PVector<Real, Dim> &p2)
{
  typedef PVector<Real, Dim> Vec;

  Vec dir = p2 - p1;

  if((p2 - v) * dir < Real())  return p2;
   
  Real dot = (v - p1) * dir;

  if(dot <= Real()) return p1;
   
  return p1 + (dot / dir.lengthsq()) * dir;
}

//d is distance between centers, r1 and r2 are radii
template<class Real>
Real getCircleIntersectionArea(const Real &d, const Real &r1, const Real &r2)
{
    Real tol(1e-8);
    
    if(r1 + r2 <= d + tol)
        return Real();
    if(r1 + d <= r2 + tol)
        return Real(M_PI) * SQR(r1);
    if(r2 + d <= r1 + tol)
        return Real(M_PI) * SQR(r2);
    
    Real sqrdif = SQR(r1) - SQR(r2);
    Real dsqrdif = SQR(d) - sqrdif; //d^2 - r1^2 + r2^2
    Real a1 = SQR(r1) * acos((SQR(d) + sqrdif) / (Real(2.) * r1 * d));
    Real a2 = SQR(r2) * acos(dsqrdif / (Real(2.) * r2 * d));
    return a1 + a2 - Real(0.5) * sqrt(SQR(Real(2.) * d * r2) - SQR(dsqrdif));
}

template<class Real>
PVector<Real, 3> projToTri(const PVector<Real, 3> &from, const PVector<Real, 3> &p1, const PVector<Real, 3> &p2, const PVector<Real, 3> &p3)
{
    typedef PVector<Real, 3> Vec;
    Real tolsq(1e-16);
    
    Vec p2p1 = (p2 - p1);
    Vec p3p1 = (p3 - p1);
    Vec normal = p2p1 % p3p1;

    if((p2p1 % (from - p1)) * normal >= Real()) { //inside s1
        bool s2 = ((p3 - p2) % (from - p2)) * normal >= Real();
        bool s3 = (p3p1 % (from - p3)) * normal <= Real();
        
        if(s2 && s3) {
            if(normal.lengthsq() < tolsq)  return p1; //incorrect, but whatever...never happens
            double dot = (from - p3) * normal;
            return from - (dot / normal.lengthsq()) * normal;
        }
        if(!s3 && (s2 || (from - p3) * p3p1 >= Real())) return projToSeg(from, p3, p1);
           
        return projToSeg(from, p2, p3);
    }
    //outside s1
    if((from - p1) * p2p1 < Real()) return projToSeg(from, p3, p1);
      
    if((from - p2) * p2p1 > Real()) return projToSeg(from, p2, p3);
       
    return projToLine(from, p1, p2p1);
}

#endif //VECUTILS_H_INCLUDED
