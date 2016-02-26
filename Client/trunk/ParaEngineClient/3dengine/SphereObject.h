#pragma once
#include "BaseObject.h"

namespace ParaEngine
{
	/**
	* a CBaseObject whose position is at the center, instead of at the bottom. 
	* This class is an alternative to CBaseObject. 
	* All this class does is overriding some virtual functions of the CBaseObject class. 
	* and use center point instead of position to store the object location. 
	*/
	class CSphereObject : public CBaseObject
	{
	public:
		CSphereObject(void);
		virtual ~CSphereObject(void);

	public:

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CSphereObject;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CSphereObject"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		//////////////////////////////////////////////////////////////////////////
		//
		// Note: I have secretly changed position to center. the position attribute of the 
		// the sphere object is not consistent with the position methods
		//
		//////////////////////////////////////////////////////////////////////////
		ATTRIBUTE_METHOD1(CSphereObject, GetPosition_s, DVector3*)	{ *p1 = cls->GetObjectCenter(); return S_OK;}
		ATTRIBUTE_METHOD1(CSphereObject, SetPosition_s, DVector3)	{cls->SetObjectCenter(p1); return S_OK;}

	public:
		/** get the object shape */
		virtual ObjectShape GetObjectShape();

		/**
		* get the center of the object in world space
		*/
		virtual DVector3 GetObjectCenter();

		/**
		* set the center of the object in world space
		*/
		virtual void SetObjectCenter(const Vector3 & v);

		/**
		* Set the object shape to Sphere and sphere parameters
		* @param fRadius: radius
		*/
		virtual void SetRadius(float fRadius);

		/**
		* @see SetRadius(float fRadius);
		*/
		virtual float GetRadius();

		/** 
		* get object position in the world space. The position of an object is the bottom center of the object. 
		* most scene object will return the position using the following rules
		*  project the center of the shape to the bottom plane of the shape, the
		*  projection point at the bottom is returned as the object's world position.
		* so it is different from the center of the object. 
		* @see GetObjectCenter(Vector3 *pV)
		*/
		virtual DVector3 GetPosition();
		/** set the object position. */
		virtual void SetPosition(const DVector3& v);

	protected:
		/** center of the object. */
		DVector3 m_vCenter;
		float m_fRadius;
	};

}
