#pragma once

namespace ParaEngine
{
	class QPoint
	{
	public:
		QPoint();
		QPoint(int xpos, int ypos);

		inline bool isNull() const;

		inline int x() const;
		inline int y() const;
		inline void setX(int x);
		inline void setY(int y);

		inline int manhattanLength() const;

		inline int &rx();
		inline int &ry();

		inline QPoint &operator+=(const QPoint &p);
		inline QPoint &operator-=(const QPoint &p);

		inline QPoint &operator*=(float factor);
		inline QPoint &operator*=(double factor);
		inline QPoint &operator*=(int factor);

		inline QPoint &operator/=(float divisor);

		static inline int dotProduct(const QPoint &p1, const QPoint &p2)
		{
			return p1.xp * p2.xp + p1.yp * p2.yp;
		}

		friend inline bool operator==(const QPoint &, const QPoint &);
		friend inline bool operator!=(const QPoint &, const QPoint &);
		friend inline const QPoint operator+(const QPoint &, const QPoint &);
		friend inline const QPoint operator-(const QPoint &, const QPoint &);
		friend inline const QPoint operator*(const QPoint &, float);
		friend inline const QPoint operator*(float, const QPoint &);
		friend inline const QPoint operator*(const QPoint &, double);
		friend inline const QPoint operator*(double, const QPoint &);
		friend inline const QPoint operator*(const QPoint &, int);
		friend inline const QPoint operator*(int, const QPoint &);
		friend inline const QPoint operator+(const QPoint &);
		friend inline const QPoint operator-(const QPoint &);
		friend inline const QPoint operator/(const QPoint &, float);

	private:
		friend class QTransform;
		int xp;
		int yp;
	};


	/*****************************************************************************
	QPoint inline functions
	*****************************************************************************/

	inline QPoint::QPoint() : xp(0), yp(0) {}

	inline QPoint::QPoint(int xpos, int ypos) : xp(xpos), yp(ypos) {}

	inline bool QPoint::isNull() const
	{
		return xp == 0 && yp == 0;
	}

	inline int QPoint::x() const
	{
		return xp;
	}

	inline int QPoint::y() const
	{
		return yp;
	}

	inline void QPoint::setX(int xpos)
	{
		xp = xpos;
	}

	inline void QPoint::setY(int ypos)
	{
		yp = ypos;
	}

	inline int QPoint::manhattanLength() const
	{
		return Math::Abs(x()) + Math::Abs(y());
	}

	inline int &QPoint::rx()
	{
		return xp;
	}

	inline int &QPoint::ry()
	{
		return yp;
	}

	inline QPoint &QPoint::operator+=(const QPoint &p)
	{
		xp += p.xp; yp += p.yp; return *this;
	}

	inline QPoint &QPoint::operator-=(const QPoint &p)
	{
		xp -= p.xp; yp -= p.yp; return *this;
	}

	inline QPoint &QPoint::operator*=(float factor)
	{
		xp = (int)Math::Round(xp*factor); yp = (int)Math::Round(yp*factor); return *this;
	}

	inline QPoint &QPoint::operator*=(double factor)
	{
		xp = (int)Math::Round((float)(xp*factor)); yp = (int)Math::Round((float)(yp*factor)); return *this;
	}

	inline QPoint &QPoint::operator*=(int factor)
	{
		xp = xp*factor; yp = yp*factor; return *this;
	}

	inline bool operator==(const QPoint &p1, const QPoint &p2)
	{
		return p1.xp == p2.xp && p1.yp == p2.yp;
	}

	inline bool operator!=(const QPoint &p1, const QPoint &p2)
	{
		return p1.xp != p2.xp || p1.yp != p2.yp;
	}

	inline const QPoint operator+(const QPoint &p1, const QPoint &p2)
	{
		return QPoint(p1.xp + p2.xp, p1.yp + p2.yp);
	}

	inline const QPoint operator-(const QPoint &p1, const QPoint &p2)
	{
		return QPoint(p1.xp - p2.xp, p1.yp - p2.yp);
	}

	inline const QPoint operator*(const QPoint &p, float factor)
	{
		return QPoint(Math::Round(p.xp*factor), Math::Round(p.yp*factor));
	}

	inline const QPoint operator*(const QPoint &p, double factor)
	{
		return QPoint((int)Math::Round((float)(p.xp*factor)), (int)Math::Round((float)(p.yp*factor)));
	}

	inline const QPoint operator*(const QPoint &p, int factor)
	{
		return QPoint(p.xp*factor, p.yp*factor);
	}

	inline const QPoint operator*(float factor, const QPoint &p)
	{
		return QPoint(Math::Round(p.xp*factor), Math::Round(p.yp*factor));
	}

	inline const QPoint operator*(double factor, const QPoint &p)
	{
		return QPoint((int)Math::Round(float(p.xp*factor)), (int)Math::Round(float(p.yp*factor)));
	}

	inline const QPoint operator*(int factor, const QPoint &p)
	{
		return QPoint(p.xp*factor, p.yp*factor);
	}

