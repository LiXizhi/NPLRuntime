#pragma once

namespace ParaEngine
{
	class QMargins
	{
	public:
		QMargins();
		QMargins(int left, int top, int right, int bottom);

		bool isNull() const;

		int left() const;
		int top() const;
		int right() const;
		int bottom() const;

		void setLeft(int left);
		void setTop(int top);
		void setRight(int right);
		void setBottom(int bottom);

		QMargins &operator+=(const QMargins &margins);
		QMargins &operator-=(const QMargins &margins);
		QMargins &operator+=(int);
		QMargins &operator-=(int);
		QMargins &operator*=(int);
		QMargins &operator/=(int);
		QMargins &operator*=(float);
		QMargins &operator/=(float);

	private:
		int m_left;
		int m_top;
		int m_right;
		int m_bottom;

		friend inline bool operator==(const QMargins &, const QMargins &);
		friend inline bool operator!=(const QMargins &, const QMargins &);
	};


	/*****************************************************************************
	QMargins inline functions
	*****************************************************************************/

	inline QMargins::QMargins() : m_left(0), m_top(0), m_right(0), m_bottom(0) {}

	inline QMargins::QMargins(int aleft, int atop, int aright, int abottom)
		: m_left(aleft), m_top(atop), m_right(aright), m_bottom(abottom) {}

	inline bool QMargins::isNull() const
	{
		return m_left == 0 && m_top == 0 && m_right == 0 && m_bottom == 0;
	}

	inline int QMargins::left() const
	{
		return m_left;
	}

	inline int QMargins::top() const
	{
		return m_top;
	}

	inline int QMargins::right() const
	{
		return m_right;
	}

	inline int QMargins::bottom() const
	{
		return m_bottom;
	}


	inline void QMargins::setLeft(int aleft)
	{
		m_left = aleft;
	}

	inline void QMargins::setTop(int atop)
	{
		m_top = atop;
	}

	inline void QMargins::setRight(int aright)
	{
		m_right = aright;
	}

	inline void QMargins::setBottom(int abottom)
	{
		m_bottom = abottom;
	}

	inline bool operator==(const QMargins &m1, const QMargins &m2)
	{
		return
			m1.m_left == m2.m_left &&
			m1.m_top == m2.m_top &&
			m1.m_right == m2.m_right &&
			m1.m_bottom == m2.m_bottom;
	}

	inline bool operator!=(const QMargins &m1, const QMargins &m2)
	{
		return
			m1.m_left != m2.m_left ||
			m1.m_top != m2.m_top ||
			m1.m_right != m2.m_right ||
			m1.m_bottom != m2.m_bottom;
	}

	inline QMargins operator+(const QMargins &m1, const QMargins &m2)
	{
		return QMargins(m1.left() + m2.left(), m1.top() + m2.top(),
			m1.right() + m2.right(), m1.bottom() + m2.bottom());
	}

	inline QMargins operator-(const QMargins &m1, const QMargins &m2)
	{
		return QMargins(m1.left() - m2.left(), m1.top() - m2.top(),
			m1.right() - m2.right(), m1.bottom() - m2.bottom());
	}

	inline QMargins operator+(const QMargins &lhs, int rhs)
	{
		return QMargins(lhs.left() + rhs, lhs.top() + rhs,
			lhs.right() + rhs, lhs.bottom() + rhs);
	}

	inline QMargins operator+(int lhs, const QMargins &rhs)
	{
		return QMargins(rhs.left() + lhs, rhs.top() + lhs,
			rhs.right() + lhs, rhs.bottom() + lhs);
	}

	inline QMargins operator-(const QMargins &lhs, int rhs)
	{
		return QMargins(lhs.left() - rhs, lhs.top() - rhs,
			lhs.right() - rhs, lhs.bottom() - rhs);
	}

	inline QMargins operator*(const QMargins &margins, int factor)
	{
		return QMargins(margins.left() * factor, margins.top() * factor,
			margins.right() * factor, margins.bottom() * factor);
	}

	inline QMargins operator*(int factor, const QMargins &margins)
	{
		return QMargins(margins.left() * factor, margins.top() * factor,
			margins.right() * factor, margins.bottom() * factor);
	}

