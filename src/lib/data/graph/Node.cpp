#include "Node.h"

#include <sstream>

#include "logging.h"

#include "TokenComponentAccess.h"
#include "TokenComponentConst.h"
#include "TokenComponentStatic.h"

Node::Node(Id id, NodeType type, NameHierarchy nameHierarchy, DefinitionKind definitionKind)
	: Token(id)
	, m_type(type)
	, m_nameHierarchy(std::move(nameHierarchy))
	, m_definitionKind(definitionKind)

{
}

Node::Node(const Node& other)
	: Token(other)
	, m_type(other.m_type)
	, m_nameHierarchy(other.m_nameHierarchy)
	, m_definitionKind(other.m_definitionKind)
	, m_childCount(other.m_childCount)
{
}

Node::~Node() = default;

NodeType Node::getType() const
{
	return m_type;
}

void Node::setType(NodeType type)
{
	if (!isType(type.getKind() | NODE_SYMBOL))
	{
		LOG_WARNING(
			"Cannot change NodeType after it was already set from " + getReadableTypeString() +
			" to " + type.getReadableTypeString());
		return;
	}
	m_type = type;
}

bool Node::isType(NodeKindMask mask) const
{
	return (m_type.getKind() & mask) > 0;
}

std::string Node::getName() const
{
	return m_nameHierarchy.getRawName();
}

std::string Node::getFullName() const
{
	return m_nameHierarchy.getQualifiedName();
}

const NameHierarchy& Node::getNameHierarchy() const
{
	return m_nameHierarchy;
}

bool Node::isDefined() const
{
	return m_definitionKind != DefinitionKind::NONE;
}

bool Node::isImplicit() const
{
	return m_definitionKind == DefinitionKind::IMPLICIT;
}

bool Node::isExplicit() const
{
	return m_definitionKind == DefinitionKind::EXPLICIT;
}

size_t Node::getChildCount() const
{
	return m_childCount;
}

void Node::setChildCount(size_t childCount)
{
	m_childCount = childCount;
}

size_t Node::getEdgeCount() const
{
	return m_edges.size();
}

void Node::addEdge(Edge* edge)
{
	m_edges.emplace(edge->getId(), edge);
}

void Node::removeEdge(Edge* edge)
{
	auto it = m_edges.find(edge->getId());
	if (it != m_edges.end())
	{
		m_edges.erase(it);
	}
}

Node* Node::getParentNode() const
{
	Edge* edge = getMemberEdge();
	if (edge)
	{
		return edge->getFrom();
	}
	return nullptr;
}

Node* Node::getLastParentNode()
{
	Node* parent = getParentNode();
	if (parent)
	{
		return parent->getLastParentNode();
	}
	return this;
}

Edge* Node::getMemberEdge() const
{
	return findEdgeOfType(Edge::EDGE_MEMBER, [this](Edge* e) { return e->getTo() == this; });
}

bool Node::isParentOf(const Node* node) const
{
	while ((node = node->getParentNode()) != nullptr)
	{
		if (node == this)
		{
			return true;
		}
	}
	return false;
}

Edge* Node::findEdge(std::function<bool(Edge*)> func) const
{
	auto it = find_if(
		m_edges.begin(), m_edges.end(), [func](std::pair<Id, Edge*> p) { return func(p.second); });

	if (it != m_edges.end())
	{
		return it->second;
	}

	return nullptr;
}

Edge* Node::findEdgeOfType(Edge::TypeMask mask) const
{
	return findEdgeOfType(mask, [](Edge*  /*e*/) { return true; });
}

Edge* Node::findEdgeOfType(Edge::TypeMask mask, std::function<bool(Edge*)> func) const
{
	auto it = find_if(m_edges.begin(), m_edges.end(), [mask, func](std::pair<Id, Edge*> p) {
		if (p.second->isType(mask))
		{
			return func(p.second);
		}
		return false;
	});

	if (it != m_edges.end())
	{
		return it->second;
	}

	return nullptr;
}

Node* Node::findChildNode(std::function<bool(Node*)> func) const
{
	auto it = find_if(m_edges.begin(), m_edges.end(), [&func](std::pair<Id, Edge*> p) {
		if (p.second->getType() == Edge::EDGE_MEMBER)
		{
			return func(p.second->getTo());
		}
		return false;
	});

	if (it != m_edges.end())
	{
		return it->second->getTo();
	}

	return nullptr;
}

void Node::forEachEdge(std::function<void(Edge*)> func) const
{
	for_each(m_edges.begin(), m_edges.end(), [func](std::pair<Id, Edge*> p) { func(p.second); });
}

void Node::forEachEdgeOfType(Edge::TypeMask mask, std::function<void(Edge*)> func) const
{
	for_each(m_edges.begin(), m_edges.end(), [mask, func](std::pair<Id, Edge*> p) {
		if (p.second->isType(mask))
		{
			func(p.second);
		}
	});
}

void Node::forEachChildNode(std::function<void(Node*)> func) const
{
	forEachEdgeOfType(Edge::EDGE_MEMBER, [func, this](Edge* e) {
		if (this != e->getTo())
		{
			func(e->getTo());
		}
	});
}

void Node::forEachNodeRecursive(std::function<void(const Node*)> func) const
{
	func(this);

	forEachEdgeOfType(Edge::EDGE_MEMBER, [func, this](Edge* e) {
		if (this != e->getTo())
		{
			e->getTo()->forEachNodeRecursive(func);
		}
	});
}

bool Node::isNode() const
{
	return true;
}

bool Node::isEdge() const
{
	return false;
}

std::string Node::getReadableTypeString() const
{
	return m_type.getReadableTypeString();
}

std::string Node::getAsString() const
{
	std::stringstream str;
	str << "[" << getId() << "] " << getReadableTypeString() << ": " << "\"" << getName()
		<< "\"";

	TokenComponentAccess* access = getComponent<TokenComponentAccess>();
	if (access)
	{
		str << " " << access->getAccessString();
	}

	if (getComponent<TokenComponentStatic>())
	{
		str << " static";
	}

	if (getComponent<TokenComponentConst>())
	{
		str << " const";
	}

	return str.str();
}

std::ostream& operator<<(std::ostream& ostream, const Node& node)
{
	ostream << node.getAsString();
	return ostream;
}
