#include <string.h>

int winyl_version_len() {
    return _WINYL_VERSION_LEN;
}
void winyl_version(char* version) {
    strncpy(version, _WINYL_VERSION, _WINYL_VERSION_LEN);
    version[_WINYL_VERSION_LEN] = '\0';
}
