QStringList splitGraphemes(const QString &line)
{
	QStringList graphemes;
	
	qsizetype startPosition = 0, endPosition = 0;
	QTextBoundaryFinder graphemeFinder(QTextBoundaryFinder::Grapheme, line);
	graphemeFinder.setPosition(startPosition);
	while ((endPosition = graphemeFinder.toNextBoundary()) != -1)
	{
		graphemes.append(line.mid(startPosition, endPosition - startPosition));
		startPosition = endPosition;
	}
	return graphemes;
}

