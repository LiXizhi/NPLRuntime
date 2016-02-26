#pragma once
namespace ParaTerrain
{
	class TextureCell;
	class Terrain;
	class Texture;

	/// \brief An abstract factory for managing terrain textures.
	///
	/// TextureFactory allows applications to provide and manage their own terrain textures at runtime,
	/// rather than letting Demeter manage textures. This is most useful for
	/// procedural texture algorithms. This is an abstract class so to provide your own textures you
	/// must derive a concrete class from it. At runtime, you can tell a particular instance of Terrain
	/// to use your TextureFactory by calling the Terrain::SetTextureFactory() method and passing an instance
	/// of your derived TextureFactory. Subsequently, whenever that instance of Terrain renders a portion of the
	/// terrain, it will query your TextureFactory object for a texture to use for rendering. Since your factory
	/// is queried while the Terrain is rendering, it is vitally important that you architect your factory so that
	/// requests for textures are very fast. If your factory procedurally generates its textures, consider at least
	/// cacheing those textures so that subsequent calls are fast even if the first call is slow.
	/// The size of textures on the terrain surface is set by calling Terrain::AllocateTextureCells(). For example,
	/// if you specify that the Terrain has 8 textures along the x-axis and 8 textures along the y-axis, and your Terrain
	/// object is 5000 meters x 5000 meters, then each texture will be 625 meters x 625 meters in world units. Therefore, you
	/// might get away with a texture size of 256x256 texels in this scenario. For larger terrains, you would want either
	/// more texture resolution or more textures along the x and y axes.
	class TextureFactory
	{
	      public:
		/// \brief Queries your factory for a texture.
		///
		/// This method is called by a Terrain object when a texture has become visible
		/// and needs to be rendered. The index is a unique ID for the texture, and
		/// the other parameters are in world units.
		/// \param index A unique identifier for the texture.
		/// These identifiers are arbitrarily assigned by the Terrain object and are guaranteed to be unique and repeatable for each texture. You
		/// can use these identifiers as a way to cache textures, or you can ignore them and use your own cacheing scheme.
		/// \param originX The location, in world units, along the x-axis, that this texture begins. This information is useful if your factory
		/// is generating textures procedurally so you can do such things as query terrain elevations and surface normals in the texture's region
		/// in order to generate texel colors.
		/// \param originY The location, in world units, along the x-axis, that this texture begins. This information is useful if your factory
		/// is generating textures procedurally so you can do such things as query terrain elevations and surface normals in the texture's region
		/// in order to generate texel colors.
		/// \param width The width of the texture in world units. Remember this value in not a width in pixels or texels, but in real modelling units.
		/// Like the origin parameters, this information is most useful for procedural texture factories that want to know about physical layout of the texture
		/// on the terrain's surface.
		/// \param height The height of the texture in world units. Remember this value in not a height in pixels or texels, but in real modelling units.
		/// Like the origin parameters, this information is most useful for procedural texture factories that want to know about physical layout of the texture
		/// on the terrain's surface.
		/// \return A pointer to a Texture object provided by your implementation of this class. The calling Terrain object will use this Texture for rendering.
		virtual Texture * GetTexture(int index, float originX, float originY, float width, float height) = 0;
		/// \brief This method is now obsolete and should not be used.
		/// \deprecated This method is now obsolete and should not be used.
		virtual void GenerateTextureCell(Terrain * pTerrain, TextureCell * pCell, float originX, float originY, float width, float height);
		/// \brief Notifies your factory that a particular texture is no longer visible.

		/// Whether or not you actually unload this texture or take any other action at this time is entirely up to you.
		/// It may be perfectly appropriate to do nothing, or you may change the priority of the texture, or you may actually
		/// unload it, depending on how your factory's cacheing scheme is built.
		/// \param index The same unique identifier that was originally passed to the GetTexture() method.
		virtual void UnloadTexture(int index) = 0;
	};
}
