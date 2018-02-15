//-----------------------------------------------------------------------------
// Class:	common headers for parax model
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8, 2014.8
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaXBone.h"
#include "ParaXModelCommon.h"

using namespace ParaEngine;

void ModelCamera::setup(int time)
{
	if (!m_bIsValid) return;
}

void ModelLight::setup(int time, uint32 l)
{
}

void ModelAttachment::setup(CParaXModel *model)
{
	/** First translate the model to the attachment point, then apply the bone transformation matrix.
	* mat = [(translate matrix)*(bone matrix)]*(worldmatrix)*/
	Matrix4 mat, mat1;
	mat1 = model->bones[bone].mat;
	mat.makeTrans(pos.x, pos.y, pos.z);
	ParaMatrixMultiply(&mat1, &mat, &mat1);
	ParaMatrixMultiply(&mat, &mat1, &CGlobals::GetWorldMatrixStack().SafeGetTop());
	CGlobals::GetWorldMatrixStack().push(mat);
}


