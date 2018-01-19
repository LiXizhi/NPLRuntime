#pragma once

#include "TextureEntity.h"
#include "MiscEntity.h"
#include "GUIScript.h"

namespace ParaEngine
{
	/**
	* a texture from a rectangular region of bitmap, and one additional texture color
	*/
	struct GUITextureElement {
	public:
		RECT rcTexture;         // Bounding rect of this element on the composite texture
		LinearColor TextureColor;
		string ToScript(int index,int option=0);
		string name;
		void SetElement( TextureEntity* pTexture, const RECT* prcTexture=NULL, Color defaultTextureColor=0xffffffff );
		TextureEntity* GetTexture() {return pTexture.get();}
		
		GUITextureElement();
		~GUITextureElement();
		void Cleanup();

		/** get bounding rect of this element on the composite texture */
		void GetRect(RECT* pOut);
	protected:

		// constructible and assignable shared pointer
		asset_ptr<TextureEntity> pTexture;
	};

	/**
	* a font name, format and font color
	*/
	struct GUIFontElement {
	public:
		DWORD dwTextFormat;     // The format argument to DrawText 
		LinearColor FontColor;
		string ToScript(int index,int option=0);
		string name;
		GUIFontElement(){
			FontColor=0xff000000;
			dwTextFormat=0;
			name="";
		}
		~GUIFontElement();
		void Cleanup();

		/**
		Set the text align and other text displaying formats
		@param dwFormat: the new format. It can be any combination of the following values. 
				DT_BOTTOM (0x00000008)
						Justifies the text to the bottom of the rectangle. This value must be combined with DT_SINGLELINE. 
				DT_CALCRECT (0x00000400)
						Determines the width and height of the rectangle. If there are multiple lines of text, ID3DXFont::DrawText uses the width of the rectangle pointed to by the pRect parameter and extends the base of the rectangle to bound the last line of text. If there is only one line of text, ID3DXFont::DrawText modifies the right side of the rectangle so that it bounds the last character in the line. In either case, ID3DXFont::DrawText returns the height of the formatted text but does not draw the text. 
				DT_CENTER (0x00000001)
					Centers text horizontally in the rectangle. 
				DT_EXPANDTABS (0x00000040)
					Expands tab characters. The default number of characters per tab is eight.
				DT_LEFT (0x00000000)
						Aligns text to the left. 
				DT_NOCLIP (0x00000100)
						Draws without clipping. ID3DXFont::DrawText is somewhat faster when DT_NOCLIP is used. 
				DT_RIGHT (0x00000002)
						Aligns text to the right. 
				DT_RTLREADING
						Displays text in right-to-left reading order for bi-directional text when a Hebrew or Arabic font is selected. The default reading order for all text is left-to-right. 
				DT_SINGLELINE (0x00000020)
						Displays text on a single line only. Carriage returns and line feeds do not break the line. 
				DT_TOP (0x00000000)
						Top-justifies text. 
				DT_VCENTER (0x00000004)
						Centers text vertically (single line only). 
				DT_WORDBREAK (0x00000010)
						Breaks words. Lines are automatically broken between words if a word would extend past the edge of the rectangle specified by the pRect parameter. A carriage return/line feed sequence also breaks the line. 
		*/
		void SetElement( SpriteFontEntity* pFont, Color defaultFontColor=0xff000000, DWORD dwTextFormat=0 );

		SpriteFontEntity* GetFont() {return pFont.get();}

	protected:
		// constructible and assignable shared pointer
		asset_ptr<SpriteFontEntity> pFont;
	};


	/**
	* A drawable element containing several textures and fonts, which can be retrieved by name or index. 
	* typically an element may contain one texture for traditional button, or 9 textures for re-sizable controls
	*/
	struct GUIStateElement{
	private:
		vector<GUIFontElement> m_fonts;
		vector<GUITextureElement> m_textures;
	public:
		map<string,DWORD> fontmap;
		map<string,DWORD> texturemap;
		size_t TextureSize(){return m_textures.size();}
		size_t FontSize(){return m_fonts.size();}
		/**
		* Currently, if you add elements using the same several times, the old ones will not 
		* be deleted. If you want to get the element with that name, you get the last added one.
		*/
		GUIFontElement* AddElement(const GUIFontElement *pElement,const char*  name=NULL);
		GUITextureElement * AddElement(const GUITextureElement *pElement,const char*  name=NULL);


