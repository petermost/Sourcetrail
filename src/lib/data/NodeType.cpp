#include "NodeType.h"

#include "utilityString.h"
#include "QtResources.h"

std::vector<NodeType> NodeType::getOverviewBundleNodes()
{
	// Defines the order of the bundles on the 'overview' view:
	return {
		NodeType(NODE_FILE),
		NodeType(NODE_MACRO),
		NodeType(NODE_ANNOTATION),
		NodeType(NODE_MODULE),
		NodeType(NODE_NAMESPACE),
		NodeType(NODE_PACKAGE),
		NodeType(NODE_CLASS),
		NodeType(NODE_INTERFACE),
		NodeType(NODE_STRUCT),
		NodeType(NODE_RECORD),
		NodeType(NODE_UNION),
		NodeType(NODE_FUNCTION),
		NodeType(NODE_GLOBAL_VARIABLE),
		NodeType(NODE_TYPE),
		NodeType(NODE_TYPEDEF),
		NodeType(NODE_ENUM)
	};
}

NodeType::NodeType(NodeKind kind): m_kind(kind) {}

bool NodeType::operator==(const NodeType& o) const
{
	return m_kind == o.m_kind;
}

bool NodeType::operator!=(const NodeType& o) const
{
	return !operator==(o);
}

bool NodeType::operator<(const NodeType& o) const
{
	return m_kind < o.m_kind;
}
NodeKind NodeType::getKind() const
{
	return m_kind;
}

Id NodeType::getId() const
{
	// TODO: add id in constructor and return it here
	return m_kind;
}

bool NodeType::isFile() const
{
	const NodeKindMask mask = NODE_FILE;
	return ((m_kind & mask) > 0);
}

bool NodeType::isBuiltin() const
{
	const NodeKindMask mask = NODE_BUILTIN_TYPE;
	return ((m_kind & mask) > 0);
}

bool NodeType::isUnknownSymbol() const
{
	const NodeKindMask mask = NODE_SYMBOL;
	return ((m_kind & mask) > 0);
}

bool NodeType::isInheritable() const
{
	// TODO: what about java enums? Cannot inherit from!
	const NodeKindMask mask = NODE_SYMBOL | NODE_BUILTIN_TYPE | NODE_TYPE | NODE_STRUCT |
		NODE_CLASS | NODE_INTERFACE;

	return ((m_kind & mask) > 0);
}

bool NodeType::isPackage() const
{
	const NodeKindMask mask = NODE_MODULE | NODE_NAMESPACE | NODE_PACKAGE;
	return ((m_kind & mask) > 0);
}

bool NodeType::isCallable() const
{
	const NodeKindMask mask = NODE_FUNCTION | NODE_METHOD;
	return ((m_kind & mask) > 0);
}

bool NodeType::isVariable() const
{
	const NodeKindMask mask = NODE_GLOBAL_VARIABLE | NODE_FIELD;
	return ((m_kind & mask) > 0);
}

bool NodeType::isUsable() const
{
	const NodeKindMask mask = NODE_SYMBOL | NODE_BUILTIN_TYPE | NODE_STRUCT | NODE_CLASS |
		NODE_ENUM | NODE_UNION | NODE_INTERFACE | NODE_ANNOTATION | NODE_TYPEDEF | NODE_RECORD;
	return ((m_kind & mask) > 0);
}

bool NodeType::isPotentialMember() const
{
	const NodeKindMask mask = NODE_METHOD | NODE_FIELD | NODE_CLASS | NODE_INTERFACE |
		NODE_ANNOTATION | NODE_STRUCT | NODE_UNION | NODE_TYPEDEF | NODE_ENUM | NODE_RECORD;

	return ((m_kind & mask) > 0);
}

bool NodeType::isCollapsible() const
{
	const NodeKindMask mask = NODE_SYMBOL | NODE_TYPE | NODE_BUILTIN_TYPE | NODE_STRUCT |
		NODE_CLASS | NODE_INTERFACE | NODE_ANNOTATION | NODE_ENUM | NODE_UNION | NODE_FILE | NODE_RECORD;
	return ((m_kind & mask) > 0);
}

bool NodeType::isVisibleAsParentInGraph() const
{
	return !isPackage();
}

bool NodeType::hasSearchFilter() const
{
	const NodeKindMask mask = NODE_BUILTIN_TYPE | NODE_MODULE | NODE_NAMESPACE | NODE_PACKAGE |
		NODE_STRUCT | NODE_CLASS | NODE_INTERFACE | NODE_ANNOTATION | NODE_GLOBAL_VARIABLE |
		NODE_FIELD | NODE_FUNCTION | NODE_METHOD | NODE_ENUM | NODE_ENUM_CONSTANT | NODE_TYPEDEF |
		NODE_FILE | NODE_MACRO | NODE_UNION | NODE_RECORD;
	return ((m_kind & mask) > 0);
}

