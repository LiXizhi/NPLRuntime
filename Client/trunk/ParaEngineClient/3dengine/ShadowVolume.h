#pragma once

namespace ParaEngine
{
/**
LightParams is a structure for which a shadow volume is built
*/
struct LightParams 
{
	Vector3       Position;		/* Position in world space */
	Vector3       Direction;		/* Direction in world space */
	LinearColor	Diffuse;		/* Diffuse color of light */
	float           Range;          /* Cutoff range */
	bool			bIsDirectional; /* true if it is a directional light */
	LightParams(){
		bIsDirectional = true;
	}
};

class ShadowVolume;

/**
The following shadow interface should be implemented for any shadow caster entity 
in the game engine. In game engine, this means the static mesh object and the animated 
biped object. BuildShadowVolume() function should be called every frame move for each model caster.
*/
class ShadowCaster
{
public:
	virtual bool IsShadowEnabled(){return false;};
	virtual void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight) = 0;
};

class CAutoCamera;
class CBaseObject;
/**
ShadowVolume is a structure for storing shadow volume geometries.
*/
class ShadowVolume
{
public:
	enum ShadowMethod {
		SHADOW_NONE = 0,
		SHADOW_Z_PASS,
		SHADOW_Z_FAIL
	};
	static float m_fMinShadowCastDistance; /// screen distance in pixels, smaller than witch shadows will not be casted

#define INITIAL_SHADOWVOLUME_SIZE		32000
	ShadowVolume(void);
	~ShadowVolume(void);
	
	int				m_nVertexArraySize;		/// initial value is 32000, doubled when exceeded
	Vector3*	m_pVertices;			/// Vertex data for rendering shadow volume
	int				m_nNumVertices;			/// number of vertices in the shadow volume, it must be multiple of 6, since 6 vertices will describe a face.
	LinearColor		m_shadowColor;			/// the color of the shadow, alpha enabled.
	ShadowMethod	m_shadowMethod;			/// in which way, the data in this shadow volume should be drawn
	LightParams*	m_pLightParam;			/// current light info associated with the shadow volume
	Plane		occlusionPyramid[5];	/// used to decide whether to use Z-Fail or Z pass method
protected:
	ParaViewport	m_viewport;				/// current viewport for projection
	Matrix4		m_matProjection;		/// current projection matrix
    Matrix4		m_matView;				/// current view matrix

public:
	/// reset the shadow volume to zero
	void Reset() { 
		m_nNumVertices = 0L; 
		m_shadowMethod = SHADOW_NONE;
	}
	
	void ReserveNewBlock(Vector3** pVertices, int nSize);
	void CommitBlock(int nSize);
	
	void BuildFromShadowCaster( ShadowCaster* pCaster, LightParams* pLight);
	
	/// recalculate occlusion pyramid
	void ReCalculateOcclusionPyramid(CAutoCamera* pCamera);

	/// update projection information
	void UpdateProjectionInfo(const ParaViewport* pViewport,
		const Matrix4 *pProj,
		const Matrix4 *pView);

	/// project point to screen
	void ProjectPoint(Vector3 *pOut,		const Vector3 *pV,		const Matrix4 *pWorld);
	/// check if the region inside the points intersect with the occlusion pyramid
	bool PointsInsideOcclusionPyramid(Vector3* pVecBounds, int nCount);

	/// set current light
	void SetLightParams(LightParams* l){m_pLightParam = l;};
	
	/// render without states overhead
	HRESULT Render( SceneState * sceneState );

	/// render to stencil with Z Pass
	HRESULT RenderZPassShadow( SceneState * sceneState );
	/// render to stencil with Z Fail
	HRESULT RenderZFailShadow( SceneState * sceneState );
};
}