	inline QMargins operator*(const QMargins &margins, float factor)
	{
		return QMargins(Math::Round(margins.left() * factor), Math::Round(margins.top() * factor),
			Math::Round(margins.right() * factor), Math::Round(margins.bottom() * factor));
	}

	inline QMargins operator*(float factor, const QMargins &margins)
	{
		return QMargins(Math::Round(margins.left() * factor), Math::Round(margins.top() * factor),
			Math::Round(margins.right() * factor), Math::Round(margins.bottom() * factor));
	}

	inline QMargins operator/(const QMargins &margins, int divisor)
	{
		return QMargins(margins.left() / divisor, margins.top() / divisor,
			margins.right() / divisor, margins.bottom() / divisor);
	}

	inline QMargins operator/(const QMargins &margins, float divisor)
	{
		return QMargins(Math::Round(margins.left() / divisor), Math::Round(margins.top() / divisor),
			Math::Round(margins.right() / divisor), Math::Round(margins.bottom() / divisor));
	}

	inline QMargins &QMargins::operator+=(const QMargins &margins)
	{
		return *this = *this + margins;
	}

	inline QMargins &QMargins::operator-=(const QMargins &margins)
	{
		return *this = *this - margins;
	}

	inline QMargins &QMargins::operator+=(int margin)
	{
		m_left += margin;
		m_top += margin;
		m_right += margin;
		m_bottom += margin;
		return *this;
	}

	inline QMargins &QMargins::operator-=(int margin)
	{
		m_left -= margin;
		m_top -= margin;
		m_right -= margin;
		m_bottom -= margin;
		return *this;
	}

	inline QMargins &QMargins::operator*=(int factor)
	{
		return *this = *this * factor;
	}

	inline QMargins &QMargins::operator/=(int divisor)
	{
		return *this = *this / divisor;
	}

	inline QMargins &QMargins::operator*=(float factor)
	{
		return *this = *this * factor;
	}

	inline QMargins &QMargins::operator/=(float divisor)
	{
		return *this = *this / divisor;
	}

	inline QMargins operator+(const QMargins &margins)
	{
		return margins;
	}

