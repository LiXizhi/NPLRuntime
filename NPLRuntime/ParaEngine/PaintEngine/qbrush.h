#pragma once
#include "qmatrix.h"
#include "qtransform.h"
#include "TextureEntity.h"

namespace ParaEngine
{
	class QBrushData;

	/** The QBrush class defines the fill pattern of shapes drawn by CPainter. */
	class QBrush
	{
	public:
		QBrush();
		QBrush(BrushStyle bs);
		QBrush(const Color &color, BrushStyle bs = SolidPattern);
		QBrush(const Color &color, TextureEntity* pTexture);
		QBrush(TextureEntity* pTexture);
		~QBrush();
		
		inline BrushStyle style() const;
		void setStyle(BrushStyle);

		inline const QMatrix &matrix() const;
		void setMatrix(const QMatrix &mat);

		inline QTransform transform() const;
		void setTransform(const QTransform &);

		TextureEntity* texture() const;
		void setTexture(TextureEntity* pTexture);

		inline const Color &color() const;
		void setColor(const Color &color);

		bool isOpaque() const;

		bool operator==(const QBrush &b) const;
		inline bool operator!=(const QBrush &b) const { return !(operator==(b)); }

	private:
		void init(const Color &color, BrushStyle bs);

		BrushStyle m_style;
		Color m_color;
		QTransform m_transform;
		asset_ptr<TextureEntity> m_pTexture;
	};

	inline BrushStyle QBrush::style() const { return m_style; }
	inline const Color &QBrush::color() const { return m_color; }
	inline const QMatrix &QBrush::matrix() const { return m_transform.toAffine(); }
	inline QTransform QBrush::transform() const { return m_transform; }
}