//-----------------------------------------------------------------------------
// Class:	particle system for ParaX Model
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.11
// Revised: 2007.3
// Note: initial particle code logic from wowmodelviewer; but is greatly improved later on based on 3dsmax SDK and ParaX Exporter.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaWorldAsset.h"
#include "EffectManager.h"
#include "SceneObject.h"
#include "ParaXBone.h"
#include "particle.h"
#include "memdebug.h"
using namespace ParaEngine;

/** @def whether the particles of a particle system will default to global particles.
* global particles will become independent objects in the scene and will not subject to
* parent transforms once it leaves its source.
*/
#define DEFAULT_TO_GLOBAL_OBJECT		true

#define MAX_PARTICLES 10000

Vector4 fromARGB(uint32 color)
{
	const float a = ((color & 0xFF000000) >> 24) / 255.0f;
	const float r = ((color & 0x00FF0000) >> 16) / 255.0f;
	const float g = ((color & 0x0000FF00) >> 8) / 255.0f;
	const float b = ((color & 0x000000FF)) / 255.0f;
	return Vector4(r, g, b, a);
}

template<class T>
T lifeRamp(float life, float mid, const T& a, const T& b, const T& c)
{
	if (life <= mid) return interpolate<T>(life / mid, a, b);
	else return interpolate<T>((life - mid) / (1.0f - mid), b, c);
}

ParticleSystem::ParticleSystem() :
	emitter(0), rotate2SpeedDirection(false), m_texture_index(0)
{
}

ParticleSystem::~ParticleSystem()
{
	SAFE_DELETE(emitter);
	ClearAllInstances();
}

void ParticleSystem::SetTextureRowsCols(int nRows, int nCols)
{
	rows = nRows;
	cols = nCols;

	//////////////////////////////////////////////////////////////////////////
	// following code will fill the tiles member with texture coordinate for later use at rendering time.
	//////////////////////////////////////////////////////////////////////////
	int nCount = rows * cols;
	tiles.clear();
	tiles.reserve(nCount);

	for (int k = 0; k < nCount; ++k)
	{
		tiles.push_back(TexCoordSet());
		TexCoordSet& tc = tiles[k];

		/*
		tc.tc[0].x = 1;
		tc.tc[0].y = 0;

		tc.tc[1].x = 1;
		tc.tc[1].y = 1;

		tc.tc[2].x = 0;
		tc.tc[2].y = 1;

		tc.tc[3].x = 0;
		tc.tc[3].y = 0;*/

		Vector2 otc[4];
		Vector2 a, b;
		int x = k % cols;
		int y = k / cols;
		a.x = x * (1.0f / cols);
		a.y = y * (1.0f / rows);
		b.x = (x + 1) * (1.0f / cols);
		b.y = (y + 1) * (1.0f / rows);

		/*otc[0] = a;
		otc[2] = b;
		otc[1].x = b.x;
		otc[1].y = a.y;
		otc[3].x = a.x;
		otc[3].y = b.y;*/
		otc[0] = a;
		otc[2] = b;
		otc[1].x = b.x;
		otc[1].y = a.y;
		otc[3].x = a.x;
		otc[3].y = b.y;


		for (int i = 0; i < 4; i++) {
			tc.tc[(i + 4 - order) & 3] = otc[i];
		}
	}
}

