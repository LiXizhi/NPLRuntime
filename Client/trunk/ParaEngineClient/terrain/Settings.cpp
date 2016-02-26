// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "Settings.h"
#include "TerrainException.h"
#include "memdebug.h"
using namespace ParaTerrain;

/** default mask texture width. */
#define DEFAULT_MASK_TEXTURE_WIDTH 128

const int32 ParaTerrain::Settings::TextureMaskBitsPerPixel = 8;

Settings::Settings()
{
	m_szMediaPath = NULL;
	m_bVerbose = false;
	m_bIsCompilerOnly = false;
	m_IsHeadless = false;
	m_UseDynamicTextures = false;
	m_IsEditor = true;	// Changed by LiXizhi
	m_TessellateMethod = Settings::TM_NEW;
	m_TessellateZWeight = 0;
	m_PickThreshold = -1.0f;
	m_bUseNormals = false;
	m_bDiagnostic = false;
	m_bPreloadTextures = false;
	m_bCompressTextures = false;
	m_bUseBorders = false;
	m_bUseVertexLocking = false;
	m_TextureMaskWidth = m_TextureMaskHeight = DEFAULT_MASK_TEXTURE_WIDTH;
	m_bIsMaskReloadQueueActive = false;
	m_IsBaseTextureEnabled = true;
	m_pImageLoader = NULL;
	m_bBackupFilesOnSave = false;

	m_fHighResTextureRadius = 50.f;
}

Settings::~Settings()
{
	if (m_szMediaPath != NULL)
		delete[]m_szMediaPath;
}

Settings *Settings::GetInstance()
{
	static Settings settings;

	return &settings;
}

void Settings::SetBackupFilesOnSave(bool bBackupOnSave)
{
	m_bBackupFilesOnSave = bBackupOnSave;
}

bool Settings::IsBackupFilesOnSave()
{
	return m_bBackupFilesOnSave;
}

void Settings::SetImageLoader(ImageLoader* pLoader)
{
	m_pImageLoader = pLoader;
}

ImageLoader* Settings::GetImageLoader()
{
	return m_pImageLoader;
}

void Settings::SetUseNormals(bool bUseNormals)
{
	m_bUseNormals = bUseNormals;
}

bool Settings::UseNormals()
{
	return m_bUseNormals;
}

bool Settings::IsBaseTextureEnabled()
{
	return m_IsBaseTextureEnabled;
}

void Settings::SetBaseTextureEnabled(bool enabled)
{
	m_IsBaseTextureEnabled = enabled;
}

void Settings::SetUseVertexLocking(bool bUseVertexLocking)
{
	m_bUseVertexLocking = bUseVertexLocking;
}

bool Settings::UseVertexLocking()
{
	return m_bUseVertexLocking;
}

void Settings::SetUseBorders(bool bUseBorders)
{
	m_bUseBorders = bUseBorders;
}

bool Settings::UseBorders()
{
	return m_bUseBorders;
}

void Settings::SetDiagnostic(bool bDiagnostic)
{
	m_bDiagnostic = bDiagnostic;
}

bool Settings::IsDiagnostic()
{
	return m_bDiagnostic;
}

bool Settings::SetProperty(const char *szProperty, const char *szValue)
{
	bool bSuccess = false;

	if (strcmp(szProperty, "verbose") == 0)
	{
		SetVerbose(strcmp(szValue, "true") == 0);
		bSuccess = true;
	}

	return bSuccess;
}

bool Settings::GetProperty(const char *szProperty, char *szValue)
{
	bool bSuccess = false;

	if (strcmp(szProperty, "verbose") == 0)
	{
		sprintf(szValue, m_bVerbose ? "true" : "false");
		bSuccess = true;
	}
	else if (strcmp(szProperty, "glinfo") == 0)
	{
		//sprintf(szValue, "OpenGL Vendor: %s; OpenGL Extensions Supported: %s", glGetString(GL_VENDOR), glGetString(GL_EXTENSIONS));
		bSuccess = true;
	}

	return bSuccess;
}

void Settings::SetPreloadTextures(bool bPreload)
{
	m_bPreloadTextures = bPreload;
}

bool Settings::GetPreloadTextures()
{
	return m_bPreloadTextures;
}

void Settings::SetPickThreshold(float threshold)
{
	m_PickThreshold = threshold;
}

float Settings::GetPickThreshold()
{
	return m_PickThreshold;
}

void Settings::SetHighResTextureRadius(float radius)
{
	m_fHighResTextureRadius = radius;
}

float Settings::GetHighResTextureRadius()
{
	return m_fHighResTextureRadius;
}
void Settings::SetUseDynamicTextures(bool use)
{
	m_UseDynamicTextures = use;
}

