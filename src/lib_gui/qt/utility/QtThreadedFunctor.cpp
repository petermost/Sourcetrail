#include "QtThreadedFunctor.h"

QtThreadedFunctorHelper::QtThreadedFunctorHelper()
	: m_freeCallbacks(1)
{
	QObject::connect(this, &QtThreadedFunctorHelper::signalExecution, this, &QtThreadedFunctorHelper::execute, Qt::QueuedConnection);
}

void QtThreadedFunctorHelper::operator()(std::function<void ()> callback)
{
	if (QThread::currentThread() == this->thread())
	{
		callback();
		return;
	}

	m_freeCallbacks.acquire();
	m_callback = callback;
	emit signalExecution();
}

void QtThreadedFunctorHelper::execute()
{
	std::function<void(void)> callback = m_callback;
	m_freeCallbacks.release();
	callback();
}

void QtThreadedFunctorHelper::handleMessage(MessageWindowClosed *)
{
	// The QT thread probably won't relay signals anymore. So this stops other
	// threads from getting stuck here (if they have less than 1000 open tasks,
	// but that should be a reasonable assumption).
	m_freeCallbacks.release(1000);
}



void QtThreadedLambdaFunctor::operator()(std::function<void ()> callback)
{
	m_helper(callback);
}
