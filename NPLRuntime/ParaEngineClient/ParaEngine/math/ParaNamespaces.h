#pragma once
namespace ParaEngine
{
	enum Axis {
		XAxis,
		YAxis,
		ZAxis
	};

	enum Orientation {
		Horizontal = 0x1,
		Vertical = 0x2
	};

	enum SizeMode {
		AbsoluteSize,
		RelativeSize
	};

	enum Initialization {
		Uninitialized
	};

	enum FillRule {
		OddEvenFill,
		WindingFill
	};

	enum ClipOperation {
		NoClip,
		ReplaceClip,
		IntersectClip
	};

	// Shape = 0x1, BoundingRect = 0x2
	enum ItemSelectionMode {
		ContainsItemShape = 0x0,
		IntersectsItemShape = 0x1,
		ContainsItemBoundingRect = 0x2,
		IntersectsItemBoundingRect = 0x3
	};

	enum TransformationMode {
		FastTransformation,
		SmoothTransformation
	};

	enum PenStyle { // pen style
		NoPen,
		SolidLine,
		DashLine,
		DotLine,
		DashDotLine,
		DashDotDotLine,
		CustomDashLine
	};

	enum PenCapStyle { // line endcap style
		FlatCap = 0x00,
		SquareCap = 0x10,
		RoundCap = 0x20,
		MPenCapStyle = 0x30
	};

	enum PenJoinStyle { // line join style
		MiterJoin = 0x00,
		BevelJoin = 0x40,
		RoundJoin = 0x80,
		SvgMiterJoin = 0x100,
		MPenJoinStyle = 0x1c0
	};

	enum BrushStyle { // brush style
		NoBrush,
		SolidPattern,
		Dense1Pattern,
		Dense2Pattern,
		Dense3Pattern,
		Dense4Pattern,
		Dense5Pattern,
		Dense6Pattern,
		Dense7Pattern,
		HorPattern,
		VerPattern,
		CrossPattern,
		BDiagPattern,
		FDiagPattern,
		DiagCrossPattern,
		LinearGradientPattern,
		RadialGradientPattern,
		ConicalGradientPattern,
		TexturePattern = 24
	};
}

