---
paths:
  - "docs/ATTACK_LOG.md"
---

# ATTACK_LOG.md 작성 규칙

새 실증을 기록할 때는 아래 템플릿을 채웁니다. 시나리오 ID는 `SCENARIO-01`부터 순차 증가시키고, 기존 번호는 재사용하지 않습니다.

```markdown
### [SCENARIO-XX] 시나리오 제목
- **공격 목표 (Attack Objective)**: 공격 대상 서비스 / 취약점 식별 영역 (예: L4 소켓 자원, L7 인증 엔드포인트 등)
- **OSI 계층 (Target Layer)**: Layer X (Network / Transport / Application 등)
- **공격 메커니즘 (Mechanics)**: 공격자가 악용하는 시스템/프로토콜/코드 결함 원리
- **실행 도구 (Tool)**: `../DoS/파일명.cpp`
- **실행 명령어 (On-demand Runner)**: `docker compose run --rm ochlos cpprun DoS/파일명.cpp [패킷수]`
- **테스트베드 실측 결과 (Empirical Results)**:
  - 공격 전/중/후 시스템 상태 (PPS, 유실률, 메모리, CPU, 소켓/세션 수, HTTP 상태코드 등)
  - 호스트 안전성 검증 결과
- **블루팀 방어 권고사항 (Blue Team Feedback)**:
  - 필요한 방어선 (방화벽 룰, 커널 튜닝, L7 Rate Limiter, 세션 회수 정책 등)
```

- 추측이 아닌 실측치만 적습니다. 규모별(1천/1만/10만 등) 수치와 측정 조건(Cold Start 여부, 반복 횟수)을 함께 남깁니다.
- 실측 후 공격 도구의 타깃이나 스푸핑 출발지를 바꾼 경우, 기존 계측치가 그대로 유효한지(재실증이 필요한지)를 해당 시나리오에 명시합니다.
- 소스 파일 링크는 이 문서가 `docs/` 안에 있으므로 `../DoS/...` 형태의 상대 경로를 씁니다.
