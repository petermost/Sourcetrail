wstring toUpperCase(const wstring &s)
{
	return boost::locale::to_upper(s);
}

wstring toLowerCase(const wstring &s)
{
	return boost::locale::to_lower(s);
}

// Boost.Locale supports UTF-32 only if the library was build with the 'BOOST_LOCALE_ENABLE_CHAR32_T' 
// switch! So UTF-32 functions convert to UTF-8, do the operation, convert back to UTF-32:

std::u32string toLowerCase(const std::u32string &in)
{
	return convertToUtf32(toLowerCase(convertToUtf8(in)));
}

static std::string doConvertToUtf8(base_converter *converter, char32_t utf32char)
{
	std::string utf8chars(converter->max_len(), '\0');
	utf::len_or_error length = converter->from_unicode(utf32char, utf8chars.data(), utf8chars.data() + utf8chars.size());
	utf8chars.resize((length != base_converter::illegal) ? length : 0);

	return utf8chars;
}

std::string convertToUtf8(const std::u32string &utf32chars)
{
	std::string utf8chars;
	auto converter = create_utf8_converter();
	
	for (char32_t c : utf32chars)
		utf8chars.append(doConvertToUtf8(converter.get(), c));

	return utf8chars;
}
