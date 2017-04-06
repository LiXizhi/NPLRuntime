//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2015.2.28
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaScriptingPainter.h"
#include "ParaScriptingGlobal.h"
#include "ParaScriptingScene.h"
#include "PaintEngine/Painter.h"
#include "util/StringHelper.h"
#include "2dengine/TextureParams.h"
#include "NPL/NPLHelper.h"


#include <luabind/luabind.hpp>

using namespace ParaScripting;

CPainter* ParaScripting::ParaPainter::m_pPainter = NULL;

bool ParaScripting::ParaPainter::Begin(const object & paintDevice)
{
	if (m_pPainter && !m_pPainter->isActive()){
		m_pPainter->begin(NULL);
	}
	return true;
}

void ParaScripting::ParaPainter::End()
{
	if (m_pPainter && m_pPainter->isActive()){
		m_pPainter->end();
	}
}

void ParaScripting::ParaPainter::Flush()
{
	if (m_pPainter)
		m_pPainter->Flush();
}

void ParaScripting::ParaPainter::DrawPoint(float x, float y)
{
	if (m_pPainter)
		m_pPainter->drawPoint(QPointF(x, y));
}

void ParaScripting::ParaPainter::DrawLine(float x1, float y1, float x2, float y2)
{
	if (m_pPainter)
		m_pPainter->drawLine(QLineF(x1, y1, x2, y2));
}


void ParaScripting::ParaPainter::DrawLineList(const object& lineList, int nLineCount, int nIndexOffset)
{
	if(m_pPainter && luabind::type(lineList) == LUA_TTABLE && nLineCount > 0)
	{
		static std::vector<Vector3> vertices;
		int nVerticesCount = nLineCount * 2;
		vertices.resize(nVerticesCount);

		for (int i = 0; i < nVerticesCount; ++i)
		{
			const object& tri = lineList[nIndexOffset + i + 1]; // lua index start from 1
			if (luabind::type(tri) == LUA_TTABLE)
			{
				vertices[i].x = object_cast<float>(tri[1]);
				vertices[i].y = object_cast<float>(tri[2]);
				vertices[i].z = object_cast<float>(tri[3]);
			}
			else
			{
				OUTPUT_LOG("error input format of DrawLineList\n");
				return;
			}
		}
		m_pPainter->drawLines(&(vertices[0]), nLineCount);
		vertices.clear();
	}
}


void ParaScripting::ParaPainter::DrawTriangleList(const object& triangleList, int nTriangleCount, int nIndexOffset)
{
	if (m_pPainter && luabind::type(triangleList) == LUA_TTABLE && nTriangleCount > 0)
	{
		static std::vector<Vector3> triangles;
		int nVerticesCount = nTriangleCount * 3;
		triangles.resize(nVerticesCount);
		
		for (int i = 0; i < nVerticesCount; ++i)
		{
			const object& tri = triangleList[nIndexOffset + i + 1]; // lua index start from 1
			if (luabind::type(tri) == LUA_TTABLE)
			{
				triangles[i].x = object_cast<float>(tri[1]);
				triangles[i].y = object_cast<float>(tri[2]);
				triangles[i].z = object_cast<float>(tri[3]);
			}
			else
			{
				OUTPUT_LOG("error input format of DrawTriangleList\n");
				return;
			}
		}
		m_pPainter->drawTriangles(&(triangles[0]), nTriangleCount);
		triangles.clear();
	}
}

void ParaScripting::ParaPainter::DrawRect(float x1, float y1, float w, float h)
{
	if (m_pPainter)
		m_pPainter->drawRect(QRectF(x1, y1, w, h));
}

void ParaScripting::ParaPainter::DrawTexture(int x, int y, int w, int h, const object& texture)
{
	std::string filename;
	if (NPL::NPLHelper::LuaObjectToString(texture, filename))
	{
		TextureParams texParams(filename.c_str());
		texParams.drawTexture(m_pPainter, x, y, w, h);
	}
	else
	{
		TextureEntity* pTextureEntity = ParaAssetObject::GetTexture(texture);
		if (m_pPainter && pTextureEntity)
			m_pPainter->drawTexture(x, y, w, h, pTextureEntity);
	}
}

void ParaScripting::ParaPainter::DrawTexture2(int x, int y, int w, int h, const object& texture, int sx, int sy, int sw, int sh)
{
	TextureEntity* pTextureEntity = ParaAssetObject::GetTexture(texture);
	if (m_pPainter && pTextureEntity)
		m_pPainter->drawTexture(x, y, w, h, pTextureEntity, sx, sy, sw, sh);
}

void ParaScripting::ParaPainter::DrawText(float x, float y, const std::string &s)
{
	if (m_pPainter)
		m_pPainter->drawText(QPointF(x, y), s);
}

void ParaScripting::ParaPainter::DrawText2(float x, float y, float w, float h, const std::string &s, int textOption)
{
	if (m_pPainter)
		m_pPainter->drawText(QRectF(x, y, w, h), s, QTextOption(textOption));
}

