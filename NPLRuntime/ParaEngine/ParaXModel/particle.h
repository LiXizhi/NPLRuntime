#pragma once

#include "animated.h"
#include "util/ParaMemPool.h"
#include "FileManager.h"
#include <list>
#include <map>


namespace ParaEngine
{
class ParticleSystem;
class RibbonEmitter;
class Bone;
class CParaXModel;
struct TextureEntity;

struct Particle {
	Vector3 pos, speed, down, origin;
	//Vector3 tpos;
	float size, life, maxlife, random_force, trail_length;
	float rotation, rotSpeed;
	int tile;
	Vector4 color;
};

typedef FixedSizedAllocator<Particle>	DL_Allocator_ParticleList;
typedef std::list<Particle, DL_Allocator_ParticleList >	ParticleListType;
/**
it represents the instance of the particle system.
*/
struct  ParticleList
{
	ParticleListType particles;
	/** whether to use absolute world coordinate system, so that when particles leave its source, it will be 
	* an independent object in the world coordinate system.*/
	bool m_bUseAbsCord:1;
	/** the last render origin when the update() function is called.*/
	Vector3 m_vLastRenderOrigin;
	/** whether this object is updated in the current frame.*/
	bool m_bUpdated:1;
	/** whether this object should be rendered (in camera frustum) */
	bool m_bRender:1;
	/** current animation */
	int m_anim;
	/** current frame */
	int m_time;
	/** remaining time to spawn new particles*/
	float m_rem;
	/** to which scene state this particle instance is associated. */
	void * m_pSceneState;
public:
	ParticleList():m_pSceneState(NULL),m_bUseAbsCord(true), m_vLastRenderOrigin(0,0,0),m_bUpdated(false),m_anim(0), m_time(0),m_rem(0){};
};

class ParticleEmitter {
protected:
	ParticleSystem *sys;
public:
	enum{
		TYPE_NONE=0,
		TYPE_PLANE_PARTICLE_EMITTER=1,
		TYPE_SPHERE_PARTICLE_EMITTER=2,
	};
	ParticleEmitter(ParticleSystem *sys): sys(sys) {}
	virtual ~ParticleEmitter() {};
	virtual Particle newParticle(int anim, int time) = 0;
	virtual int GetEmitterType()=0;
};

class PlaneParticleEmitter: public ParticleEmitter {
public:
	PlaneParticleEmitter(ParticleSystem *sys): ParticleEmitter(sys) {}
	virtual ~PlaneParticleEmitter() {};
	Particle newParticle(int anim, int time);
	virtual int GetEmitterType(){return (int)TYPE_PLANE_PARTICLE_EMITTER;};
};

class SphereParticleEmitter: public ParticleEmitter {
public:
	SphereParticleEmitter(ParticleSystem *sys): ParticleEmitter(sys) {}
	virtual ~SphereParticleEmitter() {};
	Particle newParticle(int anim, int time);
	virtual int GetEmitterType(){return (int)TYPE_SPHERE_PARTICLE_EMITTER;};
};

struct TexCoordSet {
    Vector2 tc[4];
};
/**
The particle class keeps a list of all of its instances.A particle system instance is always associated
with a owner pointer, which is now the current scene object, such as a missile or a biped. The instance may be
global or local. Global particles remain in the scene even after its owner object is released and 
is rendered as global object. The scene manager keeps a reference to all active particle systems and their
instances. It will automatically delete instances that is no longer active in the current frame. It will also 
animate and render global PS instances whose owner has been destroyed. Particles in global particle systems
should have a short(finite) life time, in order to clean themselves quickly after its owner is released. 
*/
class ParticleSystem 
{
	friend class PlaneParticleEmitter;
	friend class SphereParticleEmitter;
public:	
	/// initial speed, the initial speed magnitude of a particle is V(0) = speed + variation * rand(-1,1).
	Animated<float> speed;
	/// initial speed variation, the initial speed magnitude of a particle is V(0) = speed + variation * rand(-1,1).
	Animated<float> variation;
	/// particle acceleration magnitude along their current directions, shared by all existing particles at a given time , this is usually a positive constant like 9.8
	Animated<float> gravity;
	/// maxlife of a newly born particle in seconds
	Animated<float> lifespan;
	/// how many particles to spawn per second. 
	Animated<float> rate;
	/// the half length of the emitter plane
	Animated<float> areal;
	/// the half width of the emitter plane
	Animated<float> areaw;

	/// RESERVED: this parameter is reserved for other emitter types
	Animated<float> spread;
	/// RESERVED: this parameter is reserved for other emitter types
	Animated<float> lat;
	/// RESERVED: this parameter is reserved for other emitter types
	Animated<float> grav2;
	

	/// it controls how fast the speed magnitude of a particle dwindles as its life is approaching the end.
	/// in formula, it is V*expf(-1.0f * slowdown * p.life), where p.life is the current life of a particle in seconds.
	/// if slowdown is infinitely small, there will be no slowed down; if it is big(such as 1), the particle will has no speed after just a few seconds from birth. 
	float slowdown;
	
	// this parameter is only used for sphere emitter type It is the angle between the emitting direction and downward direction in range [-Pi/2, Pi/2]. Default value is 0, which is the downward direction like the plane emitter.
	float rotation;

