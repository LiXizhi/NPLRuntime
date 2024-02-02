#include "ParaEngine.h"
#include "GLType.h"
#include "GLFontAtlas.h"
#include "GLFontAtlasCache.h"
#include "GLFont.h"
#include "util/StringHelper.h"
#include "GLLabel.h"



using namespace ParaEngine;

const int GLLabel::DistanceFieldFontSize = 50;


bool LabelTextFormatter::multilineText(GLLabel *theLabel)
{
	auto limit = theLabel->_limitShowCount;
	const auto& strWhole = theLabel->_currentUTF16String;

	std::vector<char16_t> multiline_string;
	multiline_string.reserve(limit);

	std::vector<char16_t> last_word;
	last_word.reserve(25);

	bool   isStartOfLine = false, isStartOfWord = false;
	float  startOfLine = -1, startOfWord = -1;

	int skip = 0;

	int tIndex = 0;
	float scalsX = theLabel->getScaleX();
	float lineWidth = theLabel->_maxLineWidth;
	bool breakLineWithoutSpace = theLabel->_lineBreakWithoutSpaces;
	GLLabel::LetterInfo* info = nullptr;

	for (int j = 0; j + skip < limit; j++)
	{
		info = &theLabel->_lettersInfo.at(j + skip);

		unsigned int justSkipped = 0;

		while (info->def.validDefinition == false)
		{
			justSkipped++;
			tIndex = j + skip + justSkipped;
			if (strWhole[tIndex - 1] == '\n')
			{
				StringHelper::TrimUTF16Vector(last_word);

				last_word.push_back('\n');
				multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
				last_word.clear();
				isStartOfWord = false;
				isStartOfLine = false;
				startOfWord = -1;
				startOfLine = -1;
			}
			if (tIndex < limit)
			{
				info = &theLabel->_lettersInfo.at(tIndex);
			}
			else
				break;
		}
		skip += justSkipped;
		tIndex = j + skip;

		if (tIndex >= limit)
			break;

		char16_t character = strWhole[tIndex];

		if (!isStartOfWord)
		{
			startOfWord = info->position.x * scalsX;
			isStartOfWord = true;
		}

		if (!isStartOfLine)
		{
			startOfLine = startOfWord;
			isStartOfLine = true;
		}

		// 1) Whitespace.
		// 2) This character is non-CJK, but the last character is CJK
		bool isspace = StringHelper::IsUnicodeSpace(character);
		bool isCJK = false;
		if (!isspace)
		{
			isCJK = StringHelper::IsCJKUnicode(character);
		}

		if (isspace ||
			(!last_word.empty() && StringHelper::IsCJKUnicode(last_word.back()) && !isCJK))
		{
			// if current character is white space, put it into the current word
			if (isspace) last_word.push_back(character);
			multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
			last_word.clear();
			isStartOfWord = false;
			startOfWord = -1;
			// put the CJK character in the last word
			// and put the non-CJK(ASCII) character in the current word
			if (!isspace) last_word.push_back(character);
			continue;
		}

		float posRight = (info->position.x + info->contentSize.width) * scalsX;
		// Out of bounds.
		if (posRight - startOfLine > lineWidth + 2)
		{
			if (!breakLineWithoutSpace && !isCJK)
			{
				last_word.push_back(character);

				int found = StringHelper::GetIndexOfLastNotChar16(multiline_string, ' ');
				if (found != -1)
					StringHelper::TrimUTF16Vector(multiline_string);
				else
					multiline_string.clear();

				if (multiline_string.size() > 0)
					multiline_string.push_back('\n');

				isStartOfLine = false;
				startOfLine = -1;
			}
			else
			{
				StringHelper::TrimUTF16Vector(last_word);

				last_word.push_back('\n');

				multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
				last_word.clear();
				isStartOfWord = false;
				isStartOfLine = false;
				startOfWord = -1;
				startOfLine = -1;
				--j;
			}
		}
		else
		{
			// Character is normal.
			last_word.push_back(character);
		}
	}

	multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());

	std::u16string strNew(multiline_string.begin(), multiline_string.end());

	theLabel->_currentUTF16String = strNew;
	theLabel->computeStringNumLines();
	theLabel->computeHorizontalKernings(theLabel->_currentUTF16String);

	return true;
}

