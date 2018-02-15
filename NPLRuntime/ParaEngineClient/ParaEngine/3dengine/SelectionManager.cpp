//-----------------------------------------------------------------------------
// Class:	CSelectionManager
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.7.22
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "2dengine/GUIHighlight.h"
#include "2dengine/GUIBase.h"
#include "BaseObject.h"
#include "EventsCenter.h"
#include "PredefinedEvents.h"
#include "SelectionManager.h"

using namespace ParaEngine;

/**@def max number of groups.*/
#define MAX_GROUP_NUM 32

/** @def group ID 0-4 is always cached */
#define  MAX_CACHED_GROUPS_NUM	4

/** @def selection reference tag. */
#define  SELECTION_REF_TAG	10


CSelectionManager::CSelectionManager(void)
{
}

CSelectionManager::~CSelectionManager(void)
{
	int nGroupNum = (int)m_groups.size();
	for (int i=0;i<nGroupNum;++i)
	{
		auto& pGroup = m_groups[i];
		if(pGroup)
		{
			int nSize = (int)pGroup->m_items.size();
			for (int k=0;k<nSize;++k)
			{
				SelectedItem& item = pGroup->m_items[k];
				if(item)
				{
					if(item.Is3DObject())
					{
						item.GetAs3DObject()->OnDeSelect();
					}
					else if (item.Is2DObject())
						item.GetAs2DObject()->SetHighlightStyle(HighlightNone);
				}
			}
		}
	}
}

CSelectionManager* CSelectionManager::GetSingleton()
{
	static CSelectionManager g_selectionManager;
	return &g_selectionManager;
}

void CSelectionManager::OnRefDeleted(IRefObject* rm)
{
	if(DeleteReference(rm) == REF_SUCCEED)
	{
		RemoveObject((CBaseObject*)rm);
	}
}


int CSelectionManager::GetObjectCount(CBaseObject* pObject)
{
	if(pObject==0)
		return 0;
	int nCount = 0;
	int nGroupNum = (int)m_groups.size();
	for (int i=0;i<nGroupNum;++i)
	{
		auto& pGroup = m_groups[i];
		if(pGroup)
		{
			for (vector<SelectedItem>::iterator itCur = pGroup->m_items.begin(); itCur!=pGroup->m_items.end();++itCur)
			{
				SelectedItem& item = *itCur;
				if(item.GetObject() == pObject)
				{
					nCount++;
				}
			}
		}
	}
	return nCount;
}

void CSelectionManager::RemoveObject(CBaseObject* pObject)
{
	if(pObject==0)
		return;
	int nGroupNum = (int)m_groups.size();
	for (int i=0;i<nGroupNum;++i)
	{
		auto& pGroup = m_groups[i];
		if(pGroup)
		{
			for (vector<SelectedItem>::iterator itCur = pGroup->m_items.begin(); itCur!=pGroup->m_items.end();)
			{
				SelectedItem& item = *itCur;
				if (item.GetObject() == pObject)
				{
					itCur = pGroup->m_items.erase(itCur);
				}
				else
				{
					++itCur;
				}
			}
		}
	}
	if(pObject!=0)
	{
		pObject->OnDeSelect();
	}
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}

void CSelectionManager::RemoveObject(CGUIBase* pObject)
{
	if(pObject==0)
		return;
	int nGroupNum = (int)m_groups.size();
	for (int i=0;i<nGroupNum;++i)
	{
		auto& pGroup = m_groups[i];
		if (pGroup)
		{
			for (vector<SelectedItem>::iterator itCur = pGroup->m_items.begin(); itCur!=pGroup->m_items.end();)
			{
				SelectedItem& item = *itCur;
				if(item.GetObject() == pObject)
				{
					itCur = pGroup->m_items.erase(itCur);
				}
				else
				{
					++itCur;
				}
			}
		}
	}
	if(pObject!=0)
		pObject->SetHighlightStyle(HighlightNone);
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}