		/** create or replace an element at the given index. it is a better to call this with increasing nIndex */
		GUITextureElement * SetElement(const GUITextureElement *pElement, int nIndex);

		/** create or replace an element at the given index. it is a better to call this with increasing nIndex */
		GUIFontElement * SetElement(const GUIFontElement *pElement, int nIndex);

		GUIFontElement* GetFontElement(const string &name);
		GUIFontElement* GetFontElement(int index);;
		GUITextureElement* GetTextureElement(const string &name);
		GUITextureElement* GetTextureElement(int index);
		void init();
		void Clone(GUIStateElement* output);
	};

	/**
	* A GUI layer contains four drawable GUIStateElement, which are "normal", "highlight", "pressed", and "disabled". 
	*  typically, only one of them is active at a time and used for drawing the GUI layer. 
	*				"normal" state is the normal appearance. By default, we render a control using this state
	*				"highlight" state is the appearance when mouse is over or the control is highlighted. 
	*				"pressed" state is the appearance when the control is pressed.
	*				"disabled" state is the appearance when the control is disabled.
	*/
	struct GUILAYER {
	public:
		//-------------------------------------
		// 9 Element assignment:
		//   0 - text area
		//   1 - top left border
		//   2 - top border
		//   3 - top right border
		//   4 - left border
		//   5 - right border
		//   6 - lower left border
		//   7 - lower border
		//   8 - lower right border
		enum GUILAYER_TYPE{
			ONE_ELEMENT,NINE_ELEMENT	};
		GUIStateElement eDisabled;
		GUIStateElement eNormal;
		GUIStateElement ePressed;
		GUIStateElement eHighlight;
		GUIStateElement* eCurrent;
		vector<RECT> DrawingRects;// actual drawing rectangles of all elements, set using UpdateRects;
		bool bIsEnabled;//if the layer is enabled.
		int m_objType;

		void init(){
			eNormal.init();ePressed.init();eHighlight.init();
			eCurrent=&eNormal;
		}
		GUITextureElement * AddElement(const GUITextureElement* pElement,const char* name=NULL);
		void AddElement(const GUIFontElement* pElement,const char* name=NULL);

		void SetElement( GUITextureElement* pElement,int index=0);

		void SetElement( GUIFontElement* pElement,int index=0);

		void Clone(GUILAYER* output);
		string ToScript(int option=0);
	};

	enum GUIResourceState
	{
		GUIResourceState_Normal,
		GUIResourceState_Pressed,
		GUIResourceState_Highlight,
		GUIResourceState_Disabled,
	};
	/**
	* Each GUI control is associated with a CGUIResource object for rendering. 
	* each CGUIResource contains three layers, which are "artwork", "background" and "overlay". 
	*				"artwork" layer is the primary layer. By default, we render a control using this layer
	*				"overlay" layer is a layer on top of the "artwork" layer. 
	*				"background" layer is on the bottom of the "artwork" layer.
	* All layers are optional and created on demand. Typically, a CGUIResource has only the "artwork" layer. 
	*/
	class CGUIResource:public IObject
	{
	public:
		enum GUI_RESOURCE_TYPE{
			NONE=0,	BUTTON,	CONTAINER,	TEXT, EDITBOX, IMEEDITBOX, SCROLLBAR
		};
		enum GUI_RESOURCE_LAYER_ENUM{
			LAYER_ARTWORK=0,LAYER_OVERLAY,	LAYER_BACKGROUND,
		};

		CGUIResource();
		~CGUIResource();

		//this is the to do list of the scripts in this resource object.
		//CGUIScript		m_objScripts;
		
		/**
		* Set the active layer for setting and getting resource.
		* it will create the layer if it does not exist
		* @param layer: name of the layer
		*				We have three layers "artwork", "background" and "overlay"
		*				"artwork" layer is the primary layer. By default, we render a control using this layer
		*				"overlay" layer is a layer on top of the "artwork" layer. 
		*				"background" layer is on the bottom of the "artwork" layer.
		*				All three layers have the same property.
		* If not implicitly set, the default active layer is the "artwork" layer.
		* We advice users always sets the active layer before changing any resource.
		*/
		void SetActiveLayer(const char *layer);
		
