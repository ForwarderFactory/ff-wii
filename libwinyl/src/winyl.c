#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/winyl.h"
#include "../include/header.h"

#ifndef __DEVKITPPC__
#define net_gethostbyname gethostbyname
#endif

winyl winyl_open(char* hostname, int port) {
    winyl host;
    host.error = 0;
    host._hostname_len = -1;

    if (port < 0 || port > 65535) {
        host.error = WINYL_ERROR_PORT;
        return host;
    }

    host._hostname_len = strlen(hostname);
    host.hostname = strndup(hostname, host._hostname_len);
    
    host._dns = net_gethostbyname(host.hostname);
    if (!host._dns) {
        host.error = WINYL_ERROR_DNS;
        return host;
    }

    int amount = -1;
    for (amount = 0; host._dns->h_addr_list[amount]; amount++);
    if (amount <= 0) {
        host.error = WINYL_ERROR_NX;
        return host;
    }
    host._dns_hosts = amount;

    host.port = port;
    host._port_len = WINYL_INTLEN(host.port);
    host._addr.sin_port = htons(host.port);
    host._addr.sin_family = AF_INET;
    
    host._headers_count = 0;
    host.headers = NULL;
   
    char* _host = malloc(host._hostname_len + 1 + (host.port == 80 ? 0 : (1 + host._port_len)));
    if (!_host) {
        host.error = WINYL_ERROR_MALLOC;
        return host;
    }
    strncpy(_host, host.hostname, host._hostname_len);
    _host[host._hostname_len] = '\0';
    if (host.port != 80) {
        char* portstr = malloc(host._port_len + 2);
        if (!portstr) {
            host.error = WINYL_ERROR_MALLOC;
            return host;
        }
        snprintf(portstr, host._port_len + 2, ":%d", host.port);
        strncat(_host, portstr, host._port_len + 1);
        free(portstr);
    }

    winyl_add_header(&host, "Host", _host);
    free(_host);
    if (host.error != 0) return host;

    char* user_agent = malloc(10 + _WINYL_VERSION_LEN);
    if (!user_agent) {
        host.error = WINYL_ERROR_MALLOC;
        return host;
    }
    snprintf(user_agent, 10 + _WINYL_VERSION_LEN, "libwinyl/" _WINYL_VERSION);
    winyl_add_header(&host, "User-Agent", user_agent);
    free(user_agent);
    if (host.error != 0) return host;

    host._http_version = 10;
    winyl_change_http(&host, 11);
    host._method = malloc(4);
    strncpy(host._method, "GET", 3);
    host._method[3] = '\0';

    return host;
}

int winyl_change_http(winyl* host, int version) {
    if (version == 10) {
        winyl_remove_header(host, "Connection");
    }
    else if (version == 11) {
        winyl_add_header(host, "Connection", "close");
    }
    else return -1;
    host->_http_version = version;
    return 0;
}

void _winyl_free_header(struct winyl_header header);

void winyl_close(winyl* host) {
    if (host->hostname != NULL) free(host->hostname);
    if (host->_method != NULL) free(host->_method);

    for (int i = 0; i < host->_headers_count; i++) {
        _winyl_free_header(host->headers[i]);
    }

    free(host->headers);
}
