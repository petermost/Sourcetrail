#ifndef SEARCH_INDEX_H
#define SEARCH_INDEX_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Node.h"
#include "NodeTypeSet.h"
#include "types.h"

// SearchResult is only used as an internal type in the SearchIndex and the PersistentStorage
struct SearchResult
{
	SearchResult(std::string text, std::vector<Id> elementIds, std::vector<size_t> indices, int score)
		: text(std::move(text))
		, elementIds(std::move(elementIds))
		, indices(std::move(indices))
		, score(score)
	{
	}

	bool operator<(const SearchResult& other) const
	{
		return score > other.score;
	}

	std::string text;
	std::vector<Id> elementIds;
	std::vector<size_t> indices;
	int score;
};

class SearchIndex
{
public:
	SearchIndex();
	virtual ~SearchIndex();

	void addNode(Id id, std::string name, NodeType type = NodeType(NODE_SYMBOL));
	void finishSetup();
	void clear();

	// maxResultCount == 0 means "no restriction".
	std::vector<SearchResult> search(
		const std::string& query,
		NodeTypeSet acceptedNodeTypes,
		size_t maxResultCount,
		size_t maxBestScoredResultsLength = 0) const;

private:
	struct SearchEdge;

	struct SearchNode
	{
		SearchNode(NodeTypeSet containedTypes): containedTypes(containedTypes) {}

		std::map<Id, NodeType> elementIds;
		NodeTypeSet containedTypes;
		std::map<char, SearchEdge*> edges;
	};

	struct SearchEdge
	{
		SearchEdge(SearchNode* target, std::string s): target(target), s(std::move(s)) {}

		SearchNode* target;
		std::string s;
		std::set<char> gate;
	};

	struct SearchPath
	{
		SearchPath(std::string text, std::vector<size_t> indices, SearchNode* node)
			: text(std::move(text)), indices(std::move(indices)), node(node)
		{
		}

		std::string text;
		std::vector<size_t> indices;
		SearchNode* node;
	};

	void populateEdgeGate(SearchEdge* e);
	void searchRecursive(
		const SearchPath& path,
		const std::string& remainingQuery,
		NodeTypeSet acceptedNodeTypes,
		std::vector<SearchIndex::SearchPath>* results) const;

	static std::multiset<SearchResult> createScoredResults(
		const std::vector<SearchPath>& paths,
		NodeTypeSet acceptedNodeTypes,
		size_t maxResultCount);

	static SearchResult bestScoredResult(
		SearchResult result,
		std::map<std::string, SearchResult>* scoresCache,
		size_t maxBestScoredResultsLength);
	static void bestScoredResultRecursive(
		const std::string& lowerText,
		const std::vector<size_t>& indices,
		const size_t lastIndex,
		const size_t indicesPos,
		std::map<std::string, SearchResult>* scoresCache,
		SearchResult* result);
	static int scoreText(const std::string& text, const std::vector<size_t>& indices);

public:
	static SearchResult rescoreText(
		const std::string& fulltext,
		const std::string& text,
		const std::vector<size_t>& indices,
		int score,
		size_t maxBestScoredResultsLength);

	static bool isNoLetter(const char c);

private:
	std::vector<std::unique_ptr<SearchNode>> m_nodes;
	std::vector<std::unique_ptr<SearchEdge>> m_edges;
	SearchNode* m_root;
};

#endif	  // SEARCH_INDEX_H
