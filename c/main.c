#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "camera.h"

#define PORT 6255
#define BUFFER_SIZE 1024

#define LOG(level, ...)                                        \
    do {                                                       \
        printf("%s:%d: %s: ", __FILE__, __LINE__, (level));    \
        printf(__VA_ARGS__);                                   \
        printf("\n");                                          \
    } while (0)
#define INFO(...)  LOG(" INFO", __VA_ARGS__)
#define ERROR(...) LOG("ERROR", __VA_ARGS__)

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        ERROR("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Filling server information
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ERROR("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(client_addr);

    if (camera_init() < 0) {
        ERROR("failed to open camera");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    INFO("UDP server listening on port %d...\n", PORT);

    while (1) {

        const int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL,
                               (struct sockaddr *)&client_addr, &addr_len);

        if (n < 0) {
            ERROR("Failed to recv packet: %s", strerror(errno));
            continue;
        }

        if (addr_len != sizeof(struct sockaddr_in)) {
            ERROR("IPv6 not supported.");
            continue;
        }

        if (n != 4) {
            ERROR("Not a valid packet (too small).");
            continue;
        }

        if (memcmp(buffer, "dsc?", 4) == 0) {
            INFO("discovery request");
            sendto(sockfd, "dsc+", 4, /*flags=*/0, (const struct sockaddr*) &client_addr, addr_len);
            continue;
        }

        if (memcmp(buffer, "jpg?", 4) == 0) {
            INFO("image request");
            int image_size = 0;
            const int header_size = 4;
            unsigned char* frame = camera_read(header_size, &image_size);
            if (!frame) {
                ERROR("failed to grab camera frame");
                continue;
            }
            memcpy(frame, "jpg+", 4);
            sendto(sockfd, frame, image_size + header_size, /*flags=*/0, &client_addr, addr_len);
            free(frame);
            continue;
        }

        ERROR("unknown request");
    }

    camera_shutdown();
    close(sockfd);
    return 0;
}
