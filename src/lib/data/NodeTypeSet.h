#ifndef NODE_TYPE_SET_H
#define NODE_TYPE_SET_H

#include "Id.h"

#include <functional>
#include <vector>

class NodeType;

class NodeTypeSet
{
public:
	typedef unsigned long int MaskType;
	
	static NodeTypeSet all();
	static NodeTypeSet none();
	
	NodeTypeSet();
	NodeTypeSet(const NodeType& type);
	
	bool operator==(const NodeTypeSet& other) const;
	bool operator!=(const NodeTypeSet& other) const;
	
	std::vector<NodeType> getNodeTypes() const;
	
	void invert();
	NodeTypeSet getInverse() const;
	
	void add(const NodeTypeSet& typeSet);
	NodeTypeSet getWithAdded(const NodeTypeSet& typeSet) const;
	
	void remove(const NodeTypeSet& typeSet);
	NodeTypeSet getWithRemoved(const NodeTypeSet& typeSet) const;
	
	void keepMatching(const std::function<bool(const NodeType&)>& matcher);
	NodeTypeSet getWithMatchingKept(const std::function<bool(const NodeType&)>& matcher) const;
	
	void removeMatching(const std::function<bool(const NodeType&)>& matcher);
	NodeTypeSet getWithMatchingRemoved(const std::function<bool(const NodeType&)>& matcher) const;
	
	bool isEmpty() const;
	bool contains(const NodeType& type) const;
	bool containsMatching(const std::function<bool(const NodeType&)>& matcher) const;
	bool intersectsWith(const NodeTypeSet& typeSet) const;
	std::vector<Id> getNodeTypeIds() const;
	
private:
	NodeTypeSet(MaskType typeMask);

	MaskType m_nodeTypeMask;
};

#endif	  // NODE_TYPE_SET_H