void ParticleSystem::update(SceneState* pSceneState, float dt)
{
	/**
	* get the particle system instance for the current scene object.
	*/
	ParticleList* instancePS = GetCurrentInstance(pSceneState);
	if (instancePS == NULL || instancePS->m_bUpdated)
		return;

	/// add this particle system to the scene state. so that the scene state will automatically
	/// maintain the life of this particle system instance.
	pSceneState->AddParticleSystem(this);
	/// mark as updated instance
	instancePS->m_bUpdated = true;

	ParticleListType& particles = instancePS->particles;

	Vector3 vRenderOriginOffset;
	Matrix4 mWorld;
	Vector3 vRenderOrigin;

	if (instancePS->m_bUseAbsCord)
	{
		mWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
		vRenderOrigin = pSceneState->GetScene()->GetRenderOrigin();

		vRenderOriginOffset = instancePS->m_vLastRenderOrigin - vRenderOrigin;
		instancePS->m_vLastRenderOrigin = vRenderOrigin;// update render origin
	}

	/** animate existing particles.*/
	AnimateExistingParticles(dt, vRenderOriginOffset, instancePS);

	/** spawn new particles
	* Tricky: 2010.3.10: we will not spawn new particle at very low frame rate, such as 10FPS
	*/
	if (emitter && dt <= 0.1f) {
		int manim = instancePS->m_anim;
		int mtime = instancePS->m_time;
		float frate = rate.getValue(manim, mtime);

		// how many particles to spawn in this frame
		float ftospawn = (dt * frate) + instancePS->m_rem;
		// if there is no particle, we will immediately spawn the first particle. 
		if (ftospawn < 1.0f && particles.size()>0) {
			instancePS->m_rem = ftospawn;
			if (instancePS->m_rem < 0)
				instancePS->m_rem = 0;
		}
		else {
			if (ftospawn < 1.f)
				ftospawn = 1.0f;
			int tospawn = (int)ftospawn;
			instancePS->m_rem = ftospawn - (float)tospawn;
			for (int i = 0; i < tospawn; i++) {
				if (particles.size() >= MAX_PARTICLES)
					break;
				particles.resize(particles.size() + 1, emitter->newParticle(manim, mtime));
				if (instancePS->m_bUseAbsCord)
				{
					Particle& p = particles.back();
					// Very tricky: apply the outer world transform: to change the pos, origin and speed(direction) into absolute world space. 
					// transform to world coordinate system, suppose that the world transform is in device already
					p.pos = p.pos * mWorld;
					p.origin = p.origin * mWorld;
					p.speed = p.speed.TransformNormal(mWorld);
				}
			}
		}
	}

}

bool ParticleSystem::AnimateExistingParticles(float dt, const Vector3& vOffset, ParticleList* instancePS)
{
	if (instancePS == NULL)
		return false;
	ParticleListType& particles = instancePS->particles;

	int manim = instancePS->m_anim;
	int mtime = instancePS->m_time;
	float grav = gravity.getValue(manim, mtime);
	float mspeed = 1.0f;

	ParticleListType::iterator it;
	for (it = particles.begin(); it != particles.end(); ) {
		Particle& p = *it;

		p.speed += p.down * (grav * dt);

		if (p.random_force > 0.f)
		{
			Vector3 vec3RandomForce(randfloat(-p.random_force, p.random_force), randfloat(-p.random_force, p.random_force), randfloat(-p.random_force, p.random_force));
			p.speed += vec3RandomForce * dt;
		}

		if (slowdown > 0)
		{
			mspeed = expf(-1.0f * slowdown * p.life);
		}

		p.pos += p.speed * (mspeed * dt);

		//texture rotation  --clayman 11.4.21
		if (p.rotSpeed > 0)
		{
			float rotAtt = 1 - p.life / p.maxlife;
			p.rotation += p.rotSpeed * dt * rotAtt;
		}

		if (p.trail_length > 0.f)
		{
			p.origin = p.pos - p.speed * p.trail_length;
		}

		if (instancePS->m_bUseAbsCord)
		{
			p.pos += vOffset;
			p.origin += vOffset;
		}

		p.life += dt;
		float rlife = p.life / p.maxlife;
		// calculate size and color based on lifetime
		p.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
		p.color = lifeRamp<Vector4>(rlife, mid, colors[0], colors[1], colors[2]);

		// kill off old particles
		if (rlife >= 1.0f)
			it = particles.erase(it);
		else
			++it;
	}
	return (particles.size() > 0);
}

ParticleList* ParticleSystem::GetCurrentInstance(SceneState* pSceneState, bool bForceCreate)
{
	/**
	* get the particle system instance for the current scene object.
	*/
	void* owner = (void*)(pSceneState->GetCurrentSceneObject());
	map<void*, ParticleList*>::iterator iter = m_instances.find(owner);
	ParticleList* instancePS = NULL;
	if (iter == m_instances.end())
	{
		if (bForceCreate)
		{
			// create a new instance if not found
			instancePS = new ParticleList();
			instancePS->m_pSceneState = pSceneState;
			m_instances[owner] = instancePS;
		}
	}
	else
	{
		instancePS = iter->second;
	}
	return instancePS;
}

