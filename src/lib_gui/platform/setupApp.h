#ifndef SETUP_PLATFORM_H
#define SETUP_PLATFORM_H

#include <FilePath.h>
#include <Version.h>

void setupDefaultLocale();

Version setupAppDirectories(const FilePath &appPath);
void setupAppEnvironment(int argc, char *argv[]);

#endif
