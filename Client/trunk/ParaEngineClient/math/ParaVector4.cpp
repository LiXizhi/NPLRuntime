/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/

#include "ParaEngine.h"
#include "ParaVector4.h"
#include "ParaMath.h"

namespace ParaEngine
{
    const Vector4 Vector4::ZERO( 0, 0, 0, 0 );
}