void ParaScripting::ParaPainter::DrawSceneObject(ParaObject & obj, int nOption)
{
	if (m_pPainter && obj.IsValid())
		m_pPainter->DrawSceneObject(obj.m_pObj, nOption);
}

void ParaScripting::ParaPainter::Save()
{
	if (m_pPainter)
		m_pPainter->save();
}

void ParaScripting::ParaPainter::Restore()
{
	if (m_pPainter)
		m_pPainter->restore();
}

void ParaScripting::ParaPainter::SetCompositionMode(int mode)
{
	if (m_pPainter)
		m_pPainter->setCompositionMode((CPainter::CompositionMode)mode);
}

int ParaScripting::ParaPainter::GetCompositionMode()
{
	if (m_pPainter)
		return (int)m_pPainter->compositionMode();
	else
		return CPainter::CompositionMode_SourceOver;
}

void ParaScripting::ParaPainter::SetFont(const object& font)
{
	if (!m_pPainter)
		return;
	auto _type = luabind::type(font);
	if (_type == LUA_TTABLE)
	{
		const char* family = NPL::NPLHelper::LuaObjectToString(font["family"]);
		int size = NPL::NPLHelper::LuaObjectToInt(font["size"], -1);
		bool bold = NPL::NPLHelper::LuaObjectToBool(font["bold"]);
		QFont font_(family, size, bold ? QFont::Bold : QFont::Normal);
		m_pPainter->setFont(font_);
	}
	else if (_type == LUA_TSTRING)
	{
		string temp = NPL::NPLHelper::LuaObjectToString(font);
		string fontname, fontsize, isbold;
		int size = 11;
		bool bold = true;
		ParaEngine::StringHelper::DevideString(temp, fontname, fontsize);
		temp = fontsize;
		ParaEngine::StringHelper::DevideString(temp, fontsize, isbold);
		if (sscanf(fontsize.c_str(), "%d", &size) != 1)
			size = 11;
		bold = (isbold.find("bold") != string::npos);
		QFont font_(fontname, size, bold ? QFont::Bold : QFont::Normal);
		m_pPainter->setFont(font_);
	}
}

void ParaScripting::ParaPainter::SetPen(const object& pen)
{
	if (!m_pPainter)
		return;
	auto pen_type = luabind::type(pen);
	if (pen_type == LUA_TTABLE)
	{
		float width = NPL::NPLHelper::LuaObjectToFloat(pen["width"], 0.f);
		if (width != 0 && m_pPainter->pen().widthF()!=width)
		{
			QPen pen = m_pPainter->pen();
			pen.setWidthF(width);
			m_pPainter->setPen(pen);
		}
		Color color = NPL::NPLHelper::LuaObjectToColor(pen["color"], 0);
		auto brush = pen["brush"];
		if (NPL::NPLHelper::LuaObjectIsTable(brush))
		{
			SetBrush(brush);
		}
		if ((DWORD)color != 0)
		{
			m_pPainter->setPen(color);
		}
	}
	else if (pen_type == LUA_TSTRING || pen_type == LUA_TNUMBER)
	{
		Color color = NPL::NPLHelper::LuaObjectToColor(pen);
		m_pPainter->setPen(color);
	}
}

void ParaScripting::ParaPainter::SetBrush(const object& brush)
{
	if (!m_pPainter)
		return;
	auto pen_type = luabind::type(brush);
	if (pen_type == LUA_TTABLE)
	{
		Color color = NPL::NPLHelper::LuaObjectToColor(brush["color"], 0);
		TextureEntity* pTexture = ParaAssetObject::GetTexture(brush["texture"]);
		
		if ((DWORD)color != 0 || pTexture)
		{
			QBrush brush(color, pTexture);
			m_pPainter->setBrush(brush);
		}
	}
	else if (pen_type == LUA_TSTRING || pen_type == LUA_TNUMBER)
	{
		Color color = NPL::NPLHelper::LuaObjectToColor(brush, 0);
		m_pPainter->setPen(color);
	}
}

void ParaScripting::ParaPainter::SetBrushOrigin(float x, float y)
{
	if (m_pPainter)
		m_pPainter->setBrushOrigin((int)x, (int)y);
}

void ParaScripting::ParaPainter::SetBackground(const object& brush)
{
	if (!m_pPainter)
		return;
	auto pen_type = luabind::type(brush);
	if (pen_type == LUA_TTABLE)
	{
		Color color = NPL::NPLHelper::LuaObjectToColor(brush["color"], 0);
		TextureEntity* pTexture = ParaAssetObject::GetTexture(brush["texture"]);

		if ((DWORD)color != 0 || pTexture)
		{
			QBrush brush(color, pTexture);
			m_pPainter->setBackground(brush);
		}
	}
	else if (pen_type == LUA_TSTRING)
	{
		Color color = NPL::NPLHelper::LuaObjectToColor(brush);
		if ((DWORD)color != 0)
		{
			QBrush brush(color);
			m_pPainter->setBackground(brush);
		}
	}
}

