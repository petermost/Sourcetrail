#ifndef QT_THREADED_FUNCTOR_H
#define QT_THREADED_FUNCTOR_H

#include <functional>

#include <QObject>
#include <QSemaphore>
#include <QThread>

#include "MessageListener.h"
#include "MessageWindowClosed.h"

class QtThreadedFunctorHelper : public QObject, public MessageListener<MessageWindowClosed>
{
	Q_OBJECT

public:
	QtThreadedFunctorHelper();

	void operator()(std::function<void()> callback);

signals:
	void signalExecution();

private slots:
	void execute();

private:
	void handleMessage(MessageWindowClosed *) override;

	std::function<void()> m_callback;
	QSemaphore m_freeCallbacks;
};

template <typename... Args>
class QtThreadedFunctor
{
public:
	QtThreadedFunctor(std::function<void(Args...)> callback)
		: m_callback(std::move(callback))
	{
	}

	void operator()(Args... args)
	{
		m_helper(std::bind(m_callback, args...));
	}

private:
	QtThreadedFunctorHelper m_helper;
	std::function<void(Args...)> m_callback;
};

class QtThreadedLambdaFunctor
{
public:
	void operator()(std::function<void()> callback);

private:
	QtThreadedFunctorHelper m_helper;
};

#endif
