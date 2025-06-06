#ifndef TEXT_CODEC_H
#define TEXT_CODEC_H

#include <string>

#include <QStringDecoder>
#include <QStringEncoder>
#include <QStringList>

class TextCodec
{
public:
	static QStringList availableCodecs();

	TextCodec(const std::string& name);
	TextCodec(QStringConverter::Encoding encoding);

	std::string decode(const std::string &encodedString);
	std::string encode(const std::string &decodedString);

	QString decodeBytes(const QByteArray &encodedBytes);
	QByteArray encodeBytes(const QString &decodedString);

	std::string getName() const;

private:
	std::string m_name;
	QStringDecoder m_decoder;
	QStringEncoder m_encoder;
};

#endif	  // TEXT_CODEC_H
