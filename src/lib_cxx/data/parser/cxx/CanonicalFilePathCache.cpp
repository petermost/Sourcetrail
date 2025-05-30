#include "CanonicalFilePathCache.h"

#include "utilityClang.h"
#include "utilityString.h"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/FileManager.h>

using namespace clang;

CanonicalFilePathCache::CanonicalFilePathCache(std::shared_ptr<FileRegister> fileRegister)
	: m_fileRegister(fileRegister)
{
}

std::shared_ptr<FileRegister> CanonicalFilePathCache::getFileRegister() const
{
	return m_fileRegister;
}

FilePath CanonicalFilePathCache::getCanonicalFilePath(
	const clang::FileID& fileId, const clang::SourceManager& sourceManager)
{
	if (!fileId.isValid())
	{
		return FilePath();
	}

	auto it = m_fileIdMap.find(fileId);
	if (it != m_fileIdMap.end())
	{
		return it->second;
	}

	FilePath filePath;

	const OptionalFileEntryRef fileEntry = sourceManager.getFileEntryRefForID(fileId);
	if (fileEntry)
	{
		filePath = getCanonicalFilePath(*fileEntry);
		m_fileIdMap.emplace(fileId, filePath);
	}

	return filePath;
}

FilePath CanonicalFilePathCache::getCanonicalFilePath(const clang::FileEntryRef &entry)
{
	return getCanonicalFilePath(utility::getFileNameOfFileEntry(entry));
}

FilePath CanonicalFilePathCache::getCanonicalFilePath(const std::string& path)
{
	const std::string lowercasePath = utility::toLowerCase(path);

	auto it = m_fileStringMap.find(lowercasePath);
	if (it != m_fileStringMap.end())
	{
		return it->second;
	}

	const FilePath canonicalPath = FilePath(path).makeCanonical();
	const std::string lowercaseCanonicalPath = utility::toLowerCase(canonicalPath.str());

	m_fileStringMap.emplace(std::move(lowercasePath), canonicalPath);
	m_fileStringMap.emplace(std::move(lowercaseCanonicalPath), canonicalPath);

	return canonicalPath;
}

FilePath CanonicalFilePathCache::getCanonicalFilePath(const Id symbolId)
{
	auto it = m_symbolIdFileIdMap.find(symbolId);
	if (it != m_symbolIdFileIdMap.end())
	{
		auto it2 = m_fileIdMap.find(it->second);
		if (it2 != m_fileIdMap.end())
		{
			return it2->second;
		}
	}

	return FilePath();
}

void CanonicalFilePathCache::addFileSymbolId(const clang::FileID& fileId, const FilePath& path, Id symbolId)
{
	m_fileIdSymbolIdMap.emplace(fileId, symbolId);
	m_symbolIdFileIdMap.emplace(symbolId, fileId);
	m_fileStringSymbolIdMap.emplace(utility::toLowerCase(path.str()), symbolId);
}

Id CanonicalFilePathCache::getFileSymbolId(const clang::FileID& fileId)
{
	if (!fileId.isValid())
	{
		return 0;
	}

	auto it = m_fileIdSymbolIdMap.find(fileId);
	if (it != m_fileIdSymbolIdMap.end())
	{
		return it->second;
	}

	return 0;
}

Id CanonicalFilePathCache::getFileSymbolId(const clang::FileEntryRef &entry)
{
	return getFileSymbolId(utility::getFileNameOfFileEntry(entry));
}

Id CanonicalFilePathCache::getFileSymbolId(const std::string& path)
{
	std::string canonicalPath = utility::toLowerCase(getCanonicalFilePath(path).str());

	auto it = m_fileStringSymbolIdMap.find(canonicalPath);
	if (it != m_fileStringSymbolIdMap.end())
	{
		return it->second;
	}

	return 0;
}

FilePath CanonicalFilePathCache::getDeclarationFilePath(const clang::Decl* declaration)
{
	const clang::SourceManager& sourceManager = declaration->getASTContext().getSourceManager();
	const clang::FileID fileId = sourceManager.getFileID(declaration->getBeginLoc());
	const clang::FileEntry* fileEntry = sourceManager.getFileEntryForID(fileId);
	if (fileEntry != nullptr)
	{
		return getCanonicalFilePath(fileId, sourceManager);
	}
	return getCanonicalFilePath(sourceManager.getPresumedLoc(declaration->getBeginLoc()).getFilename());
}

std::string CanonicalFilePathCache::getDeclarationFileName(const clang::Decl* declaration)
{
	return getDeclarationFilePath(declaration).fileName();
}

bool CanonicalFilePathCache::isProjectFile(
	const clang::FileID& fileId, const clang::SourceManager& sourceManager)
{
	if (!fileId.isValid())
	{
		return false;
	}

	auto it = m_isProjectFileMap.find(fileId);
	if (it != m_isProjectFileMap.end())
	{
		return it->second;
	}

	bool ret = m_fileRegister->hasFilePath(getCanonicalFilePath(fileId, sourceManager));
	m_isProjectFileMap.emplace(fileId, ret);
	return ret;
}
