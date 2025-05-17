#ifndef WINYL_REQUEST_H
#define WINYL_REQUEST_H
/**
 * @file
 * @brief Contains the @ref winyl_request function and related
*/

#include "winyl.h"

/**
 * @brief Represents a HTTP response
 */
typedef struct winyl_response {
    /** @brief Contains the error code after any winyl call. **/
    int error;
    /** @brief Socket file descriptor. Blank if @ref WINYL_REQUEST_GET_SOCKET is not used. **/
    int socket;

    /** @brief HTTP version. See @ref WINYL_HTTP_1_0 and similar. **/
    int http_version;
    /** @brief HTTP status. **/
    int status;
    /** @brief HTTP status text. **/
    char* status_text;

    /** @brief Header collection; see @ref header.h **/
    struct winyl_header* headers;
    /** @brief Header count. **/
    int headers_count;
    /** @private **/
    winyl _headers;

    /** @brief Response body. **/
    char* body;
    /** @private **/
    int _body_len;
} winyl_response;

/** @brief Skips reading the body so the socket can be read from. **/
#define WINYL_REQUEST_GET_SOCKET 1
/** @brief Skips parsing the headers. **/
#define WINYL_REQUEST_NO_HEADERS 2
/** @brief Skips adding a \\0 to the end of the response body. **/
#define WINYL_REQUEST_NO_NUL 4
/** @brief Adds a slash to the beginning of the path passed. Useful for <a href="https://github.com/jacketizer/libyuarel">yuarel</a>. **/
#define WINYL_REQUEST_SLASH 8

/** @brief Error calling `net_socket()`. **/
#define WINYL_ERROR_SOCKET 2
/** @brief Error calling `net_connect()`. **/
#define WINYL_ERROR_CONNECT 3
/** @brief Error calling `net_send()`. **/
#define WINYL_ERROR_SEND 5
/** @brief Error calling `net_recv()`. **/
#define WINYL_ERROR_RECV 6
/** @brief Error calling `net_close()`. **/
#define WINYL_ERROR_CLOSE 9

/**
 * @brief Performs a HTTP request.
 */
winyl_response winyl_request(winyl* host, char* path, int flags);
/**
 * @brief Frees memory used up by @p response.
 */
void winyl_response_close(winyl_response* response);

#endif
