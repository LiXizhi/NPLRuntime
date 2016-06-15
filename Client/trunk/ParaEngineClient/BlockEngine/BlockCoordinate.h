
#pragma once

#include <stdint.h>
#include <math.h>

namespace ParaEngine
{
	/** basic block world coordinate */
	struct Int16x3
	{
		int16_t x;
		int16_t y;
		int16_t z;

		Int16x3():x(0),y(0),z(0){}

		Int16x3(int16_t v):x(v),y(v),z(v){}

		Int16x3(int16_t ix,int16_t iy,int16_t iz)
			:x(ix),y(iy),z(iz)
		{
		}

		Int16x3(const Int16x3& v)
			:x(v.x),y(v.y),z(v.z)
		{
		}

		inline bool operator == (const Int16x3& v) const{
			return x == v.x && y == v.y && z == v.z;
		}

		inline bool operator != (const Int16x3& v) const{
			return x != v.x || y != v.y || z != v.z;
		}

		void SetValue(int16_t v);

		void Add(Int16x3& v);

		void Add(int16_t value);

		static void Add(const Int16x3& v0,const Int16x3& v1,Int16x3& oResult);

		void Add(const Int16x3& v0,int16_t v1,Int16x3& oResult);

		void Subtract(const Int16x3& value);

		void Subtract(int16_t value);

		static void Subtract(const Int16x3& v0,const Int16x3& v1,Int16x3& oResult);

		static void Subtract(const Int16x3& v0,int16_t v1,Int16x3& oResult);

		void Divide(Int16x3& value);

		void Divide(int16_t value);

		static void DivideTo(const Int16x3& v0,const Int16x3 v1,Int16x3& oResult);

		static void DivideTo(const Int16x3& v0,int16_t v1,Int16x3& oResult);

		void Mod(int value);

		static void Mod(const Int16x3& v0,int v1,Int16x3& oResult);

		void Abs();
	};

	/** basic block world coordinate */
	struct Uint16x3
	{
		uint16_t x;
		uint16_t y;
		uint16_t z;

		Uint16x3():x(0),y(0),z(0){}

		Uint16x3(uint16_t v):x(v),y(v),z(v){}

		Uint16x3(uint16_t ix,uint16_t iy,uint16_t iz)
			:x(ix),y(iy),z(iz)
		{
		}

		Uint16x3(const Uint16x3& v)
			:x(v.x),y(v.y),z(v.z)
		{
		}

		inline bool operator == (const Uint16x3& v) const{
			return x == v.x && y == v.y && z == v.z;
		}

		inline bool operator != (const Uint16x3& v) const{
			return x != v.x || y != v.y || z != v.z;
		}

		inline bool operator == (const Int16x3& v) const{
			return x == v.x && y == v.y && z == v.z;
		}

		inline bool operator != (const Int16x3& v) const{
			return x != v.x || y != v.y || z != v.z;
		}

		Uint16x3 operator +(const Int16x3& v) const{
			return Uint16x3((uint16)(x + v.x), (uint16)(y + v.y), (uint16)(z + v.z));
		}
		Uint16x3 operator +(const Uint16x3& v) const{
			return Uint16x3((x + v.x), (y + v.y), (z + v.z));
		}
		/* get the abs distance to another coordinates. abs(dx)+abs(dy)+abs(dz) */
		uint32 AbsDistanceTo(const Uint16x3& v) const;

		/** get simple hash code*/
		DWORD GetHashCode() const { return ((x << 8) | y | (z << 20)); }
		
	};

	/** for block or chunk location that does not has boundary check.*/
	struct Int32x3
	{
		int32 x;
		int32 y;
		int32 z;

		Int32x3() :x(0), y(0), z(0){}

		Int32x3(int32 v) :x(v), y(v), z(v){}

		Int32x3(int32 ix, int32 iy, int32 iz)
			:x(ix), y(iy), z(iz)
		{
		}

		Int32x3(const Int32x3& v)
			:x(v.x), y(v.y), z(v.z)
		{
		}
	};

	/** chunk column location. */
	struct ChunkLocation
	{
	public:
		ChunkLocation(uint16_t chunkX, uint16_t chunkZ);
		ChunkLocation(int chunkPackedPos);
		uint32_t GetPackedChunkPos() const;

		static uint32_t FromChunkToPackedChunk(int chunkX, int chunkZ);

		uint32_t GetCenterWorldX()  const;

		uint32_t GetCenterWorldZ()  const;

		bool operator == (const ChunkLocation& r) const;

		struct ChunkLocationHasher
		{
			uint32_t operator()(const ChunkLocation& k) const
			{
				return k.GetPackedChunkPos();
			}
		};

		int DistanceToSquared(const ChunkLocation&  coords) const;
	public:
		uint16_t m_chunkX;
		uint16_t m_chunkZ;
	};
	
	
		


}