bool ParticleSystem::DeleteInstance(void* owner)
{
	map<void*, ParticleList*>::iterator iter = m_instances.find(owner);

	if (iter != m_instances.end())
	{
		ParticleList* instancePS = iter->second;
		SAFE_DELETE(instancePS);
		m_instances.erase(iter);
	}
	return true;
}

void ParticleSystem::setup(SceneState* pSceneState, int anim, int time)
{
	ParticleList* instancePS = GetCurrentInstance(pSceneState);
	if (instancePS == NULL)
		return;
	instancePS->m_anim = anim;
	instancePS->m_time = time;
}
void ParticleSystem::draw(SceneState* pSceneState)
{
	/**
	* get the particle system instance for the current scene object.
	*/
	ParticleList* instancePS = GetCurrentInstance(pSceneState, false);
	if (instancePS == NULL)
		return;
	instancePS->m_bRender = true;
	if (!instancePS->m_bUseAbsCord)
		drawInstance(instancePS);
}

void ParticleSystem::drawAllAbsInstances()
{
	map<void*, ParticleList*>::iterator iter, iterEnd = m_instances.end();
	for (iter = m_instances.begin(); iter != iterEnd; ++iter)
	{
		ParticleList* instancePS = iter->second;
		if (instancePS && instancePS->m_bUseAbsCord)
		{
			drawInstance(instancePS);
		}
	}
}

void ParticleSystem::drawInstance(ParticleList* instancePS)
{
	if (instancePS == NULL)
		return;
	ParticleListType& particles = instancePS->particles;

	/** draw the particle system instance.*/
	if (particles.size() <= 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	if (instancePS->m_bUseAbsCord)
	{
		CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());
	}
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();

	// setup blend mode
	if (pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER

		switch (blend) {
		case 0://OPAQUE
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			break;
		case 1://ADDITIVE BLEND
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		case 2://ALPHA_BLEND
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case 3://TRANSPARENT
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, TRUE);
			//pd3dDevice->SetRenderState( ERenderState::ALPHAREF,         0x18 );
			//pd3dDevice->SetRenderState( ERenderState::ALPHAFUNC,  D3DCMP_GREATER );
			break;
		case 4://ADDITIVE ALPHA
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			// we want the alpha to be the modulation of the texture and the diffuse color
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			break;
		}
#endif
	}
	else
	{
		switch (blend) {
		case 0://OPAQUE
			pEffect->EnableAlphaBlending(false);
			pEffect->EnableAlphaTesting(false);
			break;
		case 1://ADDITIVE BLEND
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		case 2://ALPHA_BLEND
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case 3://TRANSPARENT
			pEffect->EnableAlphaTesting(true);
			pEffect->EnableAlphaBlending(false);
			break;
		case 4://ADDITIVE ALPHA
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
#if USE_DIRECTX_RENDERER
			// we want the alpha to be the modulation of the texture and the diffuse color
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
#endif
			break;
		}
	}

	if (pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		///////////////////////////////////////////////////////////////////////////
		// fixed function pipeline
		/// Set the texture
		DeviceTexturePtr_type pTex = GetDeviceTexture();
		if (pTex)
		{
			pd3dDevice->SetTexture(0, pTex);
			CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, false, false);
			// render all
			DrawInstanceSub(instancePS);
		}
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		DeviceTexturePtr_type pTex = GetDeviceTexture();
		// OUTPUT_LOG("texture : %s %d %d \n", texture->GetKey().c_str(), (int)pTex, texture->IsLocked() ? 1 : 0);
		if (pTex)
		{
			if (pEffect->begin())
			{
				if (pEffect->BeginPass(0))
				{
					// render all
					pEffect->setTexture(0, pTex);
					pEffect->CommitChanges();

					DrawInstanceSub(instancePS);
				}
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}


	// restore old transform matrix
	if (instancePS->m_bUseAbsCord)
		CGlobals::GetWorldMatrixStack().pop();

	if (pEffect == 0)
	{
		switch (blend) {
		case 0://OPAQUE
			break;
		case 2://ALPHA_BLEND
			break;
		case 3://TRANSPARENT
			break;
		case 4://ADDITIVE ALPHA
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
#if USE_DIRECTX_RENDERER
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#endif
			break;
		default:
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		}
	}
	else
	{
		switch (blend)
		{
		case 0://OPAQUE
			break;
		case 2://ALPHA_BLEND
			break;
		case 3://TRANSPARENT
			break;
		case 4://ADDITIVE ALPHA
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
#if USE_DIRECTX_RENDERER
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#endif
			break;
		default:
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		}
	}
}

