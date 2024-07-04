#pragma once
#include "ParaXBone.h"
#include "ParaXModel.h"
#include <map>
#include <vector>
#include <list>
#include <string>

namespace ParaEngine
{
	using namespace std;
	class Bone;

	/**
	Supplying external animation loaded from ParaX or BVH files. Such animation can be shared by any animation instances. 
	Providers can also be loaded dynamically; some special animation provider can be composed by user at runtime; some may be 
	automatically generated from the physics engine. 
	@note: each instance of CBoneAnimProvider has two keys, one is id(primary and unique), the other is name(optional)
	*/
	class CBoneAnimProvider
	{
	public:
		/**
		* new an animation provider from file. 
		* @param nAnimID: -1 if one wants it to be automatically assigned. otherwise one can manually specify one. 
		*  Please note, if there is already a provider with the same ID, the old one is automatically released and replaced with the new one. 
		* @param name: optional key. Allowing user to query by a user friendly name. This can be NULL.
		* @param filename: from which file the animation data is loaded. It can be a ParaX animation file or BVH file.
		*/
		CBoneAnimProvider(int nAnimID, const char* name, const char* filename);

		virtual ~CBoneAnimProvider(void);

	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// static functions: manager class functions.
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the provider by its ID. 
		* function may return NULL if the ID does not exist. 
		*/
		static CBoneAnimProvider* GetProviderByID(int id);

		/**
		* Get provider id by name. Name is used when creating the provider instance. It is usually 
		* the same as the file path from which animation data is loaded. 
		* return -1 if not found
		*/
		static int GetProviderIDByName(const char* sName);

		/**
		* Create an animation provider from file. 
		* @param nAnimID: -1 if one wants it to be automatically assigned. otherwise one can manually specify one. 
		*  Please note, if there is already a provider with the same ID, the old one is automatically released and replaced with the new one. 
		* @param name: optional key. Allowing user to query by a user friendly name. This can be NULL.
		* @param filename: from which file the animation data is loaded. It can be a ParaX animation file or BVH file.
		* @param bOverwrite: whether to overwrite existing with the same nAnimID or name
		*/
		static CBoneAnimProvider* CreateProvider(int nAnimID, const char* name, const char* filename, bool bOverwrite = false);

		/** delete a provider by ID. 
		* @return: return true if succeed. 
		*/
		static bool DeleteProvider(int nAnimID);

		/** delete all providers.*/
		static bool CleanupAllProviders();

	private:
		static map<int, CBoneAnimProvider*> g_mapProviderIDs;
		static map<string, CBoneAnimProvider*> g_mapProviderNames;
			
	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// member functions: per instance functions.
		//
		//////////////////////////////////////////////////////////////////////////

		/** how bone animation is meant to be used with a parax model.*/
		enum MergeMode
		{
			/** replace the target model with all existing bones defined in the provider*/
			MergeMode_ReplaceExisting,
			/** replace the target model with all bones. */
			MergeMode_ReplaceALL,
		};

		/** get the animation ID*/
		int GetAnimID();

		/** get the animation name*/
		const string& GetAnimName();

		/** get the animation file name*/
		const string& GetAnimFileName();

		/** check by Bone id*/
		bool HasBone(KNOWN_BONE_NODES KnownBoneID);
		
		/** get bone by Bone id*/
		Bone* GetBone(KNOWN_BONE_NODES KnownBoneID);
		/** get bone by index. return NULL if no bones at the index */
		Bone* GetBoneByIndex(int nIndex);

		/** get bone by its unique name, it must not be empty string. */
		Bone* GetBoneByName(const std::string& sName);
	
		/** how bone animation is meant to be used with a parax model.*/
		MergeMode GetMergeMode();

		/** sub animation ID. default to 0.  In most cases, a bone animation provider contains only one animation sequence whose ID is 0. 
		* however, some provider may contain multiple animation ID.*/
		int GetSubAnimID();

		/** sub animation ID. default to 0.  In most cases, a bone animation provider contains only one animation sequence whose ID is 0. 
		* however, some provider may contain multiple animation ID.*/
		void SetSubAnimID(int nID);

		/** 
		* @param nSubAnimID: in most cases this is 0
		*/
		AnimIndex GetAnimIndex(int nSubAnimID=0);

		/** get the character move speed in the animation.
		* @param nSubAnimID: in most cases this is 0
		* @param pSpeed: out value
		*/
		void GetAnimMoveSpeed(float* pSpeed, int nSubAnimID=0);

		/** root bone has the name Root or it is the first parent bone that has an non-empty name. 
		* In most bipeds, it is the Hip of the character.
		*/
		int FindRootBone();

	private:
		/** check load asset, call this as often as one like. */
		bool LoadAsset();

		/** load from BVH file*/
		bool LoadFromBVHFile(const char* filename);

		/** load from ParaX file*/
		bool LoadFromParaXFile(const char* filename);
		
	private:
		/** only keep asset ptr if not loaded. */
		asset_ptr<ParaXEntity>	m_asset;

		/** primary key. 0-1000 are reserved. 1000-2000 are game specific; 2000 plus are automatically generated. */
		int m_nAnimID;
		/** optional key. usually the same as the file name. */
		string m_sName;
		/** from which the object is loaded. */
		string m_sFileName;

		/** sub animation ID. default to 0.  In most cases, a bone animation provider contains only one animation sequence whose ID is 0. 
		* however, some provider may contain multiple animation ID.*/
		int m_nSubAnimID;

		/** a mapping from bone id to bone index. */
		int m_boneLookup[MAX_KNOWN_BONE_NODE];

		/** all bones in this provider.*/
		vector<Bone> m_bones;

		/** animation sequence info: in most cases, there is only one animation.*/
		vector<ModelAnimation> m_anims;

		/** bone name to index mapping. */
		map<string, int> m_mapBoneNameToIndex;

		/** how bone animation is meant to be used with a parax model.*/
		MergeMode m_MergeMode;

		/** because the animation file may not be ready if we used async loading. We will check load each time. a major function is called. */
		bool m_bLoaded;
	};
}

