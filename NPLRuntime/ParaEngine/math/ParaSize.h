#pragma once

namespace ParaEngine
{
	/**
	* The QSize class defines the size of a two-dimensional
	* object using integer point precision.
	*/
	class QSize
	{
	public:
		QSize();
		QSize(int w, int h);

		inline bool isNull() const;
		inline bool isEmpty() const;
		inline bool isValid() const;

		inline int width() const;
		inline int height() const;
		inline void setWidth(int w);
		inline void setHeight(int h);
		void transpose();
		inline QSize transposed() const;

		inline void scale(int w, int h);
		inline void scale(const QSize &s);
		QSize scaled(int w, int h) const;
		QSize scaled(const QSize &s) const;

		inline QSize expandedTo(const QSize &) const;
		inline QSize boundedTo(const QSize &) const;

		inline int &rwidth();
		inline int &rheight();

		inline QSize &operator+=(const QSize &);
		inline QSize &operator-=(const QSize &);
		inline QSize &operator*=(float c);
		inline QSize &operator/=(float c);

		friend inline bool operator==(const QSize &, const QSize &);
		friend inline bool operator!=(const QSize &, const QSize &);
		friend inline const QSize operator+(const QSize &, const QSize &);
		friend inline const QSize operator-(const QSize &, const QSize &);
		friend inline const QSize operator*(const QSize &, float);
		friend inline const QSize operator*(float, const QSize &);
		friend inline const QSize operator/(const QSize &, float);

	private:
		int wd;
		int ht;
	};


	/*****************************************************************************
	QSize inline functions
	*****************************************************************************/

	inline QSize::QSize() : wd(-1), ht(-1) {}

	inline QSize::QSize(int w, int h) : wd(w), ht(h) {}

	inline bool QSize::isNull() const
	{
		return wd == 0 && ht == 0;
	}

	inline bool QSize::isEmpty() const
	{
		return wd<1 || ht<1;
	}

	inline bool QSize::isValid() const
	{
		return wd >= 0 && ht >= 0;
	}

	inline int QSize::width() const
	{
		return wd;
	}

	inline int QSize::height() const
	{
		return ht;
	}

	inline void QSize::setWidth(int w)
	{
		wd = w;
	}

	inline void QSize::setHeight(int h)
	{
		ht = h;
	}

	inline QSize QSize::transposed() const
	{
		return QSize(ht, wd);
	}

	inline void QSize::scale(int w, int h)
	{
		scale(QSize(w, h));
	}

	inline void QSize::scale(const QSize &s)
	{
		*this = scaled(s);
	}

	inline QSize QSize::scaled(int w, int h) const
	{
		return scaled(QSize(w, h));
	}

	inline int &QSize::rwidth()
	{
		return wd;
	}

	inline int &QSize::rheight()
	{
		return ht;
	}

	inline QSize &QSize::operator+=(const QSize &s)
	{
		wd += s.wd; ht += s.ht; return *this;
	}

	inline QSize &QSize::operator-=(const QSize &s)
	{
		wd -= s.wd; ht -= s.ht; return *this;
	}

	inline QSize &QSize::operator*=(float c)
	{
		wd = Math::Round(wd*c); ht = Math::Round(ht*c); return *this;
	}

	inline bool operator==(const QSize &s1, const QSize &s2)
	{
		return s1.wd == s2.wd && s1.ht == s2.ht;
	}

	inline bool operator!=(const QSize &s1, const QSize &s2)
	{
		return s1.wd != s2.wd || s1.ht != s2.ht;
	}

	inline const QSize operator+(const QSize & s1, const QSize & s2)
	{
		return QSize(s1.wd + s2.wd, s1.ht + s2.ht);
	}

	inline const QSize operator-(const QSize &s1, const QSize &s2)
	{
		return QSize(s1.wd - s2.wd, s1.ht - s2.ht);
	}

	inline const QSize operator*(const QSize &s, float c)
	{
		return QSize(Math::Round(s.wd*c), Math::Round(s.ht*c));
	}

	inline const QSize operator*(float c, const QSize &s)
	{
		return QSize(Math::Round(s.wd*c), Math::Round(s.ht*c));
	}

	inline QSize &QSize::operator/=(float c)
	{
		wd = Math::Round(wd / c); ht = Math::Round(ht / c);
		return *this;
	}

	inline const QSize operator/(const QSize &s, float c)
	{
		return QSize(Math::Round(s.wd / c), Math::Round(s.ht / c));
	}

	inline QSize QSize::expandedTo(const QSize & otherSize) const
	{
		return QSize(Math::Max(wd, otherSize.wd), Math::Max(ht, otherSize.ht));
	}

	inline QSize QSize::boundedTo(const QSize & otherSize) const
	{
		return QSize(Math::Min(wd, otherSize.wd), Math::Min(ht, otherSize.ht));
	}

	class QSizeF
	{
	public:
		QSizeF();
		QSizeF(const QSize &sz);
		QSizeF(float w, float h);

		inline bool isNull() const;
		inline bool isEmpty() const;
		inline bool isValid() const;

