#pragma once

namespace ParaEngine
{
enum CollisionFlag
{
	OPC_FIRST_CONTACT		= (1<<0),		//!< Report all contacts (false) or only first one (true)
	OPC_TEMPORAL_COHERENCE	= (1<<1),		//!< Use temporal coherence or not
	OPC_CONTACT				= (1<<2),		//!< Final contact status after a collision query
	OPC_TEMPORAL_HIT		= (1<<3),		//!< There has been an early exit due to temporal coherence
	OPC_NO_PRIMITIVE_TESTS	= (1<<4),		//!< Keep or discard primitive-bv tests in leaf nodes (volume-mesh queries)

	OPC_CONTACT_FOUND		= OPC_FIRST_CONTACT | OPC_CONTACT,
	OPC_TEMPORAL_CONTACT	= OPC_TEMPORAL_HIT | OPC_CONTACT,

	OPC_FORCE_DWORD			= 0x7fffffff
};

/** base class for all colliders. */
class Collider
{
public:
	// Constructor / Destructor
	Collider();
	virtual								~Collider();

	// Collision report

	/**
	*	Gets the last collision status after a collision query.
	*	\return		true if a collision occurred
	*/
	BOOL			GetContactStatus()			const	{ return mFlags & OPC_CONTACT;							}

	/**
	*	Gets the "first contact" mode.
	*	\return		true if "first contact" mode is on
	*/
	BOOL			FirstContactEnabled()		const	{ return mFlags & OPC_FIRST_CONTACT;					}

	/**
	*	Gets the temporal coherence mode.
	*	\return		true if temporal coherence is on
	*/
	BOOL			TemporalCoherenceEnabled()	const	{ return mFlags & OPC_TEMPORAL_COHERENCE;				}

	/**
	*	Checks a first contact has already been found.
	*	\return		true if a first contact has been found and we can stop a query
	*/
	BOOL			ContactFound()				const	{ return (mFlags&OPC_CONTACT_FOUND)==OPC_CONTACT_FOUND;	}

	/**
	*	Checks there's been an early exit due to temporal coherence;
	*	\return		true if a temporal hit has occured
	*/
	BOOL			TemporalHit()				const	{ return mFlags & OPC_TEMPORAL_HIT;						}

	/**
	*	Checks primitive tests are enabled;
	*	\return		true if primitive tests must be skipped
	*/
	BOOL			SkipPrimitiveTests()		const	{ return mFlags & OPC_NO_PRIMITIVE_TESTS;				}

	// Settings

	/**
	*	Reports all contacts (false) or first contact only (true)
	*	\param		flag		[in] true for first contact, false for all contacts
	*	\see		SetTemporalCoherence(bool flag)
	*	\see		ValidateSettings()
	*/
	void			SetFirstContact(bool flag)
	{
		if(flag)	mFlags |= OPC_FIRST_CONTACT;
		else		mFlags &= ~OPC_FIRST_CONTACT;
	}
	
	/**
	*	Enable/disable temporal coherence.
	*	\param		flag		[in] true to enable temporal coherence, false to discard it
	*	\see		SetFirstContact(bool flag)
	*	\see		ValidateSettings()
	*/
	void			SetTemporalCoherence(bool flag)
	{
		if(flag)	mFlags |= OPC_TEMPORAL_COHERENCE;
		else		mFlags &= ~OPC_TEMPORAL_COHERENCE;
	}

	/**
	*	Enable/disable primitive tests.
	*	\param		flag		[in] true to enable primitive tests, false to discard them
	*/
	void			SetPrimitiveTests(bool flag)
	{
		if(!flag)	mFlags |= OPC_NO_PRIMITIVE_TESTS;
		else		mFlags &= ~OPC_NO_PRIMITIVE_TESTS;
	}

protected:
	DWORD			mFlags;			//!< Bit flags
	
	/**
	*	Initializes a query
	*/
	virtual void InitQuery()		{ mFlags &= ~OPC_TEMPORAL_CONTACT;	}
};


} // namespace ParaEngine
