#pragma once
#include "AnimInstanceBase.h"
#include "XFileMultiAnim.h"

namespace ParaEngine
{
class CAudioEngine;
#define IDLE_TRANSITION_TIME 0.125f
#define MOVE_TRANSITION_TIME 0.25f

#define ANIMINDEX_FAIL      0xffffffff


//-----------------------------------------------------------------------------
// Name: class CBHandlerTiny
// Desc: Derived from ID3DXAnimationCallbackHandler.  Callback handler for
//       CBiped -- plays the footstep sounds
//-----------------------------------------------------------------------------
class CBipedCallbackHandler : public ID3DXAnimationCallbackHandler
{
    HRESULT CALLBACK HandleCallback( THIS_ UINT Track, LPVOID pCallbackData )
    {
        return S_OK;
    }
};

#define				ACTION_NO_MAX		11

/* biped animation callback: all instances use a single callback handler */
extern CBipedCallbackHandler   g_BipedCallbackHandler;  // callback interface to handle callback keys

struct SceneState;
/**
* Biped Animation controller. this is a managed resource by CParaWorldAsset class
* please see MultiAnimationEntity assets type for more information. Each biped has 
* a pointer to MultiAnimationEntity instance. each MultiAnimationEntity instance 
* has many animation instances(CBipedAnimInstance). 
* The CBipedObject class call LoadBipedState() at its frame move(Animate()) function
* to tell this animation instance which animation it should be playing and where to 
* render the animation in the world coordinates. Then call Animate().
* call the AdvanceTime and Draw method to render this animation. 
* Please see the biped class for examples of using this type of asset.
* 
* Note 1: Note that assets are all managed automatically by the assets manager, the scene 
* objects does not need to take care of memory management of this assets. Each time
* the biped class wants to use it, it queries it from the MultiAnimationEntity, which
* in turn is another type of assets in paraworld engine.
* 
* Note 2: Biped has a set of predefined actions, but any single x file may not contain
*       all of them. So there is a tree like degradation structure. That is actions are 
*       reorganized in a tree, with each action assigned to a tree node. Upon initializing 
*       those actions, we will start from the top of the tree. If a node(action) does not 
*       find it's exact action in the x file, then its parent's action index is used. The
*       process is recursive. So that all actions will be assigned an action index in the 
*       index file. The worst case will be all nodes has index the same as the root node 
*       which is the loiter(idle) action. 
* The following is the predefined action.
*       Loiter-->Walk-->jog
*                    -->swim
*             -->jump-->jump forward/backward
*                    -->sidestep right/left
*             -->Speak-->say good bye
*                     -->say hello
*			   -->action0-->action1-->action2~action5
*						 -->action6-->action7~action10
* There is a constriant: if the parent is default, then all its descendants are set to default
* regardless there is an exact animation in the file or not. 
*
* Note 2: Biped callback is generally used to play footstep sound. PLease refer to 
*          CBipedHandlerTiny, CallbackDataTiny
*          SetupCallbacksAndCompression() only walk,jog has callbacks, other actions not
*          AddCallbackKeysAndCompress(...) every animation needs compression.
* Note 2: I have removed any callbacks during animation. I have commented old code which add sounds through callback.
*         Currently, sound is managed externally by the GUI engine.
*/
class CXFileAnimInstance : public CAnimInstanceBase
{
protected:
	// -- data structuring
	CMultiAnim *         m_pMA;               // pointer to mesh-type-specific object
    DWORD                m_dwMultiAnimIdx;    // index identifying which CAnimInstance this object uses
    CAnimInstance *      m_pAI;               // pointer to CAnimInstance specific to this object
	
	///CSoundManager *      m_pSM;               // pointer to sound management interface
    DWORD                m_dwAnimIdxLoiter,   // Indexes of various animation sets
                         m_dwAnimIdxWalk,m_dwAnimIdxJog,m_dwAnimIdxSwim,
						 m_dwAnimIdxSpeak,
						 m_dwAnimIdxAction[ACTION_NO_MAX];
	Matrix4           m_mxOrientation;     // transform that gets the mesh into a common world space

                         
						 
    ///CallbackDataTiny     m_CallbackData[ 2 ]; // Data to pass to callback handler

    // operational status
    double               m_dTimePrev;         // global time value before last update
    double               m_dTimeCurrent;      // current global time value
    bool                 m_bPlaySounds;       // true == this instance is playing sounds
    DWORD                m_dwCurrentTrack;    // current animation track for primary animation
	Matrix4			 m_mxWorld;			  // world transform of the animation.

	// animation callback
	///ID3DXAnimationCallbackHandler *
    ///                     m_pCallbackHandler;  // pointer to callback inteface to handle callback keys

	char                 m_szASName[22];      // Current track's animation set name (for preserving across device reset)
	char                 m_szASNameTarget[22];// Target track's animation set name (for preserving across device reset)

protected:      
	// --The following are not callable by the app -- internal stuff
	void PlayAnimation( bool bContinue);
	void SmoothLoiter();
	
	HRESULT GetAnimationIndex();
    DWORD GetAnimIndex( char sString[] );
    HRESULT AddCallbackKeysAndCompress( LPD3DXANIMATIONCONTROLLER pAC, LPD3DXKEYFRAMEDANIMATIONSET pAS, DWORD dwNumCallbackKeys, D3DXKEY_CALLBACK aKeys[], DWORD dwCompressionFlags, FLOAT fCompression );
    HRESULT SetupCallbacksAndCompression();
    void SetSounds( bool bSounds );

	virtual HRESULT ResetTime();
   
public:
	HRESULT InitObject( CMultiAnim *pMA,  double dTimeCurrent);
	HRESULT InitDeviceObjects();
    HRESULT InvalidateDeviceObjects();
	HRESULT RestoreDeviceObjects();
    void	Cleanup();

	//-- rendering
	void	LoadBipedState(const char* sTargetAnim, const Matrix4* mxWorld);
	virtual void Animate( double dTimeDelta );
	virtual void AdvanceTime( double dTimeDelta);
	HRESULT Draw( SceneState * sceneState);
	virtual CAnimInstance *GetAnimInstance();

	//-- querying
	//CBipedObject*	GetAssociatedBipedObject(){return m_pBipedObject;};
	//void	SetAssociatedBipedObject(CBipedObject* pObj){m_pBipedObject = pObj;};

public:
	CXFileAnimInstance();
	~CXFileAnimInstance(void);
};
}