bool Settings::UseDynamicTextures()
{
	return m_UseDynamicTextures;
}

void Settings::SetMediaPath(const char *relativePath)
{
	//char separator = '\\';
	//if (m_szMediaPath != NULL)
	//	delete[]m_szMediaPath;
	//if (szPath[strlen(szPath) - 1] == separator)
	//{
	//	m_szMediaPath = new char[strlen(szPath) + 1];
	//	sprintf(m_szMediaPath, szPath);
	//}
	//else
	//{
	//	m_szMediaPath = new char[strlen(szPath) + 2];
	//	sprintf(m_szMediaPath, "%s%c", szPath, separator);
	//}

	if(relativePath==NULL)
		return;
	if (m_szMediaPath != NULL)
		delete[]m_szMediaPath;

	char sRelativePath[MAX_PATH];
	int nLastSlash = -1;
	for(int i=0; relativePath[i]!='\0';i++)
	{
		if(relativePath[i] == '/')
			sRelativePath[i] = '\\';
		else
			sRelativePath[i] = relativePath[i];
		if(sRelativePath[i] == '\\')
			nLastSlash = i;
	}
	sRelativePath[nLastSlash+1] = '\0';

	m_szMediaPath = new char[nLastSlash+2];
	strcpy(m_szMediaPath, sRelativePath);
}

void Settings::GetMediaPath(char **szPath)
{
	char *szOutput = new char[strlen(m_szMediaPath) + 1];
	strcpy(szOutput, m_szMediaPath);
	*szPath = szOutput;
}

const char * Settings::GetMediaPath()
{
	return m_szMediaPath;
}
bool Settings::IsMediaPathSet()
{
	if (m_szMediaPath == NULL)
		return false;
	else
		return true;
}

void Settings::PrependMediaPath(const char *szFilename, char *szFullFilename)
{
	sprintf(szFullFilename, "%s%s", m_szMediaPath, szFilename);
}

void Settings::PrependMediaPath(const char *szFilename, char *szFullFilename, int bufferLength)
{
	int length = (int)strlen(m_szMediaPath) + (int)strlen(szFilename) + 1;

	if (bufferLength < length)
		throw new TerrainException("string buffer is too small to contain requested data");

	sprintf(szFullFilename, "%s%s", m_szMediaPath, szFilename);
}

void Settings::SetHeadless(bool isHeadless)
{
	m_IsHeadless = isHeadless;
}

bool Settings::IsHeadless()
{
	return m_IsHeadless;
}

void Settings::SetVerbose(bool bVerbose)
{
	m_bVerbose = bVerbose;
}

bool Settings::IsVerbose()
{
	return m_bVerbose;
}

void Settings::SetEditor(bool isEditor)
{
	m_IsEditor = isEditor;
}

bool Settings::IsEditor()
{
	return m_IsEditor;
}

int Settings::GetScreenWidth()
{
	return m_ScreenWidth;
}

void Settings::SetScreenWidth(int width)
{
	m_ScreenWidth = width;
}

int Settings::GetScreenHeight()
{
	return m_ScreenHeight;
}

void Settings::SetScreenHeight(int height)
{
	m_ScreenHeight = height;
}

bool Settings::IsCompilerOnly()
{
	return m_bIsCompilerOnly;
}

void Settings::SetCompilerOnly(bool bIsCompilerOnly)
{
	m_bIsCompilerOnly = bIsCompilerOnly;
}

void Settings::SetTextureCompression(bool bCompress)
{
	m_bCompressTextures = bCompress;
}

bool Settings::IsTextureCompression()
{
	return m_bCompressTextures;
}

void Settings::SetTessellateMethod(int method)
{
	m_TessellateMethod = method;
}

int Settings::GetTessellateMethod()
{
	return m_TessellateMethod;
}

void Settings::SetTessellateZWeight(float method)
{
	m_TessellateZWeight = method;
}

float Settings::GetTessellateZWeight()
{
	return m_TessellateZWeight;
}

void Settings::SetTextureMaskWidth(int width)
{
	m_TextureMaskWidth = width;
	m_TextureMaskHeight = m_TextureMaskWidth;
}

int Settings::GetTextureMaskWidth()
{
	return m_TextureMaskWidth;
}

void Settings::SetTextureMaskHeight(int width)
{
	m_TextureMaskHeight = width;
	m_TextureMaskWidth = m_TextureMaskHeight;
}

int Settings::GetTextureMaskHeight()
{
	return m_TextureMaskHeight;
}

void Settings::SetMaskReloadQueueActive(bool isQueueActive)
{
	m_bIsMaskReloadQueueActive = isQueueActive;
}

bool Settings::IsMaskReloadQueueActive()
{
	return m_bIsMaskReloadQueueActive;
}

