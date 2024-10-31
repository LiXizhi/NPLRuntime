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

void ParaScripting::ParaPainter::SetPenStr(const char* sColor)
{
	if (m_pPainter)
	{
		Color color = Color::FromString(sColor);
		m_pPainter->setPen(color);
	}
}

void ParaScripting::ParaPainter::SetPenInt(uint32_t nColor)
{
	if (m_pPainter)
	{
		Color color((DWORD)(nColor));
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

// for LuaJit, only for function that maybe called thousands of time per second
extern "C" {

	PE_CORE_DECL void ParaPainter_Flush()
	{
		ParaScripting::ParaPainter::Flush();
	}

	PE_CORE_DECL void ParaPainter_Save()
	{
		ParaScripting::ParaPainter::Save();
	}

	PE_CORE_DECL void ParaPainter_Restore()
	{
		ParaScripting::ParaPainter::Restore();
	}

	PE_CORE_DECL void ParaPainter_CallField(const char*  sFieldname)
	{
		ParaScripting::ParaPainter::CallField(sFieldname);
	}

	PE_CORE_DECL void ParaPainter_SetCompositionMode(int mode)
	{
		ParaScripting::ParaPainter::SetCompositionMode(mode);
	}

	PE_CORE_DECL int ParaPainter_GetCompositionMode()
	{
		return ParaScripting::ParaPainter::GetCompositionMode();
	}

	PE_CORE_DECL void ParaPainter_SetOpacity(float fOpacity) {
		ParaScripting::ParaPainter::SetOpacity(fOpacity);
	}
	PE_CORE_DECL void ParaPainter_SetClipRegion(int x, int y, int w, int h) {
		ParaScripting::ParaPainter::SetClipRegion(x, y, w, h);
	}
	PE_CORE_DECL void ParaPainter_SetClipping(bool enable) {
		ParaScripting::ParaPainter::SetClipping(enable);
	}
	PE_CORE_DECL bool ParaPainter_HasClipping() {
		return ParaScripting::ParaPainter::HasClipping();
	}
	PE_CORE_DECL void ParaPainter_Scale(float sx, float sy) {
		ParaScripting::ParaPainter::Scale(sx, sy);
	}
	PE_CORE_DECL void ParaPainter_Shear(float sh, float sv) {
		ParaScripting::ParaPainter::Shear(sh, sv);
	}
	PE_CORE_DECL void ParaPainter_Rotate(float a) {
		ParaScripting::ParaPainter::Rotate(a);
	}
	PE_CORE_DECL void ParaPainter_Translate(float dx, float dy) {
		ParaScripting::ParaPainter::Translate(dx, dy);
	}
	PE_CORE_DECL void ParaPainter_DrawPoint(float x, float y) {
		ParaScripting::ParaPainter::DrawPoint(x, y);
	}
	PE_CORE_DECL void ParaPainter_DrawLine(float x1, float y1, float x2, float y2) {
		ParaScripting::ParaPainter::DrawLine(x1, y1, x2, y2);
	}
	PE_CORE_DECL void ParaPainter_DrawRect(float x1, float y1, float w, float h) {
		ParaScripting::ParaPainter::DrawRect(x1, y1, w, h);
	}
	PE_CORE_DECL void ParaPainter_DrawTriangleList(const Vector3* triangleList, int nTriangleCount, int nIndexOffset) {
		if (nTriangleCount > 0)
			ParaScripting::ParaPainter::GetPainter()->drawTriangles(triangleList + nIndexOffset, nTriangleCount);
	}
	PE_CORE_DECL void ParaPainter_DrawLineList(const Vector3* lineList, int nLineCount, int nIndexOffset) {
		if (nLineCount > 0)
			ParaScripting::ParaPainter::GetPainter()->drawLines(lineList + nIndexOffset, nLineCount);
	}

	PE_CORE_DECL void ParaPainter_DrawText(float x, float y, const char* s)
	{
		if (s != 0)
			ParaScripting::ParaPainter::DrawText(x, y, s);
	}
	PE_CORE_DECL void ParaPainter_DrawText2(float x, float y, float w, float h, const char* s, int textOption)
	{
		if (s != 0)
			ParaScripting::ParaPainter::DrawText2(x, y, w, h, s, textOption);
	}

	PE_CORE_DECL void ParaPainter_SetPenStr(const char* color)
	{
		ParaScripting::ParaPainter::SetPenStr(color);
	}

	PE_CORE_DECL void ParaPainter_SetPenInt(uint32_t color)
	{
		ParaScripting::ParaPainter::SetPenInt(color);
	}
};

void ParaScripting::ParaPainter::FlushCmdList(const std::string &cmds)
{
	if (m_pPainter == nullptr) return;
	auto decode_cmd_arg = [](std::string& str) -> std::string
	{
		std::ostringstream decoded;
        auto str_size = str.length();
        for (size_t i = 0; i < str_size; ++i)
        {
            if (str[i] == '%')
            {
                if ((i + 2) >= str_size)
                {
					std::cerr << "Invalid percent encoding: " << str << std::endl;
					return "";
                }
                std::string hexStr = str.substr(i + 1, 2);
                char decodedChar   = static_cast<char>(std::stoi(hexStr, nullptr, 16));
                decoded << decodedChar;
                i += 2; 
            }
            else
            {
                decoded << str[i];
            }
        }
        return decoded.str();
	};
	auto load_matrix = [](std::istringstream &iss) -> Matrix4
	{
		float value;
		std::vector<float> values;
		while (iss.good()) {
			iss >> value;
			values.push_back(value);
		}
		if (values.size() == 16) {
			return Matrix4(values[0], values[1], values[2], values[3],
				values[4], values[5], values[6], values[7],
				values[8], values[9], values[10], values[11],
				values[12], values[13], values[14], values[15]);
		}
		else if (values.size() == 12) {
			return Matrix4(
					values[0], values[1], values[2], 0,
					values[3], values[4], values[5], 0,
					values[6], values[7], values[8], 0,
					values[9], values[10], values[11], 1.f);
		}
		else {
			return Matrix4::IDENTITY;
		}
	};
	std::istringstream iss(cmds);
	std::string cmd_line;
	std::string cmd_name;
	while (iss.good() && std::getline(iss, cmd_line))
	{
		std::istringstream cmd_iss(cmd_line);
		cmd_iss >> cmd_name;
		if (cmd_name == "Translate") {
			float dx = 0, dy = 0;
			cmd_iss >> dx >> dy;
			m_pPainter->translate(dx, dy);
		}
		else if (cmd_name == "Scale") {
			float sx = 0, sy = 0;
			cmd_iss >> sx >> sy;
			m_pPainter->scale(sx, sy);
		}
		else if (cmd_name == "Rotate") {
			float angle = 0;
			cmd_iss >> angle;
			m_pPainter->rotate(angle);
		}
		else if (cmd_name == "Shear") {
			float sh = 0, sv = 0;
			cmd_iss >> sh >> sv;
			m_pPainter->shear(sh, sv);
		}
		else if (cmd_name == "Flush") {
			m_pPainter->Flush();
		}
		else if (cmd_name == "Save") {
			m_pPainter->save();
		}
		else if (cmd_name == "Restore") {
			m_pPainter->restore();
		}
		else if (cmd_name == "SetCompositionMode") {
			int mode = 0;
			cmd_iss >> mode;
			m_pPainter->setCompositionMode((CPainter::CompositionMode)mode);
		}
		else if (cmd_name == "SetFont") {
			std::string temp;
			string fontname, fontsize, isbold;
			std::getline(cmd_iss, temp);
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
		else if (cmd_name == "SetPen") {
			std::string color, width, texture;
			cmd_iss >> color >> width >> texture;
			color = decode_cmd_arg(color);
			if (color.find("#") != std::string::npos || color.find(" ") != std::string::npos) {
				m_pPainter->setPen(Color::FromString(color.c_str()));
			} else {
				m_pPainter->setPen(std::strtol(color.c_str(), nullptr, 10));
			}
			if (!width.empty() && width != "0") {
				QPen pen = m_pPainter->pen();
				pen.setWidthF(std::strtof(width.c_str(), nullptr));
				m_pPainter->setPen(pen);
			} 
		}
		else if (cmd_name == "SetBrush") {
			std::string color, texture;
			cmd_iss >> color >> texture;
			color = decode_cmd_arg(color);

			auto sColor = Color::FromString(color.c_str());
			if (texture.empty()) {
				m_pPainter->setPen(sColor);
			} else {
				texture = decode_cmd_arg(texture);
				TextureEntity* pTexture = CGlobals::GetAssetManager()->LoadTexture("", texture);
				if (pTexture != nullptr && (DWORD)sColor != 0) {
					QBrush brush(sColor, pTexture);
					m_pPainter->setBrush(brush);
				}
			}
		}
		else if (cmd_name == "SetBrushOrigin") {
			int x = 0, y = 0;
			cmd_iss >> x >> y;
			m_pPainter->setBrushOrigin(x, y);
		}
		else if (cmd_name == "SetBackground") {
			std::string color, texture;
			cmd_iss >> color >> texture;
			color = decode_cmd_arg(color);

			auto sColor = Color::FromString(color.c_str());
			if (texture.empty()) {
				m_pPainter->setPen(sColor);
			} else {
				texture = decode_cmd_arg(texture);
				TextureEntity* pTexture = CGlobals::GetAssetManager()->LoadTexture("", texture);
				if (pTexture != nullptr && (DWORD)sColor != 0) {
					QBrush brush(sColor, pTexture);
					m_pPainter->setBrush(brush);
				}
			}
		}
		else if (cmd_name == "SetOpacity") {
			float opacity = 0;
			cmd_iss >> opacity;
			m_pPainter->setOpacity(opacity);
		}
		else if (cmd_name == "SetClipRegion") {
			int x = 0, y = 0, w = 0, h = 0;
			cmd_iss >> x >> y >> w >> h;
			m_pPainter->setClipRegion(QRect(x, y, w, h));
		}
		else if (cmd_name == "SetClipping") {
			bool enable;
			cmd_iss >> enable;
			m_pPainter->setClipping(enable);
		}
		else if (cmd_name == "DrawPoint") {
			float x = 0, y = 0;
			cmd_iss >> x >> y;
			m_pPainter->drawPoint(QPointF(x, y));
		}
		else if (cmd_name == "DrawLine") {
			float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			cmd_iss >> x1 >> y1 >> x2 >> y2;
			m_pPainter->drawLine(QLineF(x1, y1, x2, y2));
		}
		else if (cmd_name == "DrawTriangleList") {
			int nTriangleCount = 0, nIndexOffset = 0;
			float x = 0, y = 0, z = 0;
			cmd_iss >> nTriangleCount >> nIndexOffset;
			static std::vector<Vector3> triangles;
			int nVerticesCount = nTriangleCount * 3;
			triangles.resize(nVerticesCount);
			for (int i = 0; i < nIndexOffset; ++i) {
				cmd_iss >> x >> y >> z;
			}
			for (int i = 0; i < nVerticesCount; ++i) {
				cmd_iss >> x >> y >> z;
				triangles[i] = Vector3(x, y, z);
			}
			m_pPainter->drawTriangles(&(triangles[0]), nTriangleCount);
			triangles.clear();
		}
		else if (cmd_name == "DrawLineList") {
			int nLineCount = 0, nIndexOffset = 0;
			float x = 0, y = 0, z = 0;
			cmd_iss >> nLineCount >> nIndexOffset;
			static std::vector<Vector3> vertices;
			int nVerticesCount = nLineCount * 2;
			vertices.resize(nVerticesCount);
			for (int i = 0; i < nIndexOffset; ++i) {
				cmd_iss >> x >> y >> z;
			}
			for (int i = 0; i < nVerticesCount; ++i) {
				cmd_iss >> x >> y >> z;
				vertices[i] = Vector3(x, y, z);
			}
			m_pPainter->drawLines(&(vertices[0]), nLineCount);
			vertices.clear();
		}
		else if (cmd_name == "DrawRect") {
			float x = 0, y = 0, w = 0, h = 0;
			cmd_iss >> x >> y >> w >> h;
			m_pPainter->drawRect(QRectF(x, y, w, h));
		}
		else if (cmd_name == "DrawTexture") {
			int x = 0, y = 0, w = 0, h = 0;
			int sx = 0, sy = 0, sw = 0, sh = 0;
			std::string texture;
			cmd_iss >> x >> y >> w >> h >> texture >> sx >> sy >> sw >> sh;
			texture = decode_cmd_arg(texture);
			if (sx == 0 && sy == 0 && sw == 0 && sh == 0) {
				TextureParams texParams(texture.c_str());
				texParams.drawTexture(m_pPainter, x, y, w, h);
			} else {
				TextureEntity* pTexture = CGlobals::GetAssetManager()->LoadTexture("", texture);
				m_pPainter->drawTexture(x, y, w, h, pTexture, sx, sy, sw, sh);
			}
		}
		else if (cmd_name == "DrawText") {
			float x = 0, y = 0, w = 0, h = 0;
			int text_option = 0;
			std::string text;
			cmd_iss >> x >> y >> w >> h >> text >> text_option;
			text = decode_cmd_arg(text);
			if (h < 0) {
				m_pPainter->drawText(QPointF(x, y), text);
			} else {
				m_pPainter->drawText(QRectF(x, y, w, h), text, QTextOption(text_option));
			}
		}
		else if (cmd_name == "SetUseWorldMatrix") {
			bool enable = false;
			cmd_iss >> enable;
			m_pPainter->SetSpriteUseWorldMatrix(enable);
		}
		else if (cmd_name == "SetMatrixMode") {
			int mode = 0;
			cmd_iss >> mode;
			m_pPainter->SetMatrixMode(mode);
		}
		else if (cmd_name == "PushMatrix") {
			m_pPainter->PushMatrix();
		}
		else if (cmd_name == "PopMatrix") {
			m_pPainter->PopMatrix();
		}
		else if (cmd_name == "LoadCurrentMatrix") {
			m_pPainter->LoadCurrentMatrix();
		}
		else if (cmd_name == "LoadIdentityMatrix") {
			m_pPainter->LoadIdentityMatrix();
		}
		else if (cmd_name == "LoadBillboardMatrix") {
			m_pPainter->LoadBillboardMatrix();
		}
		else if (cmd_name == "LoadMatrix") {
			m_pPainter->LoadMatrix(load_matrix(cmd_iss));
		}
		else if (cmd_name == "MultiplyMatrix") {
			m_pPainter->MultiplyMatrix(load_matrix(cmd_iss));
		}
		else if (cmd_name == "TranslateMatrix") {
			float x = 0, y = 0, z = 0;
			cmd_iss >> x >> y >> z;
			m_pPainter->TranslateMatrix(x, y, z);
		}
		else if (cmd_name == "RotateMatrix") {
			float angle = 0, x = 0, y = 0, z = 0;
			cmd_iss >> angle >> x >> y >> z;
			m_pPainter->RotateMatrix(angle, x, y, z);
		}
		else if (cmd_name == "ScaleMatrix") {
			float x = 1, y = 1, z = 1;
			cmd_iss >> x >> y >> z;
			m_pPainter->ScaleMatrix(x, y, z);
		}
		else if (cmd_name == "AutoLineWidth") {
			bool enable = false;
			cmd_iss >> enable;
			m_pPainter->EnableAutoLineWidth(enable);
		}
		else {
			std::cerr << "unknown command: " << cmd_name << std::endl;
		}
	}
}