void CSelectionManager::AddObject(CBaseObject* pObject, int nGroupID)
{
	if(pObject==0)
		return;
	if(nGroupID<0)
	{
		RemoveObject(pObject);
		return;
	}

	if(pObject->GetSelectGroupIndex() >= 0)
	{
		CSelectionGroup* pLastGroup = CreateGroup(pObject->GetSelectGroupIndex());
		if(pLastGroup!=0 && pLastGroup->IsExclusive())
		{
			RemoveObject(pObject);
		}
	}
	
	CSelectionGroup* pGroup = CreateGroup(nGroupID);
	if(pGroup!=NULL)
	{
		bool bSelected = false;
		int nSize = (int)pGroup->m_items.size();
		for (int k=0;k<nSize;++k)
		{
			SelectedItem& item = pGroup->m_items[k];
			if (item.GetObject() == pObject)
			{
				bSelected = true;
			}
		}

		if(!bSelected)
		{
			pGroup->m_items.push_back(SelectedItem(pObject));
			UpdateGroup(nGroupID);
		}

		if(pObject!=0)
		{
			pObject->OnSelect(nGroupID);
		}
	}
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}


void CSelectionManager::AddObject(CGUIBase* pObject, int nGroupID)
{
	if(nGroupID<0)
	{
		RemoveObject(pObject);
		return;
	}
	CSelectionGroup* pGroup = CreateGroup(nGroupID);
	if(pGroup!=NULL)
	{
		pGroup->m_items.push_back(SelectedItem(pObject));

		UpdateGroup(nGroupID);

		if(pObject!=0)
			pObject->SetHighlightStyle(Highlight4outsideArrow);
	}
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}

bool CSelectionManager::GetObject(int nGroupID, int nItemIndex, SelectedItem* pOut)
{
	if(pOut == NULL)
		return false;
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		if(nItemIndex < (int)pGroup->m_items.size())
		{
			*pOut = pGroup->m_items[nItemIndex];
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

int CSelectionManager::GetItemNumInGroup(int nGroupID)
{
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		return (int)pGroup->m_items.size();
	}
	else
		return 0;
}

void CSelectionManager::SelectGroup(int nGroupID, bool bSelect)
{
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		pGroup->m_bSelected = bSelect;
	}
}

void CSelectionManager::ClearGroup(int nGroupID)
{
	int nGroupNum = (int)m_groups.size();
	if(nGroupID == -1)
	{
		for (int i=0;i<nGroupNum;++i)
		{
			auto& pGroup = m_groups[i];
			if (pGroup)
			{
				int nSize = (int)pGroup->m_items.size();
				for (int k=0;k<nSize;++k)
				{
					SelectedItem& item = pGroup->m_items[k];
					if(item)
					{
						if(item.Is3DObject())
						{
							item.GetAs3DObject()->OnDeSelect();
						}
						else if (item.Is2DObject())
							item.GetAs2DObject()->SetHighlightStyle(HighlightNone);
					}
				}
				pGroup->m_items.clear();
			}
		}
	}
	else
	{
		if(nGroupID<nGroupNum)
		{
			auto& pGroup = m_groups[nGroupID];
			if(pGroup)
			{
				int nSize = (int)pGroup->m_items.size();
				for (int k=0;k<nSize;++k)
				{
					SelectedItem& item = pGroup->m_items[k];
					if(item)
					{
						if(item.Is3DObject())
						{
							item.GetAs3DObject()->OnDeSelect();
						}
						else if (item.Is2DObject())
							item.GetAs2DObject()->SetHighlightStyle(HighlightNone);
					}
				}

				if(nGroupID < MAX_CACHED_GROUPS_NUM)
					pGroup->m_items.clear();
				else
					m_groups[nGroupID].reset();
			}
		}
	}
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}

