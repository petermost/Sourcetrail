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

static QStringConverter::Encoding makeValidEncoding(const string &name)
{
	for (const auto encoding : AVAILABLE_ENCODINGS)
	{
		if (QStringConverter::nameForEncoding(encoding) == name)
			return encoding;
	}
	return QStringConverter::Encoding::System;
}

QStringList TextCodec::availableCodecs()
{
	QStringList availableCodecs;

	for (const auto encoding : AVAILABLE_ENCODINGS)
		availableCodecs += QStringConverter::nameForEncoding(encoding);

	return availableCodecs;
}

TextCodec::TextCodec(const string &name)
	: TextCodec(makeValidEncoding(name))
{
}

TextCodec::TextCodec(QStringConverter::Encoding encoding)
	: m_name(QStringConverter::nameForEncoding(encoding))
	, m_decoder(encoding)
	, m_encoder(encoding)
{
}

string TextCodec::decode(const string &encodedString)
{
	return static_cast<QString>(m_decoder.decode(encodedString)).toStdString();
}

string TextCodec::encode(const string &decodedString)
{
	return static_cast<QByteArray>(m_encoder.encode(QString::fromStdString(decodedString))).toStdString();
}

int TextCodec::encodedSize(const QString &decodedString)
{
	return encode(decodedString.toStdString()).size();
}

string TextCodec::getName() const
{
	return m_name;
}
