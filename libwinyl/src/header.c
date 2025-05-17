#include <stdlib.h>
#include <string.h>

#include "../include/header.h"
#include "../include/request.h"

struct winyl_header* _winyl_header_realloc(struct winyl_header* headers, int _headers_count) {
    struct winyl_header* new_headers = realloc(headers, sizeof(struct winyl_header) * _headers_count);
    if (new_headers) return new_headers;
    else {
        free(headers);
        return NULL;
    }
}

void _winyl_free_header(struct winyl_header header) {
    free(header.name);
    free(header.value);
}

int winyl_has_header(winyl* host, char* name) {
    int found = -1;
    for (int i = 0; i < host->_headers_count; i++) {
        if (strcmp(host->headers[i].name, name) == 0) {
            found = i;
            break;
        }
    }
    return found;
}

int winyl_has_header2(winyl_response* response, char* name) {
    return winyl_has_header(&response->_headers, name);
}

int winyl_add_header(winyl* host, char* name, char* value) {
    struct winyl_header header;
    header.name = strdup(name);
    header.value = strdup(value);

    int newIndex = winyl_has_header(host, header.name);
    if (newIndex == -1) {
        host->_headers_count++;
        host->headers = _winyl_header_realloc(host->headers, host->_headers_count);
        if (!host->headers) {
            _winyl_free_header(header);
            host->error = WINYL_ERROR_MALLOC;
            return -1;
        }
        newIndex = host->_headers_count - 1;
    }
    else {
        _winyl_free_header(host->headers[newIndex]);
    }

    host->headers[newIndex] = header;
    
    return newIndex;
}

int winyl_remove_header(winyl* host, char* name) {
    int found = winyl_has_header(host, name);
    if (found != -1) {
        _winyl_free_header(host->headers[found]);
        
        for (int i = found + 1; i < host->_headers_count; i++) {
            struct winyl_header header;
            header.name = strdup(host->headers[i].name);
            header.value = strdup(host->headers[i].value);
            _winyl_free_header(host->headers[i]);

            host->headers[i - 1] = header;
        }
        host->_headers_count--;
        host->headers = _winyl_header_realloc(host->headers, host->_headers_count);
        if (!host->headers) {
            host->error = WINYL_ERROR_MALLOC;
            return -1;
        }
    }
    return found;
}

char* winyl_get_header_value(winyl* host, char* name) {
    int found = winyl_has_header(host, name);
    if (found != -1) return host->headers[found].value;
    else return NULL;
}

char* winyl_get_header_value2(winyl_response* response, char* name) {
    return winyl_get_header_value(&response->_headers, name);
}
