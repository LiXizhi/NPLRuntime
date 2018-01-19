#pragma once
#include "GLType.h"
#include "ParaVector2.h"
#include <unordered_map>
#include <vector>



namespace ParaEngine
{


	enum class TextVAlignment
	{
		TOP,
		CENTER,
		BOTTOM
	};


	enum class TextHAlignment
	{
		LEFT,
		CENTER,
		RIGHT
	};


	class Texture2D;
	class FontAtlas;



	class Label : public CRefCountedOne
	{
	public:

		static const int DistanceFieldFontSize;

		struct LetterInfo
		{
			FontLetterDefinition def;

			Vector2 position;
			Size  contentSize;
			int   atlasIndex;
		};
		enum class LabelType {

			TTF,
			BMFONT,
			CHARMAP,
			STRING_TEXTURE
		};

		Label(FontAtlas *atlas = nullptr, TextHAlignment hAlignment = TextHAlignment::LEFT,
			TextVAlignment vAlignment = TextVAlignment::TOP, bool useDistanceField = false, bool useA8Shader = false);

		virtual ~Label();

		bool setTTFConfig(const TTFConfig& ttfConfig);

		FontAtlas* getFontAtlas() { return _fontAtlas; }



		virtual void setPosition(float x, float y);

		void setWidth(unsigned int width) { setDimensions(width, _labelHeight); }
		unsigned int getWidth() const { return _labelWidth; }

		void setHeight(unsigned int height) { setDimensions(_labelWidth, height); }
		unsigned int getHeight() const { return _labelHeight; }

		void setDimensions(unsigned int width, unsigned int height);
		const Size& getDimensions() const { return _labelDimensions; }


		int getStringLength() const;


		virtual void setScale(float scale);
		virtual void setScaleX(float scaleX);
		virtual void setScaleY(float scaleY);
		virtual float getScaleX() const;
		virtual float getScaleY() const;

		void setContentSize(const Size & var);

		void setAlignment(TextHAlignment hAlignment) { setAlignment(hAlignment, _vAlignment); }
		TextHAlignment getTextAlignment() const { return _hAlignment; }

		void setAlignment(TextHAlignment hAlignment, TextVAlignment vAlignment);

		void setHorizontalAlignment(TextHAlignment hAlignment) { setAlignment(hAlignment, _vAlignment); }
		TextHAlignment getHorizontalAlignment() const { return _hAlignment; }

		void setVerticalAlignment(TextVAlignment vAlignment) { setAlignment(_hAlignment, vAlignment); }
		TextVAlignment getVerticalAlignment() const { return _vAlignment; }


		bool computeHorizontalKernings(const std::u16string& stringToRender);

		void computeStringNumLines();


		inline float getPositionX() const
		{
			return _position.x;
		}

		inline float getPositionY() const
		{
			return _position.y;
		}

		void setFontScale(float s);

		bool recordPlaceholderInfo(int spriteIndex);
		bool recordLetterInfo(const Vector2& point, const FontLetterDefinition& letterDef, int spriteIndex);


	protected:
		std::string _bmFontPath;

		bool _isOpacityModifyRGB;
		bool _contentDirty;

		bool _systemFontDirty;
		std::string _systemFont;
		float         _systemFontSize;
		LabelType _currentLabelType;

		FontAtlas *                   _fontAtlas;
		std::vector<LetterInfo>       _lettersInfo;

		TTFConfig _fontConfig;

		//compatibility with older LabelTTF
		bool  _compatibleMode;

		//! used for optimization
		CCRect _reusedRect;
		int _limitShowCount;

		float _additionalKerning;
		float _commonLineHeight;
		bool  _lineBreakWithoutSpaces;
		int * _horizontalKernings;

		unsigned int _maxLineWidth;
		Size         _labelDimensions;
		unsigned int _labelWidth;
		unsigned int _labelHeight;
		TextHAlignment _hAlignment;
		TextVAlignment _vAlignment;

		int           _currNumLines;
		std::u16string _currentUTF16String;
		std::string          _originalUTF8String;

		float _fontScale;

		bool _useDistanceField;
		bool _useA8Shader;

		Color4B _effectColor;
		Color4F _effectColorF;

		GLuint _uniformEffectColor;
		GLuint _uniformTextColor;

		bool    _shadowDirty;
		bool    _shadowEnabled;
		Size    _shadowOffset;
		int     _shadowBlurRadius;
		Color3B _shadowColor;
		float   _shadowOpacity;

		int     _outlineSize;

		Color4B _textColor;
		Color4F _textColorF;

		bool _clipEnabled;
		bool _blendFuncDirty;
		bool _insideBounds;



		Size _contentSize;
		Vector2 _position;

		float _scaleY;
		float _scaleX;

		void reset();
		void setFontAtlas(FontAtlas* atlas, bool distanceFieldEnabled = false, bool useA8Shader = false);


		friend class LabelTextFormatter;
	};



	class LabelTextFormatter
	{
	public:
		static bool createStringSprites(Label* label);
		static bool multilineText(Label* label);
		static bool alignText(Label* label);
	};
}
