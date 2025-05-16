#include "SourceGroupSettingsWithCxxCrossCompilationOptions.h"

#include "ProjectSettings.h"

std::vector<std::string> SourceGroupSettingsWithCxxCrossCompilationOptions::getAvailableArchTypes()
{
	// as defined in llvm/lib/Support/Triple.cpp

	return {
		"aarch64",
		"aarch64_be",
		"aarch64_32",
		"arm",
		"armeb",
		"arc",
		"avr",
		"bpfel",
		"bpfeb",
		"hexagon",
		"mips",
		"mipsel",
		"mips64",
		"mips64el",
		"msp430",
		"powerpc64",
		"powerpc64le",
		"powerpc",
		"r600",
		"amdgcn",
		"riscv32",
		"riscv64",
		"sparc",
		"sparcv9",
		"sparcel",
		"s390x",
		"tce",
		"tcele",
		"thumb",
		"thumbeb",
		"i386",
		"x86_64",
		"xcore",
		"nvptx",
		"nvptx64",
		"le32",
		"le64",
		"amdil",
		"amdil64",
		"hsail",
		"hsail64",
		"spir",
		"spir64",
		"kalimba",
		"lanai",
		"shave",
		"wasm32",
		"wasm64",
		"renderscript32",
		"renderscript64",
	};
}

std::vector<std::string> SourceGroupSettingsWithCxxCrossCompilationOptions::getAvailableVendorTypes()
{
	return {
		"unknown",
		"apple",
		"pc",
		"scei",
		"bgp",
		"bgq",
		"fsl",
		"ibm",
		"img",
		"mti",
		"nvidia",
		"csr",
		"myriad",
		"amd",
		"mesa",
		"suse",
		"oe",
	};
}

std::vector<std::string> SourceGroupSettingsWithCxxCrossCompilationOptions::getAvailableOsTypes()
{
	return {
		"unknown",
		"cloudabi",
		"darwin",
		"dragonfly",
		"freebsd",
		"fuchsia",
		"ios",
		"kfreebsd",
		"linux",
		"lv2",
		"macosx",
		"netbsd",
		"openbsd",
		"solaris",
		"windows",
		"haiku",
		"minix",
		"rtems",
		"nacl",
		"cnk",
		"aix",
		"cuda",
		"nvcl",
		"amdhsa",
		"ps4",
		"elfiamcu",
		"tvos",
		"watchos",
		"mesa3d",
		"contiki",
		"amdpal",
		"hermit",
		"hurd",
		"wasi",
		"emscripten",
	};
}

std::vector<std::string> SourceGroupSettingsWithCxxCrossCompilationOptions::getAvailableEnvironmentTypes()
{
	return {
		"unknown",
		"gnu",
		"gnuabin32",
		"gnuabi64",
		"gnueabihf",
		"gnueabi",
		"gnux32",
		"code16",
		"eabi",
		"eabihf",
		"elfv1",
		"elfv2",
		"android",
		"musl",
		"musleabi",
		"musleabihf",
		"msvc",
		"itanium",
		"cygnus",
		"coreclr",
		"simulator",
		"macabi",
	};
}

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
		targetFlag = "--target=";
		targetFlag += m_targetArch;
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
