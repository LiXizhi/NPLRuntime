#pragma once
#include <map>

namespace ParaTerrain
{
	/** a single region layer */
	class CRegionLayer
	{
	public:
		CRegionLayer();
		~CRegionLayer();

		/** load from an image file. 
		* @param filename: file name. 
		* @param nBytesPerPixel: if 0, the value is read from the filename. for example, if file name ends with "_8bits.png", it is treated as a 1 byte per pixel. 
		*/
		void LoadFromFile(const char* filename, int nBytesPerPixel = 0);

		/** get the value in image. */
		DWORD GetPixelValue(int x, int y);

		/** pixels columns */
		inline int GetHeight() {return m_nHeight;}

		/** pixels rows */
		inline int GetWidth() {return m_nWidth;}

		/** get texture file path. */
		const string& GetFileName(){return m_filename;};

		/** set the file name. */
		void SetFileName(const string& filename);
	private:
		/** pixels columns */
		int m_nHeight;
		/** pixels rows */
		int m_nWidth;
		/** array of bytes */
		char * m_pData;

		/** number of bytes in a row. */
		int m_nBytesPerRow;

		/** relative file path. it may contain path replaceables.*/
		string m_filename;

		/** number of byte in data */
		int m_nSize;
		/** the number of bits per pixel, such as 1,2,3,4 */
		int m_nBytesPerPixel;
		int m_dwTextureFormat;
	};

	/** the editable terrain regions associated with a terrain tile. This can be used for marking sound regions, 
	* walkable area regions, vegation regions, atomosphere regions, game zones, etc. 
	*/
	class CTerrainRegions
	{
	public:
		CTerrainRegions();
		~CTerrainRegions();

	public:
		typedef std::map<string, CRegionLayer*> RegionLayer_Map_Type;
		/** set the region size in meters. */
		void SetSize(float fWidth, float fHeight);

		/** get value of a given region layer */
		DWORD GetValue(const string& sLayerName, float x, float y);

		/** get value of a given region layer */
		DWORD GetValue(CRegionLayer* pLayer, float x, float y);

		/** Load a given region from file. If a region already exit, we will ignore. */
		void LoadRegion(const string& sLayerName, const char* filename);

		/** get a region layer object by its name, in case we want to access to low level functins. 
		* return NULL if it does not exist. 
		*/
		CRegionLayer* GetRegion(const string& sLayerName);

		/** get the layers map. */
		RegionLayer_Map_Type& GetLayers() {return m_layers;}
		
	public:
		// below are all accessor functions

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
		float m_fHeight;
		float m_fWidth;

		// currently selected region index for accessor functions. -1 if invalid. 
		int m_nCurRegionIndex;

		// currently selected region name for accessor functions. 
		string m_sCurRegionName;

		/** region layer. */
		RegionLayer_Map_Type m_layers;
	};
}