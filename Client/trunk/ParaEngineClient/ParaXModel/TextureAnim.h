#pragma once

namespace ParaEngine
{
	class TextureAnim 
	{
	public:
		Animated<Vector3> trans, rot, scale;
		Vector3 tval, rval, sval;

		void calc(int anim, int time);
		void init(CParaFile &f, ModelTexAnimDef &mta, int *global);
		void setup();
	};
}

