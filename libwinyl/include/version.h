#ifndef WINYL_VERSION_H
#define WINYL_VERSION_H
/**
 * @file
 * @brief Contains functions related to the library version
 */

/**
 * @brief Returns the winyl version length
 */
int winyl_version_len();
/**
 * @brief Writes the current winyl version into @p version
 */
void winyl_version(char* version);

#endif
