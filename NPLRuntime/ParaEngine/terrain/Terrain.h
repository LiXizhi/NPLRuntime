#pragma once
#include "IAttributeFields.h"
#include "TerrainBuffer.h"
#define INVALID_TEXTURE_ID		(1000000)

/** @mainpage The Global Landscape Terrain Reference in ParaEngine
*
* @section intro_sec Introduction
*
* The Global Landscape Terrain engine used in ParaEngine.
* The core ROAM algorithm of the terrain engine is based on code from the Demeter Terrain Visualization Library by Clay Fowler, 2002.
* I have ported from its original OpenGl implementation to DirectX. The terrain engine uses ROAM or Real-time Optimally Adapting Meshes algorithm. 

* A global terrain can either be single or latticed. The latter will load terrain as tiles 
* automatically when they are potentially visible.
* a global terrain's world coordinate (x,y) is defined as below:
* x>=0, y>=0. A single terrain usually has less than 512*512 vertices and is suitable for terrain size 
* smaller than 2000*2000meters. A latticed terrain has no theoretical upper limit of terrain size. In a latticed terrain,
* each terrain tile may be 500*500meters(possibly containing 128*128 elevation vertices) and there may be any number of such tiles in the latticed terrain. 
* For example, if 64*64 tiles are available on hard disk, then the total terrain size will be 32000*32000 meters.
* Internally, terrain tile is dynamically loaded into a sorted map. So there finally is an upper limit as for the total
* number of tiles loaded concurrently. But in most cases, only the 9 tiles around the camera position need to be
* loaded for rendering. One can specify how many tiles to be cached in memory. By default it is set to 18.

* For each single terrain tile, main or base texture is automatically broken in to texture tiles (each of size 256*256).
* A common texture is repeatedly(tiling) rendered on another layer on top of base texture. 
* However, the main or base texture does not has enough resolution and, in most cases, is used for rendering terrain in the far side.
* A matrix of high-resolution textures are used to rendering terrain surface near the camera position. For a 500*500 meter terrain.
* there is usually 16*16 number of high-resolution images covering the entire terrain. If the high-resolution texture is absent, the base texture 
* and the common texture is used instead.

The main game engine communicates with the terrain engine through the CGlobalTerrain Class. Global Terrain is loaded
from text based configuration files. There are two kinds of configuration files. See below
- Configuration file for a single terrain. 
- Configuration file for a lattice based terrain.

*
* @section format_singleconfig File Format: Single Terrain Configuration
*
A single terrain object can be loaded from a configuration file.
the following file format is expected from the file, pay attention to spaces between characters:

-- The script file to be executed after the terrain is loaded. It is optional.
OnLoadFile = script/loader.lua
-- the height map file,it can either be a gray-scale image or a Para-RAW elevation file.\n
Heightmapfile = texture/data/elevation.raw\n
-- The main or base texture. this is optional.\n
[MainTextureFile = texture/data/main.jpg]\n
-- The common texture. this is optional.\n
[CommonTextureFile = texture/data/dirt.jpg]\n
-- size of this terrain in meters\n
Size = 533.3333\n
-- the height value read from the height map will be scaled by this factor.\n
ElevScale = 1.0\n
-- whether the image is vertically swapped. It takes effects on gray-scale height map image\n
Swapvertical = 1\n
-- hight resolution radius, within which the terrain is drawn in detail.this is optional.\n
[HighResRadius = 30]\n
-- we will use a LOD block to approximate the terrain at its location, if the block is smaller than fDetailThreshold pixels when projected to the 2D screen. \n
DetailThreshold = 9.0\n
-- When doing LOD with the height map, the max block size must be smaller than this one. This will be (nMaxBlockSize*nMaxBlockSize) sized region on the height map\n
MaxBlockSize = 8\n
-- the matrix size of high-resolution textures.\n
DetailTextureMatrixSize = 64\n
-- the terrain holes specified in relative positions. There can be as any number of holes following the "hole". This section is optional.\n
hole\n
(10,10)\n
(200,300)\n
-- number of texture files used in the mask file .\n
NumOfDetailTextures = 3\n
texture/data/detail1.jpg\n
texture/data/detail2.jpg\n
texture/data/detail3.jpg\n
-- Format: (matrix.x, matrix.y)={nLayer0}[{nLayer1}][{nLayer2}][{nLayer3}] \n
-- nLayer{i} is the index into the detail texture list for the i th layer 
-- this section is optional and has been to mask file. see mask file section.\n
(0,0) = {1}{2}\n 
(2,4) = {1}\n
(3,5) = {2}\n
(3,15) = {0}{1}{2}\n

The following is an example:

-- this is a sample terrain file\n
-- author:  LiXizhi\n
OnLoadFile = script/loader.lua \n
Heightmapfile = terrain/data/LlanoElev.raw\n
MainTextureFile = terrain/data/LlanoTex.jpg\n
CommonTextureFile = terrain/data/dirt2.jpg\n
Size = 2048\n
ElevScale = 40\n
Swapvertical = 1\n
HighResRadius = 30.0\n
DetailThreshold = 10.0\n
MaxBlockSize = 64\n
DetailTextureMatrixSize = 64\n
hole\n
(10,10)\n
(200,300)\n
NumOfDetailTextures = 8\n
terrain/data/LlanoTex 1 1.bmp\n
terrain/data/LlanoTex 1 2.bmp\n
terrain/data/LlanoTex 2 1.bmp\n
terrain/data/LlanoTex 2 2.bmp\n
terrain/data/LlanoTex 2 3.bmp\n
terrain/data/LlanoTex 3 1.bmp\n
terrain/data/LlanoTex 3 2.jpg\n
terrain/data/LlanoTex 3 3.bmp\n
(1,1) = {0}\n
(1,2) = {1}\n
(2,1) = {2}\n
(2,2) = {3}\n
(2,3) = {4}\n
(3,1) = {5}\n
(3,2) = {6}\n
(3,3) = {7}\n
(0,0) = {6}\n
* \n
* @section format_maskfile File Format: Terrain detail texture mask file
*
* the name of the mask file is computed as below:\n
* the fold of terrain configure file + folder with terrain config file name+".mask"
* e.g. if the terrain configure file is "sample/config/sample_0_0.config.txt"
* then the mask files will be saved to "sample/config/sample_0_0.mask"
\n
mask file format:\n
DWORD nDetailTextures; // number of detailed textures \n
// following nDetailTextures number of texture string block each of the following format\n
{\n
	DWORD nStrLen; // length of the string, do not count the "\0"\n
	BYTE str[nStrLen]; the string without the trailing "\0"\n
}\n
DWORD nCellCount; // number of cells , usually 8x8=64\n
// following nCellCount block of texture cell data, each has the following format\n
{\n
	DWORD numDetails; // number of detail textures in the cell\n
	// following numDetails block of detail texture data, each has the following format\n
	{\n
		DWORD nSharedIndex;  // index into the texture set of the associated terrain. \n
		// if nSharedIndex is 0xffffffff, there are no data following this block, otherwise it\n
		// is a raw alpha mask in the following format.\n
		BYTE MaskData[256*256]; // raw mask data\n
	}\n
}\n
\n
* \n
* @section format_tiledconfig File Format: Latticed Terrain Configuration
*
Lattice Terrain Specification. (pay attention to spaces between characters).
It consists of the size of a terrain tile and a mapping from tile position to 
their single terrain configuration file. Please note the first line "type = lattice" is mandatory, otherwise
it will be mistreated as a single terrain configuration.
if there are multiple single configuration files for the same tile position, the first one will be used.
if there are multiple tile positions referring the same single terrain configuration file, copies of the same terrain will be created at these tile positions

type = lattice\n
TileSize = 533.333\n
-- tile0_0.txt refers to a single terrain configuration file.\n
(0,0) = terrain/data/tile0_0.txt\n
(2,2) = terrain/data/tile2_2.txt\n
(3,3) = terrain/data/tile3_3.txt\n
(4,2) = terrain/data/tile4_2.txt\n
* \n
* @section format_elevation File Format: Para-Raw Terrain Elevation file
*
The file name must end with ".raw", for example. in the single terrain configuration file:
Heightmapfile = terrain/data/LlanoElev.raw\n
the engine will assume that the elevation file is of RAW terrain elevation, otherwise it is treated as 
a gray scale image. In a gray scale image RGB(0.5,0.5,0.5) is of height 0. However, gray scale image has 
only 8-bits levels, which is insufficient for most application. That is why I implemented the RAW terrain 
Elevation file.
\n
The content of RAW elevation file is just a buffer of "float[nSize][nSize]", 
please note that nSize must be power of 2 or power of 2 plus 1. i.e. 
nSize = (2*...*2) | (2*...*2+1);
* \n
* @section change_log Changes
*
The following things are changed in ParaEngine during porting:
(1) The latticed terrain engine is fully implemented.
(2) RAW elevation file of (2*2*...*2+1) quad sized height map is supported.
(3) Latticed configuration file is implemented. 
(4) Single terrain configuration file is implemented.
(5) HAPI CreateWorld is added. Quad tree depth is now automatically calculated.
(6) Blank Terrain now defaults to a plane at y=0.
(7) WoW terrain exporter implemented.
(8) Global Report class support string value now and it internally uses map instead of list for all reporting items.
(9) Latticed terrain rendering is now reported.
(10) A detailed (none tessellation radius) is added. 
(11) Tiled (or latticed)terrain objects are dynamically loaded into memory and automatically cached. 
(12) No cracks between terrain tiles. 
(13) detail texture logic has been moved to the tessellation process.
(14) The latticed terrain cache uses an indexed map, which hashes (x,y) to an integer. x,y can be any integer(negative is OK)
(15) Gray scale image is still supported. RGB(0.5,0.5,0.5) is regarded as 0 height.
(16) ParaObject::SetPosition(), now y height is relative to world coordinate system.
(17) The biped object is only normalized to terrain surface along the facing direction.
(18) TODO:there is a bug with the free-mode camera
- high-res texture tile is implemented. High res texture will be be dynamically loaded and deleted due to the camera view location.
high-res texture usually has 16 or 32 times resolution than the low-res texture. The low-res texture is always persistent in 
system memory;whereas high resolution textures are dynamically loaded and deleted from the system memory(also video memory). 
A system cache for high-res textures is maintained. A hit count/miss is recorded with each cached texture. The total number of 
cached high-res textures in memory is a constant, such as 40. Old textures are unloaded according to its hit count in the last 
32 render frames, when the cache is full. There is a mechanism to prevent current used textures to be unloaded (in case that 
all cached textures are being used simultaneously). 
- it is important that high-res textures are also tilable, otherwise there will be noticeable seams between tiles.
- texture coordinates are no longer generated, but computed at runtime.
- chopped texture are no longer flipped, because I use texture coordinates larger than 1 for tiling.
- Internally file system uses "boost 1.32.0". Currently, there are two boost version used in ParaEngine. 
the file system is linked as static library. It automatically select the release or debug version to link.
The boost build system takes quite sometime to build. and I have put the boost 1.32.0 install directory to the boost folders. 
- TODO: I want to use bump mapping for high-res textures. But it seems lots of work for the artists. 
Please refer to the terrain document 1.14. Emboss bump mapping may be used. But a more general UV bump mapping (bump/light maps)
with reflective surfaces may be supported in future version.
- A terrain buffer is used which is only rebuilt when the camera view is changed. 
- The terrain rendering has been further optimized by using a sorted set for texture grouping.

- A sort by texture group is performed before rendering each texture cell. As a 
result, we have to use triangular lists instead of triangular strips or fans. 
- Texture and elevation file loader now uses D3DX texture functions.
- Multi-texture is used when rendering the terrain with common and detailed textures.
- Norm rendering with light can be enabled by defining _USE_NORMAL_ in config file.
the terrain can also look very good when rendering with fog even if norm is not enabled.
Norm consume much memory, so it is disabled by default.
- see the config file for MAX_NUM_VISIBLE_TERRAIN_TRIANGLES. i.e. the maximum number 
of triangles to be rendered each frame. there will be cracks on the terrain if the number
of needed terrain triangles exceed this number. typically you can set this value to a very
large value.
- See the render() function for more information. 
- use the default render state and the following for better effect
m_pRenderDevice->SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR );
m_pRenderDevice->SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR );
m_pRenderDevice->SetSamplerState( 1, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR );
m_pRenderDevice->SetSamplerState( 1, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR );
-  When loading the terrain the elevation scale means the distance between the highest terrain
position and the lowest. the lowest is always 0. 
- enable WIRED_TERRAIN in config to render terrain in wires
-  Ray tracing has been completely changed so that it is computed at runtime, no memory 
consumption is needed. It might be a little slower than the original Demeter implementation,
but is sufficiently fast for several times per frame calling rate.
-  Original Demeter uses the z axis as the height, yet the para engine uses y axis. When doing
tessellation and rendering, I secretly swaps y and z when building the vertex data and doing
point projection and view culling. In this way, the terrain engine and the other paraengine uses
their own coordinate system.
-  The original Demeter culling function has been removed, instead it now uses the para engine's own
camera culling functions
-  Texture set has not been implemented yet. Please see the comment in render()
-  InitDevice and DeleteDevice are added. All texture data are preserved in memory. So that reloading
is very fast when the user changes its D3D device object. However, isEditor() must be set to true.
-  extern d3d device pointer is used to access the device.
-  m_TessellateZWeight = 200000; I still do not understand very well about this weight value.
so i changed it to 0.
* \n
* @section copyright Copyright
*
 Files in the demeter namespace inherits the GNU License from Demeter Terrain Engine.
 Other portion of ParaEngine is subject to its own License.
* \n
* @section developer Developer
*
- Li, Xizhi: Developer of ParaEngine. Developer of this global landscape engine.
- Clay Fowler: Developer of the original Demeter Terrain Visualization Library, 2002.
*/
namespace ParaTerrain
{
	class ReloadMaskRequest;
	class CRegionLayer;
	class CTerrainRegions;

