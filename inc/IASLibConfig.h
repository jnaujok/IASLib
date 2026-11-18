/**
 * @file
 * @brief IASLib's version, as set in CMakeLists.txt.
 *
 *      CMake generates IASLibConfig.h from IASLibConfig.h.in (configure_file),
 * filling in the version from the project() command. IASLib.h includes it.
 * Edit the template, not the generated header.
 */
// the cmakelists.txt configured options and settings for IASLib
/// The major version number.
#define IASLib_VERSION_MAJOR 3
/// The minor version number.
#define IASLib_VERSION_MINOR 0
/// The patch version number.
#define IASLib_VERSION_PATCH 0
/// The whole version as text ("major.minor.patch").
#define IASLib_VERSION_STRING "3.0.0"
