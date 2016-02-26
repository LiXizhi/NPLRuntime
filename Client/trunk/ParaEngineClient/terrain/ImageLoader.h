#pragma once
namespace ParaTerrain
{
	class ImageLoader
	{
		public:
			virtual void LoadImage(char* szFilename,int& width,int& height,uint8** pBuffer,bool useAlpha) = 0;
	};
}


