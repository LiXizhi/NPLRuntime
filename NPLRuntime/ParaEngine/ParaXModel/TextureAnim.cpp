//-----------------------------------------------------------------------------
// Class:	TextureAnim
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8, 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXBone.h"
#include "ParaXModel.h"
#include "TextureAnim.h"

using namespace ParaEngine;

void TextureAnim::calc(int anim, int time)
{
	if (trans.used) {
		tval = trans.getValue(anim, time);
	}
	if (rot.used) {
		rval = rot.getValue(anim, time);
	}
	if (scale.used) {
		sval = scale.getValue(anim, time);
	}
}

void TextureAnim::setup()
{
#ifdef Opengl_render

	glLoadIdentity();
	if (trans.used) {
		glTranslatef(tval.x, tval.y, tval.z);
	}
	if (rot.used) {
		glRotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
	}
	if (scale.used) {
		glScalef(sval.x, sval.y, sval.z);
	}
#endif
}