	//////////////////////////////////////////////////////////////////////////
	/// the color[0] and sizes[0] is the color and size of a particle at its birth
	/// the color[1] and sizes[1] is the color and size of a particle at when it is age is mid, assuming a lifespan in [0,1]
	/// the color[2] and sizes[2] is the color and size of a particle at its death
	/// for particle at any age [0,1], it will interpolate between these values linearly. Age is defined as particle_time/life_span;
	/// In most cases, mid is 0.5, color[0] and color[2] are transparent;or sizes[0] and size[2] are zero, either of which will let particle fade in and out gracefully.
	float mid;
	Vector4 colors[3];
	float sizes[3];

	/// all particles are offset by this value at its birth. this is usually V(0,0,0).
	Vector3 pos;
	
	/// how the texture blending is applied to each particle. In most cases, it is 2 ALPHA_BLEND or 4 ADDITIVE ALPHA.
	/// 0 for OPAQUE; 1 ADDITIVE BLEND; 2 ALPHA_BLEND(using alpha); 3 TRANSPARENT(using alpha testing); 4 ADDITIVE ALPHA;
	int blend;
	/// the triangle winding order of the quad: whether it is CCW(-1) or CW(0). It does not matter if particle is transparent.
	int order;
	/// this is 0,1,2. usually means how the particles are rendered.
	/// - 0 or 2 means normal particle which is centered on the particle origin with a given size.
	/// - 1 means that particle is rendered with origin at the particle system center and extends to the particle origin.
	int type;

	/// whether using billboard when during the particles. In most cases, this is true, because particles are usually drawn with triangle quads facing the camera.
	bool billboard;

	//////////////////////////////////////////////////////////////////////////
	// a new particle will use a random image at one of the rows*cols regions in the texture accociated with the particle system.
	// if rows=1, cols=1, the entire texture is used.
	//////////////////////////////////////////////////////////////////////////
	int rows, cols;
	// this is automatically generated from rows, cols
	std::vector<TexCoordSet> tiles; 
	// the texture associated with this particle system.
	int32 m_texture_index;
	
	//////////////////////////////////////////////////////////////////////////
	// the bone to which the particle system is attached.
	Bone *parent;

	//////////////////////////////////////////////////////////////////////////
	// the emitter object
	ParticleEmitter *emitter;

public:
	// the ParaX model in which the particle system is defined (contained).
	CParaXModel *model;

	/// this is just a rand seed for this particle system.
	float tofs;

	/** instances of the particle systems.
	* mapping from the owner object,typically this is a scene object, to the particle list of that owner
	*/
	map <void*, ParticleList*> m_instances;

	bool rotate2SpeedDirection;
public:
	ParticleSystem();
	~ParticleSystem();

	// update existing particles and spawn new ones
	void update(SceneState * pSceneState, float dt);

	/** clear all particle instances. */
	void ClearAllInstances();

	/** get associated texture */
	DeviceTexturePtr_type GetDeviceTexture();
	TextureEntity* GetTexture();

	/**
	* only update existing particles
	* @param dt: time delta
	* @param vOffset: all position and origin will be offset by this value during animation.
	*		this is usually the render origin offset between two consecutive calls.
	* @param instancePS: particle system instance.
	* @return: return true if there is still particles in the instance.
	*/
	bool AnimateExistingParticles(float dt, const Vector3& vOffset, ParticleList* instancePS);

	/** get the current particle system instance according to the current scene object. 
	* @param bForceCreate: if this is true, the instance will be created if not found.
	*/
	ParticleList* GetCurrentInstance(SceneState * pSceneState, bool bForceCreate=true);

	/** delete instances for a given scene object */
	bool DeleteInstance(void * owner);

	/** set the current animation ID and animation time, this function is usually called by its parent model to make them synchronized.*/
	void setup(SceneState * pSceneState, int anim, int time);
	/** draw the current instance, if it is a non-absolute particle system instance. */
	void draw(SceneState * pSceneState);
	/** draw a specified instance.*/
	void drawInstance(ParticleList* instancePS);
	/** draw all absolute instances.This is called for batch-rendering global(absolute) particle instances.*/
	void drawAllAbsInstances();

	/** get the parent model.*/
	CParaXModel * GetParent() {return model;}

	// only called by drawInstance();
	void DrawInstanceSub(ParticleList* instancePS);
	/**
	* set how many rows and cols that the texture associated with this particle system is evenly divided into. 
	*/
	void SetTextureRowsCols(int nRows, int nCols);
};

struct RibbonSegment {
	Vector3 pos, up, back;
	float len,len0;
};

/**
* this is another kind of particle system. 
* TODO: this class is not implemented yet. 
*/
class RibbonEmitter 
{
public:
	Animated<Vector3> color;
	AnimatedShort opacity;
	Animated<float> above, below;

	Bone *parent;
	float f1, f2;

	Vector3 pos;

	int manim, mtime;
	float length, seglen;
	int numsegs;
	
	Vector3 tpos;
	Vector4 tcolor;
	float tabove, tbelow;
	/** whether to use absolute world coordinate system, so that when particles leave its source, it will be 
	* an independent object in the world coordinate system.*/
	bool m_bUseAbsCord:1;

	int32 m_texture_index;

	std::list<RibbonSegment> segs;

public:
	CParaXModel *model;
	RibbonEmitter() :m_bUseAbsCord(true), m_texture_index(0){};

	void setup(SceneState * pSceneState, int anim, int time);
	void draw(SceneState * pSceneState);


	/** get associated texture */
	DeviceTexturePtr_type GetDeviceTexture();
	TextureEntity* GetTexture();
};
}