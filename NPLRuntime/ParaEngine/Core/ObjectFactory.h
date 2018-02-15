#pragma once

namespace ParaEngine
{
	class IAttributeFields;

	/** the virtual factory class for dynamically registered object class instantiation. */
	class CObjectFactory : public IObject
	{
	public:
		virtual ~CObjectFactory(){}
		virtual IAttributeFields* Create() = 0;
	};

	/** default factory for creating object 
	* Class T should add following macro to its header in order to use this factory class
	* ATTRIBUTE_SUPPORT_CREATE_FACTORY(CWeatherEffect);
	* or T should have Create() method manually defined.
	*/
	template<typename T>
	class CDefaultObjectFactory : public CObjectFactory
	{
	public:
		typedef T classType;
		virtual ~CDefaultObjectFactory(){}

		/** see class reference if classType::Create is not defined. */
		virtual IAttributeFields* Create()
		{
			return classType::Create();
		}
	};
}