		inline float width() const;
		inline float height() const;
		inline void setWidth(float w);
		inline void setHeight(float h);
		void transpose();
		inline QSizeF transposed() const;

		inline void scale(float w, float h);
		inline void scale(const QSizeF &s);
		QSizeF scaled(float w, float h) const;
		QSizeF scaled(const QSizeF &s) const;

		inline QSizeF expandedTo(const QSizeF &) const;
		inline QSizeF boundedTo(const QSizeF &) const;

		inline float &rwidth();
		inline float &rheight();

		inline QSizeF &operator+=(const QSizeF &);
		inline QSizeF &operator-=(const QSizeF &);
		inline QSizeF &operator*=(float c);
		inline QSizeF &operator/=(float c);

		friend inline bool operator==(const QSizeF &, const QSizeF &);
		friend inline bool operator!=(const QSizeF &, const QSizeF &);
		friend inline const QSizeF operator+(const QSizeF &, const QSizeF &);
		friend inline const QSizeF operator-(const QSizeF &, const QSizeF &);
		friend inline const QSizeF operator*(const QSizeF &, float);
		friend inline const QSizeF operator*(float, const QSizeF &);
		friend inline const QSizeF operator/(const QSizeF &, float);

		inline QSize toSize() const;

	private:
		float wd;
		float ht;
	};


	/*****************************************************************************
	QSizeF inline functions
	*****************************************************************************/

	inline QSizeF::QSizeF() : wd(-1.), ht(-1.) {}

	inline QSizeF::QSizeF(const QSize &sz) : wd((float)sz.width()), ht((float)sz.height()) {}

	inline QSizeF::QSizeF(float w, float h) : wd(w), ht(h) {}

	inline bool QSizeF::isNull() const
	{
		return Math::IsNull(wd) && Math::IsNull(ht);
	}

	inline bool QSizeF::isEmpty() const
	{
		return wd <= 0. || ht <= 0.;
	}

	inline bool QSizeF::isValid() const
	{
		return wd >= 0. && ht >= 0.;
	}

	inline float QSizeF::width() const
	{
		return wd;
	}

	inline float QSizeF::height() const
	{
		return ht;
	}

	inline void QSizeF::setWidth(float w)
	{
		wd = w;
	}

	inline void QSizeF::setHeight(float h)
	{
		ht = h;
	}

	inline QSizeF QSizeF::transposed() const
	{
		return QSizeF(ht, wd);
	}

	inline void QSizeF::scale(float w, float h)
	{
		scale(QSizeF(w, h));
	}

	inline void QSizeF::scale(const QSizeF &s)
	{
		*this = scaled(s);
	}

	inline QSizeF QSizeF::scaled(float w, float h) const
	{
		return scaled(QSizeF(w, h));
	}

	inline float &QSizeF::rwidth()
	{
		return wd;
	}

	inline float &QSizeF::rheight()
	{
		return ht;
	}

	inline QSizeF &QSizeF::operator+=(const QSizeF &s)
	{
		wd += s.wd; ht += s.ht; return *this;
	}

	inline QSizeF &QSizeF::operator-=(const QSizeF &s)
	{
		wd -= s.wd; ht -= s.ht; return *this;
	}

	inline QSizeF &QSizeF::operator*=(float c)
	{
		wd *= c; ht *= c; return *this;
	}

	inline bool operator==(const QSizeF &s1, const QSizeF &s2)
	{
		return Math::FuzzyCompare(s1.wd, s2.wd) && Math::FuzzyCompare(s1.ht, s2.ht);
	}

	inline bool operator!=(const QSizeF &s1, const QSizeF &s2)
	{
		return !Math::FuzzyCompare(s1.wd, s2.wd) || !Math::FuzzyCompare(s1.ht, s2.ht);
	}

	inline const QSizeF operator+(const QSizeF & s1, const QSizeF & s2)
	{
		return QSizeF(s1.wd + s2.wd, s1.ht + s2.ht);
	}

	inline const QSizeF operator-(const QSizeF &s1, const QSizeF &s2)
	{
		return QSizeF(s1.wd - s2.wd, s1.ht - s2.ht);
	}

	inline const QSizeF operator*(const QSizeF &s, float c)
	{
		return QSizeF(s.wd*c, s.ht*c);
	}

	inline const QSizeF operator*(float c, const QSizeF &s)
	{
		return QSizeF(s.wd*c, s.ht*c);
	}

	inline QSizeF &QSizeF::operator/=(float c)
	{
		wd = wd / c; ht = ht / c;
		return *this;
	}

	inline const QSizeF operator/(const QSizeF &s, float c)
	{
		return QSizeF(s.wd / c, s.ht / c);
	}

	inline QSizeF QSizeF::expandedTo(const QSizeF & otherSize) const
	{
		return QSizeF(Math::Max(wd, otherSize.wd), Math::Max(ht, otherSize.ht));
	}

	inline QSizeF QSizeF::boundedTo(const QSizeF & otherSize) const
	{
		return QSizeF(Math::Min(wd, otherSize.wd), Math::Min(ht, otherSize.ht));
	}

	inline QSize QSizeF::toSize() const
	{
		return QSize(Math::Round(wd), Math::Round(ht));
	}
}