	/// \brief Enumerated values that are used in the TerrainLattice to 
	/// identify neighboring Terrain objects in a grid of Terrain objects.

	/// These values are used by TerrainLattice.
	enum DIRECTION
	{
		/// No offset
		DIR_CENTER = 0,
		/// Offset along the positive y axis
		DIR_NORTH = 1,
		/// Offset along positive x and y axes
		DIR_NORTHEAST = 2,
		/// Offset along positive x axis
		DIR_EAST = 3,
		/// Offset along positive x and negative y axes
		DIR_SOUTHEAST = 4,
		/// Offset along negative y axis
		DIR_SOUTH = 5,
		/// Offset along negative x and y axes
		DIR_SOUTHWEST = 6,
		/// Offset along negative x axis
		DIR_WEST = 7,
		/// Offset along negative x and positive y axes
		DIR_NORTHWEST = 8,
		/// Placeholder for uninitialized and maximum values
		DIR_INVALID = 9
	};

	/** Which part of the terrain is modified.*/
	enum TERRAIN_MODIFIED_BITS
	{
		MODIFIED_NONE = 0,

		// detailed terrain texture(with mask) has been modified.
		MODIFIED_TEXTURE = 0x1,
		// height map has modified
		MODIFIED_HEIGHTMAP = 0x1<<1,
		// configuration such as base texture, common file, holes, etc has been modified.
		MODIFIED_CONFIGURATION = 0x1<<2,
		// holes have been changed. this should mean the same thing as MODIFIED_CONFIGURATION
		MODIFIED_HOLES = 0x1<<3,
		// if static objects have been modified, so that we will need to update the on load script
		MODIFIED_ON_LOAD_SCRIPT = 0x1<<4,

		MODIFIED_ALL = 0xffff
	};

	class TerrainBlock;
	class Terrain;
	class Texture;
	class TextureFactory;
	class TextureSet;
	class TextureCell;
	class TriangleStrip;
	class TriangleFan;
	class TextureGenerator;
	class DetailTexture;
}

#include "TerrainCommon.h"
#include "TerrainException.h"
#include "Settings.h"
#include "Texture.h"
#include "ShapeAABB.h"
#include "TextureFactory.h"
#include "TextureCell.h"
#include "TextureSet.h"
#include "TextureGenerator.h"
#include "TerrainBlock.h"
#include "TriangleFan.h"
#include "TriangleStrip.h"
#include "TerrainLattice.h"
#include "TerrainVertex.h"
#include "DetailTextureFactory.h"
#include "IAttributeFields.h"
#include <vector>
#include <set>

namespace ParaEngine
{
	struct TextureEntity;
	class CShapeAABB;
	class CShapeFrustum;
	struct AssetFileEntry;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	class Terrain;
	class CDetailTextureFactory;
	
	/** terrain mask file callback data */
	class CTerrainMaskFileCallbackData : public boost::signals2::trackable
	{
	public:
		CTerrainMaskFileCallbackData(Terrain *pTerrain) : m_pTerrain(pTerrain){}

		Terrain * m_pTerrain;

		void OnMaskFileDownloaded(int nResult, AssetFileEntry* pAssetFileEntry);
		// void operator()(int nResult, AssetFileEntry* pAssetFileEntry);
	};

	enum TerrainInfoType
	{
		tit_none = 0,
		tit_walkRegion = 1,
		tit_walkWaypoint = 2,
	};


	/// This class represents a single, contiguous chunk of terrain and is the primary public interface to Demeter. 
	/// Most applications will create a single instance of this class to represent the terrain in the application, 
	/// but multiple instances can be used to stitch together a very large world comprised of multiple terrains using 
	/// the TerrainLattice class. The simplest way to use this class to make a new instance using the constructor that
	/// takes no parameters and then use an ElevationLoader and a TextureLoader to give it vertices and textures. Alternatively,
	/// you can use the constructor that takes arrays of floats and bytes if you wish to load your own elevation and texture data.
	class Terrain : public IAttributeFields
	{
	public:
		/// \brief Constructs a (mostly) empty terrain suitable for use by an ElevationLoader.
		Terrain();
		/// \brief Constructs a new terrain from raw elevation and image data in memory buffers.

