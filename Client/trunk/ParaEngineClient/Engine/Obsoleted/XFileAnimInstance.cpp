//-----------------------------------------------------------------------------
// Class:	CXFileAnimInstance
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.3.27
// Revised: 2004.3.27, 2004.5.5
// desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
  
#include "BaseObject.h"
#include "BipedObject.h"
#include "XFileaniminstance.h"
#include "memdebug.h"
using namespace ParaEngine;


//CSound *g_apSoundsTiny[ 2 ];

/* biped animation callback: all instances use a single callback handler */
CBipedCallbackHandler  g_BipedCallbackHandler;  // callback interface to handle callback keys

CXFileAnimInstance::CXFileAnimInstance():
	m_pMA( NULL ),
	m_dwMultiAnimIdx( 0 ),
    m_pAI( NULL ),
	///m_pSM( NULL ),

    m_dTimePrev( 0.0 ),
    m_dTimeCurrent( 0.0 ),
	m_bPlaySounds( false ),
    m_dwCurrentTrack( 0 )
	///m_pCallbackHandler( NULL )
{
	m_mxOrientation = Matrix4::IDENTITY;

	m_szASName[0] = '\0';

}

CXFileAnimInstance::~CXFileAnimInstance(void)
{
	Cleanup();
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::InObject
// Desc: 
//       [pSM] : should always be NULL, since we are handling sound external by the GUI
//       It saves device references, but do not initialize those devices.
//		 it only does intializition not concerning device object. Object dependent
//       initialization is done with InitDeviceObjects().
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::InitObject( CMultiAnim *pMA, double dTimeCurrent)
{
	m_pMA = pMA;
    
    m_dTimeCurrent = m_dTimePrev = dTimeCurrent;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::GetAnimationIndex
// Desc: Automatically get animation indexes of all animation in the x file.
//		 Since a biped has minimum animation set, therefore if the X file does
//		 not provide the predefined animation set name, it's defaults to play
//		 Loiter animation or the first animation. 
//		 The animation propagation rule is like proprogating from the root of 
//       a tree to its leaves. The root is always 
//       Loiter-->Walk-->Jog
//                    -->swim
//             -->Jump-->Jump forward/backward
//                    -->sidestep right/left
//             -->Speak-->say good bye
//                     -->say hello
//			   -->Action0-->Action1-->Action2~Action5
//						 -->Action6-->Action7~Action10
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::GetAnimationIndex()
{
	//-- level one
	m_dwAnimIdxLoiter = GetAnimIndex( "Loiter" );
    m_dwAnimIdxWalk = GetAnimIndex( "Walk" );
    m_dwAnimIdxSpeak = GetAnimIndex( "Jump" );
	m_dwAnimIdxAction[0] = GetAnimIndex( "Action0" );
    
	// TODO: check that there is at least one animation in the animation file
	//-- Root node: loiter
	if( m_dwAnimIdxLoiter == ANIMINDEX_FAIL)
		m_dwAnimIdxLoiter = 0;

	//-- Walk anim and child nodes
	if(m_dwAnimIdxWalk == ANIMINDEX_FAIL)
	{
		m_dwAnimIdxWalk = m_dwAnimIdxLoiter;
		m_dwAnimIdxJog = m_dwAnimIdxLoiter;
	}
	else
	{
		m_dwAnimIdxJog = GetAnimIndex( "Jog" );
		if(m_dwAnimIdxJog == ANIMINDEX_FAIL)
			m_dwAnimIdxJog = m_dwAnimIdxWalk;
		m_dwAnimIdxSwim = GetAnimIndex( "Swim" );
		if(m_dwAnimIdxSwim == ANIMINDEX_FAIL)
			m_dwAnimIdxSwim = m_dwAnimIdxWalk;
	}
	//-- action0 and child nodes
	if(m_dwAnimIdxAction[0] == ANIMINDEX_FAIL)
	{
		for(int i=0;i<ACTION_NO_MAX; i++)
			m_dwAnimIdxAction[i]=m_dwAnimIdxLoiter;
	}
	else
	{
		//-- default to action0
		// Action1-5
		m_dwAnimIdxAction[1] = GetAnimIndex( "Action1" );
		if(m_dwAnimIdxAction[1] == ANIMINDEX_FAIL)
		{
			for(int i=1;i<=5; i++)
				m_dwAnimIdxAction[i]=m_dwAnimIdxAction[0];
		}
		else
		{
			//-- default to action1
			TCHAR action[] = "Action1";
			for(int i=2;i<=5; i++)
			{
				action[6] = (TCHAR)(action[6]+1);
				m_dwAnimIdxAction[i] = GetAnimIndex( action );
				if(m_dwAnimIdxAction[i]==ANIMINDEX_FAIL)
					m_dwAnimIdxAction[i]=m_dwAnimIdxAction[1];
			}
		}

		//-- default to action0
		// Action6-10
		m_dwAnimIdxAction[6] = GetAnimIndex( "Action6" );
		if(m_dwAnimIdxAction[6] == ANIMINDEX_FAIL)
		{
			for(int i=6;i<=10; i++)
				m_dwAnimIdxAction[i]=m_dwAnimIdxAction[0];
		}
		else
		{
			//-- default to action6
			TCHAR action[] = "Action6";
			for(int i=7;i<=10; i++)
			{
				action[6] = (TCHAR)(action[6]+1);
				m_dwAnimIdxAction[i] = GetAnimIndex( action );
				if(m_dwAnimIdxAction[i]==ANIMINDEX_FAIL)
					m_dwAnimIdxAction[i]=m_dwAnimIdxAction[6];
			}
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::InitDeviceObjects
// Desc: Initializes the class and readies it for animation
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::InitDeviceObjects()
{
    HRESULT hr;
    hr = m_pMA->CreateNewInstance( & m_dwMultiAnimIdx );
    if( FAILED( hr ) )
        return E_OUTOFMEMORY;

    m_pAI = m_pMA->GetInstance( m_dwMultiAnimIdx );

    // set up anim indices
	GetAnimationIndex();

	// set up footstep callbacks
    SetupCallbacksAndCompression();
    // compute reorientation matrix based on default orientation and bounding radius
    Matrix4 mx;
	// Set the raduis of the object as it appears in the scene to 1/7.0f units
	float fScale = 1.f / m_pMA->GetBoundingRadius() / 7.f;
	ParaMatrixScaling( & mx, fScale, fScale, fScale );
    m_mxOrientation = mx;

    // the following code is required by DirectX model trandsformation.
	ParaMatrixRotationX( & mx, -MATH_PI / 2.0f );
    ParaMatrixMultiply( & m_mxOrientation, & m_mxOrientation, & mx );
    ParaMatrixRotationY( & mx, MATH_PI / 2.0f );
    ParaMatrixMultiply( & m_mxOrientation, & m_mxOrientation, & mx );
	
    // default: play idle animation
	strcpy(m_szASNameTarget,"Loiter");

    //ComputeFacingTarget();
	PlayAnimation(false);	//force loading the first animation

    LPD3DXANIMATIONCONTROLLER pAC;
    m_pAI->GetAnimController( & pAC );
    pAC->AdvanceTime( m_dTimeCurrent, NULL );
    pAC->Release();

    return S_OK;
}
//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::RestoreDeviceObjects()
// Desc: Reinitialize necessary objects
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::RestoreDeviceObjects()
{
    // Compress the animation sets in the new animation controller
    SetupCallbacksAndCompression();

    LPD3DXANIMATIONCONTROLLER pAC;
    m_pAI->GetAnimController( & pAC );
    pAC->ResetTime();
    pAC->AdvanceTime( m_dTimeCurrent, NULL );

    // Initialize current track
    if( m_szASName[0] != '\0' )
    {
        DWORD dwActiveSet = GetAnimIndex( m_szASName );
        LPD3DXANIMATIONSET pAS = NULL;
        pAC->GetAnimationSet( dwActiveSet, &pAS );
        pAC->SetTrackAnimationSet( m_dwCurrentTrack, pAS );
        SAFE_RELEASE( pAS );
    }

    pAC->SetTrackEnable( m_dwCurrentTrack, TRUE );
    pAC->SetTrackWeight( m_dwCurrentTrack, 1.0f );
    pAC->SetTrackSpeed( m_dwCurrentTrack, 1.0f );

    SAFE_RELEASE( pAC );

    // Call animate to initialize the tracks.
    Animate( 0.0 );
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::InvalidateDeviceObjects()
// Desc: Free D3D objects so that the device can be reset.
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::InvalidateDeviceObjects()
{
    // Save the current track's animation set name
    // so we can reset it again in RestoreDeviceObjects later.
    LPD3DXANIMATIONCONTROLLER pAC = NULL;
    m_pAI->GetAnimController( & pAC );
    if( pAC )
    {
        LPD3DXANIMATIONSET pAS = NULL;
        pAC->GetTrackAnimationSet( m_dwCurrentTrack, &pAS );
        if( pAS )
        {
            if( pAS->GetName() )
                strcpy( m_szASName, pAS->GetName() );
            SAFE_RELEASE( pAS );
        }
        SAFE_RELEASE( pAC );
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::Cleanup()
// Desc: Performs cleanup tasks for CBipedAnimInstance
//-----------------------------------------------------------------------------
void CXFileAnimInstance::Cleanup()
{
    // TODO: ...
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::GetAnimInstance()
// Desc: Returns the CAnimInstance object that this instance of CBipedAnimInstance
//       embeds.
//-----------------------------------------------------------------------------
CAnimInstance *CXFileAnimInstance::GetAnimInstance()
{
    return m_pAI;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::Animate()
// Desc: Advances the local time by dTimeDelta. Determine an action for Tiny,
//       then update the animation controller's tracks to reflect the action.
//-----------------------------------------------------------------------------
void CXFileAnimInstance::Animate( double dTimeDelta )
{
	// Continue animation or Load new animation 
	// if it's different from the last one
	PlayAnimation(true);

	// loop the loiter animation back on itself to avoid the end-to-end jerk
    SmoothLoiter();
    
	ParaMatrixMultiply( & m_mxWorld, & m_mxOrientation, & m_mxWorld );
    m_pAI->SetWorldTransform( & m_mxWorld );
}




//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::ResetTime()
// Desc: Resets the local time for this CBipedAnimInstance instance.
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::ResetTime()
{
    m_dTimeCurrent = m_dTimePrev = 0.0;
    return m_pAI->ResetTime();
}




//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::AdvanceTime()
// Desc: 
//       [pvEye] : should be NULL, since we are handling sound externally in the GUI
//       Advances the local animation time by dTimeDelta, and call
//       CAnimInstance to set up its frames to reflect the time advancement.
// 
//-----------------------------------------------------------------------------
void CXFileAnimInstance::AdvanceTime( double dTimeDelta)
{
    // if we're playing sounds, set the sound source position
    //if( m_bPlaySounds )
    //{
    //    m_CallbackData[ 0 ].m_pvCameraPos = pvEye;
    //    m_CallbackData[ 1 ].m_pvCameraPos = pvEye;
    //}
    //else    // else, set it to null to let the handler know to be quiet
    //{
    //    m_CallbackData[ 0 ].m_pvCameraPos = NULL;
    //    m_CallbackData[ 1 ].m_pvCameraPos = NULL;
    //}

    m_dTimePrev = m_dTimeCurrent;
    m_dTimeCurrent += dTimeDelta;
    //return m_pAI->AdvanceTime( dTimeDelta, m_pCallbackHandler );
	//return m_pAI->AdvanceTime( dTimeDelta, &g_BipedCallbackHandler );
	m_pAI->AdvanceTime( dTimeDelta, NULL);

}




//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::Draw()
// Desc: Renders this CBipedAnimInstance instace using the current animation frames.
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::Draw( SceneState * sceneState)
{
    return m_pAI->Draw();
}


//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::SetSounds()
// Desc: Enables or disables the sound support for this instance of CBipedAnimInstance.
//       In this case, whether we hear the footstep sound or not.
//-----------------------------------------------------------------------------
void CXFileAnimInstance::SetSounds( bool bSounds )
{
    m_bPlaySounds = bSounds;
}
//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::GetAnimIndex()
// Desc: Returns the index of an animation set within this animation instance's
//       animation controller given an animation set name.
//-----------------------------------------------------------------------------
DWORD CXFileAnimInstance::GetAnimIndex( char sString[] )
{
    HRESULT hr;
    LPD3DXANIMATIONCONTROLLER pAC;
    LPD3DXANIMATIONSET pAS;
    DWORD dwRet = ANIMINDEX_FAIL;

    m_pAI->GetAnimController( & pAC );

    for( DWORD i = 0; i < pAC->GetNumAnimationSets(); ++ i )
    {
        hr = pAC->GetAnimationSet( i, & pAS );
        if( FAILED( hr ) )
            continue;

        if( pAS->GetName() &&
            !strncmp( pAS->GetName(), sString, min( strlen( pAS->GetName() ), strlen( sString ) ) ) )
        {
            dwRet = i;
            pAS->Release();
            break;
        }

        pAS->Release();
    }

    pAC->Release();

    return dwRet;
}


//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::AddCallbackKeysAndCompress()
// Desc: Replaces an animation set in the animation controller with the
//       compressed version and callback keys added to it.
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::AddCallbackKeysAndCompress( LPD3DXANIMATIONCONTROLLER pAC,
                                           LPD3DXKEYFRAMEDANIMATIONSET pAS,
                                           DWORD dwNumCallbackKeys,
                                           D3DXKEY_CALLBACK aKeys[],
                                           DWORD dwCompressionFlags,
                                           FLOAT fCompression )
{
    HRESULT hr;
    LPD3DXCOMPRESSEDANIMATIONSET pASNew = NULL;
    LPD3DXBUFFER pBufCompressed = NULL;

    hr = pAS->Compress( dwCompressionFlags, fCompression, NULL, &pBufCompressed );
    if( FAILED( hr ) )
        goto e_Exit;

    hr = D3DXCreateCompressedAnimationSet( pAS->GetName(),
                                           pAS->GetSourceTicksPerSecond(),
                                           pAS->GetPlaybackType(),
                                           pBufCompressed,
                                           dwNumCallbackKeys,
                                           aKeys,
                                           &pASNew );
	pBufCompressed->Release();

    if( FAILED( hr ) )
        goto e_Exit;

    pAC->UnregisterAnimationSet( pAS );
    pAS->Release();

    hr = pAC->RegisterAnimationSet( pASNew );
    if( FAILED( hr ) )
        goto e_Exit;

    pASNew->Release();
    pASNew = NULL;


e_Exit:
    
    if( pASNew )
        pASNew->Release();

    return hr;
}


//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::SetupCallbacksAndCompression()
// Desc: 2004-5-5 LiXizhi
//       Only compress, I have removed any callbacks. The old code is commented, which
//       Adds callback keys to the walking and jogging animation sets in the
//       animation controller for playing footstepping sound;  Then compress
//       all animation sets in the animation controller.
// you can change the input values of AddCallbackKeysAndCompress(..., .8f) to your desired ones.
// such as Compression Lossiness, which is Desired compression loss ratio, in the range from 0 to 1.
// currenly this value is set to 0.8f.
//-----------------------------------------------------------------------------
HRESULT CXFileAnimInstance::SetupCallbacksAndCompression()
{
    LPD3DXANIMATIONCONTROLLER pAC;
    LPD3DXKEYFRAMEDANIMATIONSET pASLoiter, pASWalk, pASJog;

    m_pAI->GetAnimController( & pAC );
    
	// -- compress Loiter animation
	pAC->GetAnimationSet( m_dwAnimIdxLoiter, (LPD3DXANIMATIONSET *) & pASLoiter );
	AddCallbackKeysAndCompress( pAC, pASLoiter, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );
    
	// -- compress <removed:and set callback> for Walk animation
	if(m_dwAnimIdxLoiter != m_dwAnimIdxWalk)
	{
		pAC->GetAnimationSet( m_dwAnimIdxWalk, (LPD3DXANIMATIONSET *) & pASWalk );
		AddCallbackKeysAndCompress( pAC, pASWalk, 0, NULL, D3DXCOMPRESS_DEFAULT, .4f );

		/*D3DXKEY_CALLBACK aKeysWalk[ 2 ];
		aKeysWalk[ 0 ].Time = 0;
		aKeysWalk[ 0 ].pCallbackData = & m_CallbackData[ 0 ];
		aKeysWalk[ 1 ].Time = float( pASWalk->GetPeriod() / 2.0 * pASWalk->GetSourceTicksPerSecond() );
		aKeysWalk[ 1 ].pCallbackData = & m_CallbackData[ 1 ];

		AddCallbackKeysAndCompress( pAC, pASWalk, 2, aKeysWalk, D3DXCOMPRESS_DEFAULT, .4f );*/
	
		// -- compress <removed: and set callback> for Jog animation
		if(m_dwAnimIdxWalk != m_dwAnimIdxJog)
		{
			pAC->GetAnimationSet( m_dwAnimIdxJog, (LPD3DXANIMATIONSET *) & pASJog );
		    AddCallbackKeysAndCompress( pAC, pASJog, 0, NULL, D3DXCOMPRESS_DEFAULT, .25f );

			/*D3DXKEY_CALLBACK aKeysJog[ 8 ];
			for( int i = 0; i < 8; ++ i )
			{
				aKeysJog[ i ].Time = float( pASJog->GetPeriod() / 8 * (double) i * pASWalk->GetSourceTicksPerSecond() );
				aKeysJog[ i ].pCallbackData = & m_CallbackData[ ( i + 1 ) % 2 ];
			}

			AddCallbackKeysAndCompress( pAC, pASJog, 8, aKeysJog, D3DXCOMPRESS_DEFAULT, .25f );*/
		}
	}

	// -- compress and set callback for Action# animation
	if(m_dwAnimIdxLoiter != m_dwAnimIdxAction[0])
	{
		LPD3DXKEYFRAMEDANIMATIONSET pASAction;
		pAC->GetAnimationSet( m_dwAnimIdxAction[0], (LPD3DXANIMATIONSET *) & (pASAction) );
		AddCallbackKeysAndCompress( pAC, pASAction, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );

		// -- compress and set callback for Action1 animation
		if(m_dwAnimIdxAction[0] != m_dwAnimIdxAction[1])
		{
			pAC->GetAnimationSet( m_dwAnimIdxAction[1], (LPD3DXANIMATIONSET *) & (pASAction) );
			AddCallbackKeysAndCompress( pAC, pASAction, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );
			// -- compress and set callback for Action2-5 animation
			for(int i=2;i<=5;i++)
			{
				if(m_dwAnimIdxAction[1] != m_dwAnimIdxAction[i])
				{
					pAC->GetAnimationSet( m_dwAnimIdxAction[i], (LPD3DXANIMATIONSET *) & (pASAction) );
					AddCallbackKeysAndCompress( pAC, pASAction, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );
				}
			}
		}
		// -- compress and set callback for Action6 animation
		if(m_dwAnimIdxAction[0] != m_dwAnimIdxAction[6])
		{
			pAC->GetAnimationSet( m_dwAnimIdxAction[6], (LPD3DXANIMATIONSET *) & (pASAction) );
			AddCallbackKeysAndCompress( pAC, pASAction, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );
			// -- compress and set callback for Action7-10 animation
			for(int i=7;i<=10;i++)
			{
				if(m_dwAnimIdxAction[6] != m_dwAnimIdxAction[i])
				{
					pAC->GetAnimationSet( m_dwAnimIdxAction[i], (LPD3DXANIMATIONSET *) & (pASAction) );
					AddCallbackKeysAndCompress( pAC, pASAction, 0, NULL, D3DXCOMPRESS_DEFAULT, .8f );
				}
			}
		}
	}

    GetAnimationIndex();

    pAC->Release();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::PlayAnimation()
// Desc: Initialize a new track in the animation controller for the movement
//       animation (run or walk), and set up the smooth transition from the idle
//       animation (current track) to it (new track).
// Params:  bContinue: If it's true,load new animation track only if sAnimName is 
//			different from the track that is being played
// note: m_szASNameTarget can be numbers, which is translated as index into the 
//		 model file's animation sets.
//-----------------------------------------------------------------------------
void CXFileAnimInstance::PlayAnimation(bool bContinue)
{
	// -- return if no new animation is specified
	if(bContinue && strcmp(m_szASNameTarget, m_szASName) == 0)
		return;
	strcpy(m_szASName, m_szASNameTarget);
	
	// -- create new track
	DWORD dwNewTrack = ( m_dwCurrentTrack == 0 ? 1 : 0 );
    LPD3DXANIMATIONCONTROLLER pAC;
    LPD3DXANIMATIONSET pAS;
    m_pAI->GetAnimController( & pAC );


	// TODO: use hash tablle to get the animation set
	// -- Get the animation set
	HRESULT hr;
    double dTransitionPeriod = MOVE_TRANSITION_TIME;
	if(strcmp(m_szASName, "Walk") == 0)
	{
		hr = pAC->GetAnimationSet( m_dwAnimIdxWalk, & pAS );
		dTransitionPeriod = MOVE_TRANSITION_TIME;
	}
	else if(strcmp(m_szASName, "Jog") == 0)
	{
        hr = pAC->GetAnimationSet( m_dwAnimIdxJog, & pAS );
		dTransitionPeriod = MOVE_TRANSITION_TIME;
	}
	else if( ('0'<= m_szASName[0]) && (m_szASName[0]<='9') ) 
	{// if it's a number from 0~99
		UINT nIndex = 0;
		if(('0'<= m_szASName[1]) && (m_szASName[1]<='9'))
			nIndex = (m_szASName[0] - '0')*10+m_szASName[1]-'0';
		else
			nIndex = (m_szASName[0] - '0');

        // use the name as the index of the animation set.
		hr = pAC->GetAnimationSet( nIndex, & pAS );
		dTransitionPeriod = MOVE_TRANSITION_TIME;
	}
	else //if(strcmp(m_szASName, "Loiter"))
	{
		hr = pAC->GetAnimationSet( m_dwAnimIdxLoiter, & pAS );
		dTransitionPeriod = IDLE_TRANSITION_TIME;
	}

	if( ! SUCCEEDED(hr) ) // failed to load
	{ 
		// TODO: Load default animation
		hr = pAC->GetAnimationSet( m_dwAnimIdxLoiter, & pAS );
		dTransitionPeriod = IDLE_TRANSITION_TIME;
		if( ! SUCCEEDED(hr) )// failed to load the default
			return;
	}

	// -- Enable new track and set transition weight
    pAC->SetTrackAnimationSet( dwNewTrack, pAS );
    pAS->Release();

    pAC->UnkeyAllTrackEvents( m_dwCurrentTrack );
    pAC->UnkeyAllTrackEvents( dwNewTrack );

    pAC->KeyTrackEnable( m_dwCurrentTrack, FALSE, m_dTimeCurrent + dTransitionPeriod );
    pAC->KeyTrackSpeed( m_dwCurrentTrack, 0.0f, m_dTimeCurrent, dTransitionPeriod, D3DXTRANSITION_LINEAR );
    pAC->KeyTrackWeight( m_dwCurrentTrack, 0.0f, m_dTimeCurrent, dTransitionPeriod, D3DXTRANSITION_LINEAR );
    pAC->SetTrackEnable( dwNewTrack, TRUE );
    pAC->KeyTrackSpeed( dwNewTrack, 1.0f, m_dTimeCurrent, dTransitionPeriod, D3DXTRANSITION_LINEAR );
    pAC->KeyTrackWeight( dwNewTrack, 1.0f, m_dTimeCurrent, dTransitionPeriod, D3DXTRANSITION_LINEAR );

	if(!bContinue) // restart
		pAC->SetTrackPosition( dwNewTrack, 0.0 );

    m_dwCurrentTrack = dwNewTrack;

    pAC->Release();
}

//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::SmoothLoiter()
// Desc: If Biped is loitering, check if we have reached the end of animation.
//       If so, set up a new track to play Loiter animation from the start and
//       smoothly transition to the track, so that Biped can loiter more.
//-----------------------------------------------------------------------------
void CXFileAnimInstance::SmoothLoiter()
{
    LPD3DXANIMATIONCONTROLLER pAC;
    LPD3DXANIMATIONSET pASTrack, pASLoiter;
    m_pAI->GetAnimController( & pAC );

    // check if we're loitering
    pAC->GetTrackAnimationSet( m_dwCurrentTrack, & pASTrack );
    pAC->GetAnimationSet( m_dwAnimIdxLoiter, & pASLoiter );
    if( pASTrack && pASTrack == pASLoiter )
    {
        D3DXTRACK_DESC td;
        pAC->GetTrackDesc( m_dwCurrentTrack, & td );
        if( td.Position > pASTrack->GetPeriod() - IDLE_TRANSITION_TIME )  // come within the change delta of the end
		{
			// play loiter animation again (from the beginning)
            strcpy(m_szASNameTarget, "Loiter");
			PlayAnimation(false);
		}
    }

    SAFE_RELEASE( pASTrack );
    SAFE_RELEASE( pASLoiter );
    SAFE_RELEASE( pAC );
}


//-----------------------------------------------------------------------------
// Name: CBipedAnimInstance::LoadBipedState()
// Desc: It contains all the information that the animation instance needs to know
// in order to render the current frame in the world cordinate. Information includes:
// (1) the animation track (name)that the biped think it should play
// (2) The location(position and facing provided as mxWorld matrix) where the 
//     biped think it should render the animation. 
//-----------------------------------------------------------------------------
void CXFileAnimInstance::LoadBipedState(const char* sTargetAnim, const Matrix4* mxWorld)
{
	strcpy(m_szASNameTarget, sTargetAnim);
	m_mxWorld = *mxWorld;
}