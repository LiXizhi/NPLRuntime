#pragma once
#include "TerrainCommon.h"
#include "DetailTexture.h"
#include "Texture.h"
#include "Terrain.h"

namespace ParaEngine
{
	class CParaFile;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	/// \brief Represents a square region on a Terrain surface that is textured.

	/// A Terrain's surface is divided into a grid of square regions. Each of these regions, a TextureCell,
	/// contains a texture which is part of the overall terrain texture (usually an RGB texture), and any number
	/// of detail textures (possibly zero.) Each DetailTexture is blending according to its alpha mask with the
	/// overall texture underneath it (and with any other DetailTextures underneath it.) The number
	/// texture cells in a Terrain is set in one of two ways: (the size of the overall terrain texture / 256) ^ 2, OR by
	/// what you specify in a call to Terrain::AllocateTextureCells(). The latter option is only useful if you are
	/// managing the overall terrain texture for yourself rather than letting Demeter do it automatically.
	/// There is virtually no reason to create instances of this class for yourself unless you are overriding the way
	/// Terrain manages its cells. Generally, you will use the instances of this class that are provided for you by the
	/// managing Terrain object.
	class TextureCell
	{
	public:
		typedef std::vector<DetailTexture*>  DetailTextureArray_type;
		/// \brief Constructs an empty TextureCell.
		TextureCell();
		/// \brief Constructs a TextureCell that fits in the specified slot on the managing Terrain.
		/// \param index Specifies where in the Terrain's grid of TextureCells this cell should be placed.
		TextureCell(int index);
		 ~TextureCell();
		/// \brief Sets the overall Texture for this cell.
		void SetTexture(Texture * pTexture);
		/// \brief Adds a new detail texture layer to this cell.

		/** Add a detailed texture layer. Layers will be rendered in the same order they were added in fixed function pipeline.
		* the final result is the same for programmable pipeline though. 
		*/
		void AddDetail(DetailTexture * pDetailTexture);
		
		/** remove a detailed texture by local index. 
		*@return : true if removed. */
		bool RemoveDetail(int nIndex);
		/** remove all detailed textures. 
		*@return : true if removed. */
		bool RemoveAllDetail();
		
		/// \brief Binds the overall texture to a texture object. Used for rendering
		DeviceTexturePtr_type BindTexture();
		/// \brief Binds the overall mask to a texture object.Used for rendering
		DeviceTexturePtr_type BindMask();

		/// \brief Gets the number of DetailTexture layers that have been added.
		int GetNumberOfDetails();
		/** Binds the mask of the specified DetailTexture layer.
		* @param index: if this is -1, the base layer is binded
		*/
		DeviceTexturePtr_type BindMask(int index);
		/** Binds the detail texture of the specified DetailTexture layer. 
		* @param index: if this is -1, the base layer is binded
		*/
		DeviceTexturePtr_type BindDetail(int index);
		/// \brief Unbinds the overall texture and all DetailTexture masks and detail textures.
		void UnbindAll();
		/** Gets the specified DetailTexture layer.
		* @param index: if this is -1, the base layer is returned
		*/
		DetailTexture *GetDetail(int index);
		/** Gets the specified DetailTexture layer.
		* @param pTexture: if NULL, base layer is returned.
		*/
		DetailTexture *GetDetail(Texture * pTexture);
		/** same as GetDetail(), except that the local detail texture index is returned. 
		* @return: positive index if found. -1 if it is base layer, or -2 if not found. 
		*/
		int GetDetailIndex(Texture * pTexture);

		/** normalize the given texture mask layer, so that the alpha of the specified layer is unchanged(or scaled by fScale) and 
		* the sum of all alpha values of all mask layers at all pixels in the cell is 1. 
		* it is good practice to keep every pixel mask normalized, so that we do not get overly lighted terrain surface. 
		* @param index: if this is -1, the base layer is used
		* @param fScale: the base layer alpha will be scaled by this value. default to 1.0f, which means that the alpha value of the base reference layer is unchanged. 
		*  it can also be 0.f, which will simply delete the mask layer. 
		* @return: the number of layers affected. 
		*/
		int NormalizeMask(int index, float fScale=1.0f);

		/** Gets the overall base detail texture's texture object.
		* Obsoleted, use GetDetailBase() instead. 
		*/
		Texture *GetTexture();

		/** get the base detailed texture. */
		DetailTexture *GetDetailBase();
		
		/** optimize and then write to disk. */
		void WriteMask(CParaFile& file, Terrain * pTerrain);
		/** read mask file from disk. it will remove all existing mask file. */
		void ReadMask(CParaFile& file, Terrain * pTerrain);

		/** resize all texture mask width of all layers*/
		void ResizeTextureMaskWidth(int nWidth);

		/// \brief Performs Texture::FlipHorizontal on the overall texture and all DetailTexture layers.
		void FlipHorizontal();
		/// \brief Performs Texture::FlipVertical on the overall texture and all DetailTexture layers.
		void FlipVertical();
		/// \brief Gets the width of the specified detail texture in pixels.
		int GetDetailTextureImageWidth(int detailIndex);
		/// \brief Gets the height of the specified detail texture in pixels.
		int GetDetailTextureImageHeight(int detailIndex);
		/// \brief Gets the color depth of the specified detail texture.
		int GetDetailTextureImageBitsPerPixel(int detailIndex);
		/// \brief Gets the width of the specified detail texture mask in pixels.
		static int GetDetailMaskImageWidth(int detailIndex);
		/// \brief Gets the height of the specified detail texture mask in pixels.
		static int GetDetailMaskImageHeight(int detailIndex);
		/// \brief Gets the color depth of the specified detail texture mask.
		int GetDetailMaskImageBitsPerPixel(int detailIndex);
		
		/// \brief Gets the width of the overall texture in pixels.
		int GetBaseTextureImageWidth();
		/// \brief Gets the height of the overall texture in pixels.
		int GetBaseTextureImageHeight();
		/// \brief Gets the color depth of the overall texture.
		int GetBaseTextureImageBitsPerPixel();

		/** optimize layers in the cell, by removing those layers whose texture mask is constant 0. 
		* this function is usually called when saving texture cells to disk. */
		void OptimizeLayer();
	private:
		DetailTexture * m_pBaseLayer;
		DetailTextureArray_type m_DetailTextures;
		int m_Index;
	};
}
