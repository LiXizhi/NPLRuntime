#pragma once
#include "SpriteFontEntity.h"

namespace ParaEngine
{
	class CPaintDevice;

	class QFont
	{
	public:
		enum StyleHint {
			Helvetica, SansSerif = Helvetica,
			Times, Serif = Times,
			Courier, TypeWriter = Courier,
			OldEnglish, Decorative = OldEnglish,
			System,
			AnyStyle,
			Cursive,
			Monospace,
			Fantasy
		};

		enum Weight {
			Light = 25,
			Normal = 50,
			DemiBold = 63,
			Bold = 75,
			Black = 87
		};

		enum Style {
			StyleNormal,
			StyleItalic,
			StyleOblique
		};

		QFont();
		QFont(const std::string &family, int pointSize = -1, int weight = -1, bool italic = false);
		~QFont();
		

		std::string family() const;
		void setFamily(const std::string &);

		int pointSize() const;
		void setPointSize(int);
		inline float pointSizeF() const;
		inline void setPointSizeF(float fSize);

		int weight() const;
		void setWeight(int);

		void setStyle(Style style);
		Style style() const;

		inline bool bold() const;
		inline void setBold(bool);
		inline bool italic() const;
		inline void setItalic(bool b);

		bool operator==(const QFont & r) const;
		inline bool operator!=(const QFont & r) const {
			return !operator == (r);
		};
		
		std::string key() const;

		std::string toString() const;
		bool fromString(const std::string &);

		std::string defaultFamily() const;

		SpriteFontEntity* GetSpriteFont();
	private:
		/** rebuild from cache. */
		void SetDirty();
	private:
		asset_ptr<SpriteFontEntity> m_pFontSprite;
		std::string m_family;
		int m_pointSize;
		Style m_style;
		// type of Weight
		int32 m_weight;
	};

	inline bool QFont::bold() const
	{
		return weight() > Normal;
	}

	inline void QFont::setBold(bool enable)
	{
		setWeight(enable ? Bold : Normal);
	}

	inline bool QFont::italic() const
	{
		return (style() != StyleNormal);
	}

	inline void QFont::setItalic(bool b) {
		setStyle(b ? StyleItalic : StyleNormal);
	}

	inline float QFont::pointSizeF() const { return (float)pointSize(); };
	inline void QFont::setPointSizeF(float fSize) { setPointSize((int)fSize); };
}