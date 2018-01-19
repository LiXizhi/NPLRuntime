#pragma once
#include "qmatrix.h"
#include "ParaRegion.h"
#include "ParaPoint.h"
#include "ParaRect.h"
#include "ParaLine.h"

namespace ParaScripting{
	class ParaPainter;
}

namespace ParaEngine
{
	class QTransform
	{
	public:
		enum TransformationType {
			TxNone = 0x00,
			TxTranslate = 0x01,
			TxScale = 0x02,
			TxRotate = 0x04,
			TxShear = 0x08,
			TxProject = 0x10
		};

		QTransform();
		QTransform(float h11, float h12, float h13,
			float h21, float h22, float h23,
			float h31, float h32, float h33 = 1.0);
		QTransform(float h11, float h12, float h21,
			float h22, float dx, float dy);
		explicit QTransform(const QMatrix &mtx);

		bool isAffine() const;
		bool isIdentity() const;
		bool isInvertible() const;
		bool isScaling() const;
		bool isRotating() const;
		bool isTranslating() const;

		/*!
		Returns the transformation type of this matrix.

		The transformation type is the highest enumeration value
		capturing all of the matrix's transformations. For example,
		if the matrix both scales and shears, the type would be \c TxShear,
		because \c TxShear has a higher enumeration value than \c TxScale.

		Knowing the transformation type of a matrix is useful for optimization:
		you can often handle specific types more optimally than handling
		the generic case.
		*/
		TransformationType type() const;

		inline float determinant() const;
		float det() const;

		float m11() const;
		float m12() const;
		float m13() const;
		float m21() const;
		float m22() const;
		float m23() const;
		float m31() const;
		float m32() const;
		float m33() const;
		float dx() const;
		float dy() const;

		void setMatrix(float m11, float m12, float m13,
			float m21, float m22, float m23,
			float m31, float m32, float m33);

		QTransform inverted(bool *invertible = 0) const;
		QTransform adjoint() const;
		QTransform transposed() const;

		QTransform &translate(float dx, float dy);
		QTransform &scale(float sx, float sy);
		QTransform &shear(float sh, float sv);
		QTransform &rotate(float a, ParaEngine::Axis axis = ZAxis);
		QTransform &rotateRadians(float a, ParaEngine::Axis axis = ZAxis);

		bool operator==(const QTransform &) const;
		bool operator!=(const QTransform &) const;

		QTransform &operator*=(const QTransform &);
		QTransform operator*(const QTransform &o) const;

		QTransform &operator=(const QTransform &);

		void reset();
		QPoint       map(const QPoint &p) const;
		QPointF      map(const QPointF &p) const;
		QLine        map(const QLine &l) const;
		QLineF       map(const QLineF &l) const;
		QRegion      map(const QRegion &r) const;
		QRect mapRect(const QRect &) const;
		QRectF mapRect(const QRectF &) const;
		void map(int x, int y, int *tx, int *ty) const;
		void map(float x, float y, float *tx, float *ty) const;

		const QMatrix &toAffine() const;

		QTransform &operator*=(float div);
		QTransform &operator/=(float div);
		QTransform &operator+=(float div);
		QTransform &operator-=(float div);

		static QTransform fromTranslate(float dx, float dy);
		static QTransform fromScale(float dx, float dy);

	private:
		friend class ParaPainter;
		inline QTransform(float h11, float h12, float h13,
			float h21, float h22, float h23,
			float h31, float h32, float h33, bool)
			: affine(h11, h12, h21, h22, h31, h32, true)
			, m_13(h13), m_23(h23), m_33(h33)
			, m_type(TxNone)
			, m_dirty(TxProject) {}
		inline QTransform(bool)
			: affine(true)
			, m_13(0), m_23(0), m_33(1)
			, m_type(TxNone)
			, m_dirty(TxNone) {}
		inline TransformationType inline_type() const;
		QMatrix affine;
		float   m_13;
		float   m_23;
		float   m_33;

		mutable unsigned int m_type : 5;
		mutable unsigned int m_dirty : 5;
	};
	
	/******* inlines *****/
	inline QTransform::TransformationType QTransform::inline_type() const
	{
		if (m_dirty == TxNone)
			return static_cast<TransformationType>(m_type);
		return type();
	}

	inline bool QTransform::isAffine() const
	{
		return inline_type() < TxProject;
	}
	inline bool QTransform::isIdentity() const
	{
		return inline_type() == TxNone;
	}

	inline bool QTransform::isInvertible() const
	{
		return !Math::FuzzyIsNull(determinant());
	}

