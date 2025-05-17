#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/request.h"
#include "../include/header.h"

#ifndef __DEVKITPPC__
#include <unistd.h>
#include <sys/socket.h>
#define net_socket socket
#define net_connect connect
#define net_send send
#define net_recv recv
#define net_close close
#endif

winyl_response winyl_request(winyl* host, char* path, int flags) {
    winyl_response response;
    response.error = 0;
    response.body = NULL;
    response._body_len = 0;

    response.http_version = 0;
    response.status = 0;
    response.status_text = NULL;

    response.headers = NULL;
    response._headers.headers = NULL;
    response._headers._headers_count = 0;
    response.headers_count = 0;

    response._headers.hostname = NULL;
    response._headers._method = NULL;

    response.socket = net_socket(AF_INET, SOCK_STREAM, 0);
    if (response.socket == -1) {
        response.error = WINYL_ERROR_SOCKET;
        return response;
    }

    int success = 0;
    for (int i = 0; i < host->_dns_hosts; i++) {
        host->_addr.sin_addr.s_addr = *(uint32_t*)host->_dns->h_addr_list[i];
        if (net_connect(response.socket, (struct sockaddr*)&host->_addr, sizeof(host->_addr)) < 0) {
            if (i >= (host->_dns_hosts - 1)) response.error = WINYL_ERROR_CONNECT;
            else continue;
        }
        else success = 1;
        break;
    }
    if (!success) return response;

    int initlen = strlen(host->_method) + 1 + strlen(path) + 1 + 5 + 3 + 2 + 1 + ((flags & WINYL_REQUEST_SLASH) ? 1 : 0);
    char* init = malloc(initlen);
    if (!init) {
        response.error = WINYL_ERROR_MALLOC;
        return response;
    }
    snprintf(init, initlen, "%s %s%s HTTP/%d.%d\r\n", host->_method, (flags & WINYL_REQUEST_SLASH) ? "/" : "", path, host->_http_version / 10, host->_http_version % 10);

    if (net_send(response.socket, init, initlen - 1, 0) < 0) {
        response.error = WINYL_ERROR_SEND;
        return response;
    }

    free(init);

    for (int i = 0; i < host->_headers_count; i++) {
        int headerlen = strlen(host->headers[i].name) + 2 + strlen(host->headers[i].value) + 2 + 1;
        char* header = malloc(headerlen);
        if (!header) {
            response.error = WINYL_ERROR_MALLOC;
            return response;
        }
        snprintf(header, headerlen, "%s: %s\r\n", host->headers[i].name, host->headers[i].value);
        if (net_send(response.socket, header, headerlen - 1, 0) < 0) {
            response.error = WINYL_ERROR_SEND;
            return response;
        }
        free(header);
    }
    
    if (net_send(response.socket, "\r\n", 2, 0) < 0) {
        response.error = WINYL_ERROR_SEND;
        return response;
    }

    if (!(flags & WINYL_REQUEST_NO_HEADERS)) {
        char buf[1];
        char* buf2 = malloc(1);
        if (!buf2) {
            response.error = WINYL_ERROR_MALLOC;
            return response;
        }
        buf2[0] = '\0';
        char* buf2_bkp = NULL;

        int rec;
        while(1) {
            if ((rec = net_recv(response.socket, buf, 1, 0)) < 0) {
                response.error = WINYL_ERROR_RECV;
                return response;
            }
            else if (rec == 0) break;
            
            if (buf[0] == '\r') continue;
            if (buf[0] == '\n') {
                if (strlen(buf2) <= 0) break;

                if (strncmp(buf2, "HTTP/", 5) == 0) {
                    response.http_version = 10;
                    if (buf2[7] == '1') response.http_version++;

                    for (int i = 0; i < 4; i++) {
                        buf2[i] = i == 3 ? '\0' : buf2[i + 9];
                    }
                    response.status = atoi(buf2);

                    int i;
                    for (i = 13; buf2[i] != '\0'; i++) {
                        buf2[i - 13] = buf2[i];
                    }
                    buf2[i - 13] = '\0';
                    response.status_text = strdup(buf2);
                    
                    if (!response.status_text) {
                        free(buf2);
                        response.error = WINYL_ERROR_MALLOC;
                        return response;
                    }
                }
                else {
                    int i;
                    for (i = 0; i < strlen(buf2); i++) {
                        if (buf2[i] == ':') {
                            buf2[i] = '\0';
                            break;
                        }
                    }

                    char* headername_bkp = strdup(buf2);

                    int initI = i + 2;
                    for (i = initI; buf2[i] != '\0'; i++) {
                        buf2[i - initI] = buf2[i];
                    }
                    buf2[i - initI] = '\0';
                    
                    int header_index = winyl_add_header(&response._headers, headername_bkp, buf2);
                    free(headername_bkp);

                    if (header_index == -1) {
                        response.error = response._headers.error;
                        return response;
                    }
                }

                free(buf2);
                buf2 = malloc(1);
                buf2[0] = '\0';
                
                continue;
            }

            buf2_bkp = realloc(buf2, strlen(buf2) + 2);
            if (!buf2_bkp) {
                free(buf2);
                response.error = WINYL_ERROR_MALLOC;
                return response;
            }
            else buf2 = buf2_bkp;
            strncat(buf2, buf, rec);
        }

        free(buf2);
    
        response.headers = response._headers.headers;
        response.headers_count = response._headers._headers_count;
    }

    if (flags & WINYL_REQUEST_GET_SOCKET) return response;

    int rec;
    char buf[1024];
    char* newbody = NULL;

    int addNul = !(flags & WINYL_REQUEST_NO_NUL);
    while(1) {
        if ((rec = net_recv(response.socket, buf, 1024, 0)) < 0) {
            response.error = WINYL_ERROR_RECV;
            return response;
        }
        else if (rec == 0) break;
    
        newbody = realloc(response.body, response._body_len + rec + addNul);
        if (!newbody) {
            response.error = WINYL_ERROR_MALLOC;
            return response;
        }
        else response.body = newbody;

        memcpy(response.body + response._body_len, buf, rec);
        response._body_len += rec;
    }
    if (addNul) response.body[response._body_len] = '\0';

    if (net_close(response.socket) != 0) {
        response.error = WINYL_ERROR_CLOSE;
        return response;
    }

    return response;
}

void winyl_response_close(winyl_response* response) {
    if (response->status_text) free(response->status_text);
    if (response->body) free(response->body);

    winyl_close(&response->_headers);
}
