# GEMINI.md - Ochlos (레드팀 공격 시뮬레이션 환경) AI 운영 지침 & 실증 로그

이 문서는 **Ochlos (오클로스)** 프로젝트의 아키텍처, 개발 원칙, AI 페어 프로그래밍 규칙, 그리고 공격 실증 이력을 통합 관리하는 **단일 기준 문서(Single Source of Truth)**입니다.

---

## 🎯 1. 프로젝트 정체성 및 목적 (Identity & Mission)

* **Ochlos**는 백엔드 웹 애플리케이션 및 보안 시스템([Bartimaeus](https://github.com/Bartimaeus-han/Bartimaeus_app))의 취약점을 선행 격파하고 방어선(Hardening)의 유효성을 실증하기 위한 **공격자 C2 / 레드팀 시뮬레이션 환경**입니다.
* 정보보안기사 국가기술자격 및 KISA 주요정보통신기반시설 기술적 취약점 분석·평가 기준을 바탕으로 **OSI 7계층(L3 Network ~ L7 Application)** 공격 도구를 C++20 및 제어 스크립트로 직접 구현하고 계측합니다.

---

## 🛡️ 2. 레드팀 핵심 운영 원칙 (Red Team Core Principles)

1. **공격 선행 학습 (Offensive-First)**:
   * 방어 코드를 작성하기 전, 공격자 관점에서 취약점 메커니즘을 규명하고 테스트베드를 타격하는 실증 코드를 먼저 작성합니다.
2. **OSI 7계층 명시 (Layer Mapping)**:
   * 모든 공격 시나리오는 대상이 되는 계층(L3 IP, L4 TCP, L7 HTTP/Auth 등)을 명확히 정의하고 블루팀 방어선과 대조합니다.
3. **실증적 계측 (Empirical Diagnostics)**:
   * 정성적 추측이 아닌 시스템 리소스(RSS 메모리, CPU 점유율, 소켓 고갈 상태, 응답 지연시간)의 실측치로 방어선의 붕괴 또는 방어 성공 여부를 증명합니다.
4. **블루팀 피드백 연계 (Feedback to Bartimaeus)**:
   * 공격 실증 후 도출된 취약점 분석 결과와 권고 대책을 본 문서의 실증 로그에 기록하고 블루팀(`Bartimaeus`) 방어 아키텍처(ScreeningRouter, RateLimiter 등)에 환류합니다.
5. **호스트 머신 가용성 절대 보장 및 컨테이너 격리 (Host Availability & Isolation)**:
   * 실증 과정에서 **타깃 Docker 컨테이너가 다운되는 것은 정상적인 실증 결과**이나, **개발 머신(Windows / macOS 등 호스트 OS)의 소켓 고갈, 네트워크 마비, 시스템 불안정이 발생하는 것은 엄격히 금지**합니다.
   * 모든 공격 도구는 제어된 단발/배치 타격과 Linux Docker 컨테이너 격리(`CAP_NET_RAW`)를 통해 호스트의 일상 작업을 방해하지 않도록 안전하게 운용합니다. 특히 macOS 호스트의 경우 Docker Desktop Linux VM(가상 머신)을 경유하므로, VM 브리지 네트워크 포화 및 호스트 프로세스 영향도를 상시 점검합니다.
6. **당분간 레드팀(Offensive) 시뮬레이션 전념 (Red-Team Exclusive Focus)**:
   * 당분간 블루팀 방어선 빌드나 타 방어 프로젝트 작업을 겸하지 않고, Ochlos 내의 **레드팀 공격 도구 개발, L3~L7 공격 시나리오 고도화, 그리고 Docker 기반 수제 패킷 주입 시뮬레이션에만 전적으로 집중**합니다.


---

## 🤖 3. AI 페어 프로그래밍 상호작용 규칙 (Interaction Rules)

1. **[최우선 원칙] 파트너십 페어 프로그래밍 (AI 독단적 진행 금지)**:
   * AI가 코드를 한 번에 다 완성해서 쏟아내지 않고, 학습자가 충분히 타이핑하고 이해할 시간을 갖도록 호흡을 맞추어 천천히 차분하게 진행합니다.
2. **단계별 점진적 진행 (Step-by-step)**:
   * 복잡한 네트워크 프로토콜(L3/L4/L7 패킷 구조, 체크섬, 소켓 옵션 등)을 다룰 때는 한 번에 큰 코드를 쏟아내지 않고 원리와 구조를 단계별로 검증하며 진행합니다.
3. **코드 가이드 제시 형식 (Git Diff 1회 방식)**:
   * 가독성과 변경 지점의 직관적 파악을 위해 삭제(`-`)와 추가(`+`)가 색상으로 명확히 구분되는 **Git Diff 스타일(1회 제공)** 포맷으로 제시합니다.
4. **선제적 크로스 플랫폼/컴파일러 세부사항 완결 제시**:
   * 코드 제시 시 컴파일러별 특유의 매크로(예: `_CRT_SECURE_NO_WARNINGS`), 필수 선행 헤더(예: `<netdb.h>`, `<cstdlib>`), 플랫폼별 의존성 등을 사용자가 묻기 전에 선제적으로 완결된 형태로 제시합니다.
5. **원인과 메커니즘 중심 설명**:
   * "Windows에서 왜 안 되는가?", "체크섬은 왜 필요한가?"와 같이 OS 커널과 네트워크 표준(RFC) 수준의 인과 관계를 명확히 설명합니다.
6. **간결하고 명확한 톤 유지**:
   * 군더더기 없는 정중하고 명확한 한국어(존댓말)로 커뮤니케이션합니다.
7. **LaTeX 표기 일체 금지 및 순수 유니코드 기호 강제**:
   * `$\rightarrow$`, `\rightarrow` 등의 LaTeX 수식 문법을 절대 출력하지 않으며, 화살표 및 특수 표기는 반드시 순수 유니코드 문자(`→`, `↔`, `•`)만을 사용합니다.
8. **반복 에러 처리 분기 작성 유연화 및 최종 일괄 완성 (Fast Prototyping & AI Polish)**:
   * AI는 가이드 제시 시 항상 100% 완전한 크로스 플랫폼 에러 처리/자원 해제 코드를 제공합니다.
   * 학습자는 핵심 로직 학습에 집중하기 위해 `// error message & clean sock...` 등 약식 주석으로 빠르게 작성하고 넘어갈 수 있습니다.
   * 파일의 핵심 로직 작성이 완료되면, AI가 약식 주석으로 남겨진 에러 분기들을 온전한 크로스 플랫폼 코드로 일괄 교체 및 완성합니다.


---

## 🏗️ 4. 기술 스택 및 디렉터리 구조 (Tech Stack & Layout)

* **언어 및 표준**: C++20 (Modern C++)
* **빌드 시스템**: CMake (3.15 이상), MSVC / Apple Clang (macOS) / GCC (Linux)
* **네트워크 인터페이스**:
  * **호스트 OS (Windows / macOS)**:
    * **Windows 호스트**: Winsock2 (`ws2_32.lib`) - 일반 소켓(`SOCK_STREAM`, `SOCK_DGRAM`) 연결 테스트
    * **macOS 호스트**: BSD/POSIX Sockets (`<sys/socket.h>`, `<arpa/inet.h>`) 및 Docker Desktop Linux VM 경유 네트워킹
  * **Linux 컨테이너**: POSIX Sockets, `CAP_NET_RAW` - 수제 패킷 주입(`SOCK_RAW`), SYN Flooding, L3/L4 조작
* **코드 포맷팅 & 린팅**: LLVM 스타일 (4-space indent), Clangd C++20 LSP 지원

```text
Ochlos/
├── .clang-format         # LLVM 기반 4-space C++ 포맷 규칙
├── .clangd               # Clangd C++20 인텔리센스 설정
├── .gitignore            # 컴파일 캐시 및 바이너리 제외
├── CMakeLists.txt        # 하위 소스 파일 자동 탐색 및 include/ 경로 주입
├── Dockerfile            # Linux Attacker 컨테이너 환경 (CAP_NET_RAW 지원)
├── docker-compose.yml    # Attacker On-demand 실행 및 네트워크 바인딩
├── GEMINI.md             # [현재 파일] Ochlos 프로젝트 AI 운영 지침 & 실증 로그 통합본
├── include/              # 공통 네트워크 헤더 및 유틸리티
│   └── ochlos_net.hpp    # L3 IP / L4 TCP 헤더 및 체크섬 계산 라이브러리
└── DoS/                  # 서비스 거부 공격(DoS/DDoS) 실증 도구
    ├── icmp_echo_flooding.cpp    # L3 ICMP Echo Flooding (Ping Flooding)
    ├── raw_tcp_syn_flooding.cpp  # L3/L4 Raw Socket Half-Open SYN Flooding
    ├── raw_udp_flooding.cpp      # L4 Raw Socket UDP Flooding
    └── tcp_syn_flooding.cpp      # L4 Connection Starvation
```

### 4.1 크로스 플랫폼 패킷 조작 및 바이너리 정렬 원칙 (Cross-Platform Packet Crafting)

* **소켓/시스템 헤더 분기 (`#ifdef _WIN32` vs POSIX)**:
  * Windows (`<WinSock2.h>`, `<ws2tcpip.h>`)와 Linux/POSIX (`<arpa/inet.h>`, `<netinet/in.h>`, `<unistd.h>`)를 명확히 분기하여 MSVC, GCC, Clang에서 경고/오류 없이 빌드 가능하도록 설계.
* **구조체 1바이트 패킹 (`#pragma pack(push, 1)`)**:
  * 컴파일러별 자동 메모리 패딩(Structure Padding)을 방지하여 RFC 표준 IPv4(20B) + TCP(20B) = **정확히 40바이트의 바이너리 레이아웃**을 강제 보장.
* **리틀 엔디안(Little-Endian) 비트필드(Bit-field) 배치**:
  * 현대 주요 아키텍처(x86_64, ARM64)의 메모리 저장 방식에 맞추어 IPv4 첫 바이트(`ihl: 4` 하위 비트, `version: 4` 상위 비트)를 LSB-first 순서로 배치.
  * TCP 플래그는 `uint8_t flags` 공용체(union)를 통해 1바이트 통째로 대입하여 비트필드 해석 오차를 원천 차단.
* **엔디안 변환 일관성 및 2단계 체크섬 계산 (RFC 791 / RFC 793)**:
  * 포트(`src_port`, `dest`) 및 시퀀스 번호 등 호스트 값은 `htons()`, `htonl()`을 통해 Network Byte Order(Big Endian)로 변환.
  * IP 체크섬뿐만 아니라 12바이트 의조 헤더(Pseudo Header)를 구성하여 TCP 체크섬을 정확히 계산하는 `craft_tcp_packet()` 함수로 캡슐화.


---

## 📌 5. 공격 시나리오 템플릿 (Scenario Template)

```markdown
### [SCENARIO-XX] 시나리오 제목
- **공격 목표 (Attack Objective)**: 공격 대상 서비스 / 취약점 식별 영역 (예: L4 소켓 자원, L7 인증 엔드포인트 등)
- **OSI 계층 (Target Layer)**: Layer X (Network / Transport / Application 등)
- **공격 메커니즘 (Mechanics)**: 공격자가 악용하는 시스템/프로토콜/코드 결함 원리
- **실행 도구 (Tool)**: `파일명.cpp`
- **테스트베드 실측 결과 (Empirical Results)**:
  - 공격 전/중/후 시스템 상태 (메모리, CPU, 소켓 수, HTTP 상태코드 등)
- **블루팀(Bartimaeus) 방어 권고사항 (Blue Team Feedback)**:
  - 필요한 방어선 (방화벽 룰, 커널 튜닝, L7 Rate Limiter, 세션 회수 정책 등)
```

---

## 📌 6. 공격 실증 이력 (Attack Simulation Log)

### [SCENARIO-01] L4 TCP Connection Starvation & Screening Router 실시간 패킷 탐지
- **공격 목표 (Attack Objective)**: 백엔드 웹 서버의 TCP 수신 대기열 및 워커 스레드 자원 점유 (DoS)
- **OSI 계층 (Target Layer)**: Layer 4 (Transport Layer)
- **공격 메커니즘 (Mechanics)**: 3-Way Handshake를 완료한 후 `closesocket()`(RST/FIN)을 보내지 않고 소켓을 배열에 보관한 채 대기하여 서버의 연결 풀을 고갈시킴.
- **실행 도구 (Tool)**: [DoS/tcp_syn_flooding.cpp](DoS/tcp_syn_flooding.cpp) (C++20 Winsock/POSIX)
- **테스트베드 실측 결과 (Empirical Results)**:
  * `ScreeningRouter.exe` (Port 8080) 기동 후 Ochlos 50개 커넥션 인입 시 실시간 L3 IP (`127.0.0.1`) 및 L4 출발지 Port(`5xxxx`) 감지 로그 50건 연속 출력 확인.
- **블루팀(Bartimaeus) 방어 권고사항 (Blue Team Feedback)**:
  * L3/L4 경계선에서 패킷 가시성을 확보하였으므로, 다음 단계로 검증된 트래픽을 내부 백엔드(`bartimaeus-app:9090`)로 포워딩(중계)하는 파이프라인 및 Stateless IP/Port 룰 필터링 구현 필요.

### [SCENARIO-02] L3 ICMP Echo Flooding (Ping Flooding) & IP Spoofing 실증
- **공격 목표 (Attack Objective)**: 타깃 네트워크 스택의 ICMP 처리 인터럽트(SoftIRQ) 및 대역폭/응답 버퍼 고갈 (DoS)
- **OSI 계층 (Target Layer)**: Layer 3 (Network Layer)
- **공격 메커니즘 (Mechanics)**: L3 IPv4(20B) + ICMP Echo Request(8B)의 28바이트 원시 패킷을 직접 패킹하고, 출발지 IP를 임의 주소(`100.0.0.99`)로 위조(L3 IP Spoofing)하여 무차별 방출.
- **실행 도구 (Tool)**: [DoS/icmp_echo_flooding.cpp](DoS/icmp_echo_flooding.cpp) (C++20 Raw Socket / `IP_HDRINCL`)
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
- **블루팀(Bartimaeus) 방어 권고사항 (Blue Team Feedback)**:
  * **성능 최적화**: 대량 트래픽 인입 시 I/O 병목 방지를 위해 실시간 패킷 콘솔 로깅을 비동기화하거나 N개(예: 1,000개) 단위 샘플링 출력으로 전환 필요.
  * **경계선 차단**: `ScreeningRouter`에 미사용 ICMP Type 8 패킷을 즉각 폐기하는 L3 Stateless Drop 룰셋 구현 필요.

### [SCENARIO-03] L4 Raw TCP SYN Flooding & NAPT Session State Exhaustion 실증
- **공격 목표 (Attack Objective)**: 스크리닝 라우터(ScreeningRouter)의 NAPT 세션 테이블 메모리 고갈 및 미완료 Half-Open 세션 누적 (State Exhaustion DoS)
- **OSI 계층 (Target Layer)**: Layer 4 (Transport Layer)
- **공격 메커니즘 (Mechanics)**:
  * L3 IPv4(20B) + TCP SYN(20B) = 40바이트 원시 패킷을 직접 패킹(`IP_HDRINCL`, `FLAG_SYN`).
  * 출발지 포트를 고정하지 않고 매 루프마다 난수/순환(`10000 ~ 64999`)으로 변조하여, 라우터가 매번 독립된 신규 세션으로 인식하도록 유도.
  * 3-Way Handshake(SYN-ACK / ACK)를 완료하지 않는 순수 Half-Open 형태로 무차별 송출하여 NAPT 세션 테이블의 무한 증식을 촉발.
- **실행 도구 (Tool)**: [DoS/raw_tcp_syn_flooding.cpp](DoS/raw_tcp_syn_flooding.cpp) (C++20 Raw Socket / `IP_HDRINCL`)
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
- **블루팀(Bartimaeus) 방어 권고사항 (Blue Team Feedback)**:
  * **복합 키(5-Tuple) 도입**: `(src_ip, src_port, dst_ip, dst_port, proto)` 조합으로 세션 키를 구성하여 포트 충돌 방지.
  * **Half-Open 세션 타임아웃 & 가비지 컬렉션(GC)**: SYN만 도착하고 ACK가 없는 미완료 세션은 3~5초 이내에 강제 회수하는 타이머 도입.
  * **최대 용량 제한(Capacity Cap)**: 세션 테이블 상한선 설정 및 LRU 기반 퇴출 정책 수립.

---

## 📌 7. Ochlos 인프라 및 공격 도구 개선 TODO (Backlog)

- [x] **Ochlos 공격 도구의 Docker 컨테이너화 및 On-demand Runner 구성 (Infra-Attacker)**
  - **배경 및 목적**: 호스트 OS(Windows / macOS)의 Raw Socket 보안 제약(`SOCK_RAW`, IP Spoofing 차단 및 커널 차이)을 극복하고, 크로스 플랫폼 일관성 및 L3/L4 저수준 패킷 조작 환경 확보.
  - **개발 워크플로우**:
    - **코드 편집 & Git 관리**: 로컬 호스트(VS Code)에서 평소처럼 편집 및 GitHub 커밋 유지.
    - **실시간 바인드 마운트**: `docker-compose.yml`에 `ochlos` 서비스 정의 (`volumes: - .:/app`, `cap_add: - NET_RAW`).
    - **On-demand 실행**: `docker compose run --rm ochlos ...`으로 필요할 때만 컨테이너를 띄워 컴파일 & 타격 후 자동 소멸(`--rm`).
  - **기대 효과**: Linux 커널의 `CAP_NET_RAW` 권한을 활용해 순수 TCP SYN Flooding(Half-Open, 비정상 패킷 주입) 등 고도화된 DoS 공격을 제약 없이 실증 가능.

- [ ] **L4 UDP Flooding 공격 도구 구현 및 3대 기초 Flooding 완성 (Attack-UDP)**
  - **배경 및 목적**: L3 ICMP Flooding(제어용), L4 TCP SYN Flooding(연결형)에 이어 L4 비연결형 프로토콜인 UDP Flooding 도구를 확보하여 네트워크 3대 기초 DoS 공격 라인업(TCP, ICMP, UDP)을 완전체로 구축.
  - **기술 구현 로드맵**:
    - `include/ochlos_net.hpp`에 RFC 768 표준 8바이트 `UDPHeader` 구조체 및 `craft_udp_packet()` 패킷 패킹 유틸리티 추가.
    - `DoS/udp_flooding.cpp` 작성: 수제 Raw Socket 기반 비연결형 패킷 생성, 무작위 목적지/출발지 포트 변조, 동적 패킷 수 CLI 인자 지원, 고정밀 `steady_clock` 벤치마크 루프 적용.
    - 블루팀 스크리닝 라우터(웹 전용 HTTP 8080) 대상 UDP 무차별 패킷 주입 및 비인가 프로토콜 처리 거동(무차별 포워딩 vs Port Unreachable 유발 여부) 실측.