		/// By using this constructor, you are specifying the layout of all of the vertices in the Terrain's mesh.
		/// These vertices are always a rectangular grid of 3D points at regularly spaced intervals. You are also specifying
		/// what the overall terrain texture looks like.
		/// \param pElevations An array of floating point values representing the elevation (z value) of each vertex in the terrain's mesh.
		/// The size of this array must be equal to elevWidth * elevHeight
		/// \param elevWidth The number of vertices along the x-axis (width) of the grid of vertices in pElevations.
		/// \param elevHeight The number of vertices along the y-axis (height) of the grid of vertices in pElevations.
		/// \param pTextureImage An array of unsigned char values representing the texture image that will be draped across the entire
		/// surface of the Terrain (this is the "overall" texture.) The array must contain 3 bytes per pixel - the red, green, and blue color values of the pixel respectively.
		/// It is assumed that the array does not contain any extra padding per image row. Therefore, if your image is 1024x1024 pixels, then
		/// this array should be 9,437,184 bytes in size. You can pass NULL for this parameter to build a Terrain with no texture.
		/// \param textureWidth The width of the texture image in pixels.
		/// \param textureHeight The height of the texture image in pixels.
		/// \param pDetailTextureImage This is similar to the pTextureImage parameter, but instead of describing the overall terrain texture, this buffer
		/// buffer describes the "common" texture. See Terrain::SetCommonTexture() for details. You can pass NULL for this parameter to build a Terrain with no common texture.
		/// \param detailWidth The width of the detail texture image in pixels.
		/// \param detailHeight The height of the detail texture image in pixels.
		/// \param vertexSpacing The distance, in world modeling units, between vertices in the Terrain's mesh. 
		/// This distance is constant in both the horizontal and vertical directions. For example, if your pass in an array
		/// of 1024x1024 vertices for the pElevations and elevWidth and elevHeight parameters, and pass a value of 10.0 for the
		/// vertexSpacing parameter, then the resulting Terrain will be 10240 x 10240 world units in size.
		/// \param elevationScale A scaling factor that is applied to all of the values in the pElevations array. Most
		/// applications will pass a 1.0 for this value since presumably pElevations was built by the application.
		/// \param maxNumTriangles The maximum number of triangles that the Terrain is allowed to tessellate to when rendering.
		/// This number should be relatively high and is chosen based on what you know about the geometry of your terrain. For example,
		/// if your terrain is typical then a value of 40000 works well, but if you know your Terrain contains numerous canyons and
		/// sharp cliffs that will tessellate to lots of triangles, you might need to raise this value to something on the order of 60000.
		/// The higher this number, the more memory the Terrain will use for tessellation.
		/// \param offsetX Horizontal translation of this Terrain in world units. See Terrain::SetOffserX().
		/// \param offsetY Vertical translation of this Terrain in world units. See Terrain::SetOffserY().
		Terrain(const float *pElevations, int elevWidth, int elevHeight, const uint8 * pTextureImage, int textureWidth, int textureHeight, const uint8 * pDetailTextureImage, int detailWidth, int detailHeight, float vertexSpacing, float elevationScale, int maxNumTriangles, float offsetX = 0.0f, float offsetY = 0.0f);

		/// By using this constructor, you are specifying the layout of all of the vertices in the Terrain's mesh. This version
		/// of the constructor creates a Terrain that is a flat plane with elevations of zero at all vertices. Otherwise, it is identical
		/// to the constructor that takes pElevations.
		/// \param widthVertices The number of vertices along the x-axis (width) of the grid of vertices in the mesh.
		/// \param heightVertices The number of vertices along the y-axis (height) of the grid of vertices in the mesh.
		/// \param vertexSpacing The distance, in world modeling units, between vertices in the Terrain's mesh. 
		/// This distance is constant in both the horizontal and vertical directions. For example, if your pass in an array
		/// of 1024x1024 vertices for the pElevations and elevWidth and elevHeight parameters, and pass a value of 10.0 for the
		/// vertexSpacing parameter, then the resulting Terrain will be 10240 x 10240 world units in size.
		/// \param maxNumTriangles The maximum number of triangles that the Terrain is allowed to tessellate to when rendering.
		/// This number should be relatively high and is chosen based on what you know about the geometry of your terrain. For example,
		/// if your terrain is typical then a value of 40000 works well, but if you know your Terrain contains numerous canyons and
		/// sharp cliffs that will tessellate to lots of triangles, you might need to raise this value to something on the order of 60000.
		/// The higher this number, the more memory the Terrain will use for tessellation.
		  Terrain(int widthVertices, int heightVertices, float vertexSpacing, int maxNumTriangles);
		/// \brief Constructs a (mostly) empty terrain suitable for use by an ElevationLoader.
		/// \param maxNumTriangles The maximum number of triangles that the Terrain is allowed to tessellate to when rendering.
		/// This number should be relatively high and is chosen based on what you know about the geometry of your terrain. For example,
		/// if your terrain is typical then a value of 40000 works well, but if you know your Terrain contains numerous canyons and
		/// sharp cliffs that will tessellate to lots of triangles, you might need to raise this value to something on the order of 60000.
		/// The higher this number, the more memory the Terrain will use for tessellation.
		/// \param offsetX Horizontal translation of this Terrain in world units. See Terrain::SetOffserX().
		/// \param offsetY Vertical translation of this Terrain in world units. See Terrain::SetOffserY().
		  Terrain(int maxNumTriangles, float offsetX, float offsetY);
		 ~Terrain();
	public:
		 //////////////////////////////////////////////////////////////////////////
		 // implementation of IAttributeFields

		 /** attribute class ID should be identical, unless one knows how overriding rules work.*/
		 virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CTerrainObject;}
		 /** a static string, describing the attribute class object's name */
		 virtual const char* GetAttributeClassName(){static const char name[] = "CTerrain"; return name;}
		 /** a static string, describing the attribute class object */
		 virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		 /** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		 virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		 /**
		 * Invoke an (external) editor for a given field. This is usually for NPL script field
		 * @param nFieldID : field ID
		 * @param sParameters : the parameter passed to the editor
		 * @return true if editor is invoked, false if failed or field has no editor. 
		 */
		 virtual bool InvokeEditor(int nFieldID, const string& sParameters);
		 
