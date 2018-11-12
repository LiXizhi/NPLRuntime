/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#pragma once

#include "prect.h"
#include "indexer.h"

namespace Pinocchio {

	template<class Data, int Dim>
	class DNode : public Data
	{
	public:
		typedef DNode<Data, Dim> Self;
		typedef PVector<double, Dim> Vec;
		typedef Rect<double, Dim> MyRect;

		int countNodes() const
		{
			int nodes = 1;
			if (m_Children[0] != NULL)
				for (int i = 0; i < numChildren; ++i)
					nodes += m_Children[i]->countNodes();
			return nodes;
		}

		int maxLevel() const
		{
			if (m_Children[0] == NULL)
				return 0;
			int ml = 0;
			for (int i = 0; i < numChildren; ++i)
				ml = max(ml, m_Children[i]->maxLevel());
			return 1 + ml;
		}

		Self *getParent() const { return m_Parent; }
		Self *getChild(int idx) const { return m_Children[idx]; }
		const MyRect &getRect() const { return m_Rect; }
		int getChildIndex() const { return m_ChildIndex; }

		static const int numChildren = 1 << Dim;

	private:
		DNode(MyRect r) : Data(this), m_Parent(NULL), m_Rect(r)
		{
			zeroChildren();
			Data::init();
		}

		DNode(Self *inParent, int inChildIndex) : Data(this), m_Parent(inParent), m_ChildIndex(inChildIndex)
		{
			zeroChildren();
			m_Rect = MyRect(inParent->m_Rect.getCorner(m_ChildIndex)) | MyRect(inParent->m_Rect.getCenter());
			Data::init();
		}

		~DNode()
		{
			for (int i = 0; i < numChildren; ++i)
				if (m_Children[i])
					delete m_Children[i];
		}

		void split()
		{
			for (int i = 0; i < numChildren; ++i)
				m_Children[i] = new Self(this, i);
		}

		template<class D, int DI, template<typename N, int ID> class IDX> friend class DRootNode;

		void zeroChildren() { for (int i = 0; i < numChildren; ++i) m_Children[i] = NULL; }

		//data
		Self* m_Parent;
		Self* m_Children[numChildren];
		int m_ChildIndex;

		MyRect m_Rect;
	};

	template<class Data, int Dim, template<typename Node, int IDim> class Indexer = DumbIndexer>
	class DRootNode : public DNode<Data, Dim>, public Indexer<DNode<Data, Dim>, Dim>
	{
	public:
		typedef DNode<Data, Dim> Node;
		typedef DRootNode<Data, Dim, Indexer> Self;
		typedef Indexer<Node, Dim> MyIndexer;
		typedef PVector<double, Dim> Vec;
		typedef Rect<double, Dim> MyRect;

		DRootNode(MyRect r = MyRect(Vec(), Vec().apply(bind2nd(plus<double>(), 1.)))) : Node(r)
		{
			MyIndexer::setRoot(this);
		}

		void split(Node *node)
		{
			node->split();
		}
	};

}// namespace Pinocchio

