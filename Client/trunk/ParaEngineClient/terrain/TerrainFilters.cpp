//-----------------------------------------------------------------------------
// Class:	CTerrainFilters
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12.8
// Revised: 2005.12.8
// Desc: a group of filters such as raise,lower,smooth,flatten which can be applied
// to m*m terrain array. This is usually used by the terrain modifier in paraengine.
// Some of the filter code refers to Terraform by Robert Gasch (r.gasch@chello.nl) 
// http://terraform.sourceforge.net
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TextureEntity.h"
#include "Loader.h"
#include "TTerrain.h"
#include "TerrainFilters.h"

#include "memdebug.h"

using namespace ParaEngine;

/*
* This macro updates the terrain taking into account the
* selection.
*/
#define APPLY(terrain,pos,value)                                    \
	if (terrain->selection == NULL)                                 \
	terrain->heightfield[pos] = value;                              \
  else                                                              \
  terrain->heightfield[pos] =										\
  terrain->heightfield[pos] * (1.0f - terrain->selection[pos]) +		\
  value * terrain->selection[pos];

#define g_return_if_fail(x) if(!(x)) return;
#define g_return_val_if_fail(x,value) if(!(x)) return (value);

#define g_new(datatype, size) new datatype[size]
#define g_free(obj) if(obj) {delete [] obj; obj=NULL;}

/*
* Other utility macros
*/
#define INTERPOLATE(x,y,frac)  (1-(frac))*(x) + (frac)*(y)

#define X_WRAP(x)  x = ((x + terrain->width) % terrain->width)
#define Y_WRAP(y)  y = ((y + terrain->height) % terrain->height)

#define X_CLIP(x)  x = (x<0 ? 0 : ((x>=terrain->width)?(terrain->width-1):x))
#define Y_CLIP(y)  y = (y<0 ? 0 : ((y>=terrain->height)?(terrain->height-1):y))

#define ATAN2(y,x) ((x==0 && y==0)?0:atan2(y,x))


//////////////////////////////////////////////////////////////////////////
// terrain filer
//////////////////////////////////////////////////////////////////////////
CTerrainFilters::CTerrainFilters(void)
{
	m_pTerrainData = new TTerrain();
}

CTerrainFilters::~CTerrainFilters(void)
{
	SAFE_DELETE(m_pTerrainData);
}

TTerrain* CTerrainFilters::GetTerrainData()
{
	return m_pTerrainData;
}

void CTerrainFilters::Flatten (FlattenOperation flatten_op, float elevation,float factor,
							   float center_x,
							   float center_y,
							   float radius)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	int    x, y;
	int    pos;
	int    width, height;
	float *data;
	float  fxmin, fxmax, fymin, fymax;
	int    xmin, xmax, ymin, ymax;
	int    xcent, ycent;

	g_return_if_fail (center_x >= 0.0);
	g_return_if_fail (center_y >= 0.0);
	g_return_if_fail (center_x <= 1.0);
	g_return_if_fail (center_y <= 1.0);
	
	data = terrain->heightfield;
	width = terrain->width;
	height = terrain->height;

	/* calculate max and min coordinates on 0 .. 1 scale */
	fxmin = center_x - radius;
	fxmax = center_x + radius;
	fymin = center_y - radius;
	fymax = center_y + radius;
	if (fxmin < 0.0) fxmin = 0.0;
	if (fxmax > 1.0) fxmax = 1.0;
	if (fymin < 0.0) fymin = 0.0;
	if (fymax > 1.0) fymax = 1.0;

	/* translate everything to subscript scale (0 .. n) */
	xmin = (int) (fxmin * width);
	xmax = (int) (fxmax * width);
	ymin = (int) (fymin * height);
	ymax = (int) (fymax * height);
	xcent = (int) (center_x * width);
	ycent = (int) (center_y * width);
	radius *= ((width + height) * 0.5f);
	//radius *= (width * 0.5f);
	
	float factorscale_ = width-radius;
	if(factorscale_>0.001)
		factorscale_ = 1/factorscale_;

	for (y = ymin; y < ymax; y++) 
	{
		pos = y * width + xmin;
		for (x = xmin; x < xmax; x++) 
		{
			float distance, temp;
			distance = float((xcent - x) * (xcent - x) + (ycent - y) * (ycent - y));
			if(distance>0.001f)
				distance = sqrt(distance);
			float factor_ = 0;
			if (distance <= radius)
			{
				factor_ = factor;
			}
			else
			{
				float s = (distance-radius)*factorscale_;
				factor_ = factor*s;// in fact this is 0*s+factor*(1-s)
			}
			temp = data[pos];
			if( ((flatten_op == Flatten_Op) && (temp != elevation)) || 
				((flatten_op == ShaveTop_Op) && (temp > elevation)) || 
				((flatten_op == Fill_Op) && (temp < elevation)) )
			{
				float value = elevation - (elevation - temp) * (1.0f - factor_);
				APPLY (terrain, pos, value);
			}
			pos++;
		} 
	} 
}

