#ifndef TOKEN_COMPONENT_ACCESS_H
#define TOKEN_COMPONENT_ACCESS_H

#include <string>

#include "AccessKind.h"
#include "TokenComponent.h"

class TokenComponentAccess: public TokenComponent
{
public:
	static std::string getAccessString(AccessKind access);

	TokenComponentAccess(AccessKind access);
	~TokenComponentAccess() override;

	std::shared_ptr<TokenComponent> copy() const override;

	AccessKind getAccess() const;
	std::string getAccessString() const;

private:
	const AccessKind m_access;
};

#endif	  // TOKEN_COMPONENT_ACCESS_H
