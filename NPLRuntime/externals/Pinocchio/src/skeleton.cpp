#include "skeleton.h"
#include "utils.h"
#include "debugging.h"

#include <algorithm>

void Skeleton::InitCompressed()
{
    int i;

    m_fcMapV.resize(m_fPrevV.size(), -1);
    m_fcFractionV.resize(m_fPrevV.size(), -1.);
    
    for(i = 0; i < (int)m_fPrevV.size(); ++i) {
		
		// Make sure to always include the root in the compressed skel!
       // if(fGraphV.edges[i].size() == 2 && i != 0)
       //     continue;// no compressed skeleton 2018/7/24.
        m_fcMapV[i] = m_cfMapV.size();
        m_cfMapV.push_back(i);
    }
    
    m_cPrevV.resize(m_cfMapV.size(), -1);
    m_cSymV.resize(m_cfMapV.size(), -1);
    m_cGraphV.edges.resize(m_cfMapV.size());
    m_cFeetV = vector<bool>(m_cPrevV.size(), false);
    m_cFatV = vector<bool>(m_cPrevV.size(), false);
    
    for(i = 0; i < (int)m_cfMapV.size(); ++i) {
        m_cGraphV.verts.push_back(m_fGraphV.verts[m_cfMapV[i]]);
        
        //symmetry--TODO: need to make sure all unreduced bones in chain
        //          are marked as symmetric before marking the reduced one
        if(m_fSymV[m_cfMapV[i]] >= 0)
            m_cSymV[i] = m_fcMapV[m_fSymV[m_cfMapV[i]]];
        
        //prev
        if(i > 0) {
            int curPrev = m_fPrevV[m_cfMapV[i]];
            while(m_fcMapV[curPrev]  < 0)
                curPrev = m_fPrevV[curPrev];
            m_cPrevV[i] = m_fcMapV[curPrev];
        }
    }
    
    //graph edges
    for(i = 1; i < (int)m_cPrevV.size(); ++i) {
        m_cGraphV.edges[i].push_back(m_cPrevV[i]);
        m_cGraphV.edges[m_cPrevV[i]].push_back(i);
    }
    
    m_cLengthV.resize(m_cPrevV.size(), 0.);

    //lengths/fraction computation
    for(i = 1; i < (int)m_cPrevV.size(); ++i) {
        int cur = m_cfMapV[i];
		unordered_map<int, double> lengths;
        do {
            lengths[cur] = (m_fGraphV.verts[cur] - m_fGraphV.verts[m_fPrevV[cur]]).length();
            m_cLengthV[i] += lengths[cur];
            cur = m_fPrevV[cur];
        } while(m_fcMapV[cur] == -1);
        
        for(unordered_map<int, double>::iterator it = lengths.begin(); it != lengths.end(); ++it)
            m_fcFractionV[it->first] = it->second / m_cLengthV[i];
    }
}

void Skeleton::Scale(double factor)
{
    int i;
    for(i = 0; i < (int)m_fGraphV.verts.size(); ++i)
        m_fGraphV.verts[i] *= factor;
    for(i = 0; i < (int)m_cGraphV.verts.size(); ++i) {
        m_cGraphV.verts[i] *= factor;
        m_cLengthV[i] *= factor;
    }
}

void Skeleton::InsertJointNameMap(const std::string& name, int index)
{
	m_JointNames[name] = index;
	m_IndexNameMap[index] = name;
	m_fGraphV.edges.resize(m_JointNames.size());
}

void Skeleton::MakeJoint(const string &name, const PVector3 &pos, const string &previous)
{
	int cur = m_fSymV.size();
	m_fSymV.push_back(-1);
	m_fGraphV.verts.push_back(pos); //skeletons specified in [-1,1] will be fit to object in [0,1]
	m_fGraphV.edges.resize(cur + 1);
	m_JointNames[name] = cur;
	m_IndexNameMap[cur] = name;

	if (previous == string("")) {
		m_fPrevV.push_back(-1);
	}
	else { //add a bone
		int prev = m_JointNames[previous];
		m_fGraphV.edges[cur].push_back(prev);
		m_fGraphV.edges[prev].push_back(cur);
		m_fPrevV.push_back(prev);
	}
}

void Skeleton::MakeSymmetric()
{
	typedef std::vector < std::pair< std::string, std::string> > NamePairs;
	NamePairs leftNames;
	NamePairs rightNames;
	map<string, int>::iterator iter = m_JointNames.begin();
	for (; iter != m_JointNames.end(); ++iter) {
		std::string str = iter->first;
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		std::string::size_type  strIt = str.find("LEFT");
		if (strIt != std::string::npos)leftNames.push_back(std::make_pair(str, iter->first));
		strIt = str.find("RIGHT");
		if (strIt != std::string::npos)rightNames.push_back(std::make_pair(str, iter->first));
	}
	// mark symmetric
	for (int i = 0; i < leftNames.size(); ++i) {
		std::string left = leftNames[i].first;
		std::string::size_type pL = left.find("LEFT");
		left = left.erase(pL, 4);
		for (int j = 0; j < rightNames.size(); ++j) {
			std::string right = rightNames[j].first;
			std::string::size_type pR = right.find("RIGHT");
			right = right.erase(pR, 5);
			if (right.compare(left) == 0) {
				int i1 = m_JointNames[leftNames[i].second];
				int i2 = m_JointNames[rightNames[j].second];
				if (i1 > i2)
					swap(i1, i2);
				m_fSymV[i2] = i1;
			}
		}
	}

}

void Skeleton::SetFoot(const string &name)
{
    int i = m_JointNames[name];
    m_cFeetV[m_fcMapV[i]] = true;
}

void Skeleton::SetFat(const string &name)
{
    int i = m_JointNames[name];
    m_cFatV[m_fcMapV[i]] = true;
}

bool Skeleton::IsWellConnected()
{
	// check root number
	int numRoot = 0;
	for (int i = 0; i < m_fPrevV.size(); ++i) {
		if (m_fPrevV[i] == -1)numRoot++;
	}
	if (numRoot != 1) return false;

	// check whether each node can reach root
	bool allReach = true;
	for (int i = 0; i < m_fPrevV.size(); ++i) {
		int numPassed = 0;// detect circles
		int parent = m_fPrevV[i];
		while (parent != -1 && numPassed < m_fPrevV.size()) {
			parent = m_fPrevV[parent];
			++numPassed;
		}
		if (parent >= 0) {
			allReach = false;
			break;
		}
	}

	return allReach;
}

void Skeleton::MarkLabels()
{
	typedef std::vector < std::pair< std::string, std::string> > NamePairs;
	NamePairs leftNames;
	NamePairs rightNames;
	map<string, int>::iterator iter = m_JointNames.begin();
	for (; iter != m_JointNames.end(); ++iter) {
		std::string str = iter->first;
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		std::string::size_type  strIt = str.find("LEFT");
		if (strIt != std::string::npos)leftNames.push_back(std::make_pair(str, iter->first));
		strIt = str.find("RIGHT");
		if (strIt != std::string::npos)rightNames.push_back(std::make_pair(str, iter->first));
		// mark foot
		strIt = str.find("FOOT");
		if(strIt != std::string::npos)this->SetFoot(iter->first);
		// mark fat
		strIt = str.find("FAT");
		if (strIt != std::string::npos)this->SetFat(iter->first);
	}
}