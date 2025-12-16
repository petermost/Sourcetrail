#include "SourceGroupSettingsWithCxxCrossCompilationOptions.h"

#include "ProjectSettings.h"
#include "ToolChain.h"


bool SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetOptionsEnabled() const
{
	return m_targetOptionsEnabled;
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::setTargetOptionsEnabled(bool targetOptionsEnabled)
{
	m_targetOptionsEnabled = targetOptionsEnabled;
}

std::string SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetArch() const
{
	return m_targetArch;
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::setTargetArch(const std::string& arch)
{
	m_targetArch = arch;
}

std::string SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetVendor() const
{
	return m_targetVendor;
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::setTargetVendor(const std::string& vendor)
{
	m_targetVendor = vendor;
}

std::string SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetSys() const
{
	return m_targetSys;
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::setTargetSys(const std::string& sys)
{
	m_targetSys = sys;
}

std::string SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetAbi() const
{
	return m_targetAbi;
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::setTargetAbi(const std::string& abi)
{
	m_targetAbi = abi;
}

std::string SourceGroupSettingsWithCxxCrossCompilationOptions::getTargetFlag() const
{
	std::string targetFlag;
	if (m_targetOptionsEnabled && !m_targetArch.empty())
	{
		targetFlag = ClangCompiler::targetOption(m_targetArch);
		targetFlag += "-" + (m_targetVendor.empty() ? "unknown" : m_targetVendor);
		targetFlag += "-" + (m_targetSys.empty() ? "unknown" : m_targetSys);
		targetFlag += "-" + (m_targetAbi.empty() ? "unknown" : m_targetAbi);
	}
	return targetFlag;
}

bool SourceGroupSettingsWithCxxCrossCompilationOptions::equals(const SourceGroupSettingsBase* other) const
{
	const SourceGroupSettingsWithCxxCrossCompilationOptions* otherPtr =
		dynamic_cast<const SourceGroupSettingsWithCxxCrossCompilationOptions*>(other);

	return (other && getTargetFlag() == otherPtr->getTargetFlag());
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::load(
	const ConfigManager* config, const std::string& key)
{
	setTargetOptionsEnabled(
		config->getValueOrDefault<bool>(key + "/cross_compilation/target_options_enabled", false));
	setTargetArch(
		config->getValueOrDefault<std::string>(key + "/cross_compilation/target/arch", ""));
	setTargetVendor(
		config->getValueOrDefault<std::string>(key + "/cross_compilation/target/vendor", ""));
	setTargetSys(
		config->getValueOrDefault<std::string>(key + "/cross_compilation/target/sys", ""));
	setTargetAbi(
		config->getValueOrDefault<std::string>(key + "/cross_compilation/target/abi", ""));
}

void SourceGroupSettingsWithCxxCrossCompilationOptions::save(
	ConfigManager* config, const std::string& key)
{
	config->setValue(key + "/cross_compilation/target_options_enabled", getTargetOptionsEnabled());
	config->setValue(key + "/cross_compilation/target/arch", getTargetArch());
	config->setValue(key + "/cross_compilation/target/vendor", getTargetVendor());
	config->setValue(key + "/cross_compilation/target/sys", getTargetSys());
	config->setValue(key + "/cross_compilation/target/abi", getTargetAbi());
}
