#ifndef NODE_TYPE_H
#define NODE_TYPE_H

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "FilePath.h"
#include "NodeKind.h"
#include "Tree.h"
#include "types.h"

class NodeType
{
public:
	enum StyleType
	{
		STYLE_PACKAGE = 0,
		STYLE_SMALL_NODE = 1,
		STYLE_BIG_NODE = 2,
		STYLE_GROUP = 3
	};

	struct BundleInfo
	{
		BundleInfo() = default;

		BundleInfo(std::string bundleName)
			: nameMatcher([](const std::string&) { return true; }), bundleName(bundleName)
		{
		}

		BundleInfo(std::function<bool(std::string)> nameMatcher, std::string bundleName)
			: nameMatcher(nameMatcher), bundleName(bundleName)
		{
		}

		bool isValid() const
		{
			return bundleName.size() > 0;
		}

		std::function<bool(const std::string&)> nameMatcher = nullptr;
		std::string bundleName;
	};

	static std::vector<NodeType> getOverviewBundleNodes();

	explicit NodeType(NodeKind kind);

	bool operator==(const NodeType& o) const;
	bool operator!=(const NodeType& o) const;
	bool operator<(const NodeType& o) const;

	NodeKind getKind() const;

	Id getId() const;
	bool isFile() const;
	bool isBuiltin() const;
	bool isUnknownSymbol() const;
	bool isInheritable() const;
	bool isPackage() const;
	bool isCallable() const;
	bool isVariable() const;
	bool isUsable() const;
	bool isPotentialMember() const;
	bool isCollapsible() const;
	bool isVisibleAsParentInGraph() const;
	bool hasSearchFilter() const;
	Tree<BundleInfo> getOverviewBundleTree() const;

	FilePath getIconPath() const;

	bool hasIcon() const;
	StyleType getNodeStyle() const;

	bool hasOverviewBundle() const;
	std::string getUnderscoredTypeString() const;
	std::string getReadableTypeString() const;

private:
	NodeKind m_kind;
};

#endif	  // NODE_TYPE_H
