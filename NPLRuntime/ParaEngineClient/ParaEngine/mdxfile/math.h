#pragma once

namespace ParaEngine
{

/////////////////////////////////
// MATH STUFF
typedef float vec;

class Vec2
{     
	friend class Vec3;

private:
    vec				v[2];

public:
	inline 			Vec2		( void )						{ set( 0, 0 );		 }
	inline 			Vec2		( vec x, vec y )				{ set( x, y);		 } 
        
	inline void		set			( vec x, vec y )				{ v[0] = x; v[1] = y; }
    inline void		set			( const vec *vP )				{ v[0] = vP[0]; v[1] = vP[1]; }
	
	inline vec&		operator[]	( int i )						{ return v[i]; }
    inline vec*		raw			( void )						{ return v;    }

	inline operator const vec*	( void ) const					{ return v; }
	
	inline Vec2&	operator+=	( const Vec2& v2 )				{ v[0] += v2.v[0]; v[1] += v2.v[1]; return *this; }
	inline Vec2&	operator-=	( const Vec2& v2 )				{ v[0] -= v2.v[0]; v[1] -= v2.v[1]; return *this; }
    inline Vec2		operator+	( const Vec2& v2 ) const		{ return Vec2( v[0] + v2.v[0], v[1] + v2.v[1] ); }
    inline Vec2		operator-	( const Vec2& v2 ) const		{ return Vec2( v[0] - v2.v[0], v[1] - v2.v[1] ); }
    inline Vec2		operator*	( vec s ) const					{ return Vec2( v[0] * s, v[1] * s ); }
	inline void		normalize	( void )						{ vec l = length(); v[0] /= l; v[1] /= l; }

	inline float	length		(void)
	{
		return (float)sqrt(v[0]*v[0]+v[1]*v[1]);
	}
};

class Vec3
{
	friend class Vec2;       
	friend class Mat4;

private:
	vec				v[3];

public:
					Vec3		( void )						{ set( 0, 0, 0);   }
					Vec3		( vec x, vec y, vec z )			{ set( x, y, z );  }
	inline void		set			( vec x, vec y, vec z )			{ v[0] = x; v[1] = y; v[2] = z; }
	inline void		set			( const vec *vP )				{ v[0] = vP[0]; v[1] = vP[1]; v[2] = vP[2]; }
    
    inline vec&		operator[]	( int i )						{ return v[i]; }
	inline vec*		raw			( void )						{ return v; }                

	inline operator const vec*	( void ) const					{ return v; }
    
    inline Vec3&	operator+=	( const Vec3& v3 )				{ v[0] += v3.v[0]; v[1] += v3.v[1]; v[2] += v3.v[2]; return *this; }
	inline Vec3&	operator-=	( const Vec3& v3 )				{ v[0] -= v3.v[0]; v[1] -= v3.v[1]; v[2] -= v3.v[2]; return *this; }
    inline Vec3		operator+	( const Vec3& v3 ) const		{ return Vec3( v[0] + v3.v[0] , v[1] + v3.v[1] , v[2] + v3.v[2] ); }
    inline Vec3		operator-	( const Vec3& v3 ) const		{ return Vec3( v[0] - v3.v[0] , v[1] - v3.v[1] , v[2] - v3.v[2] ); }
	inline Vec3		operator*	( vec s ) const					{ return Vec3( v[0] * s , v[1] * s , v[2] * s ); }
	inline float	operator*	( const Vec3& v3 ) const		{ return ( v[0] * v3[0] + v[1] * v3[1] + v[2] * v3[2] ); } 
	inline Vec3		operator~	( void ) const					{ return Vec3( -v[0] , -v[1], -v[2] ); }
    inline Vec3		operator^	( const Vec3& v3 ) const		{ return Vec3( v[1]*v3.v[2] - v3.v[1]*v[2], -v[0]*v3.v[2] + v3.v[0]*v[2], v[0]*v3.v[1] - v3.v[0]*v[1] ); }

	inline vec		length		( void ) const					{ return (vec)sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] ); }
    inline void		normalize	( void )						{ vec l = length(); v[0] /= l; v[1] /= l; v[2] /= l; }
};
}
// END OF MATH STUFF
/////////////////////////////////