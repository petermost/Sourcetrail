#include "Catch2.hpp"

#include <TextCodec.h>

#include <string>

using namespace std;

// German  | UTF-8     | UTF-16 | UTF-32     | Latin-1 (ISO-8859-1)
//-----------------------------------------------------------------
// ae  = ä | 0xC3 0xA4 | 0x00E4 | 0x000000E4 | 0xE4              |
// oe  = ö | 0xC3 0xB6 | 0x00F6 | 0x000000F6 | 0xF6              |
// ue  = ü | 0xC3 0xBC | 0x00FC | 0x000000FC | 0xFC              |
// AE  = Ä | 0xC3 0xC4 | 0x00C4 | 0x000000C4 | 0xC4              |
// OE  = Ö | 0xC3 0x96 | 0x00D6 | 0x000000D6 | 0xD6              |
// UE  = Ü | 0xC3 0x9C | 0x00DC | 0x000000DC | 0xDC              |
// SS  = ß | 0xC3 0x9F | 0x00DF | 0x000000DF | 0xDF              |
// EUR = € | 0xC2 0x80 | 0x0080 | 0x00000080 | 0x80 (unofficial) |
//================================================================

const string UTF8_LOWER_AE = {'\xC3', '\xA4'};
const string UTF8_LOWER_OE = {'\xC3', '\xB6'};
const string UTF8_LOWER_UE = {'\xC3', '\xBC'};
const string UTF8_UPPER_AE = {'\xC3', '\x84'};
const string UTF8_UPPER_OE = {'\xC3', '\x96'};
const string UTF8_UPPER_UE = {'\xC3', '\x9C'};
const string UTF8_SHARP_SS = {'\xC3', '\x9F'};
const string UTF8_EURO     = {'\xC2', '\x80'};

const string UTF8_CHARACTERS = UTF8_LOWER_AE + UTF8_LOWER_OE + UTF8_LOWER_UE + UTF8_UPPER_AE + UTF8_UPPER_OE + UTF8_UPPER_UE + UTF8_SHARP_SS + UTF8_EURO;

const string UTF16_LOWER_AE = {'\x00', '\xE4'};
const string UTF16_LOWER_OE = {'\x00', '\xF6'};
const string UTF16_LOWER_UE = {'\x00', '\xFC'};
const string UTF16_UPPER_AE = {'\x00', '\xC4'};
const string UTF16_UPPER_OE = {'\x00', '\xD6'};
const string UTF16_UPPER_UE = {'\x00', '\xDC'};
const string UTF16_SHARP_SS = {'\x00', '\xDF'};
const string UTF16_EURO     = {'\x00', '\x80'};

const string UTF16_CHARACTERS = UTF16_LOWER_AE + UTF16_LOWER_OE + UTF16_LOWER_UE + UTF16_UPPER_AE + UTF16_UPPER_OE + UTF16_UPPER_UE + UTF16_SHARP_SS + UTF16_EURO;

const string UTF32_LOWER_AE = {'\x00', '\x00', '\x00', '\xE4'};
const string UTF32_LOWER_OE = {'\x00', '\x00', '\x00', '\xF6'};
const string UTF32_LOWER_UE = {'\x00', '\x00', '\x00', '\xFC'};
const string UTF32_UPPER_AE = {'\x00', '\x00', '\x00', '\xC4'};
const string UTF32_UPPER_OE = {'\x00', '\x00', '\x00', '\xD6'};
const string UTF32_UPPER_UE = {'\x00', '\x00', '\x00', '\xDC'};
const string UTF32_SHARP_SS = {'\x00', '\x00', '\x00', '\xDF'};
const string UTF32_EURO     = {'\x00', '\x00', '\x00', '\x80'};

const string UTF32_CHARACTERS = UTF32_LOWER_AE + UTF32_LOWER_OE + UTF32_LOWER_UE + UTF32_UPPER_AE + UTF32_UPPER_OE + UTF32_UPPER_UE + UTF32_SHARP_SS + UTF32_EURO;