bool LabelTextFormatter::alignText(GLLabel *theLabel)
{
	int i = 0;

	int lineNumber = 0;
	int strLen = theLabel->_limitShowCount;
	std::vector<char16_t> lastLine;
	const auto& strWhole = theLabel->_currentUTF16String;

	if (theLabel->_labelWidth > theLabel->_contentSize.width)
	{
		theLabel->setContentSize(Size(theLabel->_labelWidth, theLabel->_contentSize.height));
	}

	for (int ctr = 0; ctr <= strLen; ++ctr)
	{
		char16_t currentChar = strWhole[ctr];

		if (currentChar == '\n' || currentChar == 0)
		{
			auto lineLength = lastLine.size();

			// if last line is empty we must just increase lineNumber and work with next line
			if (lineLength == 0)
			{
				lineNumber++;
				continue;
			}
			int index = static_cast<int>(i + lineLength - 1 + lineNumber);
			if (index < 0) continue;

			auto info = &theLabel->_lettersInfo.at(index);
			if (info->def.validDefinition == false)
				continue;

			float shift = 0;
			switch (theLabel->_hAlignment)
			{
			case TextHAlignment::CENTER:
			{
				float lineWidth = info->position.x + info->contentSize.width;
				shift = theLabel->_contentSize.width / 2.0f - lineWidth / 2.0f;
				break;
			}
			case TextHAlignment::RIGHT:
			{
				float lineWidth = info->position.x + info->contentSize.width;
				shift = theLabel->_contentSize.width - lineWidth;
				break;
			}
			default:
				break;
			}

			if (shift != 0)
			{
				for (unsigned j = 0; j < lineLength; ++j)
				{
					index = i + j + lineNumber;
					if (index < 0) continue;

					info = &theLabel->_lettersInfo.at(index);
					if (info)
					{
						info->position.x += shift;
					}
				}
			}

			i += lineLength;
			++lineNumber;

			lastLine.clear();
			continue;
		}

		lastLine.push_back(currentChar);
	}

	return true;
}

bool LabelTextFormatter::createStringSprites(GLLabel *theLabel)
{
	// check for string
	unsigned int stringLen = theLabel->getStringLength();
	theLabel->_limitShowCount = 0;

	// no string
	if (stringLen == 0)
		return false;

	int longestLine = 0;
	unsigned int totalHeight = theLabel->_commonLineHeight * theLabel->_currNumLines;
	int nextFontPositionX = 0;
	int nextFontPositionY = totalHeight;
	auto contentScaleFactor = GL_CONTENT_SCALE_FACTOR();

	if (theLabel->_labelHeight > 0)
	{
		auto labelHeightPixel = theLabel->_labelHeight * contentScaleFactor;
		
		/*LiXizhi 2018.4.15: why rescaling the line spacing according to the control height. not necessary.
		if (totalHeight > labelHeightPixel)
		{
			int numLines = labelHeightPixel / theLabel->_commonLineHeight;
			totalHeight = numLines * theLabel->_commonLineHeight;
		}*/

		switch (theLabel->_vAlignment)
		{
		case TextVAlignment::TOP:
			nextFontPositionY = labelHeightPixel;
			break;
		case TextVAlignment::CENTER:
			nextFontPositionY = (labelHeightPixel + totalHeight) / 2.0f;
			break;
		case TextVAlignment::BOTTOM:
			nextFontPositionY = totalHeight;
			break;
		default:
			break;
		}
	}

	CCRect charRect;
	int charXOffset = 0;
	int charYOffset = 0;
	int charAdvance = 0;

	const auto& strWhole = theLabel->_currentUTF16String;
	auto fontAtlas = theLabel->_fontAtlas;

	FontLetterDefinition tempDefinition;
	Vector2 letterPosition;
	const auto& kernings = theLabel->_horizontalKernings;

	float clipTop = 0;
	float clipBottom = 0;
	int lineIndex = 0;
	bool lineStart = true;
	bool clip = false;
	if (theLabel->_currentLabelType == GLLabel::LabelType::TTF && theLabel->_clipEnabled)
	{
		clip = true;
	}

	for (unsigned int i = 0; i < stringLen; i++)
	{
		char16_t c = strWhole[i];
		if (fontAtlas->getLetterDefinitionForChar(c, tempDefinition))
		{
			charXOffset = tempDefinition.offsetX;
			charYOffset = tempDefinition.offsetY;
			charAdvance = tempDefinition.xAdvance;
		}
		else
		{
			charXOffset = -1;
			charYOffset = -1;
			charAdvance = -1;
		}

		if (c == '\n')
		{
			lineIndex++;
			nextFontPositionX = 0;
			nextFontPositionY -= theLabel->_commonLineHeight;

			theLabel->recordPlaceholderInfo(i);
			
			// LiXizhi 2018.4.15: why we break here? it will not render multiple lines this way
			//if (nextFontPositionY < theLabel->_commonLineHeight)
			//	break;
			
			lineStart = true;
			continue;
		}
		else if (clip && tempDefinition.height > 0.0f)
		{
			if (lineStart)
			{
				if (lineIndex == 0)
				{
					clipTop = charYOffset;
				}
				lineStart = false;
				clipBottom = tempDefinition.clipBottom;
			}
			else if (tempDefinition.clipBottom < clipBottom)
			{
				clipBottom = tempDefinition.clipBottom;
			}

			if (lineIndex == 0 && charYOffset < clipTop)
			{
				clipTop = charYOffset;
			}
		}

		letterPosition.x = (nextFontPositionX + charXOffset + kernings[i]) / contentScaleFactor;
		letterPosition.y = (nextFontPositionY - charYOffset) / contentScaleFactor;

		if (theLabel->recordLetterInfo(letterPosition, tempDefinition, i) == false)
		{
			continue;
		}

		nextFontPositionX += charAdvance + kernings[i] + theLabel->_additionalKerning;

		if (longestLine < nextFontPositionX)
		{
			longestLine = nextFontPositionX;
		}
	}

	float lastCharWidth = tempDefinition.width * contentScaleFactor;
	Size tmpSize;
	tmpSize.width = longestLine;
	tmpSize.height = totalHeight;

	if (theLabel->_labelHeight > 0)
	{
		tmpSize.height = theLabel->_labelHeight * contentScaleFactor;
	}

	if (clip)
	{
		int clipTotal = (clipTop + clipBottom) / contentScaleFactor;
		tmpSize.height -= clipTotal * contentScaleFactor;
		clipBottom /= contentScaleFactor;

		for (int i = 0; i < theLabel->_limitShowCount; i++)
		{
			theLabel->_lettersInfo[i].position.y -= clipBottom;
		}
	}

	theLabel->setContentSize(tmpSize);

	return true;
}


