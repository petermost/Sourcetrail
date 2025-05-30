wstring toUpperCase(const wstring &s)
{
	return boost::locale::to_upper(s);
}

wstring toLowerCase(const wstring &s)
{
	return boost::locale::to_lower(s);
}

