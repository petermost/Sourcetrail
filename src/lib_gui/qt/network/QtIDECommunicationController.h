#ifndef QT_IDE_COMMUNICATION_CONTROLLER
#define QT_IDE_COMMUNICATION_CONTROLLER

#include <qobject.h>

#include "QtTcpWrapper.h"

#include "IDECommunicationController.h"
#include "QtThreadedFunctor.h"

class StorageAccess;

class QtIDECommunicationController: public IDECommunicationController
{
public:
	QtIDECommunicationController(QObject* parent, StorageAccess* storageAccess);
	~QtIDECommunicationController() override;

	void startListening() override;
	void stopListening() override;

	bool isListening() const override;

private:
	void sendMessage(const std::string& message) const override;

	QtTcpWrapper m_tcpWrapper;

	QtThreadedLambdaFunctor m_onQtThread;
};

#endif	  // QT_IDE_COMMUNICATION_CONTROLLER
