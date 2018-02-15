#pragma once

namespace ParaEngine
{
	class ParticleSystem;


	/** holding a reference of this class */
	struct ParticleSystemRef
	{
		ParticleSystemRef():m_ptr(NULL) {};
		explicit ParticleSystemRef(ParticleSystem* ptr);
		ParticleSystemRef(const ParticleSystemRef& r);

		~ParticleSystemRef();
		ParticleSystemRef & operator=(const ParticleSystemRef& r); // never throws;

		inline ParticleSystem* get() const {return m_ptr;}
		operator bool () const{
			return m_ptr != 0;
		}
		bool operator! () const // never throws
		{
			return m_ptr == 0;
		}
		ParticleSystem& operator* () const // never throws
		{
			PE_ASSERT(m_ptr != 0);
			return *m_ptr;
		}
		ParticleSystem * operator-> () const // never throws
		{
			PE_ASSERT(m_ptr != 0);
			return m_ptr;
		}

		inline bool operator==(const ParticleSystemRef & b)
		{
			return this->get() == b.get();
		}

		inline bool operator!=(const ParticleSystemRef & b)
		{
			return this->get() != b.get();
		}

		inline bool operator==(ParticleSystem const * b)
		{
			return this->get() == b;
		}

		inline bool operator!=( ParticleSystem const * b)
		{
			return this->get() != b;
		}

	protected:
		ParticleSystem* m_ptr;
	};



}