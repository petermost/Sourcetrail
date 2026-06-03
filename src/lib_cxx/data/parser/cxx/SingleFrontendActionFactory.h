#ifndef SINGLE_FRONTEND_ACTION_FACTORY
#define SINGLE_FRONTEND_ACTION_FACTORY

#include <clang/Tooling/Tooling.h>

#include <memory>

class SingleFrontendActionFactory: public clang::tooling::FrontendActionFactory
{
public:
	SingleFrontendActionFactory(std::unique_ptr<clang::FrontendAction> action);
	std::unique_ptr<clang::FrontendAction> create() override;

private:
	std::unique_ptr<clang::FrontendAction> m_action;
};

#endif	  // SINGLE_FRONTEND_ACTION_FACTORY