void CTerrainFilters::RadialScale (float    center_x, 
						float    center_y, 
						float    scale_factor, 
						float    min_dist,
						float    max_dist, 
						float    smooth_factor,
						int      frequency)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	int    pos, x, y, i;
	float  max_size;
	float  band_width;
	float *data;

	g_return_if_fail (center_x >= 0.0);
	g_return_if_fail (center_y >= 0.0);
	g_return_if_fail (center_x <= 1.0);
	g_return_if_fail (center_y <= 1.0);
	g_return_if_fail (min_dist >= 0.0);
	g_return_if_fail (max_dist >= 0.0);
	g_return_if_fail (min_dist <= 1.0);
	g_return_if_fail (max_dist <= 1.0);
	g_return_if_fail (smooth_factor >= 0.005);
	g_return_if_fail (smooth_factor <= 1.0);

	if (fabs (min_dist - max_dist) < 0.01)
		return;

	max_size = (float) max (terrain->width, terrain->height);

	data = terrain->heightfield;

	center_x *= terrain->width;
	center_y *= terrain->height;
	min_dist *= max_size;
	max_dist *= max_size;
	band_width = (max_dist-min_dist)/frequency;

	for (i=0; i<frequency; i++)
	{
		float min_d = min_dist + band_width * i;
		float max_d = min_dist + band_width * (i+1);
		float s_low = min_d + (max_d - min_d) * 0.5f * smooth_factor;
		float s_high = max_d - (max_d - min_d) * 0.5f * smooth_factor;
		float cd = min_d + band_width/2;

		pos = 0;
		for (y = 0; y < terrain->height; y++)
			for (x = 0; x < terrain->width; x++)
			{
				float point_old, point;
				float distance;

				distance = sqrt ((x - center_x) * (x - center_x) +
					(y - center_y) * (y - center_y));

				point_old = data[pos];

				
				if (distance >= min_d && distance <= cd)
				{
					/* Lower half of scaling area. */
					point = (distance - min_d) / (cd - min_d);

					/* Increase linearly from inner end to middle. */
					point = (point_old+scale_factor) * point+ point_old * (1 - point);

					if (distance <= s_low)
					{
						float temp;

						temp = point;
						point = (distance - min_d) / (s_low - min_d);
						point = -(cos (point * 3.0f) * 0.5f) + 0.5f;
						point = point_old * (1.0f - point) + temp * point;
					}

					APPLY (terrain, pos, point);
				}
				else if (distance <= max_d && distance >= cd)
				{
					/* Upper half of scaling area. */
					point = (max_d - distance) / (max_d - cd);

					/* Increase linearly from higher end to middle. */
					point = (point_old+scale_factor) * point + point_old * (1 - point);

					if (distance > s_high)
					{
						float temp;

						temp = point;
						point = (max_d - distance) / (max_d - s_high);
						point = -(cos (point * 3.0f) * 0.5f) + 0.5f;
						point = point_old * (1.0f - point) + temp * point;
					}

					APPLY (terrain, pos, point);
				}

				pos++;
			}
	}
}