// label

GLLabel::GLLabel(GLFontAtlas *atlas /* = nullptr */, TextHAlignment hAlignment /* = TextHAlignment::LEFT */,
	TextVAlignment vAlignment /* = TextVAlignment::TOP */, bool useDistanceField /* = false */, bool useA8Shader /* = false */)
	:
	_commonLineHeight(0.0f)
	, _additionalKerning(0.0f)
	, _lineBreakWithoutSpaces(false)
	, _maxLineWidth(0)
	, _labelWidth(0)
	, _labelHeight(0)
	, _labelDimensions(Size::ZERO)
	, _hAlignment(hAlignment)
	, _vAlignment(vAlignment)
	, _horizontalKernings(nullptr)
	, _fontAtlas(atlas)
	, _isOpacityModifyRGB(false)
	, _useDistanceField(useDistanceField)
	, _useA8Shader(useA8Shader)
	, _fontScale(1.0f)
	, _uniformEffectColor(0)
	, _currNumLines(-1)
	//, _textSprite(nullptr)
	, _contentDirty(false)
	, _shadowDirty(false)
	, _compatibleMode(false)
	, _insideBounds(true)
	, _effectColorF(Color4F::BLACK)
	, _scaleX(1.0f)
	, _scaleY(1.0f)
{
	//TODO:wangpeng maybe dont need: setAnchorPoint(Vector2::ANCHOR_MIDDLE);
	reset();
}

GLLabel::~GLLabel()
{

	delete[] _horizontalKernings;

	if (_fontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(_fontAtlas);
	}
}


bool GLLabel::setTTFConfig(const TTFConfig& ttfConfig)
{

	GLFontAtlas *newAtlas = FontAtlasCache::getFontAtlasTTF(ttfConfig);

	if (!newAtlas)
	{
		reset();
		return false;
	}
	_systemFontDirty = false;

	_currentLabelType = LabelType::TTF;
	setFontAtlas(newAtlas, ttfConfig.distanceFieldEnabled, true);

	_fontConfig = ttfConfig;
	if (_fontConfig.outlineSize > 0)
	{
		_fontConfig.distanceFieldEnabled = false;
		_useDistanceField = false;
		_useA8Shader = false;
	}
	else
	{
		if (ttfConfig.distanceFieldEnabled)
		{
			this->setFontScale(1.0f * ttfConfig.fontSize / DistanceFieldFontSize);
		}
	}

	return true;
}

void GLLabel::setFontAtlas(GLFontAtlas* atlas, bool distanceFieldEnabled /* = false */, bool useA8Shader /* = false */)
{
	if (atlas == _fontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(atlas);
		return;
	}

	if (_fontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(_fontAtlas);
		_fontAtlas = nullptr;
	}

	_fontAtlas = atlas;


	if (_fontAtlas)
	{
		_commonLineHeight = _fontAtlas->getCommonLineHeight();
		_contentDirty = true;
	}

	_useDistanceField = distanceFieldEnabled;
	_useA8Shader = useA8Shader;
}

