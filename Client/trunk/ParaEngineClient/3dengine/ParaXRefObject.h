#pragma once

namespace ParaEngine
{
	/** XRef Objects in X and ParaX model.*/
	struct ParaXRefObject
	{
	public:
		/// source object's file name relative path
		std::string m_filename;

		enum XREF_TYPE{
			XREF_ANIMATED = 0,
			XREF_STATIC,
			XREF_SCRIPT,
		};

		struct ParaXRefObject_Data
		{
			/// type see XREF_TYPE:  0 means the source object is a standard ParaX model.
			DWORD type;
			/// local transformation of the reference object in the parent model's local space
			Matrix4 localTransform;
			/// origin of the reference object in the parent model's local space, we can make this zero, and put into localTransform
			/// however, in some cases, we need the localTransform to be rotation and scaling matrix, and the origin to be the translation.
			Vector3 origin;
			/// default animation ID, typically 0
			DWORD animID;
			/// color of the model, currently not used. 
			DWORD color;
			/// followings are some reserved field for future use
			DWORD reserved0;
			FLOAT reserved1;
			FLOAT reserved2;
			FLOAT reserved3;
		};
		ParaXRefObject_Data m_data;

	public:
		ParaXRefObject(){ memset(&m_data, 0, sizeof(ParaXRefObject_Data)); };
	};
}

