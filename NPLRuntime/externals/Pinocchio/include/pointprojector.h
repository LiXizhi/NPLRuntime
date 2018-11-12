/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#pragma once

#include "vector.h"
#include "rect.h"
#include "vecutils.h"
#include "debugging.h"

#include <set>
#include <algorithm>

namespace Pinocchio {

	struct Vec3Object
	{
		Vec3Object(const PVector3 &inV) : v(inV) {}

		Rect3 boundingRect() const { return Rect3(v); }
		double operator[](int i) const { return v[i]; }
		PVector3 project(const PVector3 &) const { return v; }

		PVector3 v;
	};

	struct Tri3Object
	{
		Tri3Object(const PVector3 &inV1, const PVector3 &inV2, const PVector3 &inV3) : v1(inV1), v2(inV2), v3(inV3) {}

		Rect3 boundingRect() const { return Rect3(v1) | Rect3(v2) | Rect3(v3); }
		double operator[](int i) const { return v1[i] + v2[i] + v3[i]; } //for comparison only, no need to divide by 3
		PVector3 project(const PVector3 &v) const { return projToTri(v, v1, v2, v3); }

		PVector3 v1, v2, v3;
	};

	template<int Dim, class Obj>
	class ObjectProjector
	{
	public:
		typedef PVector<double, Dim> Vec;
		typedef Rect<double, Dim> Rec;

		ObjectProjector() {}
		ObjectProjector(const vector<Obj> &inObjs) : m_Objs(inObjs)
		{
			vector<int> orders[Dim];
			for (int d = 0; d < Dim; ++d) {
				orders[d].reserve(m_Objs.size());
				for (int i = 0; i < (int)m_Objs.size(); ++i) orders[d].push_back(i);
				std::sort(orders[d].begin(), orders[d].end(), DLess(d, m_Objs));
			}

			m_rNodes.reserve((int)m_Objs.size() * 2 - 1);
			buildRTree(orders);
		}

		Vec project(const Vec &from) const
		{
			double minDistSq = 1e37;
			Vec closestSoFar;

			int sz = 1;
			static pair<double, int> todo[10000];
			todo[0] = make_pair(m_rNodes[0].rect.distSqTo(from), 0);

			while (sz > 0) {
				if (todo[--sz].first > minDistSq) continue;
				int cur = todo[sz].second; //the top element that was just popped

				int c1 = m_rNodes[cur].child1;
				int c2 = m_rNodes[cur].child2;

				if (c1 >= 0) { //not a leaf
					double len1 = m_rNodes[c1].rect.distSqTo(from);
					if (len1 < minDistSq) todo[sz++] = make_pair(len1, c1);

					double len2 = m_rNodes[c2].rect.distSqTo(from);
					if (len2 < minDistSq) todo[sz++] = make_pair(len2, c2);

					if (sz >= 2 && todo[sz - 1].first > todo[sz - 2].first) {
						swap(todo[sz - 1], todo[sz - 2]);
					}
					if (sz > 9995) {//getting close to our array limit
						Debugging::out() << "Large todo list, likely to fail" << endl;
					}
					continue;
				}
				else {
					//leaf -- consider the object
					Vec curPt = m_Objs[c2].project(from);
					double distSq = (from - curPt).lengthsq();
					if (distSq <= minDistSq) {
						minDistSq = distSq;
						closestSoFar = curPt;
					}
				}
			}

			if (closestSoFar.length() < 0.0001) {
				int a = 0;// debug use
			}

			return closestSoFar;
		};

		bool intersected(const Rect3& rect)const
		{
			bool intersected = false;
			for (const Obj& tri : m_Objs) {
				Rect3 triRect = tri.boundingRect();
				/*if ((rect.getCenter() - triRect.getCenter()).length() <
					(rect.getSize().length() + triRect.getSize().length())*0.5){
					intersected = true;
					break;
				}*/
				if (rect.contains(tri.v1) || rect.contains(tri.v2) || rect.contains(tri.v3)) {
					intersected = true;
					break;
				}

				if (rect.getSize().length() > 0.05 && (rect.getCenter() - triRect.getCenter()).length() < (rect.getSize().length() + triRect.getSize().length())*0.5) {
					intersected = true;
					break;
				}
			}
			return intersected;
		}

		struct RNode
		{
			Rec rect;
			int child1, child2; //if child1 is -1, child2 is the object index
		};

		const vector<RNode> &getRNodes() const { return m_rNodes; }

	private:

		struct DL {
			bool operator()(const pair<double, int> &p1,
				const pair<double, int> &p2) const {
				return p1.first > p2.first;
			}
		};

		/** The key idea of the data structure is to group nearby objects and represent them with their
		* minimum bounding rectangle in the next higher level of the tree; the "R" in R-tree is for rectangle.
		* Since all objects lie within this bounding rectangle, a query that does not intersect the bounding
		* rectangle also cannot intersect any of the contained objects.
		* At the leaf level, each rectangle describes a single object; at higher levels the aggregation of an
		* increasing number of objects. This can also be seen as an increasingly coarse approximation of the data set.
		* -- from wikipedia
		*/
		int buildRTree(vector<int> orders[Dim], int curDim = 0)
		{
			int out = m_rNodes.size();
			m_rNodes.resize(out + 1);

			int num = orders[0].size();
			if (num == 1) {
				m_rNodes[out].rect = m_Objs[orders[0][0]].boundingRect();
				m_rNodes[out].child1 = -1;
				m_rNodes[out].child2 = orders[0][0];
			}
			else {
				vector<int> orders1[Dim], orders2[Dim];
				set<int> left;
				for (int i = 0; i < num / 2; ++i) {
					left.insert(orders[curDim][i]);
				}

				for (int d = 0; d < Dim; ++d) {
					orders1[d].reserve((num + 1) / 2);
					orders2[d].reserve((num + 1) / 2);
					for (int i = 0; i < num; ++i) {
						if (left.count(orders[d][i])) {
							orders1[d].push_back(orders[d][i]);
						}
						else {
							orders2[d].push_back(orders[d][i]);
						}
					}
				}

				m_rNodes[out].child1 = buildRTree(orders1, (curDim + 1) % Dim);
				m_rNodes[out].child2 = buildRTree(orders2, (curDim + 1) % Dim);
				m_rNodes[out].rect = m_rNodes[m_rNodes[out].child1].rect | m_rNodes[m_rNodes[out].child2].rect;
			}
			return out;
		}

		class DLess
		{
		public:
			DLess(int inDim, const vector<Obj> &inObjs) : dim(inDim), objs(inObjs) {}
			bool operator()(int i1, int i2) { return objs[i1][dim] < objs[i2][dim]; }
		private:
			int dim;
			const vector<Obj> &objs;
		};

		vector<RNode> m_rNodes;
		vector<Obj> m_Objs;
	};

} // namespace Pinocchio