void ParaScripting::ParaPainter::SetOpacity(float fOpacity)
{
	if (m_pPainter)
		m_pPainter->setOpacity(fOpacity);
}

void ParaScripting::ParaPainter::SetClipRegion(int x, int y, int w, int h)
{
	if (m_pPainter)
		m_pPainter->setClipRegion(QRect(x, y, w, h));
}

void ParaScripting::ParaPainter::SetClipping(bool enable)
{
	if (m_pPainter)
		m_pPainter->setClipping(enable);
}

bool ParaScripting::ParaPainter::HasClipping()
{
	if (m_pPainter)
		return m_pPainter->hasClipping();
	else
		return false;
}

void ParaScripting::ParaPainter::Scale(float sx, float sy)
{
	if (m_pPainter)
		m_pPainter->scale(sx, sy);
}

void ParaScripting::ParaPainter::Shear(float sh, float sv)
{
	if (m_pPainter)
		m_pPainter->shear(sh, sv);
}

void ParaScripting::ParaPainter::Rotate(float a)
{
	if (m_pPainter)
		m_pPainter->rotate(a);
}

void ParaScripting::ParaPainter::Translate(float dx, float dy)
{
	if (m_pPainter)
		m_pPainter->translate(dx, dy);
}

void ParaScripting::ParaPainter::SetTransform(const object& trans, bool combine)
{
	if (!m_pPainter)
		return;
	if (luabind::type(trans) == LUA_TTABLE)
	{
		auto affine = trans["affine"];
		if (luabind::type(affine) == LUA_TTABLE)
		{
			float _m11 = NPL::NPLHelper::LuaObjectToFloat(affine["m11"], 1.f);
			float _m12 = NPL::NPLHelper::LuaObjectToFloat(affine["m12"], 0.f);
			float _m21 = NPL::NPLHelper::LuaObjectToFloat(affine["m21"], 0.f);
			float _m22 = NPL::NPLHelper::LuaObjectToFloat(affine["m22"], 1.f);
			float _dx = NPL::NPLHelper::LuaObjectToFloat(affine["dx"], 0.f);
			float _dy = NPL::NPLHelper::LuaObjectToFloat(affine["dy"], 0.f);

			// float m_13 = NPL::NPLHelper::LuaObjectToFloat(trans["m_13"], 0.f);
			// float m_23 = NPL::NPLHelper::LuaObjectToFloat(trans["m_23"], 0.f);
			// float m_33 = NPL::NPLHelper::LuaObjectToFloat(trans["m_23"], 1.f);
			QTransform t(QMatrix(_m11, _m12, _m21, _m22, _dx, _dy));
			// int trans_type = NPL::NPLHelper::LuaObjectToInt(trans["m_type"], -1);
			// if (trans_type > 0)
			// 	t.m_type = trans_type;
			m_pPainter->setWorldTransform(t, combine);
		}
		else
		{
			float _m11 = NPL::NPLHelper::LuaObjectToFloat(trans["m11"], 1.f);
			float _m12 = NPL::NPLHelper::LuaObjectToFloat(trans["m12"], 0.f);
			float _m21 = NPL::NPLHelper::LuaObjectToFloat(trans["m21"], 0.f);
			float _m22 = NPL::NPLHelper::LuaObjectToFloat(trans["m22"], 1.f);
			float _dx = NPL::NPLHelper::LuaObjectToFloat(trans["dx"], 0.f);
			float _dy = NPL::NPLHelper::LuaObjectToFloat(trans["dy"], 0.f);
			QTransform t(QMatrix(_m11, _m12, _m21, _m22, _dx, _dy));
			m_pPainter->setWorldTransform(t, combine);
		}
	}
}

luabind::object ParaScripting::ParaPainter::GetTransform(const object& out)
{
	if (luabind::type(out) == LUA_TTABLE)
	{
		const QMatrix& mat = m_pPainter->worldTransform().toAffine();
		out["m11"] = mat.m11();
		out["m12"] = mat.m12();
		out["m21"] = mat.m21();
		out["m22"] = mat.m22();
		out["dx"] = mat.dx();
		out["dy"] = mat.dy();
	}
	return object(out);
}

const ParaAttributeObject& ParaScripting::ParaPainter::GetAttributeObject()
{
	static ParaAttributeObject obj;
	obj.m_pAttribute = m_pPainter;
	obj.m_pAttClass = NULL;
	return obj;
}

void ParaScripting::ParaPainter::CallField(const char* sFieldname)
{
	ParaAttributeObject att(m_pPainter);
	att.CallField(sFieldname);
}

void ParaScripting::ParaPainter::SetField(const char* sFieldname, const object& input)
{
	ParaAttributeObject att(m_pPainter);
	att.SetField(sFieldname, input);
}

luabind::object ParaScripting::ParaPainter::GetField(const char* sFieldname, const object& output)
{
	ParaAttributeObject att(m_pPainter);
	return att.GetField(sFieldname, output);
}

