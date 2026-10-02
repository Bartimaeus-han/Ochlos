# Ochlos 공격 실증 로그 (Attack Simulation Log)

레드팀(Ochlos) 공격 도구로 테스트베드(블루팀 웹 시스템)의 방어선을 타격한 실증 기록입니다. 각 시나리오의 작성 형식은 `.claude/rules/attack-log.md`에 있습니다.

망 구조·블루팀 방어 현황판과 공방전의 다음 차례는 이 저장소 밖(상위 관제 폴더)에서 관리합니다. 이 문서는 이 저장소가 수행한 실증 계측치만 소유합니다.

---

### [SCENARIO-01] L4 TCP Connection Starvation & Screening Router 실시간 패킷 탐지
- **공격 목표 (Attack Objective)**: 백엔드 웹 서버의 TCP 수신 대기열 및 워커 스레드 자원 점유 (DoS)
- **OSI 계층 (Target Layer)**: Layer 4 (Transport Layer)
- **공격 메커니즘 (Mechanics)**: 3-Way Handshake를 완료한 후 `closesocket()`(RST/FIN)을 보내지 않고 소켓을 배열에 보관한 채 대기하여 서버의 연결 풀을 고갈시킴.
- **실행 도구 (Tool)**: [../DoS/tcp_syn_flooding.cpp](../DoS/tcp_syn_flooding.cpp) (C++20 Winsock/POSIX)
- **테스트베드 실측 결과 (Empirical Results)**:
  * `ScreeningRouter.exe` (Port 8080) 기동 후 Ochlos 50개 커넥션 인입 시 실시간 L3 IP (`127.0.0.1`) 및 L4 출발지 Port(`5xxxx`) 감지 로그 50건 연속 출력 확인.
- **블루팀 방어 권고사항 (Blue Team Feedback)**:
  * L3/L4 경계선에서 패킷 가시성을 확보하였으므로, 다음 단계로 검증된 트래픽을 내부 백엔드(`bartimaeus-app:9090`)로 포워딩(중계)하는 파이프라인 및 Stateless IP/Port 룰 필터링 구현 필요.

### [SCENARIO-02] L3 ICMP Echo Flooding (Ping Flooding) & IP Spoofing 실증
- **공격 목표 (Attack Objective)**: 타깃 네트워크 스택의 ICMP 처리 인터럽트(SoftIRQ) 및 대역폭/응답 버퍼 고갈 (DoS)
- **OSI 계층 (Target Layer)**: Layer 3 (Network Layer)
- **공격 메커니즘 (Mechanics)**: L3 IPv4(20B) + ICMP Echo Request(8B)의 28바이트 원시 패킷을 직접 패킹하고, 출발지 IP를 임의 주소로 위조(L3 IP Spoofing)하여 무차별 방출.
  * 본 실측 당시 위조 출발지는 `100.0.0.99`였으나, 해당 대역(`100.0.0.0/8`)이 사설이 아닌 공인 라우팅 대역(ARIN 할당)임을 확인하여 이후 도구를 RFC 5737 TEST-NET-2(`198.51.100.99`)로 교체함.
  * 위조 출발지는 ICMP Echo Reply의 도달 지점만 결정하고 타깃 인입 경로(`screening-router` 컨테이너)에는 관여하지 않으므로, 하기 계측치(PPS·유실률)는 영향 없이 그대로 유효함 — 재실증 불필요.
- **실행 도구 (Tool)**: [../DoS/icmp_echo_flooding.cpp](../DoS/icmp_echo_flooding.cpp) (C++20 Raw Socket / `IP_HDRINCL`)
- **실행 명령어 (On-demand Runner)**:
  ```bash
  # 패킷 수 인자 지정 가능 (기본값: 1000)
  docker compose run --rm ochlos cpprun DoS/icmp_echo_flooding.cpp [패킷수]
  ```
