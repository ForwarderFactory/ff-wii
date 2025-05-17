#ifndef WINYL_H
#define WINYL_H
/**
 * @file
 * @brief Contains the @ref winyl_open function and related
 */

#ifdef __DEVKITPPC__
#include <network.h>
#else
#include <netdb.h>
#endif

/** @brief Calculates the amount of digits in a positive integer **/
#define WINYL_INTLEN(x) ({int r = 1; int v = x; while(v > 9) { r++; v /= 10; }; r;})

/** @brief HTTP/1.0 for @ref winyl_change_http **/
#define WINYL_HTTP_1_0 10
/** @brief HTTP/1.1 for @ref winyl_change_http **/
#define WINYL_HTTP_1_1 11

struct winyl_header;

/**
 * @brief Represents a HTTP host
 */
typedef struct winyl {
    /** @brief Contains the error code after any winyl call. **/
    int error;

    /** @brief Set by @ref winyl_open **/
    char* hostname;
    /** @brief Set by @ref winyl_open **/
    int port;
    /** @brief Header collection; see @ref header.h **/
    struct winyl_header* headers;

    /** @private **/
    int _hostname_len;
    /** @private **/
    int _port_len;
    /** @private **/
    int _headers_count;

    /** @private **/
    int _http_version;
    /** @private **/
    char* _method;
    /** @private **/
    struct hostent* _dns;
    /** @private **/
    int _dns_hosts;
    /** @private **/
    struct sockaddr_in _addr;
} winyl;

/** @brief Speficied port is not valid. **/
#define WINYL_ERROR_PORT 7
/** @brief Call to `net_gethostbyname()` failed. **/
#define WINYL_ERROR_DNS 1
/** @brief Specified domain does not exist. **/
#define WINYL_ERROR_NX 4
/** @brief Allocating memory failed. **/
#define WINYL_ERROR_MALLOC 8

/**
 * @brief Opens an HTTP host with the specified port.
 */
winyl winyl_open(char* hostname, int port);
/**
 * @brief Frees memory used up by @p host.
 */
void winyl_close(winyl* host);

/**
 * @brief Changes HTTP version used for requests to @p host.
 */
int winyl_change_http(winyl* host, int version);

#endif
