#pragma once

#include "TerrainCommon.h"
#include "Texture.h"
#include "Terrain.h"

namespace ParaEngine
{
	class CParaFile;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	//class Texture;
	//class Terrain;

	/// \brief A DetailTexture is a "layer" that can be painted onto to put texture "splats" on a terrain surface.

	/// You should rarely need to use this class directly. Demeter uses it to manage alpha blending of detail textures
	/// onto the terrain surface. There is one instance of this class for each texture "cell" in a Terrain. The number
	/// texture cells in a Terrain is set in one of two ways: (the size of the overall terrain texture / 256) ^ 2, OR by
	/// what you specify in a call to Terrain::AllocateTextureCells().
	class DetailTexture
	{
	public:
		/// \brief Constructs a new DetailTexture that allows splatting of the specified Texture as a detail texture.
		DetailTexture(Texture * pTexture, Texture * pMask);
		DetailTexture(Texture * pTexture = NULL, unsigned char DefaultMaskValue = 0);
		DetailTexture(CParaFile& file,Terrain * pTerrain);
		~DetailTexture();
		/// \brief Sets the alpha mask to be used for this painting layer. This should be an alpha-only Texture.
		void SetMask(Texture *);
		/// \brief Sets the Texture that this painting layer uses as a detail texture.
		void SetTexture(Texture *);

		/// \brief Binds the mask layer to an OpenGL texture object.
		DeviceTexturePtr_type BindMask();
		/// \brief Binds the detail texture to an OpenGL texture object.
		DeviceTexturePtr_type BindTexture();
		/// \brief Unloads both the mask and detail OpenGL texture objects.
		void Unbind();
		/// \brief Gets the alpha mask layer.
		Texture *GetMask();

		/** regenerate the Mask file based on the dimension of the detailed texture. This function is usually called, 
		when the mask file of the texture is not created so far. 
		* @param DefaultMaskValue: the pixel color of the mask. 
		* @return: the mask file is returned. 
		*/
		Texture* RegenerateMask(unsigned char DefaultMaskValue = 0xff);

		/// \brief Gets the detail texture.
		Texture *GetTexture();
		
		void WriteMask(CParaFile& file, Terrain * pTerrain);
		void ReadMask(CParaFile& file, Terrain * pTerrain);

		/// \brief Works exactly like Texture::FlipVertical().
		void FlipVertical();
		/// \brief Works exactly like Texture::FlipHorizontal().
		void FlipHorizontal();

		bool IsOwnMask() {return m_bOwnMask;};
		void SetOwnMask(bool bOwn) {m_bOwnMask = bOwn;};

		bool IsOwnTexture() {return m_bOwnTexture;};
		void SetOwnTexture(bool bOwn) {m_bOwnTexture = bOwn;};
	private:
		/// if true, m_pMask will be deleted when this object is deleted. 
		bool m_bOwnMask;
		Texture * m_pMask;

		/// if true, m_pTexture will be deleted when this object is deleted. 
		bool m_bOwnTexture;
		Texture *m_pTexture;
	};
}
