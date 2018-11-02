
#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

#include "utils.h"
namespace ParaEngine
{
struct SequenceAnim
{
	char	name[80];
	int		startFrame;
	int		endFrame;
	float	moveSpeed;
	int		nonLooping;
	float	rarity;
	int		unk6;
	float	boundsRadius;

	D3DXVECTOR3	mins;
	D3DXVECTOR3	maxs;
};

/**
A collection of animation sequences. They are used to compose interactive character animations in the game engine.
*/
class CSequence
{
public:
	CSequence(void);
	~CSequence(void);

	void			Read(TypePointer inP,int inSize);
	int				GetSequencesNum(){return numSequences;};
	SequenceAnim*	GetSequence(int n);
public:
	int				numSequences;
	SequenceAnim *	sequences;
};
}
#endif//__SEQUENCE_H__