		/**
		* Set the active layer for setting and getting resource.
		* it will create the layer if it does not exist
		* @param nLayerID:
		*/
		void SetActiveLayer(GUI_RESOURCE_LAYER_ENUM nLayerID = LAYER_ARTWORK);
		
		/** set the current layer type */
		GUI_RESOURCE_LAYER_ENUM GetActiveLayer();

		/**
		* return whether a given layer exist in the resource. One needs to call SetActiveLayer() in order to create a layer.
		* @param nLayerID:
		*/
		bool HasLayer(const char *layer);

		/**
		* return whether a given layer exist in the resource. One needs to call SetActiveLayer() in order to create a layer.
		* @param nLayerID:
		*/
		bool HasLayer(GUI_RESOURCE_LAYER_ENUM nLayerID = LAYER_ARTWORK);

		/** get the layer id by name
		* w e have three layers "artwork", "background" and "overlay"
		*				"artwork" layer is the primary layer. By default, we render a control using this layer
		*				"overlay" layer is a layer on top of the "artwork" layer. 
		*				"background" layer is on the bottom of the "artwork" layer.
		*				All three layers have the same property.
		*/
		GUI_RESOURCE_LAYER_ENUM  GetLayerIDByName(const char* layer=NULL);

		/** set the current layer type */
		void SetLayerType(int etype);
		
		/**
		* Set a font for the current state in the active layer of the resource object;
		* @param fontname: name of the font which has been previously loaded.
		*/
		void SetFont(const char *fontname,int index=0);
		void SetFontColor(DWORD color, int index=0);
		void SetFontFormat(DWORD dwFormat, int index=0);
		void SetFontTransparency(DWORD transparency,int index=0);

		void SetTexture(const char *fontname,int index=0);
		void SetTextureColor(DWORD color, int index=0);
		void SetTextureTransparency(DWORD transparency,int index=0);

		///** 
		//updates all rectangles to screen coordinate with reference to the new control position
		//@param rcWindow: The control's rectangle.
		//*/
		//void UpdateRects(RECT rcWindow);

		virtual void Clone(IObject *pobj)const;
		virtual IObject *Clone()const;
		virtual int Release();
		virtual bool Equals(const IObject *obj)const;
		void SetDrawingRects(RECT *rcDest,int index=0);

		RECT GetDrawingRects(int index=0)const;

		GUIFontElement* GetFontElement(int index=0);
		GUITextureElement* GetTextureElement(int index=0);
		GUIFontElement* GetFontElement(const char *fontname);
		GUITextureElement* GetTextureElement(const char* texturename);
		void SetElement( GUITextureElement* pElement,int index=0);
		void SetElement( GUIFontElement* pElement,int index=0);
		void CloneState(const char * statename);
		int GetLayerType();

		void		AddElement(const GUIFontElement* pElement,const char*  name=NULL);
		GUITextureElement * AddElement(const GUITextureElement* pElement,const char*  name=NULL);
		/**
		* Set the current state for setting and getting resources;
		* @param statename: The given state name;
		*				We have four states "normal", "highlight", "pressed" and "disabled"
		*				"normal" state is the normal appearance. By default, we render a control using this state
		*				"highlight" state is the appearance when mouse is over or the control is highlighted. 
		*				"pressed" state is the appearance when the control is pressed.
		*				"disabled" state is the appearance when the control is disabled.
		*				All four layers have the same property.
		*				If it's NULL or "", we will set the font of the "normal" state;
		* We advice users always sets the current state before changing any resource.
		*/
		void SetCurrentState(const char* statename=NULL);
		void SetCurrentState(GUIResourceState state);
		GUIResourceState GetCurrentState() {return m_CurrentState;};
		size_t TextureSize(){return m_pActiveLayer->eCurrent->TextureSize();}
		size_t FontSize(){return m_pActiveLayer->eCurrent->FontSize();}
		
		void Clear();
		string ToScript(int option=0);
		//the resource type
		int m_objType;
	protected:
		//we design a three layers, it fits for most cases
		GUILAYER*			m_objBackground;
		GUILAYER*			m_objArtwork;
		GUILAYER*			m_objOverlay;
		GUILAYER*			m_pActiveLayer;

		GUIResourceState m_CurrentState;
	};
}