- **테스트베드 실측 결과 (Empirical Results)**:
  * **공격 도구 최적화**: 송출 루프 내부의 `write()` 시스템 콜(`std::cout`) 및 `sleep`을 완전 제거하여 초당 약 40,000 ~ 52,000 PPS의 최대 라인 레이트 버스트 송출 역량 확보.
  * **규모별 Cold-Start(컨테이너 재시작 클린 상태) 실측 통계 (각 3회 반복)**:
    * **1,000개 폭격 (지속 0.02초, ~40,000 PPS)**: 평균 수신 570개 / **평균 유실률 43.0%**
    * **10,000개 폭격 (지속 0.25초, ~39,500 PPS)**: 평균 수신 4,758개 / **평균 유실률 52.4%**
    * **100,000개 폭격 (지속 1.90초, ~52,000 PPS)**: 평균 수신 37,844개 / **평균 유실률 62.1%** (최대 63.6% 유실)
  * **병목 및 DoS 메커니즘 규명**:
    * 블루팀 `ScreeningRouter`의 단일 스레드 `std::cout` 동기 콘솔 출력 지연으로 인해 수신 큐 소비 속도가 인입 속도를 따라가지 못함 (라우터 최대 처리량: 약 20,000 PPS).
    * 결과적으로 초과 인입된 패킷이 리눅스 커널의 소켓 수신 버퍼(`SO_RCVBUF`)에서 대량 폐기(Drop)되어, 폭격 지속 시 **인입 트래픽의 60% 이상이 증발하는 명백한 서비스 거부(DoS) 상태를 완벽히 실증**.
  * **호스트 안전성 검증**: 10만 개 연속 폭격 시에도 호스트 OS(Windows/macOS) 및 도커 데몬 영향도 0에 수렴(`vmmemWSL` 메모리 1.8GB 안정 유지, 호스트 프리징 없음).
- **블루팀 방어 권고사항 (Blue Team Feedback)**:
  * **성능 최적화**: 대량 트래픽 인입 시 I/O 병목 방지를 위해 실시간 패킷 콘솔 로깅을 비동기화하거나 N개(예: 1,000개) 단위 샘플링 출력으로 전환 필요.
  * **경계선 차단**: `ScreeningRouter`에 미사용 ICMP Type 8 패킷을 즉각 폐기하는 L3 Stateless Drop 룰셋 구현 필요.

### [SCENARIO-03] L4 Raw TCP SYN Flooding & NAPT Session State Exhaustion 실증
- **공격 목표 (Attack Objective)**: 스크리닝 라우터(ScreeningRouter)의 NAPT 세션 테이블 메모리 고갈 및 미완료 Half-Open 세션 누적 (State Exhaustion DoS)
- **OSI 계층 (Target Layer)**: Layer 4 (Transport Layer)
- **공격 메커니즘 (Mechanics)**:
  * L3 IPv4(20B) + TCP SYN(20B) = 40바이트 원시 패킷을 직접 패킹(`IP_HDRINCL`, `FLAG_SYN`).
  * 출발지 포트를 고정하지 않고 매 루프마다 난수/순환(`10000 ~ 64999`)으로 변조하여, 라우터가 매번 독립된 신규 세션으로 인식하도록 유도.
  * 3-Way Handshake(SYN-ACK / ACK)를 완료하지 않는 순수 Half-Open 형태로 무차별 송출하여 NAPT 세션 테이블의 무한 증식을 촉발.
- **실행 도구 (Tool)**: [../DoS/raw_tcp_syn_flooding.cpp](../DoS/raw_tcp_syn_flooding.cpp) (C++20 Raw Socket / `IP_HDRINCL`)
- **실행 명령어 (On-demand Runner)**:
  ```bash
  # 패킷 수 인자 지정 가능 (기본값: 1000)
  docker compose run --rm ochlos cpprun DoS/raw_tcp_syn_flooding.cpp [패킷수]
  ```
- **테스트베드 실측 결과 (Empirical Results)**:
  * **공격 도구 최적화**: 콘솔 I/O 병목 및 `sleep` 제거, CLI 동적 패킷 수 인자 및 포트 오버플로우 방지(`10000 + (i % 55000)`) 적용으로 초당 약 30,000 PPS 송출 성능 확보.
  * **규모별 실측 데이터 (Cold Start 기반)**:
    * **1,000개 폭격 (32.68 ms, 30,598 PPS)**: 878개 수신 (유실률 12.2%), 메모리 680 KiB 유지 (소량 세션).
    * **10,000개 폭격 (344.25 ms, 29,049 PPS)**: 9,141개 수신 (유실률 8.59%), 메모리 **680 KiB → 1.00 MiB (약 50% 팽창)**.
    * **100,000개 폭격 (3,251.14 ms, 30,758 PPS)**: 83,229개 수신 (유실률 16.77%), 메모리 **680 KiB → 3.02 MiB (초기 대비 약 4.5배 급증)**.
  * **취약점 및 공격 메커니즘 규명**:
    * **상태 고갈(State Exhaustion)**: 라우터가 `SYN` 단 1개만으로도 3-Way Handshake 완료 검증 및 만료 시간(TTL) 없이 `std::unordered_map`에 노드를 무한 할당함을 실증.
    * **단일 포트 키(Single-Port Key) 한계 식별**: `client_port` 단일 필드를 Key로 사용함에 따라 서로 다른 IP 간 세션 충돌/덮어쓰기 위험 및 65,535개 물리적 키 한계 확인.
  * **호스트 안전성 검증**: 10만 개 폭격 시에도 `vmmemWSL` 메모리 1.8GB 안정 유지, 호스트 CPU 및 데몬 영향 없음 확인.