void CTerrainFilters::GaussianHill (float    center_x,
						 float    center_y,
						 float    radius,
						 float    hscale,
						 float    standard_deviation,
						 float    smooth_factor)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	int    x, y;
	int    pos;
	int    width, height;
	float *data;
	float  fxmin, fxmax, fymin, fymax;
	float  sdist, sdistr;
	int    xmin, xmax, ymin, ymax;
	int    xcent, ycent;

	g_return_if_fail (center_x >= 0.0);
	g_return_if_fail (center_y >= 0.0);
	g_return_if_fail (center_x <= 1.0);
	g_return_if_fail (center_y <= 1.0);
	g_return_if_fail (smooth_factor >= 0.0);
	g_return_if_fail (smooth_factor <= 1.0);
	
	data = terrain->heightfield;
	width = terrain->width;
	height = terrain->height;

	/* calculate max and min coordinates on 0 .. 1 scale */
	fxmin = center_x - radius;
	fxmax = center_x + radius;
	fymin = center_y - radius;
	fymax = center_y + radius;
	if (fxmin < 0.0) fxmin = 0.0;
	if (fxmax > 1.0) fxmax = 1.0;
	if (fymin < 0.0) fymin = 0.0;
	if (fymax > 1.0) fymax = 1.0;

	/* translate everything to subscript scale (0 .. n) */
	xmin = (int) (fxmin * width);
	xmax = (int) (fxmax * width);
	ymin = (int) (fymin * height);
	ymax = (int) (fymax * height);
	xcent = (int) (center_x * width);
	ycent = (int) (center_y * width);
	radius *= ((width + height) * 0.5f);
	sdist = radius * smooth_factor;
	sdistr = radius - sdist;

	standard_deviation = 2*standard_deviation * standard_deviation;
	for (y = ymin; y < ymax; y++) 
	{
		float ya;

		ya = (((float) y + 0.5f) / height) - center_y;
		ya = ya * ya;

		pos = y * terrain->width + xmin;
		for (x = xmin; x < xmax; x++) 
		{
			float xa, distance, temp;

			distance = sqrt (float((xcent - x) * (xcent - x) +
				(ycent - y) * (ycent - y)));
			if (distance <= radius)
			{
				float radius;

				temp = data[pos];
				xa = (((float) x + 0.5f) / width) - center_x;
				xa = xa * xa;
				radius = xa + ya;

				/* see if we should be smoothing */
				if (distance > sdistr)
				{
					float sf;

					distance -= sdistr;
					sf = 1.0f - distance / sdist;
					temp +=  sf * hscale *
						exp (-radius / standard_deviation);
				}
				else
					temp += hscale * exp (-radius / standard_deviation);

				APPLY (terrain, pos, temp);
			}

			pos++;
		} 
	} 
}

void CTerrainFilters::Spherical ( float    offset)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	int    x, y;
	int    width;
	int    height;
	float *raster;

	g_return_if_fail (offset >= 0.0);
	g_return_if_fail (offset <= 1.0);

	width = terrain->width;
	height = terrain->height;
	raster = g_new (float, width);

	for (y = 0; y < height; y++)
	{
		float  *data;
		float  scale;
		float  from_x;
		float  fract_x;
		int      int_x;

		scale = ((y * 2.0f) / (height - 1)) - 1.0f;
		scale = sqrt (1.0f - scale * scale);

		data = &terrain->heightfield[y * width];
		memcpy (raster, data, width * sizeof (float));

		/* Stretch a row of the terrain outwards */
		for (x = 0; x < width; x++)
		{
			float a, b;
			float height;

			from_x = (x - width / 2.0f) * scale + width / 2.0f;
			int_x = (int) from_x;
			fract_x = from_x - int_x;

			if (int_x < 0)
				a = 0.0f;
			else
				a = raster[int_x];

			if (int_x + 1 >= width)
				b = 0.0f;
			else
				b = raster[int_x + 1];

			height = a * (1.0f - fract_x) + b * fract_x;
			data[x] = height;
		}

		from_x = (width * offset * 0.5f - width / 2.0f) * scale + width / 2.0f;
		int_x = (int) from_x;

		/* Make the terrain tilable in the X direction */
		for (x = 0; x < int_x; x++)
		{
			float k;

			k = ((float) x) / int_x;
			data[width - x - 1] = data[width - x - 1] * k + data[x] * (1.0f - k);
		}
	}
	g_free (raster);
}


float CTerrainFilters::grid_neighbour_sum_size(TTerrain*terrain,
						  int      x, 
						  int      y,
						  int      size)
{
	int   xx;
	int   yy;
	int   xoff = max (x-size, 0);
	int   yoff = max (y-size, 0);
	int   xlim = min (x+size, terrain->width-1);
	int   ylim = min (y+size, terrain->height-1);
	float sum = 0;

	g_return_val_if_fail (x>=0, -1);
	g_return_val_if_fail (y>=0, -1);
	g_return_val_if_fail (size>0, -1);

	for (xx=xoff; xx<=xlim; xx++)
		for (yy=yoff; yy<=ylim; yy++)
			if (xx!=x || yy!=y)
				sum += terrain->heightfield[yy*terrain->width+xx];

	return sum;
} 

