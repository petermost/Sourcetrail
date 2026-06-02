# Generates the licenses.h file
# For adding a new 3rd Party License add the License File to the 3rd Party License Folder
# and a new AddLicense Line in below

# These global variables are referenced in 'licenses.h.in':
set(LICENSES "")
set(LICENSE_ARRAY "")
set(LICENSEFOLDER "${CMAKE_SOURCE_DIR}/bin/app/data/license/3rd_party_licenses")

function(ReadLicense licenseFile licenseVariable)
    file(READ ${licenseFile} tempVariable)
    STRING(REGEX REPLACE "\"" "\\\\\"" tempVariable "${tempVariable}")
    STRING(REGEX REPLACE "\n" "\\\\n\"\n\t\"" tempVariable "${tempVariable}")

    set(var "\n\nstatic const char *${licenseVariable}=\n\t\"")
    set(LICENSES "${LICENSES}${var}${tempVariable}\"\;" PARENT_SCOPE)
endfunction(ReadLicense)

function(AddLicense softwareName softwareVersion softwareURL licenseFile)
		# Make a valid C++ variable name:
		STRING(REPLACE " " "_" softwareVariableName "${softwareName}")

		# Remove the quotes from a version literal:
		STRING(REPLACE "\"" "" softwareVersion "${softwareVersion}")

        ReadLicense(${licenseFile} ${softwareVariableName}_license)
        set(LICENSES ${LICENSES} PARENT_SCOPE)
        set(LICENSE_ARRAY "${LICENSE_ARRAY}\n\t{ \"${softwareName}\", \"${softwareVersion}\", \"${softwareURL}\", ${softwareVariableName}_license }," PARENT_SCOPE)
endfunction(AddLicense)

function(configureLicenseFile outputFile)
	ReadLicense(${CMAKE_SOURCE_DIR}/LICENSE.txt Sourcetrail_license)
	set(LICENSE_APP "{ \"Sourcetrail\", \"${Sourcetrail_VERSION}\", \"http://sourcetrail.de\", Sourcetrail_license }")

	AddLicense("Qt" "${Qt6_VERSION}" "https://www.qt.io/" "${LICENSEFOLDER}/license_qt.txt")
	AddLicense("Boost" "${Boost_VERSION}" "https://www.boost.org/" "${LICENSEFOLDER}/license_boost.txt")
	AddLicense("Clang" "${LLVM_VERSION}" "https://clang.llvm.org/" "${LICENSEFOLDER}/license_clang.txt")
	AddLicense("SQLite" "${SQLite3_VERSION}" "https://www.sqlite.org/" "${LICENSEFOLDER}/license_sqlite.txt")
	AddLicense("CppSQLite3" "${CppSQLite3_VERSION}" "https://www.codeproject.com/Articles/6343/CppSQLite-C-Wrapper-for-SQLite" "${LICENSEFOLDER}/license_cpp_sqlite.txt")
	AddLicense("JDT Core" "" "https://github.com/eclipse-jdt/eclipse.jdt.core" "${LICENSEFOLDER}/license_eclipse.txt")
	AddLicense("Gradle" "" "https://github.com/gradle/gradle" "${LICENSEFOLDER}/license_gradle.txt")
	AddLicense("TinyXML" "${tinyxml_VERSION}" "https://sourceforge.net/projects/tinyxml/" "${LICENSEFOLDER}/license_tinyxml.txt")
	# AddLicense("Catch2" "${Catch2_VERSION}" "https://github.com/catchorg/Catch2" "${LICENSEFOLDER}/license_catch.txt")
	# AddLicense("GoogleTest" "${GTest_VERSION}" "https://github.com/google/googletest" "${LICENSEFOLDER}/license_googletest.txt")
	# AddLicense("OpenSSL" "" "https://www.openssl.org/" "${LICENSEFOLDER}/license_openssl.txt")

	set(LICENSE_ARRAY "${LICENSE_ARRAY}\n")

	configureFile("cmake/licenses.h.in" ${outputFile})
endfunction()
