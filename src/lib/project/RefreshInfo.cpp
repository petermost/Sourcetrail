#include "RefreshInfo.h"

using namespace std;

ostream &operator << (ostream &stream, RefreshMode mode)
{
	return stream << static_cast<int>(mode);
}
