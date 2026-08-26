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
   * 실증 과정에서 **타깃 Docker 컨테이너가 다운되는 것은 정상적인 실증 결과**이나, **개발 머신(Windows 호스트 OS)의 소켓 고갈, 네트워크 마비, 시스템 불안정이 발생하는 것은 엄격히 금지**합니다.
   * 모든 공격 도구는 제어된 단발/배치 타격과 Linux Docker 컨테이너 격리(`CAP_NET_RAW`)를 통해 호스트의 일상 작업을 방해하지 않도록 안전하게 운용합니다.

---

## 🤖 3. AI 페어 프로그래밍 상호작용 규칙 (Interaction Rules)

1. **[최우선 원칙] 파트너십 페어 프로그래밍 (AI 독단적 진행 금지)**:
   * AI가 코드를 한 번에 다 완성해서 쏟아내지 않고, 학습자가 충분히 타이핑하고 이해할 시간을 갖도록 호흡을 맞추어 천천히 차분하게 진행합니다.
2. **단계별 점진적 진행 (Step-by-step)**:
   * 복잡한 네트워크 프로토콜(L3/L4/L7 패킷 구조, 체크섬, 소켓 옵션 등)을 다룰 때는 한 번에 큰 코드를 쏟아내지 않고 원리와 구조를 단계별로 검증하며 진행합니다.
3. **원인과 메커니즘 중심 설명**:
   * "Windows에서 왜 안 되는가?", "체크섬은 왜 필요한가?"와 같이 OS 커널과 네트워크 표준(RFC) 수준의 인과 관계를 명확히 설명합니다.
4. **간결하고 명확한 톤 유지**:
   * 군더더기 없는 정중하고 명확한 한국어(존댓말)로 커뮤니케이션합니다. LaTeX 수식 대신 유니코드 기호(`→`, `↔`)를 사용합니다.

---

## 🏗️ 4. 기술 스택 및 디렉터리 구조 (Tech Stack & Layout)

* **언어 및 표준**: C++20 (Modern C++)
* **빌드 시스템**: CMake (3.15 이상), MSVC 2026 / GCC (Linux)
* **네트워크 인터페이스**:
  * **Windows 호스트**: Winsock2 (`ws2_32.lib`) - 일반 소켓(`SOCK_STREAM`, `SOCK_DGRAM`) 연결 테스트
  * **Linux 컨테이너**: POSIX Sockets, `CAP_NET_RAW` - 수제 패킷 주입(`SOCK_RAW`), SYN Flooding, L3/L4 조작
* **코드 포맷팅 & 린팅**: LLVM 스타일 (4-space indent), Clangd C++20 LSP 지원

```text
Ochlos/
├── .clang-format         # LLVM 기반 4-space C++ 포맷 규칙
├── .clangd               # Clangd C++20 인텔리센스 설정
├── .gitignore            # 컴파일 캐시 및 바이너리 제외
├── CMakeLists.txt        # 하위 소스 파일 자동 탐색 및 include/ 경로 주입
├── GEMINI.md             # [현재 파일] Ochlos 프로젝트 AI 운영 지침 & 실증 로그 통합본
├── include/              # 공통 네트워크 헤더 및 유틸리티
│   └── ochlos_net.hpp    # L3 IP / L4 TCP 헤더 및 체크섬 계산 라이브러리
└── DoS/                  # 서비스 거부 공격(DoS/DDoS) 실증 도구
    ├── raw_tcp_syn_flooding.cpp  # L3/L4 Raw Socket Half-Open SYN Flooding
    └── tcp_syn_flooding.cpp      # L4 Connection Starvation
```

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

---

## 📌 7. Ochlos 인프라 및 공격 도구 개선 TODO (Backlog)

- [ ] **Ochlos 공격 도구의 Docker 컨테이너화 및 On-demand Runner 구성 (Infra-Attacker)**
  - **배경 및 목적**: Windows 호스트 OS의 Raw Socket 보안 제약(`SOCK_RAW`, IP Spoofing 차단)을 극복하고, 크로스 플랫폼 일관성 및 L3/L4 저수준 패킷 조작 환경 확보.
  - **개발 워크플로우**:
    - **코드 편집 & Git 관리**: 로컬 호스트(VS Code)에서 평소처럼 편집 및 GitHub 커밋 유지.
    - **실시간 바인드 마운트**: `docker-compose.yml`에 `ochlos` 서비스 정의 (`volumes: - .:/app`, `cap_add: - NET_RAW`).
    - **On-demand 실행**: `docker compose run --rm ochlos ...`으로 필요할 때만 컨테이너를 띄워 컴파일 & 타격 후 자동 소멸(`--rm`).
  - **기대 효과**: Linux 커널의 `CAP_NET_RAW` 권한을 활용해 순수 TCP SYN Flooding(Half-Open, 비정상 패킷 주입) 등 고도화된 DoS 공격을 제약 없이 실증 가능.