Tree<NodeType::BundleInfo> NodeType::getOverviewBundleTree() const
{
	switch (m_kind)
	{
	case NODE_FILE:
		return Tree<BundleInfo>(BundleInfo("Files"));
	case NODE_MACRO:
		return Tree<BundleInfo>(BundleInfo("Macros"));
	case NODE_NAMESPACE:
	{
		Tree<BundleInfo> tree(BundleInfo("Namespaces"));
		tree.children.push_back(Tree<BundleInfo>(BundleInfo(
			[](const std::string& nodeName) {
				return nodeName.find("anonymous namespace") != std::string::npos;
			},
			"Anonymous Namespaces")));
		return tree;
	}
	case NODE_MODULE:
		return Tree<BundleInfo>(BundleInfo("Modules"));
	case NODE_PACKAGE:
		return Tree<BundleInfo>(BundleInfo("Packages"));
	case NODE_CLASS:
		return Tree<BundleInfo>(BundleInfo("Classes"));
	case NODE_INTERFACE:
		return Tree<BundleInfo>(BundleInfo("Interfaces"));
	case NODE_ANNOTATION:
		return Tree<BundleInfo>(BundleInfo("Annotations"));
	case NODE_STRUCT:
		return Tree<BundleInfo>(BundleInfo("Structs"));
	case NODE_FUNCTION:
		return Tree<BundleInfo>(BundleInfo("Functions"));
	case NODE_GLOBAL_VARIABLE:
		return Tree<BundleInfo>(BundleInfo("Global Variables"));
	case NODE_TYPE:
		return Tree<BundleInfo>(BundleInfo("Types"));
	case NODE_TYPEDEF:
		return Tree<BundleInfo>(BundleInfo("Typedefs"));
	case NODE_ENUM:
		return Tree<BundleInfo>(BundleInfo("Enums"));
	case NODE_UNION:
		return Tree<BundleInfo>(BundleInfo("Unions"));
	case NODE_RECORD:
		return Tree<BundleInfo>(BundleInfo("Records"));
	default:
		break;
	}
	return Tree<BundleInfo>();
}

FilePath NodeType::getIconPath() const
{
	if (isPackage())
	{
		// this icon cannot be changed
		return QtResources::GRAPH_VIEW_NAMESPACE;
	}

	switch (m_kind)
	{
	case NODE_ANNOTATION:
		return QtResources::GRAPH_VIEW_ANNOTATION;
	case NODE_ENUM:
		return QtResources::GRAPH_VIEW_ENUM;
	case NODE_TYPEDEF:
		return QtResources::GRAPH_VIEW_TYPEDEF;
	case NODE_MACRO:
		return QtResources::GRAPH_VIEW_MACRO;
	case NODE_FILE:
		return QtResources::GRAPH_VIEW_FILE;
	default:
		return FilePath();
	}
}

bool NodeType::hasIcon() const
{
	if (isPackage())
	{
		return true;
	}

	const NodeKindMask mask = NODE_ANNOTATION | NODE_ENUM | NODE_TYPEDEF | NODE_FILE | NODE_MACRO;
	return ((m_kind & mask) > 0);
}

NodeType::StyleType NodeType::getNodeStyle() const
{
	switch (m_kind)
	{
	case NODE_MODULE:
	case NODE_NAMESPACE:
	case NODE_PACKAGE:
		return STYLE_PACKAGE;
	case NODE_SYMBOL:
	case NODE_TYPE:
	case NODE_BUILTIN_TYPE:
	case NODE_STRUCT:
	case NODE_CLASS:
	case NODE_UNION:
	case NODE_INTERFACE:
	case NODE_RECORD:
	case NODE_ANNOTATION:
	case NODE_ENUM:
	case NODE_TYPEDEF:
	case NODE_TYPE_PARAMETER:
	case NODE_FILE:
	case NODE_MACRO:
		return STYLE_BIG_NODE;
	case NODE_FUNCTION:
	case NODE_METHOD:
	case NODE_GLOBAL_VARIABLE:
	case NODE_FIELD:
	case NODE_ENUM_CONSTANT:
	default:
		return STYLE_SMALL_NODE;
	}
}

bool NodeType::hasOverviewBundle() const
{
	return !getOverviewBundleTree().data.isValid();
}

std::string NodeType::getUnderscoredTypeString() const
{
	return utility::replace(utility::replace(getReadableTypeString(), "-", "_"), " ", "_");
}

std::string NodeType::getReadableTypeString() const
{
	return getReadableNodeKindString(m_kind);
}