void ParticleSystem::DrawInstanceSub(ParticleList* instancePS)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	ParticleListType& particles = instancePS->particles;
	if (particles.empty())
		return;
	Vector3 bv0, bv1, bv2, bv3;
	Matrix4 mbb;
	mbb.identity();
	Vector3 look(0, 1, 0);
	Matrix4 viewMat;
	if (billboard) {
		// get a billboard matrix
		Matrix4 mtrans = CGlobals::GetViewMatrixStack().SafeGetTop();
		viewMat = mtrans;

		if (!instancePS->m_bUseAbsCord)
		{
			// when it is not absolute coordinate, the world transform is should be calculated. 
			const Matrix4& mtransWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
			mtrans = mtransWorld * mtrans;
		}
		mtrans = mtrans.inverse();
		Vector3 camera = mtrans.getTrans();
		if (instancePS->m_bUseAbsCord)
		{
			// use average position(gravity point) of the particle system.
			Vector3 basePos(0, 0, 0);
			ParticleListType::const_iterator it, itEnd = particles.end();
			for (it = particles.begin(); it != itEnd; ++it)
			{
				basePos += (*it).pos;
			}
			float fM = (float)(particles.size());
			basePos.x /= fM;
			basePos.y /= fM;
			basePos.z /= fM;
			look = (camera - basePos).normalisedCopy();
		}
		else
			look = (camera - pos).normalisedCopy();

		Vector3 up = ((Vector3(0, 1, 0) * mtrans) - camera).normalisedCopy();
		Vector3 right = (up % look).normalisedCopy();
		up = (look % right).normalisedCopy();

		// calculate the billboard matrix
		mbb.m[1][0] = right.x;
		mbb.m[1][1] = right.y;
		mbb.m[1][2] = right.z;
		mbb.m[2][0] = up.x;
		mbb.m[2][1] = up.y;
		mbb.m[2][2] = up.z;
		mbb.m[0][0] = look.x;
		mbb.m[0][1] = look.y;
		mbb.m[0][2] = look.z;
	}

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize);

	SPRITEVERTEX* vb_vertices;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;

	if (type == 0 || type == 2) {
		// TODO: type 2 is reserved and use the same rendering as type 0
		/* a quad facing camera at local model's origin.
		bv0----bv1
		|       |
		bv3----bv2
		*/
		float f = 0.707106781f; // sqrt(2)/2
		if (billboard) {
			bv0 = Vector3(0, -f, +f) * mbb;
			bv1 = Vector3(0, +f, +f) * mbb;
			bv2 = Vector3(0, +f, -f) * mbb;
			bv3 = Vector3(0, -f, -f) * mbb;
		}
		else {
			bv0 = Vector3(-f, 0, +f);
			bv1 = Vector3(+f, 0, +f);
			bv2 = Vector3(+f, 0, -f);
			bv3 = Vector3(-f, 0, -f);
		}
		// TODO: per-particle bill-boarding in a non-expensive way?
		// currently, i use the gravity point of all particles in the particle list for rotation of billboarding.

		/// each particle is rendered as a quad, which require 2 triangles or 6 vertices.
		int nTotalVertices = (int)particles.size() * 6;
		ParticleListType::const_iterator iterCur = particles.begin();
		ParticleListType::const_iterator iterEnd = particles.end();
		// the first triangle is 0, the second is 1.
		int nTriID = 0;
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;

				for (int i = 0; i < nLockedNum && (iterCur != iterEnd); ++i)
				{
					float fSize = iterCur->size;
					int nTile = iterCur->tile;
					DWORD nColor = LinearColor((const float*)&(iterCur->color));
					int nVB = 3 * i;

					/////////////////////////////////////////////////////////////////////////////
					if (rotate2SpeedDirection)
					{
						Vector3 camFoward(mbb.m[0][0], mbb.m[0][1], mbb.m[0][2]);
						float rotation;

						//camera forward vector
						Vector3 rotAxis = Vector3(mbb.m[0][0], mbb.m[0][1], mbb.m[0][2]);
						Vector3 speedDir = iterCur->speed;

						//Project particle speed to camera's right-up plane.When particle speed aligned 
						//with view direction the projected vector will be a point and derives wrong result
						//any better way to calculate rotation angle?   clayman 2012.7.24
						float dot = speedDir.dotProduct(camFoward);
						Vector3 projectDir = iterCur->speed - dot * camFoward;
						projectDir.normalise();

						//cale rotation angle
						dot = projectDir.dotProduct(Vector3(mbb.m[2][0], mbb.m[2][1], mbb.m[2][2]));
						rotation = acos(dot);
						Vector3 referenceAxis = projectDir.crossProduct(Vector3(mbb.m[2][0], mbb.m[2][1], mbb.m[2][2]));
						dot = referenceAxis.dotProduct(rotAxis);
						if (dot < 0)
							rotation = -rotation;

						Quaternion rot(rotAxis, -rotation);
						Matrix4 rotMat(rot);
						rotMat = mbb * rotMat;
						bv0 = Vector3(0, -f, +f) * rotMat;
						bv1 = Vector3(0, +f, +f) * rotMat;
						bv2 = Vector3(0, +f, -f) * rotMat;
						bv3 = Vector3(0, -f, -f) * rotMat;
					}

					//rotate particle  --clayman 11.4.26
					if (iterCur->rotation > 0)
					{
						float c = cos(iterCur->rotation);
						float s = sin(iterCur->rotation);
						if (billboard)
						{
							bv0 = Vector3(0, -f * c - f * s, f * c - f * s) * mbb;
							bv1 = Vector3(0, f * c - f * s, f * c + f * s) * mbb;
							bv2 = Vector3(0, f * c + f * s, -f * c + f * s) * mbb;
							bv3 = Vector3(0, -f * c + f * s, -f * c - f * s) * mbb;
						}
						else
						{
							bv0 = Vector3(-f * c - f * s, 0, f * c - f * s);
							bv1 = Vector3(f * c - f * s, 0, f * c + f * s);
							bv2 = Vector3(f * c + f * s, 0, -f * c + f * s);
							bv3 = Vector3(-f * c + f * s, 0, -f * c - f * s);
						}
					}
					//////////////////////////////////////////////////////////////////////////

					if (nTriID == 0)
					{ // first triangle in the quad particle
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;

						++nVB;
						vb_vertices[nVB].p = (iterCur->pos + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[1].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[1].y;
						vb_vertices[nVB].color = nColor;
						++nVB;

						if (iterCur->trail_length == 0.f)
							vb_vertices[nVB].p = (iterCur->pos + bv2 * fSize);
						else
							vb_vertices[nVB].p = (iterCur->origin + bv2 * fSize);

						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
					}
					else
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						if (iterCur->trail_length == 0.f)
							vb_vertices[nVB].p = (iterCur->pos + bv2 * fSize);
						else
							vb_vertices[nVB].p = (iterCur->origin + bv2 * fSize);

						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
						++nVB;

						if (iterCur->trail_length == 0.f)
							vb_vertices[nVB].p = (iterCur->pos + bv3 * fSize);
						else
							vb_vertices[nVB].p = (iterCur->origin + bv3 * fSize);

						vb_vertices[nVB].tu = tiles[nTile].tc[3].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[3].y;
						vb_vertices[nVB].color = nColor;
					}

					if (++nTriID >= 2)
					{
						nTriID = 0;
						iterCur++;
					}
				}
				pBufEntity->Unlock();
				if (pBufEntity->IsMemoryBuffer())
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
					nNumFinishedVertice += (nLockedNum * 3);
				else
					break;
			}
			else
				break;
		} while (1);
	}
	else if (type == 1) {

		// particles from origin to position
		bv0 = Vector3(0, -1.0f, 0) * mbb;
		bv1 = Vector3(0, +1.0f, 0) * mbb;

		// each particle is rendered as a quad, which require 2 triangles or 6 vertices.
		int nTotalVertices = (int)particles.size() * 6;
		ParticleListType::const_iterator iterCur = particles.begin();
		ParticleListType::const_iterator iterEnd = particles.end();
		// the first triangle is 0, the second is 1.
		int nTriID = 0;
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;

				for (int i = 0; i < nLockedNum && (iterCur != iterEnd); ++i)
				{
					float fSize = iterCur->size;
					int nTile = iterCur->tile;
					DWORD nColor = LinearColor((const float*)&(iterCur->color));
					int nVB = 3 * i;

					if (nTriID == 0)
					{ // first triangle in the quad particle
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->pos + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[1].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[1].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
					}
					else
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[3].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[3].y;
						vb_vertices[nVB].color = nColor;
					}
					if (++nTriID >= 2)
					{
						nTriID = 0;
						iterCur++;
					}
				}
				pBufEntity->Unlock();
				if (pBufEntity->IsMemoryBuffer())
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
					nNumFinishedVertice += (nLockedNum * 3);
				else
					break;
			}
			else
				break;
		} while (1);
	}
}

