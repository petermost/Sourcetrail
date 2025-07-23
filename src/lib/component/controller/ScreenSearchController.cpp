#include "ScreenSearchController.h"

#include "ScreenSearchView.h"

void ScreenSearchController::clear() {}

void ScreenSearchController::foundMatches(ScreenSearchResponder* responder, size_t matchCount)
{
	if (matchCount)
	{
		bool proceed = aidkit::access([this, responder, matchCount](auto &matches)
		{
			size_t responderId = getResponderId(responder);
			if (!responderId)
			{
				return false;
			}

			std::vector<std::pair<size_t, size_t>> newMatches;
			for (size_t i = 0; i < matchCount; i++)
			{
				newMatches.push_back(std::make_pair(responderId, i));
			}

			size_t i = 0;
			while (i < matches.size() && responderId > matches[i].first)
			{
				i++;
			}

			matches.insert(matches.begin() + i, newMatches.begin(), newMatches.end());
			m_matchIndex = matches.size();

			return true;
		}, m_matches);

		if (!proceed)
			return;
	}
	getView<ScreenSearchView>()->setMatchCount(m_matches.access()->size());
}

void ScreenSearchController::addResponder(ScreenSearchResponder* responder)
{
	if (responder)
	{
		m_responders.access()->push_back(responder);
		getView<ScreenSearchView>()->addResponder(responder->getName());
	}
}

void ScreenSearchController::removeResponder(ScreenSearchResponder* responder)
{
	if (responder)
	{
		aidkit::access([responder](auto &responders)
		{
			auto it = std::find(responders.begin(), responders.end(), responder);
			if (it != responders.end())
			{
				responders.erase(it);
			}
		}, m_responders);
	}
}

void ScreenSearchController::search(const std::string& query, const std::set<std::string>& responderNames)
{
	clearMatches();

	aidkit::access([this, &query, &responderNames](auto &responders)
	{
		for (ScreenSearchResponder *responder: responders)
		{
			if (!responder->isVisible())
			{
				continue;
			}

			if (query.size() && responderNames.find(responder->getName()) != responderNames.end())
			{
				responder->findMatches(this, query);
			}
		}
	}, m_responders);
}

void ScreenSearchController::activateMatch(bool next)
{
	bool proceed = aidkit::access([this, next](auto &matches, auto &responders)
	{
		if (!matches.size())
		{
			return false;
		}

		if (m_matchIndex != matches.size())
		{
			auto match = matches[m_matchIndex];
			responders[match.first - 1]->deactivateMatch(match.second);
		}

		if (next)
		{
			if (m_matchIndex == matches.size())
			{
				m_matchIndex = 0;
			}
			else
			{
				m_matchIndex = (m_matchIndex + 1) % matches.size();
			}
		}
		else
		{
			if (m_matchIndex == 0)
			{
				m_matchIndex = matches.size() - 1;
			}
			else
			{
				m_matchIndex--;
			}
		}
		return true;
	}, m_matches, m_responders);

	if (!proceed)
		return;

	auto match = (*m_matches.access())[m_matchIndex];
	(*m_responders.access())[match.first - 1]->activateMatch(match.second);

	getView<ScreenSearchView>()->setMatchIndex(m_matchIndex + 1);
}

void ScreenSearchController::clearMatches()
{
	m_matches.access()->clear();
	m_matchIndex = 0;

	getView<ScreenSearchView>()->setMatchCount(0);

	aidkit::access([](auto &responders)
	{
		for (ScreenSearchResponder *responder: responders)
		{
			responder->clearMatches();
		}
	}, m_responders);
}

size_t ScreenSearchController::getResponderId(ScreenSearchResponder* responder) const
{
	return aidkit::access([responder](auto &responders) -> size_t
	{
		for (size_t i = 0; i < responders.size(); i++)
		{
			if (responders[i] == responder)
			{
				return i + 1;
			}
		}
		return 0;
	}, m_responders);
}

void ScreenSearchController::handleActivation(const MessageActivateBase*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageActivateLocalSymbols*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageActivateTrailEdge*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageChangeFileView*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageCodeShowDefinition*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageDeactivateEdge*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageGraphNodeBundleSplit*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageGraphNodeExpand*  /*message*/)
{
	clearMatches();
}

void ScreenSearchController::handleMessage(MessageGraphNodeHide*  /*message*/)
{
	clearMatches();
}
