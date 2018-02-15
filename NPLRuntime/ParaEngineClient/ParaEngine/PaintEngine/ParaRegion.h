#pragma once
#include "ParaRect.h"
#include <vector>

namespace ParaEngine
{
	class QRegion
	{
	public:
		enum RegionType { Rectangle, Ellipse };

		QRegion();
		QRegion(int x, int y, int w, int h, RegionType t = Rectangle);
		QRegion(const QRect &r, RegionType t = Rectangle);
		QRegion(const QRegion &region);
		~QRegion();
		QRegion &operator=(const QRegion &);

		inline void swap(QRegion &other) { 
			std::swap(*this, other); 
		}

		bool isEmpty() const;
		bool isNull() const;

		bool contains(const QPoint &p) const;
		bool contains(const QRect &r) const;

		/*
		* Returns \c true if r is guaranteed to be fully contained in this region.
		* A false return value does not guarantee the opposite.
		*/
		inline bool contains(const QRegion &r) const {
			return contains(r.extents);
		}

		/*
		* Returns \c true if this region is guaranteed to be fully contained in r.
		*/
		inline bool within(const QRect &r1) const {
			const QRect &r2 = extents;
			return r2.left() >= r1.left() && r2.right() <= r1.right()
				&& r2.top() >= r1.top() && r2.bottom() <= r1.bottom();
		}

		inline void updateInnerRect(const QRect &rect) {
			const int area = rect.width() * rect.height();
			if (area > innerArea) {
				innerArea = area;
				innerRect = rect;
			}
		}

		inline void vectorize() {
			if (numRects == 1) {
				if (!rects.size())
					rects.resize(1);
				rects[0] = extents;
			}
		}

		inline void append(const QRect *r);
		void append(const QRegion *r);
		void prepend(const QRect *r);
		void prepend(const QRegion *r);
		inline bool canAppend(const QRect *r) const;
		inline bool canAppend(const QRegion *r) const;
		inline bool canPrepend(const QRect *r) const;
		inline bool canPrepend(const QRegion *r) const;

		inline bool mergeFromRight(QRect *left, const QRect *right);
		inline bool mergeFromLeft(QRect *left, const QRect *right);
		inline bool mergeFromBelow(QRect *top, const QRect *bottom,
			const QRect *nextToTop,
			const QRect *nextToBottom);
		inline bool mergeFromAbove(QRect *bottom, const QRect *top,
			const QRect *nextToBottom,
			const QRect *nextToTop);

	public:

		void translate(int dx, int dy);
		inline void translate(const QPoint &p) { translate(p.x(), p.y()); }
		QRegion translated(int dx, int dy) const;
		inline QRegion translated(const QPoint &p) const { return translated(p.x(), p.y()); }

		QRegion united(const QRegion &r) const;
		QRegion united(const QRect &r) const;
		QRegion intersected(const QRegion &r) const;
		QRegion intersected(const QRect &r) const;
		QRegion subtracted(const QRegion &r) const;
		QRegion xored(const QRegion &r) const;

		bool intersects(const QRegion &r) const;
		bool intersects(const QRect &r) const;

		QRect boundingRect() const;
		std::vector<QRect> Rects() const;
		void setRects(const QRect *rect, int num);
		int rectCount() const;

		const QRegion operator|(const QRegion &r) const;
		const QRegion operator+(const QRegion &r) const;
		const QRegion operator+(const QRect &r) const;
		const QRegion operator&(const QRegion &r) const;
		const QRegion operator&(const QRect &r) const;
		const QRegion operator-(const QRegion &r) const;
		const QRegion operator^(const QRegion &r) const;
		QRegion& operator|=(const QRegion &r);
		QRegion& operator+=(const QRegion &r);
		QRegion& operator+=(const QRect &r);
		QRegion& operator&=(const QRegion &r);
		QRegion& operator&=(const QRect &r);
		QRegion& operator-=(const QRegion &r);
		QRegion& operator^=(const QRegion &r);

		bool operator==(const QRegion &r) const;
		inline bool operator!=(const QRegion &r) const { return !(operator==(r)); }

	private:
		QRegion copy() const;   // helper of detach.
		void detach();
		friend bool qt_region_strictContains(const QRegion &region, const QRect &rect);

	private:
		int numRects;
		std::vector<QRect> rects;
		QRect extents;
		QRect innerRect;
		int innerArea;
	};
}

