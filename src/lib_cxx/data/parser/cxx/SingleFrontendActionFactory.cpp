#include "SingleFrontendActionFactory.h"

SingleFrontendActionFactory::SingleFrontendActionFactory(std::unique_ptr<clang::FrontendAction> action)
	: m_action(std::move(action))
{
}

std::unique_ptr<clang::FrontendAction> SingleFrontendActionFactory::create()
{
	return std::unique_ptr<clang::FrontendAction>(std::move(m_action));
}