- **블루팀 방어 권고사항 (Blue Team Feedback)**:
  * **복합 키(5-Tuple) 도입**: `(src_ip, src_port, dst_ip, dst_port, proto)` 조합으로 세션 키를 구성하여 포트 충돌 방지.
  * **Half-Open 세션 타임아웃 & 가비지 컬렉션(GC)**: SYN만 도착하고 ACK가 없는 미완료 세션은 3~5초 이내에 강제 회수하는 타이머 도입.
  * **최대 용량 제한(Capacity Cap)**: 세션 테이블 상한선 설정 및 LRU 기반 퇴출 정책 수립.

### [SCENARIO-04] L4 Raw UDP Flooding & L3/L4 Boundary Protocol Routing Behavior 실증
- **공격 목표 (Attack Objective)**: 타깃 L3/L4 라우터의 비인가 프로토콜 처리 거동 분석 및 I/O 버퍼 고갈 (DoS)
- **OSI 계층 (Target Layer)**: Layer 4 (Transport Layer - UDP)
- **공격 메커니즘 (Mechanics)**:
  * L3 IPv4(20B) + UDP Header(8B) + 더미 페이로드("OCHLOS_UDP_BENCHMARK_DATA", 25B) = 53바이트 원시 패킷 패킹 (`IP_HDRINCL`, `IPPROTO_UDP`).
  * 출발지 IP 위조(`10.0.0.99`) 및 출발지 포트 순환(`10000 ~ 64999`)을 적용하여 RFC 768 Pseudo Header 기반 UDP 체크섬 계산 및 주입.
  * 웹 전용 포트(`screening-router:8080`)로 비인가 비연결형 UDP 패킷 대량 방출.
- **실행 도구 (Tool)**: [../DoS/raw_udp_flooding.cpp](../DoS/raw_udp_flooding.cpp) (C++20 Raw Socket / `IP_HDRINCL`)
- **실행 명령어 (On-demand Runner)**:
  ```bash
  docker compose run --rm ochlos cpprun DoS/raw_udp_flooding.cpp [패킷수]
  ```
- **테스트베드 실측 결과 (Empirical Results)**:
  * **공격 도구 성능 최적화**: 초당 약 45,000 ~ 85,000 PPS의 최대 라인 레이트 버스트 송출 역량 확보.
  * **규모별 실측 데이터**:
    * **10개 (기능/정렬 검증)**: 10/10 송출 (1.22 ms) → ScreeningRouter 10건 수신 (`Proto: 17, Size: 53 bytes`), 패킷 정렬 및 체크섬 정상 확인.
    * **1,000개 폭격 (22.00 ms, 45,454 PPS)**: 889개 수신 (유실률 11.10%).
    * **10,000개 폭격 (158.58 ms, 63,059 PPS)**: 4,942개 수신 (유실률 50.58%).
    * **100,000개 폭격 (1,173.55 ms, 85,211 PPS)**: 34,941개 수신 (유실률 65.06%).
  * **취약점 및 방어선 거동 규명**:
    * **I/O 병목 DoS 재현**: ICMP 폭격과 동일하게 단일 스레드 `std::cout` 콘솔 I/O 지연으로 인해 커널 소켓 수신 버퍼(`SO_RCVBUF`) 포화 및 최대 65% 패킷 유실 발생.
    * **무차별 L3 DNAT/SNAT 및 DMZ 주입**: 라우터가 프로토콜 필터링 없이 UDP 패킷을 DNAT/SNAT하여 DMZ(`ReverseProxy`)로 전달하나, ReverseProxy는 TCP 소켓만 청취하므로 비인가 트래픽이 DMZ 대역폭을 불필요하게 소모함.
    * **세션 테이블 안전성**: `session_table` 등록 조건이 `ip_header->protocol == IPPROTO_TCP`로 한정되어 있어 UDP 폭격 시에는 라우터 메모리 누수나 세션 테이블 팽창이 발생하지 않음 (메모리 2.66 MiB 안정 유지).
  * **호스트 안전성 검증**: 10만 개 폭격 시 호스트 OS 영향 0, 컨테이너 격리 상태 안정 유지.
- **블루팀 방어 권고사항 (Blue Team Feedback)**:
  * **L3/L4 Stateless Default-Deny 구현**: 웹 서비스에 불필요한 UDP(17) 및 비인가 ICMP(1) 프로토콜 패킷을 라우터 진입 즉시 Drop 처리하여 DMZ 유입 차단.
  * **비동기/샘플링 로깅 도입**: 대량 트래픽 인입 시 I/O 병목으로 인한 정상 패킷 Drop 방지.