void CSelectionManager::UpdateGroup(int nGroupID)
{
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		int nMaxItemsNumber = pGroup->m_nMaxItemNumber;
		int nOldSize = (int)pGroup->m_items.size();
		if( nOldSize > nMaxItemsNumber)
		{
			int nOffset = nOldSize - nMaxItemsNumber;

			if(pGroup->m_bRemoveFromBack)
			{
				// remove from back
				int i=0;
				for (i=nOldSize - nOffset;i<nOldSize;i++)
				{
					SelectedItem& item = pGroup->m_items[i];
					if(item)
					{
						if(item.Is3DObject())
						{
							item.GetAs3DObject()->OnDeSelect();
						}
						else if (item.Is2DObject())
							item.GetAs2DObject()->SetHighlightStyle(HighlightNone);
					}
				}

				pGroup->m_items.resize(nMaxItemsNumber);
			}
			else
			{
				// remove from front

				int i=0;
				for (i=0;i<nOffset;i++)
				{
					SelectedItem& item = pGroup->m_items[i];
					if(item)
					{
						if(item.Is3DObject())
							item.GetAs3DObject()->OnDeSelect();
						else if (item.Is2DObject())
							item.GetAs2DObject()->SetHighlightStyle(HighlightNone);
					}
				}

				for (i=0; i< nMaxItemsNumber; ++i)
				{
					pGroup->m_items[i] = pGroup->m_items[i+nOffset];
				}
				pGroup->m_items.resize(nMaxItemsNumber);
			}
		}
	}
}

void CSelectionManager::SetMaxItemNumberInGroup(int nGroupID, int nMaxItemsNumber)
{
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		pGroup->m_nMaxItemNumber = nMaxItemsNumber;
		UpdateGroup(nGroupID);
	}
	else
	{
		if(nMaxItemsNumber>1)
		{
			CSelectionGroup* pGroup = CreateGroup(nGroupID);
			if(pGroup!=NULL)
			{
				pGroup->m_nMaxItemNumber = nMaxItemsNumber;
				UpdateGroup(nGroupID);
			}
		}
	}
}

int CSelectionManager::GetMaxItemNumberInGroup(int nGroupID)
{
	CSelectionGroup* pGroup = GetGroup(nGroupID);
	if(pGroup!=NULL)
	{
		return pGroup->m_nMaxItemNumber;
	}
	return 1;
}

CSelectionGroup* CSelectionManager::GetGroup(int nGroupID)
{
	int nGroupNum = (int)m_groups.size();
	if(nGroupID<nGroupNum)
	{
		return m_groups[nGroupID].get();
	}
	else
		return NULL;
}

CSelectionGroup* CSelectionManager::CreateGroup(int nGroupID)
{
	int nGroupNum = (int)m_groups.size();
	if(nGroupID<nGroupNum)
	{
		if(!m_groups[nGroupID])
		{
			m_groups[nGroupID] = new CSelectionGroup();
		}
		return m_groups[nGroupID].get();
	}
	else if(nGroupID<MAX_GROUP_NUM)
	{
		m_groups.resize(nGroupID+1);
		m_groups[nGroupID] = new CSelectionGroup();		
		return m_groups[nGroupID].get();
	}
	else
	{
		OUTPUT_LOG("error: group ID can not exceed %d\r\n", MAX_GROUP_NUM);
		return NULL;
	}
	CGlobals::GetEventsCenter()->PostEvent(PredefinedEvents::SelectionChanged);
}

int ParaEngine::CSelectionManager::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return (int)m_groups.size();
	else
		return 0;
}

IAttributeFields* ParaEngine::CSelectionManager::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nRowIndex < (int)m_groups.size())
		return m_groups.at(nRowIndex).get();
	return NULL;
}


int ParaEngine::CSelectionGroup::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return nColumnIndex == 0 ? GetItemCount() : 0;
}

IAttributeFields* ParaEngine::CSelectionGroup::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	return nRowIndex < GetItemCount() ? GetItem(nRowIndex).m_pObjectRef.get() : NULL;
}
