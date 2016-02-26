//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.2.13
// Desc: based on QT framework. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaSize.h"
using namespace ParaEngine;


void QSize::transpose()
{
	int tmp = wd;
	wd = ht;
	ht = tmp;
}

QSize QSize::scaled(const QSize &s) const
{
	if (wd == 0 || ht == 0) {
		return s;
	}
	else {
		bool useHeight;
		int64 rw = int64(s.ht) * int64(wd) / int64(ht);
		useHeight = (rw >= s.wd);
		if (useHeight) {
			return QSize((int32)rw, (int32)s.ht);
		}
		else {
			return QSize(s.wd,
				int32(int64(s.wd) * int64(ht) / int64(wd)));
		}
	}
}

void QSizeF::transpose()
{
	float tmp = wd;
	wd = ht;
	ht = tmp;
}

QSizeF QSizeF::scaled(const QSizeF &s) const
{
	if (Math::IsNull(wd) || Math::IsNull(ht)) {
		return s;
	}
	else {
		bool useHeight;
		float rw = s.ht * wd / ht;

		useHeight = (rw >= s.wd);
		
		if (useHeight) {
			return QSizeF(rw, s.ht);
		}
		else {
			return QSizeF(s.wd, s.wd * ht / wd);
		}
	}
}