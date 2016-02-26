//-----------------------------------------------------------------------------
// Class:	Matrix3x3
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.12.21
// Desc: Based on Pierre Terdiman's work in the ODE physics engine.
//-----------------------------------------------------------------------------
#include "DxStdAfx.h"
#include "ParaEngine.h"
#include "FPU.h"
#include "Matrix3x3.h"

using namespace ParaEngine;

Matrix3x3::Matrix3x3(void)
{
}

Matrix3x3::~Matrix3x3(void)
{
}

Matrix3x3::Matrix3x3(const Matrix3x3& mat)				
{ 
	CopyMemory(m, &mat.m, 9*sizeof(float));	
}

Matrix3x3::Matrix3x3(const D3DXMATRIX& mat4x4)
{
	m[0][0] = mat4x4._11;	m[0][1] = mat4x4._12;	m[0][2] = mat4x4._13;
	m[1][0] = mat4x4._21;	m[1][1] = mat4x4._22;	m[1][2] = mat4x4._23;
	m[2][0] = mat4x4._31;	m[2][1] = mat4x4._32;	m[2][2] = mat4x4._33;
}

void			Matrix3x3::Copy(const Matrix3x3& source)				
{ 
	CopyMemory(m, source.m, 9*sizeof(float));			
}

void			Matrix3x3::Zero()
{ 
	ZeroMemory(&m, sizeof(m));
}

D3DXVECTOR3			Matrix3x3::operator*(const D3DXVECTOR3& v)		const		
{ 
	return D3DXVECTOR3(D3DXVec3Dot(&GetRow(0), &v), D3DXVec3Dot(&GetRow(1),&v), D3DXVec3Dot(&GetRow(2),&v)); 
}

bool			Matrix3x3::IsIdentity()			const
{
	if(IR(m[0][0])!=IEEE_1_0)	return false;
	if(IR(m[0][1])!=0)			return false;
	if(IR(m[0][2])!=0)			return false;

	if(IR(m[1][0])!=0)			return false;
	if(IR(m[1][1])!=IEEE_1_0)	return false;
	if(IR(m[1][2])!=0)			return false;

	if(IR(m[2][0])!=0)			return false;
	if(IR(m[2][1])!=0)			return false;
	if(IR(m[2][2])!=IEEE_1_0)	return false;

	return true;
}

void Matrix3x3::Transpose()
{
	IR(m[1][0]) ^= IR(m[0][1]);	IR(m[0][1]) ^= IR(m[1][0]);	IR(m[1][0]) ^= IR(m[0][1]);
	IR(m[2][0]) ^= IR(m[0][2]);	IR(m[0][2]) ^= IR(m[2][0]);	IR(m[2][0]) ^= IR(m[0][2]);
	IR(m[2][1]) ^= IR(m[1][2]);	IR(m[1][2]) ^= IR(m[2][1]);	IR(m[2][1]) ^= IR(m[1][2]);
}

// Cast operator
Matrix3x3::operator D3DXMATRIX() const
{
	return D3DXMATRIX(
		m[0][0],	m[0][1],	m[0][2],	0.0f,
		m[1][0],	m[1][1],	m[1][2],	0.0f,
		m[2][0],	m[2][1],	m[2][2],	0.0f,
		0.0f,		0.0f,		0.0f,		1.0f);
}