void GLLabel::reset()
{
	TTFConfig temp;
	_fontConfig = temp;

	_systemFontDirty = false;
	_systemFont = "Helvetica";
	_systemFontSize = 12;

	if (_fontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(_fontAtlas);
		_fontAtlas = nullptr;
	}

	_currentLabelType = LabelType::STRING_TEXTURE;

	_shadowBlurRadius = 0;

	_textColor = Color4B::WHITE;
	_textColorF = Color4F::WHITE;

	_shadowEnabled = false;
	_clipEnabled = false;
	_blendFuncDirty = false;
}



void GLLabel::setPosition(float x, float y)
{
	_position.x = x;
	_position.y = y;
}

bool GLLabel::recordLetterInfo(const Vector2& point, const FontLetterDefinition& letterDef, int spriteIndex)
{
	if (static_cast<std::size_t>(spriteIndex) >= _lettersInfo.size())
	{
		LetterInfo tmpInfo;
		_lettersInfo.push_back(tmpInfo);
	}

	_lettersInfo[spriteIndex].def = letterDef;
	_lettersInfo[spriteIndex].position = point;
	_lettersInfo[spriteIndex].contentSize.width = _lettersInfo[spriteIndex].def.width;
	_lettersInfo[spriteIndex].contentSize.height = _lettersInfo[spriteIndex].def.height;
	_limitShowCount++;

	return _lettersInfo[spriteIndex].def.validDefinition;
}

bool GLLabel::recordPlaceholderInfo(int spriteIndex)
{
	if (static_cast<std::size_t>(spriteIndex) >= _lettersInfo.size())
	{
		LetterInfo tmpInfo;
		_lettersInfo.push_back(tmpInfo);
	}

	_lettersInfo[spriteIndex].def.validDefinition = false;
	_limitShowCount++;

	return false;
}

void GLLabel::setFontScale(float fontScale)
{
	_fontScale = fontScale * GL_CONTENT_SCALE_FACTOR();
}


bool GLLabel::computeHorizontalKernings(const std::u16string& stringToRender)
{
	if (_horizontalKernings)
	{
		delete[] _horizontalKernings;
		_horizontalKernings = nullptr;
	}

	int letterCount = 0;
	_horizontalKernings = _fontAtlas->getFont()->getHorizontalKerningForTextUTF16(stringToRender, letterCount);

	if (!_horizontalKernings)
		return false;
	else
		return true;
}


void GLLabel::computeStringNumLines()
{
	int quantityOfLines = 1;

	if (_currentUTF16String.empty())
	{
		_currNumLines = 0;
		return;
	}

	// count number of lines
	size_t stringLen = _currentUTF16String.length();
	for (size_t i = 0; i < stringLen - 1; ++i)
	{
		if (_currentUTF16String[i] == '\n')
		{
			quantityOfLines++;
		}
	}

	_currNumLines = quantityOfLines;
}


void GLLabel::setAlignment(TextHAlignment hAlignment, TextVAlignment vAlignment)
{
	if (hAlignment != _hAlignment || vAlignment != _vAlignment)
	{
		_hAlignment = hAlignment;
		_vAlignment = vAlignment;

		_contentDirty = true;
	}
}

void  GLLabel::setContentSize(const Size & var)
{
	_contentSize = var;
}

//TODO:wangepng scale
void GLLabel::setScale(float scale)
{
	if (_useDistanceField)
	{
		scale *= _fontScale;
	}
	//Node::setScale(scale);
}

void GLLabel::setScaleX(float scaleX)
{
	if (_useDistanceField)
	{
		scaleX *= _fontScale;
	}
	//Node::setScaleX(scaleX);
}

void GLLabel::setScaleY(float scaleY)
{
	if (_useDistanceField)
	{
		scaleY *= _fontScale;
	}
	//Node::setScaleY(scaleY);
}

float GLLabel::getScaleY() const
{
	if (_useDistanceField)
	{
		return _scaleY / _fontScale;
	}
	else
	{
		return _scaleY;
	}
}

float GLLabel::getScaleX() const
{
	if (_useDistanceField)
	{
		return _scaleX / _fontScale;
	}
	else
	{
		return _scaleX;
	}
}


int GLLabel::getStringLength() const
{
	return static_cast<int>(_currentUTF16String.length());
}

void GLLabel::setDimensions(unsigned int width, unsigned int height)
{
	if (height != _labelHeight || width != _labelWidth)
	{
		_labelWidth = width;
		_labelHeight = height;
		_labelDimensions.width = width;
		_labelDimensions.height = height;

		_maxLineWidth = width;
		_contentDirty = true;
	}
}