Particle PlaneParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	// TODO: maybe evaluate these outside the spawn function, since they will be common for a given frame?
	float w = sys->areal.getValue(anim, time) * 0.5f;
	float l = sys->areaw.getValue(anim, time) * 0.5f;
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	p.pos = sys->pos + Vector3(randfloat(-l, l), 0, randfloat(-w, w));
	p.pos = p.pos * sys->parent->mat;

	Vector3 dir = Vector3(0, -1.f, 0) * sys->parent->mrot;
	p.down = Vector3(0, -1.0f, 0); // dir * -1.0f;
	//p.speed = dir.normalisedCopy() * randfloat(spd1,spd2);   // ?
	p.speed = dir.normalisedCopy() * spd * (1.0f + randfloat(-var, var));

	p.life = 0;
	p.maxlife = sys->lifespan.getValue(anim, time);

	p.random_force = 0.f;
	if (sys->spread.ranges.size() > 0)
	{
		p.random_force = sys->spread.getValue(anim, time);
	}

	p.trail_length = 0.f;
	if (sys->type == 0 || sys->type == 2)
	{
		// for type 0 and 2, the latitude is interpreted as trail time. 
		if (sys->lat.ranges.size() > 0)
		{
			p.trail_length = sys->lat.getValue(anim, time);
		}
	}

	p.size = sys->sizes[0];
	p.color = sys->colors[0];

	p.origin = p.pos;

	p.tile = randint(0, sys->rows * sys->cols - 1);

	//texture rotation  ----clayman 2011.4.22
	p.rotSpeed = 0;
	if (sys->grav2.ranges.size() > 0)
	{
		float radValue = randfloat(-0.2f, 0.2f);
		float rotSpeed = sys->grav2.getValue(0, 0);
		p.rotSpeed = rotSpeed * (1 + radValue);   //give each particle a random rotation speed;
	}
	p.rotation = p.rotSpeed * 2; //give each particle a random init rotation
	return p;
}

