#ifndef TOOLTIP_INFO_H
#define TOOLTIP_INFO_H

#include <memory>

#include "Vector2.h"
#include "types.h"

class SourceLocationFile;

struct TooltipSnippet
{
	std::string code;
	std::shared_ptr<SourceLocationFile> locationFile;
};

struct TooltipInfo
{
	bool isValid() const
	{
		return title.size() || snippets.size();
	}

	std::string title;

	int count = -1;
	std::string countText;

	std::vector<TooltipSnippet> snippets;

	Vec2i offset;
};

#endif	  // TOOLTIP_INFO_H