	inline bool QTransform::isScaling() const
	{
		return type() >= TxScale;
	}
	inline bool QTransform::isRotating() const
	{
		return inline_type() >= TxRotate;
	}

	inline bool QTransform::isTranslating() const
	{
		return inline_type() >= TxTranslate;
	}

	inline float QTransform::determinant() const
	{
		return affine._m11*(m_33*affine._m22 - affine._dy*m_23) -
			affine._m21*(m_33*affine._m12 - affine._dy*m_13) + affine._dx*(m_23*affine._m12 - affine._m22*m_13);
	}
	inline float QTransform::det() const
	{
		return determinant();
	}
	inline float QTransform::m11() const
	{
		return affine._m11;
	}
	inline float QTransform::m12() const
	{
		return affine._m12;
	}
	inline float QTransform::m13() const
	{
		return m_13;
	}
	inline float QTransform::m21() const
	{
		return affine._m21;
	}
	inline float QTransform::m22() const
	{
		return affine._m22;
	}
	inline float QTransform::m23() const
	{
		return m_23;
	}
	inline float QTransform::m31() const
	{
		return affine._dx;
	}
	inline float QTransform::m32() const
	{
		return affine._dy;
	}
	inline float QTransform::m33() const
	{
		return m_33;
	}
	inline float QTransform::dx() const
	{
		return affine._dx;
	}
	inline float QTransform::dy() const
	{
		return affine._dy;
	}

	inline QTransform &QTransform::operator*=(float num)
	{
		if (num == 1.)
			return *this;
		affine._m11 *= num;
		affine._m12 *= num;
		m_13 *= num;
		affine._m21 *= num;
		affine._m22 *= num;
		m_23 *= num;
		affine._dx *= num;
		affine._dy *= num;
		m_33 *= num;
		if (m_dirty < TxScale)
			m_dirty = TxScale;
		return *this;
	}
	inline QTransform &QTransform::operator/=(float div)
	{
		if (div == 0)
			return *this;
		div = 1 / div;
		return operator*=(div);
	}
	inline QTransform &QTransform::operator+=(float num)
	{
		if (num == 0)
			return *this;
		affine._m11 += num;
		affine._m12 += num;
		m_13 += num;
		affine._m21 += num;
		affine._m22 += num;
		m_23 += num;
		affine._dx += num;
		affine._dy += num;
		m_33 += num;
		m_dirty = TxProject;
		return *this;
	}
	inline QTransform &QTransform::operator-=(float num)
	{
		if (num == 0)
			return *this;
		affine._m11 -= num;
		affine._m12 -= num;
		m_13 -= num;
		affine._m21 -= num;
		affine._m22 -= num;
		m_23 -= num;
		affine._dx -= num;
		affine._dy -= num;
		m_33 -= num;
		m_dirty = TxProject;
		return *this;
	}

	inline bool FuzzyCompare(const QTransform& t1, const QTransform& t2)
	{
		return Math::FuzzyCompare(t1.m11(), t2.m11())
			&& Math::FuzzyCompare(t1.m12(), t2.m12())
			&& Math::FuzzyCompare(t1.m13(), t2.m13())
			&& Math::FuzzyCompare(t1.m21(), t2.m21())
			&& Math::FuzzyCompare(t1.m22(), t2.m22())
			&& Math::FuzzyCompare(t1.m23(), t2.m23())
			&& Math::FuzzyCompare(t1.m31(), t2.m31())
			&& Math::FuzzyCompare(t1.m32(), t2.m32())
			&& Math::FuzzyCompare(t1.m33(), t2.m33());
	}

	// mathematical semantics
	inline QPoint operator*(const QPoint &p, const QTransform &m)
	{
		return m.map(p);
	}
	inline QPointF operator*(const QPointF &p, const QTransform &m)
	{
		return m.map(p);
	}
	inline QLineF operator*(const QLineF &l, const QTransform &m)
	{
		return m.map(l);
	}
	inline QLine operator*(const QLine &l, const QTransform &m)
	{
		return m.map(l);
	}
	
	inline QRegion operator *(const QRegion &r, const QTransform &m)
	{
		return m.map(r);
	}
	
	inline QTransform operator *(const QTransform &a, float n)
	{
		QTransform t(a); t *= n; return t;
	}
	inline QTransform operator /(const QTransform &a, float n)
	{
		QTransform t(a); t /= n; return t;
	}
	inline QTransform operator +(const QTransform &a, float n)
	{
		QTransform t(a); t += n; return t;
	}
	inline QTransform operator -(const QTransform &a, float n)
	{
		QTransform t(a); t -= n; return t;
	}
}