#pragma once

#include "animated.h"
#include "util/ParaMemPool.h"
#include "FileManager.h"
#include <deque>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace ParaEngine
{
class ParticleSystem;
class RibbonEmitter;
class Bone;
class CParaXModel;
struct TextureEntity;

struct Particle {
	Vector3 pos, speed, down, origin;
	float size, life, maxlife, random_force, trail_length;
	float rotation, rotSpeed;
	int tile;
	Vector4 color;
	Vector4 colorTint{1.0f, 1.0f, 1.0f, 1.0f};
	float trailTimer = 0.0f;
	float twinklePhase = 0.0f;
};

struct ParticleBurst {
	int animIndex = -1; // -1 applies to any animation, >=0 applies to specific animation index
	int timeMs = 0;
	int count = 0;
	int countVar = 0;
};

typedef std::vector<Particle> ParticleListType;

/**
* Represents a per-scene-object instance of a ParticleSystem.
*/
struct ParticleList
{
	ParticleListType particles;
	Vector3 m_positionSum{0, 0, 0};
	/** whether to use absolute world coordinate system */
	bool m_bUseAbsCord : 1;
	/** the last render origin when update() is called */
	Vector3 m_vLastRenderOrigin;
	/** whether this object is updated in the current frame */
	bool m_bUpdated : 1;
	/** whether this object should be rendered (in camera frustum) */
	bool m_bRender : 1;
	/** current animation */
	int m_anim;
	/** current frame */
	int m_time;
	/** previous frame for deterministic burst triggering */
	int m_lastAnim;
	int m_lastTime;
	/** remaining fractional particles to spawn */
	float m_rem;
	/** scene state to which this instance belongs */
	void* m_pSceneState;
	/** owner pointer key in ParticleSystem::m_instances */
	void* m_ownerKey;
public:
	ParticleList()
		: m_bUseAbsCord(true)
		, m_vLastRenderOrigin(0, 0, 0)
		, m_bUpdated(false)
		, m_bRender(false)
		, m_anim(0)
		, m_time(0)
		, m_lastAnim(-1)
		, m_lastTime(-1)
		, m_rem(0)
		, m_pSceneState(NULL)
		, m_ownerKey(NULL)
	{}
};

class ParticleEmitter {
protected:
	ParticleSystem* sys;
public:
	enum {
		TYPE_NONE = 0,
		TYPE_PLANE_PARTICLE_EMITTER = 1,
		TYPE_SPHERE_PARTICLE_EMITTER = 2,
		TYPE_SPHERE_SHELL_EMITTER = 3,
		TYPE_PATTERN_PARTICLE_EMITTER = 4,
		TYPE_CONE_JET_EMITTER = 5,
	};
	enum PatternSubType {
		PATTERN_RING = 0,
		PATTERN_HEART = 1,
		PATTERN_STAR5 = 2,
		PATTERN_DOUBLE_RING = 3,
		PATTERN_SPIRAL = 4,
	};
	ParticleEmitter(ParticleSystem* sys) : sys(sys) {}
	virtual ~ParticleEmitter() {}
	virtual Particle newParticle(int anim, int time) = 0;
	virtual int GetEmitterType() = 0;
};

class PlaneParticleEmitter : public ParticleEmitter {
public:
	PlaneParticleEmitter(ParticleSystem* sys) : ParticleEmitter(sys) {}
	virtual ~PlaneParticleEmitter() {}
	Particle newParticle(int anim, int time) override;
	int GetEmitterType() override { return (int)TYPE_PLANE_PARTICLE_EMITTER; }
};

class SphereParticleEmitter : public ParticleEmitter {
public:
	SphereParticleEmitter(ParticleSystem* sys) : ParticleEmitter(sys) {}
	virtual ~SphereParticleEmitter() {}
	Particle newParticle(int anim, int time) override;
	int GetEmitterType() override { return (int)TYPE_SPHERE_PARTICLE_EMITTER; }
};

class SphereShellParticleEmitter : public ParticleEmitter {
public:
	SphereShellParticleEmitter(ParticleSystem* sys) : ParticleEmitter(sys) {}
	virtual ~SphereShellParticleEmitter() {}
	Particle newParticle(int anim, int time) override;
	int GetEmitterType() override { return (int)TYPE_SPHERE_SHELL_EMITTER; }
};

class PatternParticleEmitter : public ParticleEmitter {
public:
	PatternParticleEmitter(ParticleSystem* sys) : ParticleEmitter(sys) {}
	virtual ~PatternParticleEmitter() {}
	Particle newParticle(int anim, int time) override;
	int GetEmitterType() override { return (int)TYPE_PATTERN_PARTICLE_EMITTER; }
};

class ConeJetParticleEmitter : public ParticleEmitter {
public:
	ConeJetParticleEmitter(ParticleSystem* sys) : ParticleEmitter(sys) {}
	virtual ~ConeJetParticleEmitter() {}
	Particle newParticle(int anim, int time) override;
	int GetEmitterType() override { return (int)TYPE_CONE_JET_EMITTER; }
};

struct TexCoordSet {
	Vector2 tc[4];
};

class ParticleSystem
{
	friend class PlaneParticleEmitter;
	friend class SphereParticleEmitter;
	friend class SphereShellParticleEmitter;
	friend class PatternParticleEmitter;
	friend class ConeJetParticleEmitter;
public:
	Animated<float> speed;
	Animated<float> variation;
	Animated<float> gravity;
	Animated<float> lifespan;
	Animated<float> rate;
	Animated<float> areal;
	Animated<float> areaw;
	Animated<float> spread;
	Animated<float> lat;
	Animated<float> grav2;

	float slowdown = 0.0f;
	float rotation = 0.0f;

	float mid = 0.5f;
	Vector4 colors[3];
	float sizes[3]{1.0f, 1.0f, 1.0f};

	Vector3 pos{0, 0, 0};

	int blend = 4;
	int order = 0;
	int type = 0;
	bool billboard = true;

	int rows = 1, cols = 1;
	std::vector<TexCoordSet> tiles;
	int32 m_texture_index = 0;

	Bone* parent = nullptr;
	ParticleEmitter* emitter = nullptr;

	// Extended v2 fireworks & VFX parameters
	bool m_bExtendedV2 = false;
	bool useWorldGravity = false;
	bool localSpace = false;
	int16 patternType = 0;           // PatternSubType when emitter is PatternParticleEmitter
	int16 trailEmitterIndex = -1;    // sibling emitter index spawned along flight path
	int16 deathEmitterIndex = -1;    // sibling emitter index spawned when particle expires
	int16 deathCount = 0;
	int16 deathCountVar = 0;
	Vector3 emitDir{0.0f, 1.0f, 0.0f};
	float minPolar = 0.0f;           // cone half-angle or inner polar angle (radians)
	float maxPolar = 3.14159265f;    // outer polar angle (radians)
	float shellThickness = 0.0f;     // 0 = crisp surface shell, 1 = filled volume
	float trailInterval = 0.03f;     // seconds between trail sub-emitter spawns
	float inheritVelocity = 0.0f;    // fraction of parent velocity inherited by sub-emitter
	float twinkleFreq = 0.0f;        // Hz for strobe/glitter alpha flicker
	float twinkleAmount = 0.0f;      // [0..1] alpha modulation depth
	std::vector<ParticleBurst> bursts;
	std::vector<Vector4> palette;

public:
	CParaXModel* model = nullptr;
	float tofs = 0.0f;
	std::map<void*, ParticleList*> m_instances;
	bool rotate2SpeedDirection = false;

public:
	ParticleSystem();
	~ParticleSystem();

	static ParticleEmitter* CreateEmitterByType(ParticleSystem* sys, int emitterType);

	void update(SceneState* pSceneState, float dt);
	void ClearAllInstances();

	DeviceTexturePtr_type GetDeviceTexture();
	TextureEntity* GetTexture();

	bool AnimateExistingParticles(float dt, const Vector3& vOffset, ParticleList* instancePS);
	ParticleList* GetCurrentInstance(SceneState* pSceneState, bool bForceCreate = true);
	bool DeleteInstance(void* owner);

	void setup(SceneState* pSceneState, int anim, int time);
	void draw(SceneState* pSceneState);
	void drawInstance(ParticleList* instancePS);
	void drawAllAbsInstances();

	CParaXModel* GetParent() { return model; }
	void DrawInstanceSub(ParticleList* instancePS);
	void SetTextureRowsCols(int nRows, int nCols);

	void InitCommonParticleFields(Particle& p, int anim, int time) const;
	void SpawnSubParticles(int targetEmitterIdx, void* ownerKey, SceneState* pSceneState,
		const Vector3& basePos, const Vector3& parentVel, int count,
		const Vector3& lastRenderOrigin, int anim, int time);
};

struct RibbonSegment {
	Vector3 pos{0, 0, 0};
	Vector3 up{0, 1, 0};
	Vector3 back{0, 0, 1};
	float len = 0.0f;
	float len0 = 0.0f;
	float age = 0.0f;
	Vector4 headColor{1.0f, 1.0f, 1.0f, 1.0f};
};

struct RibbonInstance {
	std::deque<RibbonSegment> segs;
	Vector3 m_vLastPos{0, 0, 0};
	Vector3 m_vLastRenderOrigin{0, 0, 0};
	void* m_pSceneState = nullptr;
	bool m_bUseAbsCord = true;
	bool m_bInitialized = false;
	bool m_bUpdated = false;
	bool m_bRender = false;
	int m_anim = 0;
	int m_time = 0;
};

/**
* Per-instance 3D trail / ribbon emitter attached to a bone on CParaXModel.
*/
class RibbonEmitter
{
public:
	Animated<Vector3> color;
	AnimatedShort opacity;
	Animated<float> above, below;

	Bone* parent = nullptr;
	float f1 = 0.0f, f2 = 0.0f;

	Vector3 pos{0, 0, 0};

	int manim = 0, mtime = 0;
	float length = 0.0f, seglen = 0.2f;
	int numsegs = 16;

	Vector3 tpos{0, 0, 0};
	Vector4 tcolor{1, 1, 1, 1};
	float tabove = 0.1f, tbelow = 0.1f;
	bool m_bUseAbsCord : 1;

	int32 m_texture_index = 0;
	std::list<RibbonSegment> segs; // retained for legacy compatibility

	// Extended v2 ribbon parameters
	bool m_bExtendedV2 = false;
	bool orientCamera = true;
	bool hasEndColor = false;
	Vector4 colorEnd{1.0f, 1.0f, 1.0f, 0.0f};
	float maxLife = 0.6f;
	int blend = 4; // 1=ADDITIVE, 2=ALPHA_BLEND, 4=ADDITIVE_ALPHA
	float widthEndScale = 0.0f; // 0 = taper to sharp point at tail, 1 = constant width
	float maxStepDist = 8.0f;   // break/reset ribbon on large teleport jumps

	std::map<void*, RibbonInstance*> m_instances;

public:
	CParaXModel* model = nullptr;
	RibbonEmitter() : m_bUseAbsCord(true), m_texture_index(0) {}
	~RibbonEmitter();

	CParaXModel* GetParent() { return model; }
	RibbonInstance* GetCurrentInstance(SceneState* pSceneState, bool bForceCreate = true);
	void ClearAllInstances();
	bool DeleteInstance(void* owner);

	void setup(SceneState* pSceneState, int anim, int time);
	void update(SceneState* pSceneState, float dt);
	bool AnimateExistingSegments(float dt, const Vector3& vOffset, RibbonInstance* inst);
	void draw(SceneState* pSceneState);
	void drawInstance(RibbonInstance* inst);
	void DrawInstanceSub(RibbonInstance* inst);

	DeviceTexturePtr_type GetDeviceTexture();
	TextureEntity* GetTexture();
};

/** Built-in procedural 64x64 RGBA particle textures (builtin://glow, spark, star, ring, smoke, ribbon, flare). */
bool IsBuiltinParticleTextureName(const std::string& name);
asset_ptr<TextureEntity> LoadOrCreateBuiltinParticleTexture(const std::string& name);
bool BuildProceduralParticlePNG(const std::string& kind, std::vector<char>& outPngBytes);

} // namespace ParaEngine