	inline const QPoint operator+(const QPoint &p)
	{
		return p;
	}

	inline const QPoint operator-(const QPoint &p)
	{
		return QPoint(-p.xp, -p.yp);
	}

	inline QPoint &QPoint::operator/=(float c)
	{
		xp = Math::Round(xp / c);
		yp = Math::Round(yp / c);
		return *this;
	}

	inline const QPoint operator/(const QPoint &p, float c)
	{
		return QPoint(Math::Round(p.xp / c), Math::Round(p.yp / c));
	}

	class QPointF
	{
	public:
		QPointF();
		QPointF(const QPoint &p);
		QPointF(float xpos, float ypos);

		inline float manhattanLength() const;

		inline bool isNull() const;

		inline float x() const;
		inline float y() const;
		inline void setX(float x);
		inline void setY(float y);

		inline float &rx();
		inline float &ry();

		inline QPointF &operator+=(const QPointF &p);
		inline QPointF &operator-=(const QPointF &p);
		inline QPointF &operator*=(float c);
		inline QPointF &operator/=(float c);

		static inline float dotProduct(const QPointF &p1, const QPointF &p2)
		{
			return p1.xp * p2.xp + p1.yp * p2.yp;
		}

		friend inline bool operator==(const QPointF &, const QPointF &);
		friend inline bool operator!=(const QPointF &, const QPointF &);
		friend inline const QPointF operator+(const QPointF &, const QPointF &);
		friend inline const QPointF operator-(const QPointF &, const QPointF &);
		friend inline const QPointF operator*(float, const QPointF &);
		friend inline const QPointF operator*(const QPointF &, float);
		friend inline const QPointF operator+(const QPointF &);
		friend inline const QPointF operator-(const QPointF &);
		friend inline const QPointF operator/(const QPointF &, float);

		QPoint toPoint() const;

	private:
		friend class QMatrix;
		friend class QTransform;

		float xp;
		float yp;
	};


	/*****************************************************************************
	QPointF inline functions
	*****************************************************************************/

	inline QPointF::QPointF() : xp(0), yp(0) { }

	inline QPointF::QPointF(float xpos, float ypos) : xp(xpos), yp(ypos) { }

	inline QPointF::QPointF(const QPoint &p) : xp((float)p.x()), yp((float)p.y()) { }

	inline float QPointF::manhattanLength() const
	{
		return Math::Abs(x()) + Math::Abs(y());
	}

	inline bool QPointF::isNull() const
	{
		return Math::IsNull(xp) && Math::IsNull(yp);
	}

	inline float QPointF::x() const
	{
		return xp;
	}

	inline float QPointF::y() const
	{
		return yp;
	}

	inline void QPointF::setX(float xpos)
	{
		xp = xpos;
	}

	inline void QPointF::setY(float ypos)
	{
		yp = ypos;
	}

	inline float &QPointF::rx()
	{
		return xp;
	}

	inline float &QPointF::ry()
	{
		return yp;
	}

	inline QPointF &QPointF::operator+=(const QPointF &p)
	{
		xp += p.xp;
		yp += p.yp;
		return *this;
	}

	inline QPointF &QPointF::operator-=(const QPointF &p)
	{
		xp -= p.xp; yp -= p.yp; return *this;
	}

	inline QPointF &QPointF::operator*=(float c)
	{
		xp *= c; yp *= c; return *this;
	}

	inline bool operator==(const QPointF &p1, const QPointF &p2)
	{
		return Math::FuzzyIsNull(p1.xp - p2.xp) && Math::FuzzyIsNull(p1.yp - p2.yp);
	}

	inline bool operator!=(const QPointF &p1, const QPointF &p2)
	{
		return !Math::FuzzyIsNull(p1.xp - p2.xp) || !Math::FuzzyIsNull(p1.yp - p2.yp);
	}

	inline const QPointF operator+(const QPointF &p1, const QPointF &p2)
	{
		return QPointF(p1.xp + p2.xp, p1.yp + p2.yp);
	}

	inline const QPointF operator-(const QPointF &p1, const QPointF &p2)
	{
		return QPointF(p1.xp - p2.xp, p1.yp - p2.yp);
	}

	inline const QPointF operator*(const QPointF &p, float c)
	{
		return QPointF(p.xp*c, p.yp*c);
	}

	inline const QPointF operator*(float c, const QPointF &p)
	{
		return QPointF(p.xp*c, p.yp*c);
	}

	inline const QPointF operator+(const QPointF &p)
	{
		return p;
	}

	inline const QPointF operator-(const QPointF &p)
	{
		return QPointF(-p.xp, -p.yp);
	}

	inline QPointF &QPointF::operator/=(float divisor)
	{
		xp /= divisor;
		yp /= divisor;
		return *this;
	}

	inline const QPointF operator/(const QPointF &p, float divisor)
	{
		return QPointF(p.xp / divisor, p.yp / divisor);
	}

	inline QPoint QPointF::toPoint() const
	{
		return QPoint(Math::Round(xp), Math::Round(yp));
	}
}