const string LATIN_1_LOWER_AE = {'\xE4'};
const string LATIN_1_LOWER_OE = {'\xF6'};
const string LATIN_1_LOWER_UE = {'\xFC'};
const string LATIN_1_UPPER_AE = {'\xC4'};
const string LATIN_1_UPPER_OE = {'\xD6'};
const string LATIN_1_UPPER_UE = {'\xDC'};
const string LATIN_1_SHARP_SS = {'\xDF'};
const string LATIN_1_EURO     = {'\x80'};

const string LATIN_1_CHARACTERS = LATIN_1_LOWER_AE + LATIN_1_LOWER_OE + LATIN_1_LOWER_UE + LATIN_1_UPPER_AE + LATIN_1_UPPER_OE + LATIN_1_UPPER_UE + LATIN_1_SHARP_SS + LATIN_1_EURO;

using Encoding = QStringConverter::Encoding;

const string EMPTY_STRING;

void testFunctionForTooltipWithUmlauteInSignature(/* äöüÄÖÜß€ */ )
{
}

static TextCodec makeCodec(Encoding encoding)
{
	TextCodec textCodec(QStringConverter::nameForEncoding(encoding));
	REQUIRE(textCodec.isValid());
	return textCodec;
}

//=================================================================================================

TEST_CASE("TextCodec empty decoded string")
{
	TextCodec codec = makeCodec(Encoding::System);

	string decodedString = codec.decode(EMPTY_STRING);

	REQUIRE(decodedString == EMPTY_STRING);
}

TEST_CASE("TextCodec empty encoded string")
{
	TextCodec codec = makeCodec(Encoding::System);

	string encodedString = codec.encode(EMPTY_STRING);

	REQUIRE(encodedString == EMPTY_STRING);
}

//=================================================================================================

TEST_CASE("TextCodec ascii decoding")
{
	TextCodec codec = makeCodec(Encoding::System);

	string decodedString = codec.decode("abcdef");

	REQUIRE(decodedString == "abcdef");
}

TEST_CASE("TextCodec ascii encoding")
{
	TextCodec codec = makeCodec(Encoding::System);

	string encodedString = codec.encode("12345");

	REQUIRE(encodedString == "12345");
}

//=================================================================================================

TEST_CASE("TextCodec encode utf8 to utf16")
{
	TextCodec codec = makeCodec(Encoding::Utf16BE);

	string utf16Umlaute = codec.encode(UTF8_CHARACTERS);

	REQUIRE(utf16Umlaute == UTF16_CHARACTERS);
}

TEST_CASE("TextCodec decode utf16 to utf8")
{
	TextCodec codec = makeCodec(Encoding::Utf16BE);

	string utf8Umlaute = codec.decode(UTF16_CHARACTERS);

	REQUIRE(utf8Umlaute == UTF8_CHARACTERS);
}

//=================================================================================================

TEST_CASE("TextCodec encode utf8 to utf32")
{
	TextCodec codec = makeCodec(Encoding::Utf32BE);

	string utf32Umlaute = codec.encode(UTF8_CHARACTERS);

	REQUIRE(utf32Umlaute == UTF32_CHARACTERS);
}

TEST_CASE("TextCodec decode utf32 to utf8")
{
	TextCodec codec = makeCodec(Encoding::Utf32BE);

	string utf8Umlaute = codec.decode(UTF32_CHARACTERS);

	REQUIRE(utf8Umlaute == UTF8_CHARACTERS);
}

//=================================================================================================

TEST_CASE("TextCodec encode utf8 to latin1")
{
	TextCodec codec = makeCodec(Encoding::Latin1);

	string latin1Umlaute = codec.encode(UTF8_CHARACTERS);

	REQUIRE(latin1Umlaute == LATIN_1_CHARACTERS);
}

TEST_CASE("TextCodec decode latin1 to utf8")
{
	TextCodec codec = makeCodec(Encoding::Latin1);

	string utf8Umlaute = codec.decode(LATIN_1_CHARACTERS);

	REQUIRE(utf8Umlaute == UTF8_CHARACTERS);
}
