#include "utilityMainFunction.h"

#include "NameHierarchy.h"
#include "SearchMatch.h"
#include "utilityString.h"

#include <cassert>
#include <string>

using namespace std;
using namespace utility;

// Note: This only fixes duplicated main functions, other duplicated functions still experience the described problem!
// Related issues:
// - "Nodes for different symbols with the same name are merged" (https://github.com/CoatiSoftware/Sourcetrail/issues/233)
// - "Not obvious which main() is chosen in Custom Trail dialog" (https://github.com/CoatiSoftware/Sourcetrail/issues/728)
// TODO: replace duplicate main definition fix with better solution.

static const string MAIN_NAME = "main";
static const string DECODED_MAIN_NAME = ".:main:."; // Could this cause problems with module partitions? (https://en.cppreference.com/w/cpp/language/modules.html#Module_partitions)

bool isMainFunction(const NameHierarchy &nameHierarchy)
{
	return nameHierarchy.size() == 1 && nameHierarchy.back().hasSignature() && nameHierarchy.back().getName() == MAIN_NAME;
}

void uniquifyMainFunction(NameHierarchy *main, const string &uniqueAppendix)
{
	assert(isMainFunction(*main));

	const NameElement::Signature signature = main->back().getSignature();

	main->pop();
	main->push(NameElement(DECODED_MAIN_NAME + uniqueAppendix, signature.getPrefix(), signature.getPostfix()));
}

bool isUniquifiedMainFunction(const NameHierarchy &nameHierarchy)
{
	return nameHierarchy.size() == 1 && nameHierarchy.back().hasSignature() && !nameHierarchy.back().getName().empty()
		&& nameHierarchy.back().getName()[0] == DECODED_MAIN_NAME[0] && isPrefix(DECODED_MAIN_NAME, nameHierarchy.back().getName());
}

void deuniquifyMainFunction(NameHierarchy *main)
{
	assert(isUniquifiedMainFunction(*main));

	NameElement::Signature signature = main->back().getSignature();
	main->pop();
	main->push(NameElement(MAIN_NAME, signature.getPrefix(), signature.getPostfix()));
}

bool isMainFunction(const SearchMatch &match)
{
	return match.nodeType.getKind() == NODE_FUNCTION && match.tokenNames.size() != 0 && match.tokenNames[0].getRawName() == MAIN_NAME;
}