Particle SphereParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float l = sys->areal.getValue(anim, time);
	float w = sys->areaw.getValue(anim, time);
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	float alpha = randfloat(0, 2 * MATH_PI);

	float cosAlpha = cosf(alpha);
	float sinAlpha = sinf(alpha);

	float cosTheta = cosf(sys->rotation);
	float sinTheta = sinf(sys->rotation);

	//////////////////////////////////////////////////////////////////////////
	// a new particle for sphere emitter
	// TODO: we may spawn particles in other patterns.

	// particles will appear on the edge of a circle in the xz plane,whose shape is defined by l and w.
	Vector3 born_pos(l * cosAlpha, 0, w * sinAlpha);
	p.pos = sys->pos + born_pos;
	p.pos = p.pos * sys->parent->mat;
	p.origin = p.pos;

	Vector3 born_dir(sinTheta * cosAlpha, -cosTheta, sinTheta * sinAlpha);
	Vector3 dir = (born_dir)*sys->parent->mrot;
	p.speed = dir/*.normalisedCopy()*/ * spd * (1.0f + randfloat(-var, var));

	p.down = Vector3(0, -1.0f, 0) * sys->parent->mrot; // the gravitational field is always downward.

	p.life = 0;
	p.maxlife = sys->lifespan.getValue(anim, time);

	p.random_force = 0.f;
	if (sys->spread.ranges.size() > 0)
	{
		p.random_force = sys->spread.getValue(anim, time);
	}

	p.trail_length = 0.f;
	if (sys->type == 0 || sys->type == 2)
	{
		// for type 0 and 2, the latitude is interpreted as trail time. 
		if (sys->lat.ranges.size() > 0)
		{
			p.trail_length = sys->lat.getValue(anim, time);
		}
	}

	p.size = sys->sizes[0];
	p.color = sys->colors[0];

	p.tile = randint(0, sys->rows * sys->cols - 1); // randomly pick a texture tile to use

	//give each particle a random rotation speed   --clayman 2011.4.22
	p.rotSpeed = 0;
	if (sys->grav2.ranges.size() > 0)
	{
		float radValue = randfloat(-0.2f, 0.2f);
		float rotSpeed = sys->grav2.getValue(0, 0);
		p.rotSpeed = rotSpeed * (1 + radValue);
	}
	p.rotation = p.rotSpeed * 2;
	return p;
}

