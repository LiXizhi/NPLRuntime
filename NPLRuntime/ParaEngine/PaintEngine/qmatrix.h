#pragma once

#include "ParaRegion.h"
#include "ParaPoint.h"
#include "ParaLine.h"
#include "ParaRect.h"

namespace ParaEngine
{
	
	// 2D transform matrix
	class QMatrix
	{
	public:
		QMatrix();
		QMatrix(float m11, float m12, float m21, float m22,
			float dx, float dy);
		QMatrix(const QMatrix &matrix);

		void setMatrix(float m11, float m12, float m21, float m22,
			float dx, float dy);

		float m11() const { return _m11; }
		float m12() const { return _m12; }
		float m21() const { return _m21; }
		float m22() const { return _m22; }
		float dx() const { return _dx; }
		float dy() const { return _dy; }

		void map(int x, int y, int *tx, int *ty) const;
		void map(float x, float y, float *tx, float *ty) const;
		QRect mapRect(const QRect &) const;
		QRectF mapRect(const QRectF &) const;

		QPoint map(const QPoint &p) const;
		QPointF map(const QPointF&p) const;
		QLine map(const QLine &l) const;
		QLineF map(const QLineF &l) const;
		

		void reset();
		inline bool isIdentity() const;

		QMatrix &translate(float dx, float dy);
		QMatrix &scale(float sx, float sy);
		QMatrix &shear(float sh, float sv);
		QMatrix &rotate(float a);

		bool isInvertible() const { return !Math::FuzzyIsNull(_m11*_m22 - _m12*_m21); }
		float determinant() const { return _m11*_m22 - _m12*_m21; }

		QMatrix inverted(bool *invertible = 0) const;

		bool operator==(const QMatrix &) const;
		bool operator!=(const QMatrix &) const;

		QMatrix &operator*=(const QMatrix &);
		QMatrix operator*(const QMatrix &o) const;

		QMatrix &operator=(const QMatrix &);

	private:
		inline QMatrix(bool)
			: _m11(1.)
			, _m12(0.)
			, _m21(0.)
			, _m22(1.)
			, _dx(0.)
			, _dy(0.) {}
		inline QMatrix(float am11, float am12, float am21, float am22, float adx, float ady, bool)
			: _m11(am11)
			, _m12(am12)
			, _m21(am21)
			, _m22(am22)
			, _dx(adx)
			, _dy(ady) {}
		friend class QTransform;
		float _m11, _m12;
		float _m21, _m22;
		float _dx, _dy;
	};
	
	// mathematical semantics
	inline QPoint operator*(const QPoint &p, const QMatrix &m)
	{
		return m.map(p);
	}
	inline QPointF operator*(const QPointF &p, const QMatrix &m)
	{
		return m.map(p);
	}
	inline QLineF operator*(const QLineF &l, const QMatrix &m)
	{
		return m.map(l);
	}
	inline QLine operator*(const QLine &l, const QMatrix &m)
	{
		return m.map(l);
	}
	
	inline bool QMatrix::isIdentity() const
	{
		return Math::FuzzyIsNull(_m11 - 1) && Math::FuzzyIsNull(_m22 - 1) && Math::FuzzyIsNull(_m12)
			&& Math::FuzzyIsNull(_m21) && Math::FuzzyIsNull(_dx) && Math::FuzzyIsNull(_dy);
	}

	inline bool FuzzyCompare(const QMatrix& m1, const QMatrix& m2)
	{
		return Math::FuzzyCompare(m1.m11(), m2.m11())
			&& Math::FuzzyCompare(m1.m12(), m2.m12())
			&& Math::FuzzyCompare(m1.m21(), m2.m21())
			&& Math::FuzzyCompare(m1.m22(), m2.m22())
			&& Math::FuzzyCompare(m1.dx(), m2.dx())
			&& Math::FuzzyCompare(m1.dy(), m2.dy());
	}
}