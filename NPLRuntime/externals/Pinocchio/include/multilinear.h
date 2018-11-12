/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#pragma once

#include "prect.h"

namespace Pinocchio {

	//multilinear function over unit hypercube
	template<class Value, int Dim>
	class Multilinear
	{
	public:
		Multilinear()
		{
			for (int i = 0; i < num; ++i)
				values[i] = Value();
		}

		void setValue(int idx, const Value &value) { values[idx] = value; }
		const Value &getValue(int idx) const { return values[idx]; }

		template<class Real>
		Real evaluate(const PVector<Real, Dim> &v) const
		{
			Real out(0);
			for (int i = 0; i < num; ++i) {
				PVector<Real, Dim> corner;
				BitComparator<Dim>::assignCorner(i, v, PVector<Real, Dim>(1.0) - v, corner);
				Real factor = corner.accumulate(ident<Real>(), multiplies<Real>());
				out += (factor * Real(values[i]));
			}
			return out;
		}

		template<class Real>
		Real integrate(const Rect<Real, Dim> &r) const
		{
			return r.isEmpty() ? Real() : evaluate(r.getCenter()) * r.getContent();
		}

	private:
		Multilinear(const Multilinear &); //noncopyable

		template<class Real>
		static Real pos(const Real &r1, const Real &r2)
		{
			if (r1 <= Real(0) && r2 <= Real(0))
				return Real(0);
			if (r1 >= Real(0) && r2 >= Real(0))
				return Real(1);
			if (r2 >= r1)
				return r2 / (r2 - r1);
			return r1 / (r1 - r2);
		}


		static const int num = (1 << Dim);
		Value values[num];
	};

} // namespace Pinocchio