TextureEntity* ParaEngine::RibbonEmitter::GetTexture()
{
	if (model)
	{
		return model->textures[m_texture_index].get();
	}
	return NULL;
}

ParaEngine::DeviceTexturePtr_type ParaEngine::RibbonEmitter::GetDeviceTexture()
{
	auto pTexture = GetTexture();
	return pTexture != 0 ? pTexture->GetTexture() : 0;
}

void RibbonEmitter::setup(SceneState* pSceneState, int anim, int time)
{
	if (segs.size() == 0)
	{
		return;
	}
	Vector3 ntpos = pos * parent->mat;
	Vector3 ntup = (pos + Vector3(0, 0, 1)) * parent->mat;
	ntup -= ntpos;
	ntup.normalise();
	float dlen = (ntpos - tpos).length();

	manim = anim;
	mtime = time;

	// move first segment
	RibbonSegment& first = *segs.begin();
	if (first.len > seglen) {
		// add new segment
		first.back = (tpos - ntpos).normalisedCopy();
		first.len0 = first.len;
		RibbonSegment newseg;
		newseg.pos = ntpos;
		newseg.up = ntup;
		newseg.len = dlen;
		segs.push_front(newseg);
	}
	else {
		first.up = ntup;
		first.pos = ntpos;
		first.len += dlen;
	}

	// kill stuff from the end
	float l = 0;
	bool erasemode = false;
	for (std::list<RibbonSegment>::iterator it = segs.begin(); it != segs.end(); ) {
		if (!erasemode) {
			l += it->len;
			if (l > length) {
				it->len = l - length;
				erasemode = true;
			}
			++it;
		}
		else {
			it = segs.erase(it);
		}
	}

	tpos = ntpos;
	tcolor = Vector4(color.getValue(anim, time), opacity.getValue(anim, time));

	tabove = above.getValue(anim, time);
	tbelow = below.getValue(anim, time);
}

void RibbonEmitter::draw(SceneState* pSceneState)
{
	if (segs.size() <= 1)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	/// blending additive
	pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
	pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);

	// we want the alpha to be the modulation of the texture and the diffuse color
