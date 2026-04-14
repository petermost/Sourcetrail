#ifndef SETUP_PLATFORM_H
#define SETUP_PLATFORM_H

#include <FilePath.h>

void setupDefaultLocale();

void setupAppDirectories(const FilePath &appPath);
void setupAppEnvironment(int argc, char *argv[]);

#endif