float CTerrainFilters::grid_neighbour_average_size(TTerrain*terrain,
							 int      x, 
							 int      y,
							 int      size)
{
	int   xoff = max (x-size, 0);
	int   yoff = max (y-size, 0);
	int   xlim = min (x+size, terrain->width-1);
	int   ylim = min (y+size, terrain->height-1);

	float num = (float)((((xlim-xoff)+1) * ((ylim-yoff)+1)) - 1);
	float sum;

	g_return_val_if_fail (num>0, -1);
	sum = grid_neighbour_sum_size (terrain, x, y, size);

	return sum/num;
}

void CTerrainFilters::Roughen_Smooth (bool  roughen, bool  big_grid,float factor)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);

	float *data;
	int    width, height;
	int    x, y;

	g_return_if_fail (factor >= 0.0);
	g_return_if_fail (factor <= 1.0);

	data = terrain->heightfield;
	width = terrain->width;
	height = terrain->height;

	for (y = 1; y < height - 1; y++)
		for (x = 1; x < width - 1; x++)
		{
			int    pos = y * width + x;
			int    size = (big_grid ? 2 : 1);
			float  value, original, average;

			original = data[pos];
			average = grid_neighbour_average_size (terrain, x, y, size);

			if (roughen)
				value = original - factor * (average - original);
			else
				value = original + factor * (average - original);

			APPLY (terrain, pos, value);
		}
}

void CTerrainFilters::Merge (TTerrain *terrain_1,
				 TTerrain *terrain_2,
				 float    weight_1,
				 float    weight_2,
				 MergeOperation      operation)
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	int      x, y;
	int      offset_x_1, offset_y_1;
	int      offset_x_2, offset_y_2;
	int      width_1, width_2;
	float   *data, *data_1, *data_2;
	
	offset_x_1 = (terrain_1->width - terrain->width) >> 1;
	offset_y_1 = (terrain_1->height - terrain->height) >> 1;
	offset_x_2 = (terrain_2->width - terrain->width) >> 1;
	offset_y_2 = (terrain_2->height - terrain->height) >> 1;
	
	// By LXZ: the size of the input are arbitrary now.
	int nYfrom = 0;
	int nXfrom = 0;
	if(offset_y_1<0 || offset_y_2<0)
	{
		nYfrom = (offset_y_1<offset_y_2) ? -offset_y_1 : -offset_y_2;
	}
	if(offset_x_1<0 || offset_x_2<0)
	{
		nXfrom = (offset_x_1<offset_x_2) ? -offset_x_1 : -offset_x_2;
	}
	int nXTo = terrain->width - nXfrom;
	int nYTo = terrain->height - nYfrom;

	data = terrain->heightfield;
	data_1 = terrain_1->heightfield;
	data_2 = terrain_2->heightfield;

	width_1 = terrain_1->width;
	width_2 = terrain_2->width;
	
	switch (operation)
	{
	case Subtract: /* Subtract */
		weight_2 = -weight_2;

	case Addition: /* Addition */
		for (y = nYfrom; y < nYTo; y++)
			for (x = nXfrom; x < nXTo; x++)
				data[y * terrain->width + x] =
				data_1[(offset_y_1 + y) * width_1 + (offset_x_1 + x)] * weight_1 +
				data_2[(offset_y_2 + y) * width_2 + (offset_x_2 + x)] * weight_2;
		break;

	case Multiplication: /* Multiplication */
		for (y = nYfrom; y < nYTo; y++)
			for (x = nXfrom; x < nXTo; x++)
				data[y * terrain->width + x] =
				(1.0f + data_1[(offset_y_1 + y) * width_1 + (offset_x_1 + x)] * weight_1) *
				(1.0f + data_2[(offset_y_2 + y) * width_2 + (offset_x_2 + x)] * weight_2);
		break;

	case Division: /* Division */
		for (y = nYfrom; y < nYTo; y++)
			for (x = nXfrom; x < nXTo; x++)
				data[y * terrain->width + x] =
				(1.0f + data_1[(offset_y_1 + y) * width_1 +
				(offset_x_1 + x)] * weight_1) /
				(1.0f + data_2[(offset_y_2 + y) * width_2 +
				(offset_x_2 + x)] * weight_2);
		break;

	case Minimum: /* Minimum */
		for (y = nYfrom; y < nYTo; y++)
			for (x = nXfrom; x < nXTo; x++)
				data[y * terrain->width + x] = min (
				data_1[(offset_y_1 + y) * width_1 + (offset_x_1 + x)] * weight_1,
				data_2[(offset_y_2 + y) * width_2 + (offset_x_2 + x)] * weight_2);
		break;

	case Maximum: /* Maximum */
		for (y = nYfrom; y < nYTo; y++)
			for (x = nXfrom; x < nXTo; x++)
				data[y * terrain->width + x] = max (
				data_1[(offset_y_1 + y) * width_1 + (offset_x_1 + x)] * weight_1,
				data_2[(offset_y_2 + y) * width_2 + (offset_x_2 + x)] * weight_2);
		break;
	}
}
// interpolation functions
template<class T>
inline T interpolate(const float r, const T &v1, const T &v2)
{
	return  (v1*(1.0f - r) + v2*r);
}


