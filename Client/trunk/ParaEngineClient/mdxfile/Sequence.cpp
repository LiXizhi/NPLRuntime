//----------------------------------------------------------------------
// Class:	CSequence
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer

#include "dxStdAfx.h"
#include "sequence.h"

using namespace ParaEngine;


CSequence::CSequence(void):numSequences(0),sequences(NULL)
{
}

CSequence::~CSequence(void)
{
	if(sequences)
		delete []sequences;
}

void CSequence::Read(TypePointer inP,int inSize)
{
	numSequences = inSize / sizeof(SequenceAnim);
	sequences = new SequenceAnim[numSequences];
	memcpy(sequences, inP.c, inSize);
}

SequenceAnim* CSequence::GetSequence(int n)
{
	if(n>=0 && n<numSequences)
		return &sequences[n];
	else
		return NULL;
}