#if USE_DIRECTX_RENDERER
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
#endif

	// texture
	pd3dDevice->SetTexture(0, GetDeviceTexture());

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize);

	DWORD nColor = LinearColor((const float*)&(tcolor));
	float fCurrentLength = 0;

	SPRITEVERTEX* vb_vertices;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;

	// each particle is rendered as a quad strip, which require 2 triangles or 2 vertices.
	int nTotalVertices = ((int)segs.size() + 1) * 2;
	list<RibbonSegment>::const_iterator iterCur = segs.begin();
	list<RibbonSegment>::const_iterator iterEnd = segs.end();
	// the first triangle is 0, the second is 1.
	int nTriID = 0;
	bool bFinished = false;
	do
	{
		if ((nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice),
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice - 2;
			int nVB;
			for (nVB = 0; nVB < nLockedNum; ++nVB)
			{
				float u = fCurrentLength / length;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up);
				vb_vertices[nVB].tu = u;
				vb_vertices[nVB].tv = 0;
				vb_vertices[nVB].color = nColor;
				++nVB;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up);
				vb_vertices[nVB].tu = u;
				vb_vertices[nVB].tv = 1;
				vb_vertices[nVB].color = nColor;
				iterCur++;
				fCurrentLength += iterCur->len;
			}
			if (iterEnd == iterCur)
			{ // last particle.
				iterCur--;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up + (iterCur->len / iterCur->len0) * iterCur->back);
				vb_vertices[nVB].tu = 1;
				vb_vertices[nVB].tv = 0;
				vb_vertices[nVB].color = nColor;
				++nVB;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up + (iterCur->len / iterCur->len0) * iterCur->back);
				vb_vertices[nVB].tu = 1;
				vb_vertices[nVB].tv = 1;
				vb_vertices[nVB].color = nColor;
				bFinished = true;
			}
			else
			{ // we have to break to two strips due to the size of the dynamic buffer returned.
				float u = fCurrentLength / length;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up);
				vb_vertices[nVB].tu = u;
				vb_vertices[nVB].tv = 0;
				vb_vertices[nVB].color = nColor;
				++nVB;
				vb_vertices[nVB].p = (iterCur->pos + tabove * iterCur->up);
				vb_vertices[nVB].tu = u;
				vb_vertices[nVB].tv = 1;
				vb_vertices[nVB].color = nColor;
			}
			pBufEntity->Unlock();

			// TODO: move this out of the unlock and lock pair
			CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
			if (pEffect == 0)
			{
#ifdef USE_DIRECTX_RENDERER
				///////////////////////////////////////////////////////////////////////////
				// fixed function pipeline
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLESTRIP, pBufEntity->m_dwBase, nLockedNum / 2);
#endif
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// programmable pipeline
				if (pEffect->begin())
				{
					if (pEffect->BeginPass(0))
					{
						pEffect->CommitChanges();
						if (pBufEntity->IsMemoryBuffer())
							RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLESTRIP, nLockedNum / 2, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
						else
							RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLESTRIP, pBufEntity->GetBaseVertex(), nLockedNum / 2);

					}
					pEffect->EndPass(0);
				}
				pEffect->end();
			}


			if ((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
				nNumFinishedVertice += nNumLockedVertice;
			else
				break;
		}
		else
			break;
	} while (!bFinished);

	pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
#if USE_DIRECTX_RENDERER
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#endif
	pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

}

void ParaEngine::ParticleSystem::ClearAllInstances()
{
	/** delete all particle system instances*/
	map<void*, ParticleList*>::iterator iter, iterEnd = m_instances.end();
	for (iter = m_instances.begin(); iter != iterEnd; ++iter)
	{
		SAFE_DELETE(iter->second);
	}
	m_instances.clear();
}

TextureEntity* ParaEngine::ParticleSystem::GetTexture()
{
	if (model)
	{
		return model->textures[m_texture_index].get();
	}
	return NULL;
}

ParaEngine::DeviceTexturePtr_type ParaEngine::ParticleSystem::GetDeviceTexture()
{
	auto pTexture = GetTexture();
	return pTexture != 0 ? pTexture->GetTexture() : 0;
}
