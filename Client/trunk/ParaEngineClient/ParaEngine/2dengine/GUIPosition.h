#pragma once
#include <string>

#ifndef WIN32
/*
* DrawText() Format Flags
*/
#define DT_TOP                      0x00000000
#define DT_LEFT                     0x00000000
#define DT_CENTER                   0x00000001
#define DT_RIGHT                    0x00000002
#define DT_VCENTER                  0x00000004
#define DT_BOTTOM                   0x00000008
#define DT_WORDBREAK                0x00000010
#define DT_SINGLELINE               0x00000020
#define DT_EXPANDTABS               0x00000040
#define DT_TABSTOP                  0x00000080
#define DT_NOCLIP                   0x00000100
#define DT_EXTERNALLEADING          0x00000200
#define DT_CALCRECT                 0x00000400
#define DT_NOPREFIX                 0x00000800
#define DT_INTERNAL                 0x00001000
#endif

namespace ParaEngine
{
	using namespace std;
	class CBaseObject;
	class IScene;

	/**
	* position of the GUI object
	*/
	struct CGUIPosition
	{
	public:
		CGUIPosition();
		CGUIPosition(const RECT& rect_);
		CGUIPosition(int x, int y, int width, int height);

		/** relative to 2D screen
		*/
		enum GUI_ALIGNMENT_TYPE
		{
			///-	relative to the upper, left. The upper-left point is treated as (0,0)
			upper_left = 0,
			///-	relative to the bottom, right. The bottom-right point is treated as (0,0)
			bottom_right,
			///-	relative to the bottom, left. The bottom-left point is treated as (0,0)
			bottom_left,
			///-	relative to the upper, right. The upper-right point is treated as (0,0)
			upper_right,
			///-	relative to the center top, where height and width is fixed
			center_top,
			///-	relative to the center bottom, where height and width is fixed
			center_bottom,
			///-	relative to the center left, where height and width is fixed
			center_left,
			///-	relative to the center right, where height and width is fixed
			center_right,
			///-	relative to the center center. The central point is treated as (0,0)
			center,
			///-    align to the top, it specifies the distances to the top and the left/right edge
			middle_top,
			///-    align to the bottom, it specifies the distances to the bottom and the left/right edge
			middle_bottom,
			///-    align to the left, it specifies the distances to the left and the top/bottom edge
			middle_left,
			///-    align to the right, it specifies the distances to the right and the top/bottom edge
			middle_right,
			///-	align to left top and right bottom. This is like fill in the parent window.
			fill,

		};

		/**
		How the position of the GUI object is specified. It can be object in pure
		screen pixels (a rectangular), or in scaled screen coordinates (a percentage)
		*/
		enum CGUIPosition_TYPE
		{
			relative_to_screen = 10,
			relative_to_3Dobject = 11,
			relative_to_parent = 12
		};

	public:
		void SetWidth(int width);
		void SetHeight(int height);
		void SetDepth(float Depth);
		/**
		* updates the 3d object depth if the object is relative to 3d object
		* It's used internally. If users want to modify the depth of the UI, use SetDepth() instead
		*/
		void Update3DDepth(float Depth);

		bool IsNormalWidthHeight() const;

		int GetWidth()const;
		int GetHeight()const;
		int GetLeft() const;
		int GetTop() const;
		void SetLeft(int X);
		void SetTop(int Y);
		float GetDepth()const;
		void SetXY(int X, int Y);

		void SetSize(int width, int height);

		/** reposition the control using the same parameter definition used when control is created.
		* see InitObject() for parameter definition. */
		void Reposition(const char* alignment, int left, int top, int width, int height);

		/**
		"_lt": align to left top of the screen
		"_lb": align to left bottom of the screen
		"_ct": align to center of the screen
		"_ctt": align to center top of the screen
		"_ctb": align to center bottom of the screen
		"_ctl": align to center left of the screen
		"_ctr": align to center right of the screen
		"_rt": align to right top of the screen
		"_rb": align to right bottom of the screen
		"_mt": align to middle top
		"_ml": align to middle left
		"_mr": align to middle right
		"_mb": align to middle bottom
		"_fi": align to left top and right bottom. This is like fill in the parent window.
		the layout is given below:\n
		_lt _mt _rt	\n
		_ml _ct _mr \n
		_lb _mb _rb \n
		*@return 1 if width, height is used as right, bottom. otherwise 0
		*/
		int SetAlignment(const char* alignment);

		/**
		* Get the standard position of pIn relative to pParentRect
		* @param pOut [out]: output in standard rect
		* @param pParentRect [in]: the standard rect of the parent of pIn;
		* @param pIn [in]: any rect.
		*/
		void CalculateAbsPosition(RECT* pOut, const RECT* pParentRect) const;

	
		bool IsRelativeTo3DObject() const { return m_nPositionType == relative_to_3Dobject; }
		bool IsRelativeToParent() const { return m_nPositionType == relative_to_parent; }
		bool IsRelativeToScreen() const { return m_nPositionType == relative_to_screen; }

		/** same as CalculateAbsPosition */
		static void GetAbsPosition(RECT* pOut, const RECT* pParentRect, const CGUIPosition* pIn);

		CGUIPosition_TYPE GetPositionType() const { return m_nPositionType; }
		void SetPositionType(CGUIPosition_TYPE val) { m_nPositionType = val; }

		void SetRelatveTo3DObject(const char* s3DObjectName, float fOffset = 0.f);
		void SetRelatveTo3DObject(CBaseObject* p3DObject, float fOffset = 0.f);

		bool Is3DObjectSpecified() const;
		CBaseObject* GetRelative3DObject(IScene * pScene = NULL) const;
	public:
		/** internal value in absolute screen coordinates */
		RECT  rect;

		union _relativity
		{
			struct RectScreenPosition
			{
				GUI_ALIGNMENT_TYPE alignType;
			}To2D;

			struct SceneObjectRelative
			{
				/// pointer to the 3D scene object
				float			fOffSet;
				//CBaseObject *	p3DSceneObject;	
				float m_v3DPosition[3];
			}To3D;
		}Relative;

		/** 3d object name that this gui object is attached to. If empty, m_n3DObjectID is used. */
		std::string  m_s3DObjectName; 
		/** 3d object id that this gui object is attached to.*/
		int m_n3DObjectID;
	protected:
		/** relative to screen | relative to 3D scene object */
		CGUIPosition_TYPE	m_nPositionType;
		float m_fDepth;
	};
}