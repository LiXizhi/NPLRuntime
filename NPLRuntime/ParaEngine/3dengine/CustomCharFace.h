#pragma once

namespace ParaEngine
{
	/**
	* a CartoonFace slot component data in compact format.
	* it contains 16bits style id, 16bits color, 8bits scaling, 8bits rotation, 8bits for x translation, 8bits for y translation.
	*/
	class FaceComponent
	{
	private:
		/** WORD[0]: component type, WORD[1] color */
		WORD data[2];
		/** scaling, rotation and translation: SRT[0] for scaling,SRT[1] for rotation, SRT[2] for translation_X, SRT[3] for translation_Y */
		unsigned char  SRT[4];
	public:
		/** initialize to full color, 0 type, 0 scale, 0 rotation, 0 position.*/
		FaceComponent();

		/** initialize to full color, 0 type, 0 scale, 0 rotation, 0 position.*/
		void reset();

		/** Get style*/
		int GetStyle() const;

		/** set style */
		void SetStyle(int style);

		/** get color */
		DWORD GetColor()const;

		/** set color */
		void SetColor(DWORD color);

		/** scaling  [-1,1]*/
		float GetScaling()const;

		/** scaling  [-1,1] */
		void SetScaling(float scaling);

		/** Rotation  (-3.14,3.14] */
		float GetRotation()const;

		/** Rotation (-3.14,3.14]*/
		void SetRotation(float fRotation);

		/** position  in pixel
		* @param x: (-128,128]
		* @param y: (-128,128]
		*/
		void SetPosition(int* x, int* y);

		/** position  in pixel
		* @param x: (-128,128]
		* @param y: (-128,128]
		*/
		void GetPosition(int* x, int* y)const;
	};

	class FaceTextureComponent : public  FaceComponent
	{
	public:
		FaceTextureComponent(){};
		FaceTextureComponent(const FaceComponent& component) :FaceComponent(component){}

		/** whether the texture component is loaded*/
		bool CheckLoad();
	public:
		std::string  name;
		ref_ptr<TextureEntity> m_texture;

	};

	/** data structure for a cartoon face*/
	class CartoonFace
	{
	public:
		CartoonFace();
		CartoonFace(FaceComponent* _components);
	public:
		FaceComponent components[CFS_TOTAL_NUM];

		bool m_bNeedUpdate;

		/** so the cartoon face will be rebuilt the next time the model is refreshed.*/
		void SetModified(bool bModified = true);;

		/** set all components to their default settings. */
		void Reset();

		/** make component texture. this function is similar to makeItemTexture except that it is always unisex.
		@param region: the texture region index.
		@param name: it is the tex1 read from the cartoon face component database*/
		std::string makeComponentTexture(int region, const char *name);

		/** build the synthesized texture, and save the result into the tex texture entity
		* @param tex: [out] pointer to the texture
		*/
		void compose(asset_ptr<TextureEntity>& tex, CharModelInstance* pCharInst);

		/** get a given face component
		* @param index: type of CartoonFaceSlots
		*/
		FaceComponent& GetComponent(int index){ return components[index]; };

		/** the cached file name is created from the concatenated string of component.names. */
		const string& GetCacheFileName();

		void GetFaceComponents(std::vector <FaceTextureComponent>& layers);
	private:
		string m_sCachedFileName;
		bool m_bNeedUpdateCache;
	};
}