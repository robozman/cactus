/*
    Filename:   cactus.c
    Author:     Robert Zampino
    Contant:    rzampino@mines.edu
    Purpose:    Implementing a Minecraft b1.7.3 server in C
    Licence:    AGPLv3 (See LICENCE)
    Thanks:     wiki.vg for protocol documentation
*/
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>
#include "chunk_packets.h"

#define PORT 9955

int recieve_packet(int fd)
{
        // unsigned char buffer[1024] = {0};
        uint8_t buffer[1024] = {0};
        int result_len = recv(fd, buffer, 1024, 0);

        printf("\n");
        printf("buffer[0]: %x\n", buffer[0]);
        int packet_id = buffer[0];
        if (packet_id == 0)
                packet_id == buffer[1];
        switch (buffer[0]) {
        case 0x02:
                printf("Case 0x02\n");
                for (int i = 0; i < (result_len + 1) / sizeof(uint16_t); i++) {
                        printf("%x", buffer[i]);
                }
                printf("\n");
                for (int i = 2; i < buffer[1] + 2; i++) {
                        printf("%c", buffer[i]);
                }
                break;
        case 0x01:
                printf("Case 0x01\n");

                printf("Protocol Version: %d\n", buffer[4]);
                for (int i = 0; i < (result_len) / sizeof(uint16_t); i++) {
                        printf("%x ", buffer[i]);
                }
                break;
        case 0x0d:
                printf("Case 0x0d\n");
                break;
        case 0x0b:
                printf("Case 0x0b\n");
                break;
        default:
                // printf("DEFAULT CASE\n");
                // for (int i = 0; i < 512; i++) {
                //        printf("%x", buffer[i]);
                //}
                break;
        }
        printf("\n");
        return buffer[0];
}

void respond_to_packet(int fd, int packet_num)
{
        uint16_t disconnect_packet[] = {0xFF, 0x13, 'T', 'h', 'e', ' ', 's', 'e', 'r', 'v', 'e',
                                        'r',  ' ',  'i', 's', ' ', 'f', 'u', 'l', 'l', '!'};
        uint16_t login_packet[] = {0x01, 0x0000, 0x0001, 0x0001, 0x1174, 0x1111, 0x1111, 0x1111, 0x0011};
        // uint16_t login_packet[] = {0x01, 0x0000, 0x0001, 0x0000, 0x1111, 0x1111, 0x1111, 0x1111, 0x00};
        uint16_t spawn_packet[] = {0x05, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

        uint8_t time_packet[] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t block_change[] = {0x35, 0x00, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
        uint8_t pre_chunk_packet[] = {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        
        uint16_t packet0x20[5] = {0x02, 0x01, '-'};

        uint8_t keep_alive_packet[] = {0x00};

        int bytes_sent;
        // printf("packet_num: %d", packet_num);
        switch (packet_num) {
        case 0x00:
                bytes_sent = send(fd, keep_alive_packet, 1, 0);
                printf("SENT %d bytes\n", bytes_sent);
                break;
        case 0x02:
                // bytes_sent = send(fd, 0x20, 1, 0);
                // bytes_sent = send(fd, 0x10, 1, 0);
                // bytes_sent = send(fd, '-', 1, 0);

                // for (int i = 0; i < 3; i++) {
                //         printf("0x%x ", packet[i]);
                // }
                // printf("\n");
                bytes_sent = send(fd, packet0x20, 5, 0);

                printf("SENT %d bytes\n", bytes_sent);
                break;
        case 0x01:
                // bytes_sent = send(fd, disconnect_packet, 41, 0);
                bytes_sent = send(fd, login_packet, 18, 0);
                printf("SENT %d  bytes\n", bytes_sent);
                bytes_sent = send(fd, spawn_packet, 13, 0);
                printf("SENT %d  bytes\n", bytes_sent);
                bytes_sent = send(fd, time_packet, 9, 0);
                printf("SENT %d  bytes\n", bytes_sent);


                //bytes_sent = send(fd, pre_chunk_packet, 10, 0);
                //printf("SENT %d  bytes\n", bytes_sent);
                //bytes_sent = send(fd, chunk_packet1, 5000, 0);
                //printf("SENT %d bytes\n", bytes_sent);
                //bytes_sent = send(fd, chunk_packet2, sizeof(chunk_packet2) - 1, 0);
                //printf("SENT %d bytes\n", bytes_sent);

                break;
        case 0x0b:
                break;
        case 0xFF:
        default:
                break;
        }
}

int main(int argc, char *argv[])
{

        int server_fd, current_fd, packet_num;
        int opt = 1;
        struct sockaddr_in address;
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        int addrlen = sizeof(address);
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
        }
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        addr_size = sizeof(their_addr);
        current_fd = accept(server_fd, (struct sockaddr *) &their_addr, &addr_size);
        int i = 0;
        while (true) {
                packet_num = recieve_packet(current_fd);
                respond_to_packet(current_fd, packet_num);
                if (i >= 10)
                        break;
                i++;
        }
        while (true) {
                sleep(1);
                respond_to_packet(current_fd, 0);
        }
        // necurrent_fd = accept(server_fd, (struct sockaddr *) &their_addr, &addr_size);

        // current_fd = accept(server_fd, (struct sockaddr *) &their_addr, &addr_size);
        // packet_num = recieve_packet(current_fd);
}