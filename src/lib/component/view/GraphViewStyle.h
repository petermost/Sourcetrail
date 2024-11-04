#ifndef GRAPH_VIEW_STYLE_H
#define GRAPH_VIEW_STYLE_H

#include <map>
#include <memory>

#include "Vector2.h"

#include "AccessKind.h"
#include "GroupType.h"
#include "Node.h"

class GraphViewStyleImpl;

class GraphViewStyle
{
public:
	static Vec2i alignOnRaster(Vec2i position);

	struct NodeMargins
	{
		NodeMargins();

		int left = 0;
		int right = 0;

		int top = 0;
		int bottom = 0;

		int spacingX = 0;
		int spacingY = 0;
		int spacingA = 0;

		int minWidth = 0;

		float charWidth = 0.0f;
		float charHeight = 0.0f;

		int iconWidth = 0;
	};

	struct NodeColor
	{
		std::string fill;
		std::string border;
		std::string text;
		std::string icon;
		std::string hatching;
	};

	struct NodeStyle
	{
		NodeStyle();

		NodeColor color;

		int cornerRadius = 0;

		int borderWidth = 0;
		bool borderDashed = false;

		std::string fontName;
		size_t fontSize = 0;
		bool fontBold = false;

		Vec2i textOffset;

		FilePath iconPath;
		Vec2i iconOffset;
		size_t iconSize = 0;

		bool hasHatching = false;
	};

	struct EdgeStyle
	{
		EdgeStyle();

		std::string color;

		float width = 0;
		int zValue = 0;

		int arrowLength = 0;
		int arrowWidth = 0;
		bool arrowClosed = false;

		int cornerRadius = 0;
		int verticalOffset = 0;

		Vec2i originOffset;
		Vec2i targetOffset;

		bool dashed = false;
	};

	static std::shared_ptr<GraphViewStyleImpl> getImpl();
	static void setImpl(std::shared_ptr<GraphViewStyleImpl> impl);

	static void loadStyleSettings();

	static size_t getFontSizeForStyleType(NodeType::StyleType type);
	static size_t getFontSizeOfAccessNode();
	static size_t getFontSizeOfExpandToggleNode();
	static size_t getFontSizeOfCountCircle();
	static size_t getFontSizeOfQualifier();
	static size_t getFontSizeOfTextNode(int fontSizeDiff);
	static size_t getFontSizeOfGroupNode();

	static std::string getFontNameForDataNode();
	static std::string getFontNameOfAccessNode();
	static std::string getFontNameOfExpandToggleNode();
	static std::string getFontNameOfTextNode();
	static std::string getFontNameOfGroupNode();

	static NodeMargins getMarginsForDataNode(NodeType::StyleType type, bool hasIcon, bool hasChildren);
	static NodeMargins getMarginsOfAccessNode(AccessKind access);
	static NodeMargins getMarginsOfExpandToggleNode();
	static NodeMargins getMarginsOfBundleNode();
	static NodeMargins getMarginsOfTextNode(int fontSizeDiff);
	static NodeMargins getMarginsOfGroupNode(GroupType type, bool hasName);

	static NodeStyle getStyleForNodeType(
		NodeType type,
		bool defined,
		bool isActive,
		bool isFocused,
		bool isCoFocused,
		bool hasChildren,
		bool hasQualifier,
		Id nodeId=0);
	static NodeStyle getStyleOfAccessNode();
	static NodeStyle getStyleOfExpandToggleNode();
	static NodeStyle getStyleOfCountCircle();
	static NodeStyle getStyleOfBundleNode(bool isFocused);
	static NodeStyle getStyleOfQualifier();
	static NodeStyle getStyleOfTextNode(int fontSizeDiff);
	static NodeStyle getStyleOfGroupNode(GroupType type, bool isCoFocused);

	static EdgeStyle getStyleForEdgeType(
		Edge::EdgeType type, bool isActive, bool isFocused, bool isTrailEdge, bool isAmbiguous, Id edgeId=0);

	static int toGridOffset(int x);
	static int toGridSize(int x);
	static int toGridGap(int x);

	static float getZoomFactor();

	static const std::string& getFocusColor();
	static const NodeColor& getNodeColor(const std::string& typeStr, bool highlight, Id nodeId=0);
	static const std::string& getEdgeColor(const std::string& type, Id edgeId=0);
	static const NodeColor& getScreenMatchColor(bool focus);

	static int s_gridCellSize;
	static int s_gridCellPadding;

	static std::map<Id, std::string> s_customEdgeColors;
	static std::map<Id, NodeColor> s_customNodeColors;
	static std::map<Id, NodeColor> s_fullNodeColors;

private:
	static NodeStyle getStyleForNodeType(
		NodeType::StyleType type,
		const std::string& underscoredTypeString,
		const FilePath& iconPath,
		bool defined,
		bool isActive,
		bool isFocused,
		bool isCoFocused,
		bool hasChildren,
		bool hasQualifier,
		Id nodeId);

	static float getCharWidth(NodeType::StyleType type);
	static float getCharHeight(NodeType::StyleType type);

	static float getCharWidth(const std::string& fontName, size_t fontSize);
	static float getCharHeight(const std::string& fontName, size_t fontSize);

	static std::map<NodeType::StyleType, float> s_charWidths;
	static std::map<NodeType::StyleType, float> s_charHeights;

	static std::shared_ptr<GraphViewStyleImpl> s_impl;

	static int s_fontSize;
	static std::string s_fontName;
	static float s_zoomFactor;

	static std::string s_focusColor;
	static std::map<std::string, NodeColor> s_nodeColors;
	static std::map<std::string, std::string> s_edgeColors;
	static std::map<bool, NodeColor> s_screenMatchColors;
};

#endif	  // GRAPH_VIEW_STYLE_H
