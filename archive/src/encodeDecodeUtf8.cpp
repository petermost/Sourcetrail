std::string encodeToUtf8(const std::wstring &s)
{
	return boost::locale::conv::utf_to_utf<char>(s);
}

std::wstring decodeFromUtf8(const std::string &s)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(s);
}

