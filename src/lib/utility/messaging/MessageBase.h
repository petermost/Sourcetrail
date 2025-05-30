#ifndef MESSAGE_BASE_H
#define MESSAGE_BASE_H

#include <ostream>
#include <sstream>

#include "Id.h"
#include "TabIds.h"

class MessageBase
{
public:
	MessageBase()
		: m_id(s_nextId++)
		, m_schedulerId(TabId::NONE)
	{
	}

	virtual ~MessageBase() = default;

	virtual std::string getType() const = 0;
	virtual void dispatch() = 0;

	Id getId() const
	{
		return m_id;
	}

	TabId getSchedulerId() const
	{
		return m_schedulerId;
	}

	void setSchedulerId(TabId schedulerId)
	{
		m_schedulerId = schedulerId;
	}

	bool sendAsTask() const
	{
		return m_sendAsTask;
	}

	void setSendAsTask(bool sendAsTask)
	{
		m_sendAsTask = sendAsTask;
	}

	bool isParallel() const
	{
		return m_isParallel;
	}

	void setIsParallel(bool isParallel)
	{
		m_isParallel = isParallel;
	}

	bool isReplayed() const
	{
		return m_isReplayed;
	}

	void setIsReplayed(bool isReplayed)
	{
		m_isReplayed = isReplayed;
	}

	bool isLast() const
	{
		return m_isLast;
	}

	void setIsLast(bool isLast)
	{
		m_isLast = isLast;
	}

	bool isLogged() const
	{
		return m_isLogged;
	}

	void setIsLogged(bool isLogged)
	{
		m_isLogged = isLogged;
	}

	void setKeepContent(bool keepContent)
	{
		m_keepContent = keepContent;
	}

	bool keepContent() const
	{
		return m_keepContent;
	}

	virtual void print(std::ostream& os) const = 0;

	std::string str() const
	{
		std::stringstream ss;
		ss << getType() << " ";
		print(ss);
		return ss.str();
	}

private:
	static Id s_nextId;

	Id m_id;
	TabId m_schedulerId;

	bool m_isParallel = false;
	bool m_isReplayed = false;

	bool m_sendAsTask = true;
	bool m_keepContent = false;

	bool m_isLast = true;
	bool m_isLogged = true;
};

#endif	  // MESSAGE_BASE_H
