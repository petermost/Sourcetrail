#include "TextCodec.h"

#include <array>

using namespace std;

static constexpr array AVAILABLE_ENCODINGS = {
	QStringConverter::Encoding::Utf8,
	QStringConverter::Encoding::Utf16,
	QStringConverter::Encoding::Utf16LE,
	QStringConverter::Encoding::Utf16BE,
	QStringConverter::Encoding::Utf32,
	QStringConverter::Encoding::Utf32LE,
	QStringConverter::Encoding::Utf32BE,
	QStringConverter::Encoding::Latin1,
	QStringConverter::Encoding::System
};
static_assert(AVAILABLE_ENCODINGS.size() == QStringConverter::Encoding::LastEncoding + 1, "Encoding missing");

QStringList TextCodec::availableCodecs()
{
	QStringList availableCodecs;

	for (const QStringConverter::Encoding encoding : AVAILABLE_ENCODINGS)
		availableCodecs += QStringConverter::nameForEncoding(encoding);

	return availableCodecs;
}

TextCodec::TextCodec(const string &name)
	: m_name(name)
	, m_decoder(m_name.c_str())
	, m_encoder(m_name.c_str())
{
}

string TextCodec::decode(const string &encodedString)
{
	if (m_encoder.isValid())
		return static_cast<QString>(m_decoder.decode(encodedString)).toStdString();
	else
		return encodedString;
}

string TextCodec::encode(const string &decodedString)
{
	if (m_encoder.isValid())
		return static_cast<QByteArray>(m_encoder.encode(QString::fromStdString(decodedString))).toStdString();
	else
		return decodedString;
}

int TextCodec::encodedSize(const QString &decodedString)
{
	return encode(decodedString.toStdString()).size();
}

bool TextCodec::isValid() const
{
	return m_decoder.isValid() && m_encoder.isValid();
}

string TextCodec::getName() const
{
	return m_name;
}
