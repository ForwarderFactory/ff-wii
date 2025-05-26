#pragma once

#ifdef __DEVKITPPC__
#include <network.h>
#include <sys/dir.h>
#include <sys/select.h>
#include <ogc/system.h>
#include <fcntl.h>
#include <unistd.h>
#else
#define net_socket socket
#define net_connect connect
#define net_send send
#define net_recv recv
#define net_close close
#define net_gethostbyname gethostbyname
#define net_gethostip gethostip
#define net_init() 0
#define net_select select

#include <sys/socket.h>
#endif
#include <string>
#include <stdexcept>

namespace ff::net {
    enum class Version {
        HTTP_1_0,
        HTTP_1_1,
    };
    enum class Method {
        GET,
        POST,
    };

    struct Request {
        std::string hostname{};
        std::string path{"/"};
        int port{80};
        std::string user_agent{"ff-wii/1.0"};
        Method method{Method::GET};
        Version version{Version::HTTP_1_1};
        std::string body{};
        std::vector<std::pair<std::string, std::string>> headers{};
    };

    struct Response {
        int status_code{};
        std::string body{};
        std::string raw_body{};
        std::vector<std::pair<std::string, std::string>> headers{};
    };

    static std::string decode_chunked(const std::string& encoded) {
        std::string decoded;
        size_t pos = 0;

        while (pos < encoded.size()) {
            size_t crlf = encoded.find("\r\n", pos);
            if (crlf == std::string::npos) break;

            std::string size_str = encoded.substr(pos, crlf - pos);
            size_t chunk_size = 0;
            try {
                chunk_size = std::stoul(size_str, nullptr, 16);
            } catch (...) {
                break;
            }

            if (chunk_size == 0) break;

            pos = crlf + 2;
            if (pos + chunk_size > encoded.size()) break;

            decoded.append(encoded.substr(pos, chunk_size));
            pos += chunk_size + 2;
        }

        return decoded;
    }

    class Client {
        public:
            explicit Client() {
                if (net_init() < 0) {
                    throw std::runtime_error{"failed to init networking"};
                }
            }
            ~Client() {
                net_deinit();
            }

            static std::string get_wii_ip() {
                char my_ip[16];
                unsigned int ip = net_gethostip();
                snprintf(my_ip, sizeof(my_ip), "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
                return {my_ip};
            }

            static Response get(const Request& request) {
                const auto sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

                if (sock < 0) {
                    throw std::runtime_error{"failed to create socket"};
                }

                if (request.path.empty() || request.path[0] != '/') {
                    net_close(sock);
                    throw std::runtime_error{"path must start with /"};
                }

                sockaddr_in server{};
                memset(&server, 0, sizeof(server));
                server.sin_family = AF_INET;
                server.sin_port = htons(request.port);

                hostent* host = net_gethostbyname(request.hostname.c_str());
                if (!host || !host->h_addr_list[0]) {
                    SYS_Report("DNS resolution failed\n");
                    throw std::runtime_error("DNS resolution failed");
                }
                if (host->h_addrtype != AF_INET) {
                    net_close(sock);
                    throw std::runtime_error{"unsupported address type"};
                }

                memcpy(&server.sin_addr, host->h_addr_list[0], sizeof(server.sin_addr));

                if (net_connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
                    net_close(sock);
                    throw std::runtime_error{"failed to connect to server"};
                }

                std::string body;
                std::string method_str = (request.method == Method::GET) ? "GET" : "POST";
                std::string version_str = (request.version == Version::HTTP_1_0) ? "HTTP/1.0" : "HTTP/1.1";

                body = method_str + " " + request.path + " " + version_str + "\r\n";
                body += "Host: " + request.hostname + "\r\n";

                if (!request.user_agent.empty()) {
                    body += "User-Agent: " + request.user_agent + "\r\n";
                }

                for (const auto& [key, value] : request.headers) {
                    if (key == "Host" || key == "User-Agent" || key == "Connection" || key == "Content-Length" || key == "GET" || key == "POST") {
                        throw std::runtime_error{"illegal header: " + key};
                    }
                    body += key + ": " += value + "\r\n";
                }

				// we must close the connection after the request
				// cloudflare is retarded and keeps the connection open if we don't close it, even if the
				// server closes it.
                body += "Connection: close\r\n";

                if (request.method == Method::POST || !request.body.empty()) {
                    body += "Content-Length: " + std::to_string(request.body.size()) + "\r\n";
                    body += "\r\n" + request.body;
                } else {
                    body += "\r\n";
                }

                if (net_send(sock, body.c_str(), static_cast<int32_t>(body.size()), 0) < 0) {
                    net_close(sock);
                    throw std::runtime_error{"failed to send request"};
                }

                std::string response{};
                char buffer[1024];
                int bytes_received = 0;

                while ((bytes_received = net_recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
                    buffer[bytes_received] = '\0'; // null-terminate the string
                    response += buffer;
                }

                Response ret{};

                const auto pos = response.find("\r\n\r\n");
                if (pos == std::string::npos) {
                    SYS_Report("no header terminator\n");
                    ret.body = response;
                    return ret;
                }

                std::string headers_str = response.substr(0, pos);
                ret.body = response.substr(pos + 4); // skip the \r\n\r\n

                std::istringstream headers_stream(headers_str);
                std::string status_line;
                if (std::getline(headers_stream, status_line)) {
                    if (status_line.back() == '\r') status_line.pop_back();
                    auto code_pos = status_line.find(' ');
                    if (code_pos != std::string::npos) {
                        auto code_end = status_line.find(' ', code_pos + 1);
                        if (code_end != std::string::npos) {
                            try {
                                ret.status_code = std::stoi(status_line.substr(code_pos + 1, code_end - code_pos - 1));
                            } catch (...) {
                                ret.status_code = -1;
                            }
                        }
                    }
                }

                std::string line;
                while (std::getline(headers_stream, line)) {
                    if (line.back() == '\r') line.pop_back();
                    auto colon_pos = line.find(':');
                    if (colon_pos != std::string::npos) {
                        auto key = line.substr(0, colon_pos);
                        auto value = line.substr(colon_pos + 1);
                        auto trim = [](std::string& s) {
                            s.erase(0, s.find_first_not_of(" \t"));
                            s.erase(s.find_last_not_of(" \t") + 1);
                        };
                        trim(key);
                        trim(value);
                        ret.headers.emplace_back(key, value);
                    }
                }

                for (const auto& [key, value] : ret.headers) {
                    if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos) {
                        ret.body = decode_chunked(ret.body);
                        break;
                    }
                }

                return ret;
            }
    };
}