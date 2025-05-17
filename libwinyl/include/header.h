#ifndef WINYL_HEADER_H
#define WINYL_HEADER_H
/**
 * @file
 * @brief Contains functions related to @ref winyl_header
 */

#include "winyl.h"
#include "request.h"

/** @brief Represents a HTTP header.
 *
 * Do not instantiate manually!
 */
struct winyl_header {
    /** @brief header name **/
    char* name;
    /** @brief header value **/
    char* value;
};

/**
 * @brief Returns the index of the header with @p name.
 */
int winyl_has_header(winyl* host, char* name);
/**
 * @brief Adds a header to the request. Returns the index of it.
 */
int winyl_add_header(winyl* host, char* name, char* value);
/**
 * @brief Removes a header from the request with @p name.
 */
int winyl_remove_header(winyl* host, char* name);
/**
 * @brief Gets the value of header with @p name.
 */
char* winyl_get_header_value(winyl* host, char* name);

/**
 * @brief Variant of @ref winyl_has_header that works with responses.
 */
int winyl_has_header2(winyl_response* response, char* name);
/**
 * @brief Variant of @ref winyl_get_header_value that works with responses.
 */
char* winyl_get_header_value2(winyl_response* response, char* name);

#endif
