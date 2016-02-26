#pragma once
#include "AttributeField.h"
#include "IAnimated.h"

namespace ParaEngine
{
	/** for a dynamic attribute field. dynamic attribute can be created at runtime,which is different 
	* from CAttributeField which is compile-time attribute. 
	* Dynamic attribute can also be animated or key framed. see also: Animated
	*/
	class CDynamicAttributeField : public CRefCounted, public CVariable
	{
	public:
		CDynamicAttributeField(const std::string& name, DWORD dwType = FieldType_unknown);
		CDynamicAttributeField();
		virtual ~CDynamicAttributeField();

		const std::string& GetName() const { return m_sName; }

		void operator = (const CVariable& val);
	public:
		std::string m_sName;
	};

	typedef ref_ptr<CDynamicAttributeField> DynamicFieldPtr;
}