void CTerrainFilters::SetConstEdgeHeight(float fHeight, int nSmoothPixels)
{
	if(nSmoothPixels<=0)
		return;

	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);
	
	int    width, height;
	float *data;

	data = terrain->heightfield;
	width = terrain->width;
	height = terrain->height;
	
	for (int i=0; i<width; i++)
	{
		int nOffset = i*height;

		for(int j=0;j<height;j++)
		{
			// number of pixels from the edge
			int nFromEdgePixels = min(min(i, width-i), min(j, height-j));
			if(nFromEdgePixels < nSmoothPixels)
			{
				terrain->heightfield[nOffset+j] = interpolate((float)nFromEdgePixels/(float)nSmoothPixels, fHeight, terrain->heightfield[nOffset+j]);
			}
		}
	}
}

/** create a ramp : inclined slope
LiXizhi 2009.1.30
*/
void ParaEngine::CTerrainFilters::Ramp( float x1, float y1,  float height1, float x2, float y2,  float height2, float radius, float borderpercentage, float factor )
{
	TTerrain*terrain = m_pTerrainData;
	PE_ASSERT(terrain!=NULL);

	float *data;
	int    width, height;
	int    x, y;
	bool big_grid = false;

	g_return_if_fail (x1 >= 0.0);
	g_return_if_fail (x2 >= 0.0);
	g_return_if_fail (y2 <= 1.0);
	g_return_if_fail (y2 <= 1.0);
	g_return_if_fail (x1!=x2 || y1!=y2);
	g_return_if_fail (factor >= 0.0);
	g_return_if_fail (factor <= 1.0);

	data = terrain->heightfield;
	width = terrain->width;
	height = terrain->height;
	
	/**
	Let the point be C (Cx,Cy) and the line be AB (Ax,Ay) to (Bx,By).    The length of the line segment AB is L:

	L= sqrt( (Bx-Ax)^2 + (By-Ay)^2 ) .Let P be the point of perpendicular projection of C onto AB. Let r be a parameter to indicate P's location along the line containing AB, with the following meaning:

	r=0      P = A
	r=1      P = B
	r<0      P is on the backward extension of AB
	r>1      P is on the forward extension of AB
	0<r<1    P is interior to ABCompute r with this:

	     (Ay-Cy)(Ay-By)-(Ax-Cx)(Bx-Ax)
	r = -----------------------------
	              L^2

	Use another parameter s to indicate the location along PC, with the following meaning:

	s<0      C is left of AB
	s>0      C is right of AB
	s=0      C is on ABCompute s as follows:

	      (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
	s = -----------------------------
	             L^2
	*/
	Vector2 vA(x1*width,y1*height);
	Vector2 vB(x2*width,y2*height);
	radius *= width;
	float borderwidth = borderpercentage*radius;
	float fLengthSq = (vB-vA).squaredLength();

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			int    pos = y * width + x;
			int    size = (big_grid ? 2 : 1);
			float  value, original, newvalue;
			original = data[pos];
			newvalue = original;

			Vector2 vC((float)x,(float)y);
			float r = ((vA.y-vC.y)*(vA.y-vB.y) - (vA.x-vC.x)*(vB.x-vA.x))/fLengthSq;
			float s = fabs(((vA.y-vC.y)*(vB.x-vA.x) - (vA.x-vC.x)*(vB.y-vA.y))/sqrt(fLengthSq));

			float fStrength = 1;
			if(r>=0 && r<=1)
			{
				newvalue = height1+r*(height2-height1);

				if(s <= (radius-borderwidth))
					fStrength = 1;
				else if(s < radius && borderwidth>0)
				{
					fStrength = (radius-s)/borderwidth;
				}
				else
					fStrength = 0;
			}
			else
				fStrength = 0;
			value = original + factor*fStrength * (newvalue - original);
			APPLY (terrain, pos, value);
		}
	}
}