		 ATTRIBUTE_METHOD1(Terrain, IsModified_s, bool*)		{*p1 = cls->IsModified(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetModified_s, bool)		{cls->SetModified(p1); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, IsEmpty_s, bool*)		{*p1 = cls->IsEmpty(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetSize_s, float*)		{*p1 = cls->GetWidth(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetOnloadScript_s, const char**)		{*p1 = cls->GetOnloadScript().c_str(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetTerrainElevFile_s, const char**)		{*p1 = cls->GetTerrainElevFile().c_str(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetTerrainConfigFile_s, const char**)		{*p1 = cls->GetTerrainConfigFile().c_str(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetTerrainBaseTextureFile_s, const char**)		{*p1 = cls->GetTerrainBaseTextureFile().c_str(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetTerrainBaseTextureFile_s, const char*)	{cls->SetTerrainBaseTextureFile(p1); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetTerrainCommonTextureFile_s, const char**)		{*p1 = cls->GetTerrainCommonTextureFile().c_str(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetTerrainCommonTextureFile_s, const char*)	{cls->SetTerrainCommonTextureFile(p1); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetNumOfRegions_s, int*)		{*p1 = cls->GetNumOfRegions(); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetCurrentRegionIndex_s, int*)		{*p1 = cls->GetCurrentRegionIndex(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetCurrentRegionIndex_s, int)		{cls->SetCurrentRegionIndex(p1); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetCurrentRegionName_s, const char**)		{*p1 = cls->GetCurrentRegionName().c_str(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetCurrentRegionName_s, const char*)	{cls->SetCurrentRegionName(p1); return S_OK;}

		 ATTRIBUTE_METHOD1(Terrain, GetCurrentRegionFilepath_s, const char**)		{*p1 = cls->GetCurrentRegionFilepath().c_str(); return S_OK;}
		 ATTRIBUTE_METHOD1(Terrain, SetCurrentRegionFilepath_s, const char*)	{cls->SetCurrentRegionFilepath(p1); return S_OK;}

		 /**
		 * clean up all 
		 */
		 void Cleanup();

		/** whether the terrain geometry, texture has been modified. If any of them has been modified, this function will return true. */
		bool IsModified();
		/** return true if any part specified in dwModifiedBits has been modified. see TERRAIN_MODIFIED_BITS. 
		* @param dwModifiedBits: this is any combination of TERRAIN_MODIFIED_BITS. */
		bool IsModified(DWORD dwModifiedBits);
		/** Turn on/off the specified flag indicating whether the terrain has been modified or not.
		* @param bIsModified: true to turn on the bits specified in dwModifiedBits, false otherwise.
		* @param dwModifiedBits: this is any combination of TERRAIN_MODIFIED_BITS. */
		void SetModified(bool bIsModified, DWORD dwModifiedBits);

		/** whether the terrain is a fake terrain without vertices. */
		bool IsEmpty();

		/** set the terrain modified to true for all parts of the terrain.*/
		void SetModified(bool bIsModified);

		/// this function currently does nothing
		void InitDeviceObjects();
		/// must call this function when device is lost.
		void DeleteDeviceObjects();
		/// must call this function when device is invalid.
		void InvalidateDeviceObjects();

		void RendererRecreated();

		/**
		* Breaks the Terrain down into triangles strips.
		* Based on the current viewing parameters, this method breaks the terrain down into a 
		* visually optimum set of triangles that can be rendered. Normally, an application will never 
		* call this method directly, but will instead call the method ModelViewMatrixChanged() to allow 
		* Demeter to take care of this automatically.
		*/
		int Tessellate();
		/**
		* repair cracks during tessellation.
		* the triangle strips built from Tessellate() will has cracks between neighboring triangles of different size.
		* in these cases, the bigger triangle strips are replaced with triangle fans, so that there will be no cracks.
		* Here "cracks" means that the surface is not a continuous surface. 
		* @see Tessellate()
		*/
		void RepairCracks();
		/**
		* Rebuild the rendering buffer from triangle strips and fans generated from Tessellate() and RepairCracks().
		* @see Tessellate()
		* @see RepairCracks()
		*/
		void RebuildRenderBuffer();
		/**
		* Render the terrain from Render Buffer. 
		* Applications should always call ModelViewMatrixChanged() at least once prior to calling 
		* Render(), so that Demeter will have a chance to tessellate the terrain.
		* @see RebuildRenderBuffer()
		*/
		void Render();


		/** get the rendering report string for debugging purposes */
		string GetRenderReport();

		/// \brief Sets the "detail threshold", which controls how much Demeter simplifies the terrain. 

		/// Higher thresholds will increase performance but make the terrain look worse, while lower 
		/// thresholds will reduce performance and increase visual quality. Extremely high thresholds 
		/// can cause significant visual artifacts.
		/// @param threshold An error metric, measured in screen pixels. Any terrain blocks smaller than this threshold will be simplified.
		/// Currently this is a squared value.i.e. If it is 7 pixels, then the value should be 49.
		void SetDetailThreshold(float threshold);
		/// \brief Returns the "detail threshold."
		float GetDetailThreshold() const;
		/// \brief Sets the maximum size of blocks that can be simplified when the terrain is tessellated.

		/// This setting can be used by applications that allow much of the terrain to be visible from above, 
		/// such as flight simulators, to prevent oversimplification of terrain in the distance. 
		/// Setting this value too low will adversely affect performance.
		/// \param stride Specifies the number of vertices along one edge of the block (blocks are always square.)
		void SetMaximumVisibleBlockSize(int stride);
		int GetMaximumVisibleBlockSize();
		/// \brief Returns the width of the terrain in vertices (this is the count of vertices along the world's x-axis.)
		int GetWidthVertices() const;
		/// \brief Returns the height of the terrain in vertices (this is the count of vertices along the world's y-axis.)
		int GetHeightVertices() const;
		/// \brief Returns the width of the terrain in world units (this is the length of the terrain along the world's x-axis.)
		float GetWidth() const;
		/// \brief Returns the height of the terrain in world units (this is the length of the terrain along the world's y-axis.)
		float GetHeight() const;
		/// \brief Returns the number of real units between vertices in the terrain's mesh.
		float GetVertexSpacing() const;
		/// \brief Returns the elevation (z-coordinate) in real units of the specified point on the terrain.
		/// if the specified point is inside a hole, we will return (-INFINITY)
		/// \param x The x location of the point on the Terrain's surface in local units.
		/// \param y The y location of the point on the Terrain's surface in local units.
		float GetElevation(float x, float y) const;
		/// \brief Returns the elevation (z-coordinate) in real units of the specified point on the terrain.
		/// if the specified point is inside a hole, we will return (-INFINITY)
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		float GetElevationW(float x, float y) const;

		/** get value of a given terrain region layer 
		* @param x The x location of the point on the Terrain's surface in local units.
		* @param y The y location of the point on the Terrain's surface in local units.
		*/
		DWORD GetRegionValue(const string& sLayerName, float x, float y);

		/** get value of a given terrain region layer 
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		DWORD GetRegionValueW(const string& sLayerName, float x, float y);


		/// \brief Returns the elevation (z-coordinate) in real units of the specified terrain vertex.
		/// \param index The index of the vertex in the Terrain's mesh to get the elevation of.
		float GetElevation(int index) const;
		/// \brief Returns the surface normal of the terrain at the specified point.
		/// \param x The x location of the point on the Terrain's surface in local units.
		/// \param y The y location of the point on the Terrain's surface in local units.
		/// \param normalX Gets filled with the surface normal x component
		/// \param normalY Gets filled with the surface normal y component
		/// \param normalZ Gets filled with the surface normal z component
		void GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ) const;
		/// \brief Returns the surface normal of the terrain at the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// \param normalX Gets filled with the surface normal x component
		/// \param normalY Gets filled with the surface normal y component
		/// \param normalZ Gets filled with the surface normal z component
		void GetNormalW(float x, float y, float &normalX, float &normalY, float &normalZ) const;
		/// \brief Returns the elevation (z-coordinate) in real units of the highest point on the terrain.
		float GetMaxElevation() const;
		/// \brief Returns the index of the vertex closest to the specified point.
		/// \param x The x location of the point on the Terrain's surface in local units.
		/// \param y The y location of the point on the Terrain's surface in local units.
		int GetVertex(float x, float y) const;
		/// \brief Returns the index of the vertex closest to the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		int GetVertexW(float x, float y) const;
		/// \brief Returns the elevation (z-coordinate) in real units of the specified vertex on the terrain.
		/// \param index The index of the vertex in the Terrain's mesh to get the elevation of.
		float GetVertexElevation(int index) const;
		/// \brief Sets the elevation (z-coordinate) in real units of the specified vertex on the terrain.
		/// \param index The index of the vertex in the Terrain's mesh to set the elevation of.
		/// \param newElevation The new z-value (elevation) of the vertex to be set.
		/// \param recalculate_geometry Specified whether or not terrain block bounding boxes should be updated as a result of this vertex changed. You would pass false here only when you are sequentially setting many vertices, in which case you would pass true only on the last vertex changed (this prevents redundant recalculation for each vertex changed.)
		void SetVertexElevation(int index, float newElevation, bool recalculate_geometry = true);
		/// \brief Sets the elevation (z-coordinate) in real units of the nearest vertex to the specified point.
		/// \param x The x location of the point on the Terrain's surface in local units.
		/// \param y The y location of the point on the Terrain's surface in local units.
		/// \param newElevation The new z-value (elevation) of the vertex to be set.
		void SetVertexElevation(float x, float y, float newElevation);
		
		/// set a new terrain hole at the specified location.
		/// Currently, we will allow user to dynamically dig holes in terrain. After calling this function,
		/// the user must manually Call UpdateHoles() to inform that the holes in the terrain has been updated.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// @see UpdateHoles();
		void SetHole(float x, float y, bool bIsHold);

		/// set a new terrain hole at the specified local location.
		/// Currently, we will allow user to dynamically dig holes in terrain. After calling this function,
		/// the user must manually Call UpdateHoles() to inform that the holes in the terrain has been updated.
		/// \param x: x component of the hole. it is relative to this terrain tile, not to the global world.
		/// \param y: x component of the hole. it is relative to this terrain tile, not to the global world.
		/// @see SetHole();
		void SetHoleLocal(float x, float y, bool bIsHold);

		/// call this function after you have set new holes in the terrain. 
		void UpdateHoles();

		/// this value can only be a power of 2, such as 2, 4, 8, etc. default value is 2 or 4. 
		/// it denotes how many successive vertex in the height map represent a hole
		/// the larger the value is the larger the hole vertex is.
		void SetHoleScale(int nHoleScale);

		/// Whether the terrain contains a hole at the specified location. 
		/// Currently, we allow user to load a low resolution hole maps at the beginning of terrain creation.
		/// \param x The x location of the point on the Terrain's surface in local units.
		/// \param y The y location of the point on the Terrain's surface in local units.
		/// @return: true if the position specified by (x,y) is inside a terrain hole
		bool IsHole(float x, float y);
		/// Whether the terrain contains a hole at the specified location. 
		/// Currently, we allow user to load a low resolution hole maps at the beginning of terrain creation.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// @return: true if the position specified by (x,y) is inside a terrain hole
		bool IsHoleW(float x, float y);

		/// create hole map from terrain hole map.
		/// @param pHoleData: if this value is NULL, a blank hole map without any holes will be created.
		/// Otherwise, it is interpreted as an array of X*X BYTE values. If the BYTE value is 1, it is interpreted as a hole, otherwise it is false. 
		void CreateHoleMap(BYTE* pHoleData=NULL, int nLength = 0);

		/// \brief Recalculates bounding box for the quad spanning from vertex 1 to vertex 2
		/// \param index1 The index of the top-left vertex
		/// \param index2 The index of the bottom-right vertex
		void RecalcGeometry(int index1, int index2);
		/** recalculate the entire terrain. It is actually fast. */
		void RecalcGeometry();
		/// \brief Recalculates the normal vector of the vertex closes to the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		void RecalcNormal(float x, float y);
		/// \brief Recalculates the normal vector of the specified vertex.
		/// \param vertexIndex The index of the vertex in the Terrain's mesh.
		void RecalcNormal(int vertexIndex);
		/// \brief Replaces all of the Terrain's vertices with a new set of vertices.
		/// \param pElevations An array of floating point values representing the elevation (z value) of each vertex in the terrain's mesh.
		/// The size of this array must be equal to elevWidth * elevHeight
		/// \param elevWidth The number of vertices along the x-axis (width) of the grid of vertices in pElevations.
		/// \param elevHeight The number of vertices along the y-axis (height) of the grid of vertices in pElevations.
		/// \param fTerrainSize: size of this terrain
		/// of 1024x1024 vertices for the pElevations and elevWidth and elevHeight parameters, and pass a value of 10.0 for the
		/// vertexSpacing parameter, then the resulting Terrain will be 10240 x 10240 world units in size.
		/// \param elevationScale A scaling factor that is applied to all of the values in the pElevations array. Most
		/// applications will pass a 1.0 for this value since presumably pElevations was built by the application.
		void SetAllElevations(const float *pElevations, int elevWidth, int elevHeight, float fTerrainSize, float elevationScale = 1.0f);
		/// \brief Returns the total number of vertices in the terrain's mesh.
		int GetNumberOfVertices() const;
		/// \brief Returns the width (in vertices) of the terrain's texture cells. See the TextureCell class.
		float GetTextureTileWidth() const;
		/// \brief Returns the height (in vertices) of the terrain's texture cells. See the TextureCell class.
		float GetTextureTileHeight() const;
		/// \brief Returns the number of texture cells along the terrain's x-axis. See the TextureCell class.
		int GetNumberOfTextureTilesWidth() const;
		/// \brief Returns the number of texture cells along the terrain's y-axis. See the TextureCell class.
		int GetNumberOfTextureTilesHeight() const;
		/// \brief Applies the specified graphics image as a texture to the terrain. 

		/// This is done by breaking the specified image up into smaller textures of 256x256 called 
		/// TextureCells and mapping these contiguously onto the terrain's surface.
		/// Textures are automatically applied by TextureLoaders and by the unsigned char
		/// array constructors, so use of this method is entirely optional.
		/// \param pTextureImage An array of unsigned char values representing the texture image that will be draped across the entire
		/// surface of the Terrain (this is the "overall" texture.) The array must contain 3 bytes per pixel - the red, green, and blue color values of the pixel respectively.
		/// It is assumed that the array does not contain any extra padding per image row. Therefore, if your image is 1024x1024 pixels, then
		/// this array should be 9,437,184 bytes in size. You can pass NULL for this parameter to build a Terrain with no texture.
		/// \param width The width of the texture image in pixels.
		/// \param height The height of the texture image in pixels.
		bool SetTexture(const uint8 * pTextureImage, int width, int height);
		/**
		* Set the base texture to texture cells. The base texture will be chopped according to the texture cell configuration.
		* e.g. if the texture cell is 8x8, then the base texture is chopped to 64 small textures and set to each texture cell.
		* @param numTextureCellsX: The number of cells in the X direction of this Terrain's grid of TextureCells.
		* @param numTextureCellsY: The number of cells in the Y direction of this Terrain's grid of TextureCells.
		*/
		bool SetBaseTexture(const string& filename, int numTextureCellsX=8, int numTextureCellsY=8);
		/// \brief Uses the specified graphics image to apply a "common" texture to the entire surface of 
		/// the terrain. 

		/// A common texture is repeated across the entire terrain and is blended with the 
		/// terrain's overall texture (if blending is supported by the user's hardware.) 
		/// This is used to provide a texture to give the ground some definition when the camera is 
		/// close to the ground, but is an inferior to alternative to the use of specific DetailTexture instances.
		/// \param pImage An array of unsigned char values representing the texture image that will be draped across the entire
		/// surface of the Terrain (this is the "overall" texture.) The array must contain 3 bytes per pixel - the red, green, and blue color values of the pixel respectively.
		/// It is assumed that the array does not contain any extra padding per image row. Therefore, if your image is 1024x1024 pixels, then
		/// this array should be 9,437,184 bytes in size. You can pass NULL for this parameter to build a Terrain with no texture.
		/// \param width The width of the texture image in pixels.
		/// \param height The height of the texture image in pixels.
		bool SetCommonTexture(const uint8 * pImage, int width, int height);
		/// \brief Uses the specified Texture to apply a "common" texture to the entire surface of 
		/// the terrain. 

		/**
		* A common texture is repeated across the entire terrain and is blended with the 
		* terrain's overall texture (if blending is supported by the user's hardware.) 
		* This is used to provide a texture to give the ground some definition when the camera is 
		* close to the ground, but is an inferior to alternative to the use of specific DetailTexture instances.
		*/
		void SetCommonTexture(const char* fileName);

		/// A common texture is repeated across the entire terrain and is blended with the 
		/// terrain's overall texture (if blending is supported by the user's hardware.) 
		/// This is used to provide a texture to give the ground some definition when the camera is 
		/// close to the ground, but is an inferior to alternative to the use of specific DetailTexture instances.
		/// \param pTexture A Texture object to use as the common texture.
		void SetCommonTexture(Texture * pTexture);
		/// \brief Returns the Texture object representing the common texture that has been applied to the Terrain, if any.
		Texture *GetCommonTexture() const;
		
		/// \brief Notifies Demeter that OpenGL's modelview matrix has been modified, allowing Demeter 
		/// to tessellate the terrain based on the new modelview matrix. 
		/// It is IMPERATIVE that his method be called every time the modelview matrix is changed, even 
		/// if this is in every single rendering cycle of the application. Based on the current viewing 
		/// parameters, the terrain will be broken down into a visually optimum set of triangles that can 
		/// be rendered.
		int ModelViewMatrixChanged();

		/** Indicates whether or not the specified cuboid is inside of the viewing frustum 
		* (as defined at the previous call to ModelViewMatrixChanged().)
		* @param cuboid : The box to test. make sure this is in local terrain coordinate system. 
		* @return: 
		* 2 requires more testing(meaning either intersecting)
		* 1 is fast accept(meaning the box is fully inside the frustum)
		* 0 is fast reject, 
		*/
		int CuboidInFrustum(const CShapeBox & cuboid) const;
		/**
		* return true if the cuboid intersect with the Fog End radius
		* @param cuboid: make sure this is in local terrain coordinate system. 
		*/
		bool CuboidInFogRadius(const CShapeBox & cuboid) const;

		/** return which side the cuboid is on */
		Plane::Side GetCuboidFogSide(const CShapeBox & cuboid) const;

		/** convert the box to render coordinate system */
		void BoxToRenderCoordinate(CShapeBox & cuboid) const;

		/** get a length matrics from the box to the current eye position. */
		float GetBoxToEyeMatrics(const CShapeBox & cuboid) const;

		/// \brief Casts a ray from the specified point, in the specified direction, and calculates the ray's point of intersection with the terrain. 
		/// This method makes use of the terrain's quad tree to optimize the ray-tracing.
		/// if the ray intersect with holes in the terrain, a negative value will be returned as well.
		/// IMPORTANT: it is assumed that the ray is specified in world coordinate system
		/// \return The distance from the starting point to the intersection. A negative value is returned if the ray misses the Terrain.
		/// \param startX The starting point of the ray.
		/// \param startY The starting point of the ray.
		/// \param startZ The starting point of the ray.
		/// \param dirX The direction of the ray - this vector should be normalized.
		/// \param dirY The direction of the ray - this vector should be normalized.
		/// \param dirZ The direction of the ray - this vector should be normalized.
		/// \param intersectX Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectY Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectZ Filled with the intersection point of this ray with the Terrain surface.
		float IntersectRayW(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance = INFINITY);

		/// \brief Casts a ray from the specified point, in the specified direction, and calculates the ray's point of intersection with the terrain. 
		/// This method makes use of the terrain's quad tree to optimize the ray-tracing.
		/// if the ray intersect with holes in the terrain, a negative value will be returned as well.
		/// IMPORTANT: it is assumed that the ray is specified in local coordinate system
		/// \return The distance from the starting point to the intersection. A negative value is returned if the ray misses the Terrain.
		/// \param startX The starting point of the ray.
		/// \param startY The starting point of the ray.
		/// \param startZ The starting point of the ray.
		/// \param dirX The direction of the ray - this vector should be normalized.
		/// \param dirY The direction of the ray - this vector should be normalized.
		/// \param dirZ The direction of the ray - this vector should be normalized.
		/// \param intersectX Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectY Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectZ Filled with the intersection point of this ray with the Terrain surface.
		float IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance = INFINITY);
		/// \brief An overload of IntersectRay that also provides information about the texture coordinates of the intersected point in addition the position of the point.
		/// if the ray intersect with holes in the terrain, a negative value will be returned as well.
		/// IMPORTANT: it is assumed that the ray is specified in local coordinate system
		/// \return The distance from the starting point to the intersection. A negative value is returned if the ray misses the Terrain.
		/// \param startX The starting point of the ray.
		/// \param startY The starting point of the ray.
		/// \param startZ The starting point of the ray.
		/// \param dirX The direction of the ray - this vector should be normalized.
		/// \param dirY The direction of the ray - this vector should be normalized.
		/// \param dirZ The direction of the ray - this vector should be normalized.
		/// \param intersectX Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectY Filled with the intersection point of this ray with the Terrain surface.
		/// \param intersectZ Filled with the intersection point of this ray with the Terrain surface.
		/// \param textureCellX Filled with the X position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param textureCellY Filled with the Y position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param texU Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		/// \param texV Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		float IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, int &textureCellX, int &textureCellY, float &texU, float &texV, float fMaxDistance = INFINITY);
		/// \brief Quickly find the 3D point on the terrain where a given point on the screen is (for example, for mouse picking.) 

		/// This is cheaper and faster than using the ray tracing methods, but far less accurate.
		/// The accuracy of this method depends upon the z-buffer depth and the screen resolution.
		/// \param screenX The X position of the pixel on the screen that was "clicked"
		/// \param screenY the Y position of the pixel on the screen that was "clicked" - you will often
		/// to invert this value since the OpenGL screen Y-axis runs opposite that of most window systems.
		/// For example, you may need to pass <Code>Settings::GetInstance()->GetScreenHeight() - mouseY</Code> instead of just passing <Code>mouseY</Code>.
		/// \param pickedX Filled with the picked point on Terrain surface.
		/// \param pickedY Filled with the picked point on Terrain surface.
		/// \param pickedZ Filled with the picked point on Terrain surface.
		/// \return Returns false is the picked point is not on the Terrain.
		bool Pick(int screenX, int screenY, float &pickedX, float &pickedY, float &pickedZ) const;
		/// \brief Quickly find the 3D point on the terrain where a given point on the screen is (for example, for mouse picking.) 

		/// This is cheaper and faster than using the ray tracing methods, but far less accurate.
		/// The accuracy of this method depends upon the z-buffer depth and the screen resolution.
		/// \param screenX The X position of the pixel on the screen that was "clicked"
		/// \param screenY the Y position of the pixel on the screen that was "clicked" - you will often
		/// to invert this value since the OpenGL screen Y-axis runs opposite that of most window systems.
		/// For example, you may need to pass <Code>Settings::GetInstance()->GetScreenHeight() - mouseY</Code> instead of just passing <Code>mouseY</Code>.
		/// \param pickedX Filled with the picked point on Terrain surface.
		/// \param pickedY Filled with the picked point on Terrain surface.
		/// \param pickedZ Filled with the picked point on Terrain surface.
		/// \param textureCellX Filled with the X position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param textureCellY Filled with the Y position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param texU Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		/// \param texV Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		/// \return Returns false is the picked point is not on the Terrain.
		bool Pick(int screenX, int screenY, float &pickedX, float &pickedY, float &pickedZ, int &textureCellX, int &textureCellY, float &texU, float &texV) const;
		/// \brief Returns the texture coordinates of a given point on the terrain in world coordinates.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// \param textureCellX Filled with the X position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param textureCellY Filled with the Y position of the intersected TextureCell object in the Terrain's grid of TextureCells.
		/// \param texU Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		/// \param texV Filled with the coordinate within the TextureCell where the intersection point is (range from 0.0 to 1.0.)
		void GetTextureCoordinates(float x, float y, int &textureCellX, int &textureCellY, float &texU, float &texV) const;
		/// \brief Paints a "splat" of the specified shared texture at the specified position on the terrain in world coordinates.

		/// See the TextureSet class for details on how to get a detailTextureIndex for painting.
		/// \param detailTexture The shared index of a Texture object that is part of this Terrain object's TextureSet.
		/// \param brushRadius The width of the brush to paint with in DetailTexture layer pixels. There are typically 256 of these pixels across a TextureCell.
		/// \param brushIntensity The intensity with which to paint, ranging from 0.0 to 1.0. This determines how opaque the painted "splat" will be.
		/// \param maxIntensity The maximum intensity of the painted splat, ranging from 0.0 to 1.0, accounting for mixing with a splat that may already reside on the surface.
		/// \param erase Specifies whether to add the splat to the surface or remove existing splat texels already on the surface (pass false to paint and pass true to erase existing splats)
		/// \param x The x location of the point to paint on the Terrain's surface in world units.
		/// \param y The y location of the point to paint on the Terrain's surface in world units.
		void Paint(ParaEngine::TextureEntity* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y);
		void Paint(int detailTextureIndex, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y);
		/// \brief Indicates whether or not OpenGL multitexture extensions are available from the OpenGL driver that this terrain instance is currently rendering against (as determined at the time the terrain instance was first constructed.)
		bool IsMultiTextureSupported() const;
		/// \brief Sets this Terrain object's location within a lattice of Terrains that are being managed by a TerrainLattice object.
		/// \param x The position within a TerrainLattice object's grid of Terrains that this object will fill.
		/// \param y The position within a TerrainLattice object's grid of Terrains that this object will fill.
		void SetLatticePosition(int x, int y);
		/// \brief Gets this Terrain object's location within a lattice of Terrains that are being managed by a TerrainLattice object.
		/// \param x Filled with the position within a TerrainLattice object's grid of Terrains that this object fills.
		/// \param y Filled with the position within a TerrainLattice object's grid of Terrains that this object fills.
		void GetLatticePosition(int &x, int &y);
		/// \brief Installs an application-provided TextureFactory, disabling the default one supplied by 
		/// Demeter. This allows applications to manage textures for themselves.
		/// \param pFactory A pointer to your TextureFactory object that will serve textures to this instance of Terrain.
		void SetTextureFactory(TextureFactory * pFactory);
		/// \brief Gets the TextureCell at the specified position within this Terrain's grid of cells.
		/// \param index The index of the TextureCell within this Terrain's grid of TextureCells.
		TextureCell * GetTextureCell(int index);
		/**
		* Get the texture cell by world coordinate position. 
		*/
		TextureCell * GetTextureCellW(float x, float y);
		/// \brief Gets the TextureCell at the specified position within this Terrain's grid of cells.                                  
		/// \param textureCellX The x position of the TextureCell within this Terrain's grid of TextureCells.
		/// \param textureCellY The y position of the TextureCell within this Terrain's grid of TextureCells.
		TextureCell * GetTextureCell(int textureCellX, int textureCellY);
		/// \brief Writes a binary "surface" file for fine-grained detail textures. 
		/// \deprecated This file format has been replaced by the Demeter Texture Editor XML format.
		/// \param szFilename A filename to write to. The media path will be prepended to this file. See Settings::SetMediaPath().
		void Write(char *szFilename);
		/// \brief Reads a binary "surface" file for fine-grained detail textures. NOTE: the format of this file will be changed soon.
		/// \deprecated This file format has been replaced by the Demeter Texture Editor XML format.
		/// \param szFilename A filename to read from. The media path will be prepended to this file. See Settings::SetMediaPath().
		void Read(char *szFilename);
		/// \brief Gets the TextureSet that represents this Terrain object's pool of shared Textures.
		TextureSet *GetTextureSet();
		/// \brief Sets the TextureSet that represents this Terrain object's pool of shared Textures.
		/// replacing the default one that was created automatically.
		/// \param pTextureSet The new TextureSet, presumably one that was created by your application.
		void SetTextureSet(TextureSet * pTextureSet);
		/// \brief Installs a new TextureCell at the specified position within this Terrain's grid of
		/// TextureCells.
		/// \param index The index of the TextureCell within this Terrain's grid of TextureCells.
		/// \param pCell The new TextureCell to install, presumably one that was created by your application.
		void SetTextureCell(int index, TextureCell * pCell);
		/// \brief Write all of this Terrain's textures to raw binary files.
		/// \param szTerrainName A filename to write to. The media path will be prepended to this file. See Settings::SetMediaPath().
		void WriteRawTextures(const char *szTerrainName);
		/// \brief Write all of this Terrain's vertices to a raw binary file.
		/// \param szFilename A filename to write to. The media path will be prepended to this file. See Settings::SetMediaPath().
		void WriteRawElevations(const char *szFilename);
		/// \brief Sets the size of this Terrain's grid of TextureCell objects. 

		/// You must specify a number of cells such that the following equation evalues to integral values for
		/// <Code>tw</Code> and <Code>tv</Code>, otherwise your Terrain will show seams between textures:<Code>\par
		/// vx = number of terrain mesh vertices x\par
		/// vy = number of terrain mesh vertices y\par
		/// nx = numTextureCellsX\par
		/// ny = numTextureCellsY\par
		/// vs = terrain's vertex spacing\par
		/// tw = width of a TextureCell in vertices\par
		/// th = height of a TextureCell in vertices\par
		/// tw = ((vx + nx - 1) / nx - 1) / vs\par
		/// th = ((vy + ny - 1) / ny - 1) / vs</Code>\par
		/// See the TextureCell class for details.
		/// \param numTextureCellsX The number of cells in the X direction of this Terrain's grid of TextureCells.
		/// \param numTextureCellsY The number of cells in the Y direction of this Terrain's grid of TextureCells.
		void AllocateTextureCells(int numTextureCellsX, int numTextureCellsY);
		/// \brief \deprecated Formerly used by TerrainLattice for procedural textures.
		void GenerateTexture(int widthTexels, int heightTexels, const char *szGeneratorName);
		/// \brief \deprecated Formerly used by TerrainLattice for procedural textures.
		void GenerateTexture(int widthTexels, int heightTexels, TextureGenerator * pGenerator);
		/// \brief \deprecated Formerly used by TerrainLattice for procedural textures.
		static void RegisterTextureGenerator(std::string name, TextureGenerator * pGenerator);
		/// \brief \deprecated Formerly used by TerrainLattice for procedural textures.
		static void RegisterDefaultGenerators();
		/// \brief Returns the width of the overall terrain texture in pixels.
		int GetBaseTextureWidth();
		/// \brief Returns the height of the overall terrain texture in pixels.
		int GetBaseTextureHeight();
		
		/// \brief Increment the reference count by one, indicating that this object has another pointer which is referencing it.

		/// This method is provided for use of Demeter with the Open Scene Graph.
		inline void ref() const
		{
			++m_refCount;
		}
		/// \brief Decrement the reference count by one, indicating that a pointer to this object is referencing it.

		/// If the reference count goes to zero, it is assumed that this object is no longer referenced 
		/// and is automatically deleted. This method is provided for use of Demeter with the 
		/// Open Scene Graph.
		inline void unref() const
		{
			--m_refCount;
			if (m_refCount <= 0)
				delete this;
		}
		/// \brief Gets the raw bits of the alpha mask of the specified detail texture within the specified TextureCell.
		uint8 *GetMaskBits(int textureCellX, int textureCellY, int detailIndex, int &maskWidth, int &maskHeight);
		/// \brief Unbinds and rebinds the alpha mask of the specified TextureCell so that changes to the mask become visible when rendering.
		void ReloadMask(int textureCellX, int textureCellY, int detailIndex);

		/** load the terrain mask file from disk once again. This function only works when terrain is not editable. 
		* otherwise there is no need to load mask file manually here. This function is called when device is lost. 
		* @param bForceReload: if true, it will reload mask file from disk. 
		*/
		void LoadMaskFromDisk(bool bForceReload = false);

		/** normalize the given texture mask layer, so that the alpha of the specified layer is unchanged and 
		* the sum of all alpha values of all mask layers at all pixels in the cell is 1. 
		* it is good practice to keep every pixel mask normalized, so that we do not get overly lighted terrain surface. 
		* @param detailIndex: if this is -1, the base layer is used
		* @param bReload: if true, it will reload all mask layers
		* @return: the number of layers affected. 
		*/
		int NormalizeMask(int textureCellX, int textureCellY, int detailIndex, bool bReload=true);

		/// \brief Set this Terrain object's origin.
		/// This is used by the TerrainLattice class when a grid of Terrains is being managed, but can also be used as a simple way to apply translation to a Terrain.
		void SetOffset(float x, float y);
		/// \brief Gets this Terrain object's origin.

		/// This is used by the TerrainLattice class when a grid of Terrains is being managed, but can also be used as a simple way to apply translation to a Terrain.
		float GetOffsetX() const;
		/// \brief Gets this Terrain object's origin.

		/// This is used by the TerrainLattice class when a grid of Terrains is being managed, but can also be used as a simple way to apply translation to a Terrain.
		float GetOffsetY() const;
		
		void DigCrater(float centerX, float centerY, float radius, int textureId = -1);

		void GetVertices(float centerX, float centerY, float radius, std::vector < TerrainVertex > &vertices);

		/** get the on load script which created all objects on this terrain.*/
		const string& GetOnloadScript();
		
		/** get the terrain elevation file */
		const string& GetTerrainElevFile();
		
		/** get the terrain elevation file */
		const string& GetTerrainConfigFile() {return m_sConfigFile;};
		
		/** get the terrain base texture file */
		const string& GetTerrainBaseTextureFile(){return m_sBaseTextureFile; };
		void SetTerrainBaseTextureFile(const string& str);

		/** get the terrain common texture file */
		const string& GetTerrainCommonTextureFile(){return m_sCommonTextureFile; };
		void SetTerrainCommonTextureFile(const string& str);


		/** Get the bounding box from the last tessellation result. The returned box is always in world coordinate. */
		CShapeBox GetBoundingBoxW();

		/**
		* Get the bounding boxes for a given view frustum. The returned boxes are always in world coordinate with render origin applied  and transform by modelView
		* @param boxes [in|out] 
		* @param modelView: if not null, all returned boxes will be transformed by it, before testing with the frustum 
		* @param frustum: the view frustum to test against.
		* @param nMaxBoxesNum: maximum number of boxes returned. Since it internally uses a breadth first traversal of the terrain quad tree,
		*	larger boxes will return when there are more boxes in the frustum than nMaxBoxesNum
		* @param nSmallestBoxStride what is the stride size of the smallest boxes returned. 
		*/
		void GetBoundingBoxes(vector<CShapeAABB>& boxes, const Matrix4* modelView, const CShapeFrustum& frustum, int nMaxBoxesNum=1024, int nSmallestBoxStride=4);

		/** get the default base layer mask. All bits are 0xFF. */
		DetailTexture* GetDefaultBaseLayerMask();

		/** get the regions defined in this terrain. May return NULL if no regions defined. */
		CTerrainRegions* GetRegions();
		/** create region object if not created before. */
		CTerrainRegions* CreateGetRegions();

		/** get number terrain regions */
		int GetNumOfRegions();

		/** set terrain region index, so that we can later call GetCurrentRegionName() and GetCurrentRegionFilepath(), to access region properties. 
		*/
		void SetCurrentRegionIndex(int nRegion);
		/** Get terrain region index, so that we can later call GetCurrentRegionName() and GetCurrentRegionFilepath(), to access region properties. 
		* -1 if nothing is selected. 
		*/
		int GetCurrentRegionIndex();

		/** set the currently selected region name.  The current region Index is changed to the current selected one or -1 if invalid. 
		* please note that: this does not create the region layer 
		*/
		void SetCurrentRegionName(const string& name);
		
		/** get the currently selected region name. */
		const string& GetCurrentRegionName();

		/** set the current region file path. */
		void SetCurrentRegionFilepath(const string& filename);

		/** get the current region file path. */
		const string& GetCurrentRegionFilepath();

	private:
		
		// the terrain configuration file.
		string m_sConfigFile;
		// the base terrain texture file.
		string m_sBaseTextureFile;
		// the common terrain texture file.
		string m_sCommonTextureFile;
		// the on load script file name of this terrain
		string m_sOnLoadScript;
		// elevation file 
		string m_sElevFile;

		/** this is the default base layer mask file in this terrain. All bits are 0xFF. */
		DetailTexture* m_pDefaultBaseLayerMask;
		/** bounding box from the last tessellation result. This is always in local coordinate. */
		CShapeBox m_BoundingBox;
		//void RenderTextureGroup(int i);
		int m_WidthVertices;
		int m_HeightVertices;
		/// the radius around which no tessellation is performed.
		float m_fHighResTextureRadius;
		float m_DetailThreshold;
		float m_VertexSpacing;
		/// this is the main structure for ROAM algorithm
		TerrainBlock *m_pRootBlock;
		unsigned char* m_pVertexStatus;
		/** the terrain regions. maybe NULL if no region is defined. */
		CTerrainRegions* m_pRegions;
		/// number of hole vertex
		int m_nNumOfHoleVertices;
		/// a bitwise map of holes in the terrain. Objects may go through holes 
		/// (into interior areas of the game world, such as an underground maze or cave)
		/// if the map contains no holes, this array will be null.
		bool*	m_pHolemap;
		/// this value can only be a power of 2, such as 2, 4, 8, etc. default value is 2 or 4. 
		/// it denotes how many successive vertex in the height map represent a hole
		/// the larger the value is the larger the hole is.
		short   m_nHoleScale;
		/// number of vertices in the map
		int m_NumberOfVertices;
		/// vertex array in the entire terrain.
		Vector3 *m_pVertices;
		/// collection of chopped textures, each is 256*256 bits.
		std::vector < TextureCell * >m_TextureCells;
		float m_TextureTileWidth, m_TextureTileHeight;
		uint32 m_NumberOfTextureTilesWidth;
		uint32 m_NumberOfTextureTilesHeight;
		int m_TileSize;
		uint8 **m_pTiles;
		uint32 m_NumberOfTextureTiles;
		Texture *m_pCommonTexture;
		float m_FogColorRed, m_FogColorGreen, m_FogColorBlue, m_FogColorAlpha;
		int m_MaximumVisibleBlockSize;
		vector<TriangleStrip> m_pTriangleStrips;
		vector<TriangleFan> m_pTriangleFans;
		uint32 m_CountStrips, m_CountFans;
		bool m_bMultiTextureSupported;
		uint32 m_MaxNumberOfPrimitives;
		float m_MaxElevation;
		float m_OffsetX, m_OffsetY;
		int m_LatticePositionX, m_LatticePositionY;
		float m_fTerrainSize;
		TextureSet *m_pTextureSet;
		float *m_pTextureMain;
		float *m_pTextureDetail;
		Vector3 *m_pNormals;
		mutable int m_refCount;
		CTerrainMaskFileCallbackData* m_pMaskFileCallbackData;
	    
		static std::map < std::string, TextureGenerator * >m_TextureGenerators;
		int m_BaseTextureWidth, m_BaseTextureHeight;
		std::map < std::string, ReloadMaskRequest * >m_ReloadMaskRequests;
		/// the height of the lowest visible terrain point. This may be used to render the ocean.
		float m_fLowestVisibleHeight;
		DWORD m_dwModified;
		static float m_DetailTextureRepeats;
		// whether to draw terrain in the fog in the distance
		bool m_bFogEnabled;
		bool m_bMaskFileInited;

		
		/// intermediate date for rendering
		TerrainBuffer m_TerrainBuffer;
		/// Current Eye position in the terrain's local unit systems, we will use detail texture near the eye position.
		Vector3 m_vEye;
		/// the terrain will be offset by this amount when rendering. 
		/// This may be different from m_Offset,which is used for global physics.
		Vector3 m_vRenderOffset;
		/// it just switch m_vRenderOffset_internal.xyz = m_vRenderOffset.xzy
		Vector3 m_vRenderOffset_internal;

		/** set the render offset 
		@param vOffset: this is the ParaEngine's default coordinate, where y is world up. However, in terrain engine, z is world up. */
		inline void SetRenderOffset(const Vector3& vOffset)
		{
			m_vRenderOffset = vOffset;
			m_vRenderOffset_internal.x = vOffset.x;
			m_vRenderOffset_internal.y = vOffset.z;
			m_vRenderOffset_internal.z = vOffset.y;
		}
		/** set the eye position for the terrain. 
		* @param vEye: this is in world coordinates. */
		void SetEyePosition(const Vector3& vEye);

		void Init(int maxNumPrims, float offX, float offY);
		void Init(const uint8 * pTextureImage, int textureWidth, int textureHeight, const uint8 * pDetailTextureImage, int detailWidth, int detailHeight, float offsetX = 0.0f, float offsetY = 0.0f);
		void UpdateNeighbor(Terrain * pTerrain, DIRECTION direction);

		inline void SetVertexStatus(int index, unsigned char status){if(m_pVertexStatus != NULL)	m_pVertexStatus[index] = status;};
		inline unsigned char GetVertexStatus(int index){return m_pVertexStatus[index];};

		void BuildBlocks();
		void ChopTexture(const uint8 * pImage, int width, int height, int tileSize);
		void PreloadTextures();
		void BuildVertices(int widthVertices, int heightVertices, float vertexSpacing);
		/** get a vertex by its index. No boundary checking is performed.
		* the vertex can be directly rendered with global offset applied.
		* @param nIndex: the index of the vertex
		* @param v: [out]the vertex which will be filled with the data.
		*/
		inline void GetRenderVertex(int nIndex, Vector3& v){
			Vector3& vec = m_pVertices[nIndex];
			v.x = vec.x+m_vRenderOffset.x;
			v.y = vec.z+m_vRenderOffset.y;
			v.z = vec.y+m_vRenderOffset.z;
		}
		/** get a normal by its index. No boundary checking is performed.
		* @param nIndex: the index of the vertex
		* @param v: [out]the normal which will be filled with the data.
		*/
		void GetRenderNorm(int nIndex, Vector3& v)
		{
			Vector3& vec= m_pNormals[nIndex];
			v.x = vec.x;
			v.y = vec.z;
			v.z = vec.y;
		}

		TriangleStrip* GetTriStrip(int nIndex);
		TriangleStrip* GetSafeTriStrip(int nIndex);
		TriangleFan* GetTriFan(int nIndex);
		TriangleFan* GetSafeTriFan(int nIndex);
	public:
		
		/** get texture ID by point.
		* texture ID is the index of low-res textures in range [0, 8*8)
		* @param x: in local coordinate
		* @param y: in local coordinate 
		* @ return: the texture ID returned. may be negative */
		int GetTextureIDbyPoint(float x, float y, float& tileX, float& tileY);
		
		/** load detailed (high-res) textures, which is usually a large collection of files */
		//void LoadDetailedTextures(const char * filename);
		/** Get the low-res texture coordinate at a specified vertex index */
		inline void GetLowResTexCord(int nIndex, float& u, float& v){
			const Vector2& uv = s_LowResTexCords[nIndex];
			u = uv.x;
			v = uv.y;
		}
		/** Get the high-res texture coordinate at a specified vertex index */
		inline void GetHighResTexCord(int nIndex, float& u, float& v){
			const Vector2& uv = s_HighResTexCords[nIndex];
			u = uv.x;
			v = uv.y;
		}
		/** get both low resolution and detailed texture coordinates */
		inline void GetTexCord(int nIndex, float& lowU, float& lowV, float& detailU, float& detailV){
			const Vector2& uv = s_LowResTexCords[nIndex];
			lowU = uv.x;
			lowV = uv.y;
			const Vector2& uv2 = s_HighResTexCords[nIndex];
			detailU = uv2.x;
			detailV = uv2.y;
		}
		
		/** array of low resolution texture coordinates. array size is m_WidthVertices*m_WidthVertices */
		static Vector2* s_LowResTexCords;
		/** array of high resolution texture coordinates. array size is m_WidthVertices*m_WidthVertices */
		static Vector2* s_HighResTexCords;

		/** init static UV coordinate constants, so that we need not calculate at runtime */
		static void StaticInitUV(int nWidthVertices, int nNumberOfTextureTiles, int nDetailTextureRepeats);

		/** this is a dummy object that to ensure static objects are destroyed properly*/
		struct DestroyStaticObject_
		{
			DestroyStaticObject_(){}
			~DestroyStaticObject_()
			{
				SAFE_DELETE_ARRAY(Terrain::s_LowResTexCords);
				SAFE_DELETE_ARRAY(Terrain::s_HighResTexCords);
			}
		};

		/// set the radius in world coordinates, within eye position of which high-res textures will be used if possible.
		void SetHighResTextureRadius(float radius);
		/// \brief Gets the radius setting as set in a call to SetHighResTextureRadius().
		float GetHighResTextureRadius();
		/// \brief Specifies how many times detail textures are repeated within their parent textures. Higher values will make the detail textures smaller.
		void SetDetailTextureRepeats(float repeats);
		/// \brief Gets the repeats setting as set in a call to SetDetailTextureRepeats().
		float GetDetailTextureRepeats();

		/** Set the height of the lowest visible terrain point. This may be used to render the ocean*/
		void  SetLowestVisibleHeight(float fHeight);
		/** Get the height of the lowest visible terrain point. This may be used to render the ocean*/
		float GetLowestVisibleHeight();
		/** compute mask file from the terrain config file.
		* the fold of terrain configure file + folder with terrain config file name+".mask"
		* e.g. if the terrain configure file is "sample/config/sample_0_0.config.txt"
		* then the mask files will be saved to "sample/config/sample_0_0.mask"
		*/
		void GetMaskFile(string& sMaskFile, const string& sConfigFile);

		/** save terrain configurations to file. this function will save file regardless of the modified bits; it will not clear the modified bits either.
		* @param filePath: if this is "", the current config file name will be used.
		*/
		bool SaveConfig(const string& filePath);

		/** save terrain height map to file. this function will save file regardless of the modified bits; it will not clear the modified bits either.
		* @param filePath: if this is "", the current elevation file name will be used.
		*/
		bool SaveElevation(const string& filePath);

		/** save detail texture to texture mask file. this function will save file regardless of the modified bits; it will not clear the modified bits either.
		* @param filePath: if this is "", the current mask file name will be used.
		*/
		bool SaveDetailTexture(const string& filePath);

		/** Load from Configuration file. Modified bits are cleared. Please note that the on_load script is not called. 
		* one must manually call OnLoad() to load scene objects in this terrain.
		* @param fSize: if fSize is not 0, it will override the size read from the configuration file
		* @param relativePath: if sFileName is not found, it will try appending relativePath before sFileName.
		*  relativePath is trimmed to the last slash character. i.e. "terrain/abc.txt" and "terrain/" are the same relative path.
		* @return : return true if succeeded
		*/
		bool LoadFromConfigFile(const char* sFileName, float fSize=0, const char* relativePath=NULL);

		/** this function is automatically called after the terrain has been loaded. 
		* it usually does the following things in the given order: 
			- call the terrain tile's on load script;  
			- load all NPC from database in this terrain tile.
		Please note that the on load script might in turn create new terrain objects.*/
		void OnLoad();

		/**
		* Save Terrain to file according to the modified bits.
		* @param filename: file name of the terrain config file. All related files, such as elevation file and alpha mask files 
		*   will be saved in the save directory of the terrain config file.If this is NULL, the load config file will be used. 
		*   and will be overridden.
		* @param fSize: if fSize is not 0, it will override the size of this terrain tile by the value supplied.
		* @param relativePath: if sFileName is not found, it will try appending relativePath before sFileName.
		*	relativePath is trimmed to the last slash character. i.e. "terrain/abc.txt" and "terrain/" are the same relative path.
		* @return : return true if succeeded
		*/
		bool SaveToFile(const char* filename=NULL);

		/** read mask file */
		bool ReadMaskFile(CParaFile& fileMask);

		/** resize all texture mask width */
		void ResizeTextureMaskWidth(int nWidth);

		/// for high-res texture of terrain surfaces
		ParaTerrain::CDetailTextureFactory* m_pDetailedTextureFactory;
	protected:
		/**
		*generate terrain normal for rendering with light on. This function will only generate normal when the m_pNormal is NULL.
		* @param bForceRegenerate true to regenerate any way. 
		* @return 
		*/
		bool GenerateTerrainNormal(bool bForceRegenerate=false);

		friend class Triangle;
		friend class TriangleStrip;
		friend class TerrainBlock;
		friend class TriangleFan;
		friend class TerrainLattice;
		friend class TerrainBuffer;
		friend class Loader;
		friend class TextureSet;


		private:
			GeoMipmapIndicesGroup* m_pGeoMipmapIndicesGroup;
			TerrainBlock* m_pBlocks;
		private:
			ParaIndexBuffer m_pIndexBuffer;
			ParaVertexBuffer m_pCollisionBuffer;
			ParaVertexBuffer m_pEditorMeshVB;
		public:
			void SetSharedIB(ParaIndexBuffer ib){ m_pIndexBuffer = ib; }
			void SwitchLodStyle(bool useGeoMipmap, ParaIndexBuffer pSharedIndices, GeoMipmapIndicesGroup* geoMipmapIndicesGroup);
			
		public:
			bool m_useGeoMipmap;
			void SetSharedIndexInfoGroup(GeoMipmapIndicesGroup* geoMipmapIndicesGroup){m_pGeoMipmapIndicesGroup = geoMipmapIndicesGroup;}
			void BuildGeoMipmapBuffer();
			void RenderGeoMipmap();
			void TessellateGeoMipmap();
			void BuildGeoMipmapBlocks();
			void SetBlockLod(int indexX,int indexY,int level,GeoMipmapChunkType chunkType);

			bool IsWalkable(float x,float y) const;
			bool IsWalkableW(float x,float y) const;

		//terrain editor code  --clayman
		private:
			bool m_isEditorMode;
			bool m_enableTerrainCollision;
			/** whether to chop the base texture in to 8*8 smaller textures, or just use it as a single large texture. */
			bool m_bChopBaseTexture;
			string m_terrInfoFile;

				

			uint32* m_pTerrainInfoData;

			void BuildHelperMeshBuffer();
			void SaveInfoData(const string& filePath);
			void CreateTerrainInfoData();

			int m_visibleEditorMeshFlag;
			uint32 m_visibleDataMask;
			uint8 m_visibleDataBitOffset;

		public:
			void SetTerrainInfoData(const uint32* pInfoData,int width);
			void SetVertexInfo(int idx,uint32 data,uint32 bitMask,uint32 bitOffset);
			uint32 GetVertexInfo(int idx,uint32 bitMask,uint8 bitOffset);
			void RefreshTerrainInfo();
			bool IsWalkable(float x, float y,Vector3& oNormal) const;
			void SetVisibleDataMask(uint32 mask,uint8 bitOffset);
			void SetEditorMode(bool enable);
	};

	 
	class ReloadMaskRequest
	{
	      public:
		ReloadMaskRequest(int textureCellX, int textureCellY, int detailIndex);
		 ~ReloadMaskRequest();
		int m_TextureCellX, m_TextureCellY, m_DetailIndex;
	};
}