	inline QMargins operator-(const QMargins &margins)
	{
		return QMargins(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
	}



	class QMarginsF
	{
	public:
		QMarginsF();
		QMarginsF(float left, float top, float right, float bottom);
		QMarginsF(const QMargins &margins);

		bool isNull() const;

		float left() const;
		float top() const;
		float right() const;
		float bottom() const;

		void setLeft(float left);
		void setTop(float top);
		void setRight(float right);
		void setBottom(float bottom);

		QMarginsF &operator+=(const QMarginsF &margins);
		QMarginsF &operator-=(const QMarginsF &margins);
		QMarginsF &operator+=(float addend);
		QMarginsF &operator-=(float subtrahend);
		QMarginsF &operator*=(float factor);
		QMarginsF &operator/=(float divisor);

		inline QMargins toMargins() const;

	private:
		float m_left;
		float m_top;
		float m_right;
		float m_bottom;
	};


	/*****************************************************************************
	QMarginsF inline functions
	*****************************************************************************/

	inline QMarginsF::QMarginsF() : m_left(0), m_top(0), m_right(0), m_bottom(0) {}

	inline QMarginsF::QMarginsF(float aleft, float atop, float aright, float abottom)
		: m_left(aleft), m_top(atop), m_right(aright), m_bottom(abottom) {}

	inline QMarginsF::QMarginsF(const QMargins &margins)
		: m_left((float)margins.left()), m_top((float)margins.top()), m_right((float)margins.right()), m_bottom((float)margins.bottom()) {}

	inline bool QMarginsF::isNull() const
	{
		return Math::FuzzyIsNull(m_left) && Math::FuzzyIsNull(m_top) && Math::FuzzyIsNull(m_right) && Math::FuzzyIsNull(m_bottom);
	}

	inline float QMarginsF::left() const
	{
		return m_left;
	}

	inline float QMarginsF::top() const
	{
		return m_top;
	}

	inline float QMarginsF::right() const
	{
		return m_right;
	}

	inline float QMarginsF::bottom() const
	{
		return m_bottom;
	}


	inline void QMarginsF::setLeft(float aleft)
	{
		m_left = aleft;
	}

	inline void QMarginsF::setTop(float atop)
	{
		m_top = atop;
	}

	inline void QMarginsF::setRight(float aright)
	{
		m_right = aright;
	}

	inline void QMarginsF::setBottom(float abottom)
	{
		m_bottom = abottom;
	}

	inline bool operator==(const QMarginsF &lhs, const QMarginsF &rhs)
	{
		return Math::FuzzyCompare(lhs.left(), rhs.left())
			&& Math::FuzzyCompare(lhs.top(), rhs.top())
			&& Math::FuzzyCompare(lhs.right(), rhs.right())
			&& Math::FuzzyCompare(lhs.bottom(), rhs.bottom());
	}

	inline bool operator!=(const QMarginsF &lhs, const QMarginsF &rhs)
	{
		return !operator==(lhs, rhs);
	}

	inline QMarginsF operator+(const QMarginsF &lhs, const QMarginsF &rhs)
	{
		return QMarginsF(lhs.left() + rhs.left(), lhs.top() + rhs.top(),
			lhs.right() + rhs.right(), lhs.bottom() + rhs.bottom());
	}

	inline QMarginsF operator-(const QMarginsF &lhs, const QMarginsF &rhs)
	{
		return QMarginsF(lhs.left() - rhs.left(), lhs.top() - rhs.top(),
			lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom());
	}

	inline QMarginsF operator+(const QMarginsF &lhs, float rhs)
	{
		return QMarginsF(lhs.left() + rhs, lhs.top() + rhs,
			lhs.right() + rhs, lhs.bottom() + rhs);
	}

	inline QMarginsF operator+(float lhs, const QMarginsF &rhs)
	{
		return QMarginsF(rhs.left() + lhs, rhs.top() + lhs,
			rhs.right() + lhs, rhs.bottom() + lhs);
	}

	inline QMarginsF operator-(const QMarginsF &lhs, float rhs)
	{
		return QMarginsF(lhs.left() - rhs, lhs.top() - rhs,
			lhs.right() - rhs, lhs.bottom() - rhs);
	}

	inline QMarginsF operator*(const QMarginsF &lhs, float rhs)
	{
		return QMarginsF(lhs.left() * rhs, lhs.top() * rhs,
			lhs.right() * rhs, lhs.bottom() * rhs);
	}

	inline QMarginsF operator*(float lhs, const QMarginsF &rhs)
	{
		return QMarginsF(rhs.left() * lhs, rhs.top() * lhs,
			rhs.right() * lhs, rhs.bottom() * lhs);
	}

	inline QMarginsF operator/(const QMarginsF &lhs, float divisor)
	{
		return QMarginsF(lhs.left() / divisor, lhs.top() / divisor,
			lhs.right() / divisor, lhs.bottom() / divisor);
	}

	inline QMarginsF &QMarginsF::operator+=(const QMarginsF &margins)
	{
		return *this = *this + margins;
	}

	inline QMarginsF &QMarginsF::operator-=(const QMarginsF &margins)
	{
		return *this = *this - margins;
	}

	inline QMarginsF &QMarginsF::operator+=(float addend)
	{
		m_left += addend;
		m_top += addend;
		m_right += addend;
		m_bottom += addend;
		return *this;
	}

	inline QMarginsF &QMarginsF::operator-=(float subtrahend)
	{
		m_left -= subtrahend;
		m_top -= subtrahend;
		m_right -= subtrahend;
		m_bottom -= subtrahend;
		return *this;
	}

	inline QMarginsF &QMarginsF::operator*=(float factor)
	{
		return *this = *this * factor;
	}

	inline QMarginsF &QMarginsF::operator/=(float divisor)
	{
		return *this = *this / divisor;
	}

	inline QMarginsF operator+(const QMarginsF &margins)
	{
		return margins;
	}

	inline QMarginsF operator-(const QMarginsF &margins)
	{
		return QMarginsF(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
	}

	inline QMargins QMarginsF::toMargins() const
	{
		return QMargins(Math::Round(m_left), Math::Round(m_top), Math::Round(m_right), Math::Round(m_bottom));
	}
}
