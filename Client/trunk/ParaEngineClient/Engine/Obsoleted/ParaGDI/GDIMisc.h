#pragma once
namespace ParaEngine{

	/**
	* This enumeration is used in the CGDIGraphicsPath class
	*/
	enum PathPointType{
		///Indicates that the point is the start of a figure
		PathPointTypeStart = 0,
		///Indicates that the point is one of the two endpoints of a line.
		PathPointTypeLine = 1,
		///not used
		PathPointTypeBezier = 3,
		///Indicates that the point is a rectangle
		PathPointTypeRectangle = 4,
		///not used
		PathPointTypePathTypeMask = 0x7,
		///not used
		PathPointTypePathDashMode = 0x10,
		///not used
		PathPointTypePathMarker = 0x20,
		///not used
		PathPointTypeCloseSubpath = 0x80,
		///Indicates a Clear operation happens to clear the render target
		PathPointTypeOperationClear = 0x81,
		///not used
		PathPointTypeBezier3 = 3
	};

	/**
	* This enumeration is used in the CGDIBrush class
	*/
	enum BrushType{
		///Indicates a brush of type CGDISolidBrush. A solid brush paints a single, constant color that can be opaque or transparent
		BrushTypeSolidColor = 0,
		///not used
		BrushTypeHatchFill = 1,
		/**
		* Indicates a brush of type CGDITextureBrush. A texture brush paints an image. The image or texture 
		* is either a portion of a specified image or a scaled version of a specified image. The type of 
		* image (metafile or nonmetafile) determines whether the texture is a portion of the image or a scaled version of the image
		*/
		BrushTypeTextureFill = 2,
		///not used
		BrushTypePathGradient = 3,
		///not used
		BrushTypeLinearGradient = 4
	};

	/**
	* This enumeration is used in the CGDIPen class
	*/
	enum PenType{
		///Indicates that the pen draws with a solid color
		PenTypeSolidColor = BrushTypeSolidColor,
		///not used
		PenTypeHatchFill = BrushTypeHatchFill,
		///Indicates that the pen draws with a texture that is specified by a CGDITextureBrush object. 
		PenTypeTextureFill = BrushTypeTextureFill,
		///not used
		PenTypePathGradient = BrushTypePathGradient,
		///not used
		PenTypeLinearGradient = BrushTypeLinearGradient,
		PenTypeUnknown = -1
	};
	/**
	* This enumeration is used in all GDI classes
	*/
	 enum Status{
		Ok = 0,
		GenericError = 1,
		InvalidParameter = 2,
		OutOfMemory = 3,
		ObjectBusy = 4,
		InsufficientBuffer = 5,
		NotImplemented = 6,
		Win32Error = 7,
		WrongState = 8,
		Aborted = 9,
		FileNotFound = 10,
		ValueOverflow = 11,
		AccessDenied = 12,
		UnknownImageFormat = 13,
		FontFamilyNotFound = 14,
		FontStyleNotFound = 15,
		NotTrueTypeFont = 16,
		UnsupportedGdiplusVersion = 17,
		GdiplusNotInitialized = 18,
		PropertyNotFound = 19,
		PropertyNotSupported = 20,
		ProfileNotFound = 21,
		//used in shape sensitive operations
		IncorrectShape=22
	} ;

	/**
	* This enumeration is used in CGDIGraphicsPath
	*/
	enum VerticesMergeOption{
		///replace all vertices
		VerticesMergeReplace=0,
		///append vertices to the current vertices
		VerticesMergeAppend=1
	};
	/**
	* This enumeration is used when drawing lines
	*/
	enum LineCap{
		//not used;
		LineCapFlat = 0,
		//Specifies a square cap. The center of the square is the last point in the line. The height and width of the square are the line width. 
		LineCapSquare = 1,
		//not used;
		LineCapRound = 2,
		//not used;
		LineCapTriangle = 3,
		//not used;
		LineCapNoAnchor = 0x10,
		//not used;
		LineCapSquareAnchor = 0x11,
		//not used;
		LineCapRoundAnchor = 0x12,
		//not used;
		LineCapDiamondAnchor = 0x13,
		//not used;
		LineCapArrowAnchor = 0x14,
		//not used;
		LineCapCustom = 0xff
	} ;
	/**
	* This enumeration is used in CGDIPen
	*/
	enum PenScale{
		//no scale
		PenScaleNone=0,
		//scale to fit the size of a rectangle, 
		PenScaleFitRect=1,
		//not used
		PenScaleFitHeight=2,
		//not used
		PenScaleFitWidth=3
	};

	/**
	* This enumeration is used internally in CGDIRegion
	*/
	enum RegionShape{
		//Not defined shape
		RegionShapeNone=0,
		//Rectangle
		RegionShapeRectangle=1,
		//Free line
		RegionShapeFree=0
	};
}
