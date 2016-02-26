#pragma once
#include "qbrush.h"

namespace ParaEngine
{
	class QBrush;
	
	/** The QPen class defines how a CPainter should draw lines and outlines of shapes.
	The pen style defines the line type. The brush is used to fill
	strokes generated with the pen. Use the QBrush class to specify
	fill styles.
	*/
	class QPen
	{
	public:
		QPen();
		QPen(PenStyle);
		QPen(const Color &color);
		QPen(const QBrush &brush, float width, PenStyle s = SolidLine,
			PenCapStyle c = SquareCap, PenJoinStyle j = BevelJoin);
		~QPen();

		PenStyle style() const;
		void setStyle(PenStyle);

		std::vector<float> dashPattern() const;
		void setDashPattern(const std::vector<float> &pattern);

		float dashOffset() const;
		void setDashOffset(float doffset);

		float miterLimit() const;
		void setMiterLimit(float limit);

		float widthF() const;
		void setWidthF(float width);

		int width() const;
		void setWidth(int width);

		Color color() const;
		void setColor(const Color &color);

		QBrush brush() const;
		void setBrush(const QBrush &brush);

		bool isSolid() const;

		PenCapStyle capStyle() const;
		void setCapStyle(PenCapStyle pcs);

		PenJoinStyle joinStyle() const;
		void setJoinStyle(PenJoinStyle pcs);

		bool isCosmetic() const { return true; };
		void setCosmetic(bool cosmetic) {};

		bool operator==(const QPen &p) const;
		inline bool operator!=(const QPen &p) const { return !(operator==(p)); }
		
	private:
		float m_width;
		QBrush m_brush;
		PenStyle m_style;
		PenCapStyle m_capStyle;
		PenJoinStyle m_joinStyle;
		mutable std::vector<float> m_dashPattern;
		float m_dashOffset;
		float m_miterLimit;
	};
}