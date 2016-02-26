#pragma once

namespace ParaEngine
{
	class TTerrain;
	

	/**
	* Perform filtering on a terrain height field.
	* set or get the terrain data by calling GetTerrainData() function.
	*/
	class CTerrainFilters
	{
	public:
		CTerrainFilters(void);
		~CTerrainFilters(void);

		/** used in the merge function */
		enum MergeOperation{
			Addition=0,
			Subtract,
			Multiplication,
			Division,
			Minimum,
			Maximum
		};
		enum FlattenOperation{
			/** Flatten the terrain up to the specified elevation */
			Fill_Op,
			/** Flatten the terrain down to the specified elevation */
			ShaveTop_Op,
			/** Flatten the terrain up and down to the specified elevation */
			Flatten_Op
		};
	public:
		/** get the terrain data that is associated with this filter. 
		* The user can read from or write to this terrain data.*/
		TTerrain* GetTerrainData();

		/** Flatten the terrain both up and down to the specified elevation, using using the 
		* tightness parameter to determine how much the altered points are allowed 
		* to deviate from the specified elevation. 
		* @param elevation: the desired height
		* @param factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		* @param center_x: the center of the affected circle. value in the range [0,1]
		* @param center_y: the center of the affected circle.value in the range [0,1]
		* @param radius: the radius of the affected circle.value in the range [0,0.5]
		*/
		void Flatten (FlattenOperation flatten_op, float elevation,float factor, float center_x,	float center_y,	float radius);
		/**
		* Note: terrain data should be in normalized space with height in the range [0,1]. 
		* Picks a point and scales the surrounding terrain in a circular manner. 
		* Can be used to make all sorts of circular shapes. Still needs some work. 
		*  radial_scale: pick a point (center_x, center_y) and scale the points 
		*      where distance is mindist<=distance<=maxdist linearly.  The formula
		*      we'll use for a nice sloping smoothing factor is (-cos(x*3)/2)+0.5.
		*/
		void RadialScale (float center_x, float center_y, float scale_factor, float min_dist,float max_dist, float smooth_factor, int frequency);

		/*
		*  This creates a Gaussian hill at the specified location with the specified parameters.
		*  it actually adds the hill to the original terrain surface.
		*  Here ElevNew(x,y) = 
				|(x,y)-(center_x,center_y)| < radius*smooth_factor,	ElevOld(x,y)+hscale*exp(-[(x-center_x)^2+(y-center_y)^2]/(2*standard_deviation^2) ),
				|(x,y)-(center_x,center_y)| > radius*smooth_factor, minimize hill effect.
		* @param center_x: the center of the affected circle. value in the range [0,1]
		* @param center_y: the center of the affected circle.value in the range [0,1]
		* @param radius: the radius of the affected circle.value in the range [0,0.5]
		* @param hscale: scale factor. One can think of it as the maximum height of the Gaussian Hill. this value can be negative
		* @param standard_deviation: standard deviation of the unit height value. should be in the range (0,1). 
		*  0.1 is common value. larger than that will just make a flat hill with smoothing.
		* @param smooth_factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void GaussianHill (float x,float y,float radius,float hscale,float standard_deviation,float smooth_factor);

		/**
		* offset in a spherical region
		*/
		void Spherical ( float    offset);
		/* 
		* return the sum of the neighboring cells in a square size with 
		* sides 1+(size*2) long
		*/
		static float grid_neighbour_sum_size(TTerrain*terrain,int x, int y,int size);
		/* 
		* return the average of the neighboring cells in a square size with 
		* sides 1+(size*2) long
		*/
		static float grid_neighbour_average_size(TTerrain*terrain,
			int      x, 
			int      y,
			int      size);
		/**
		square filter for sharpening and smoothing. 
		Use neighbour-averaging to roughen or smooth the height field. The factor 
		determines how much of the computed roughening is actually applied to the 
		height field. In it's default invocation, the 4 directly neighboring 
		squares are used to calculate the roughening. If you select big sampling grid, 
		all 8 neighboring cells will be used. 
		* @param roughen: true for sharpening, false for smoothing.
		* @param big_grid: true for 8 neighboring cells, false for 4. 
		* @param factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void Roughen_Smooth (bool roughen, bool big_grid,float factor);

		/** create a ramp (inclined slope) from height(x1,y1) to height(x2,y2). The ramp's half width is radius. 
		* this is usually used to created a slope path connecting a high land with a low land. 
		* @param radius: The ramp's half width
		* @param borderpercentage: borderpercentage*radius is how long the ramp boarder is to linearly interpolate with the original terrain. specify 0 for sharp ramp border.
		* @param factor: in range[0,1]. it is the smoothness to merge with other border heights.Specify 1.0 for a complete merge
		*/
		void Ramp(float x1, float y1, float height1, float x2, float y2, float height2, float radius, float borderpercentage=0.5f, float factor=1.0f);
		
		/**
		* load height field from file
		* @param fHeight : height of the edge 
		* @param nSmoothPixels:  the number of pixels to smooth from the edge of the height field. 
		* if this is 0, the original height field will be loaded unmodified. if it is greater than 0, the loaded height field 
		* will be smoothed for nSmoothPixels from the edge, where the edge is always fHeight. The smooth function is linear. For example,
		* - 0% of original height  for the first pixel from the edge 
		* - 1/nSmoothPixels of original height for the second pixel from the edge. Lerp(1/nSmoothPixels, fheight, currentHeight)
		* - 2/nSmoothPixels of original height for the third.Lerp(2/nSmoothPixels, fheight, currentHeight )
		* - 100% for the nSmoothPixels-1 pixel 
		*/
		void SetConstEdgeHeight(float fHeight=0, int nSmoothPixels=7);

		/**
		* merge two terrains, and save the result to the current terrain. The three terrains are aligned by their center. 
		* the input terrain can be the current terrain. The two input terrain must not be normalized.
		*/
		void Merge (TTerrain *terrain_1,
			TTerrain *terrain_2,
			float    weight_1,
			float    weight_2,
			MergeOperation      operation);

	private:
		/** the terrain height field array to be additively applied to the target.*/
		TTerrain* m_pTerrainData;
	};
}
