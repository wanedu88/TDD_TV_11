# TV Channel Controller Test Plan — Conversation Transcript
_Exported on 5/19/2026 from Cursor (current session)_

---

**User**

@include @test/TVControllerTest.cpp @TDD_TV_Requirements.txt 

[P] 시니어 QA 리드입니다.
[C] C++17, Google Test, CMake, (가능하면 gcov/lcov)
[T] 테스트 계획서를 작성해줘.
    - TEST_F 기반 단위 테스트 범위/우선순위
    - 경계값 케이스 목록 (세 자리 입력)
    - 예외/특이 케이스 목록
    - 커버리지 목표 (예: 90%+)와 gcov/lcov 측정·개선 전략
[F] Markdown 문서. docs/test_plan.md로 저장

---

**Cursor**

`TDD_TV_Requirements.txt`, `test/TVControllerTest.cpp`, `include/TVChannelController.h`, `CMakeLists.txt`, `include/fakeTuner.h`, `README.md` 등을 분석한 뒤 테스트 계획서를 작성했습니다.

### 문서 구성 (`docs/test_plan.md`)

1. **목적 및 범위** — In/Out of Scope, `TVChannelController` vs 레거시 제외
2. **테스트 픽스처** — `ControllerTest`, `FakeTuner` / `MockTunerForController` 역할, Given-When-Then, TDD 순서
3. **TEST_F 범위·우선순위** — FR-01~06별 P0/P1/P2, 현재 11건 vs 목표 27건+ 매트릭스
4. **경계값 (세 자리 입력)** — B1~B8, 두 자리 자동 적용 D1~D5, 채널 0~99 R1~R2
5. **예외·특이** — E1~E3 (`EXPECT_THROW`), S1~S9, Mock/Fake 선택 가이드
6. **요구사항 추적 매트릭스** — FR별 TEST_F 현재/목표
7. **커버리지** — Line ≥90%, Branch ≥85%, `ENABLE_COVERAGE` CMake 옵션, lcov/gcovr 절차, 5단계 개선 전략
8. **완료 정의 (DoD)** — FR-01~06, Green, 90% 커버리지
9. **부록** — 현재 11개 `TEST_F` 스냅샷 (FR-04~06 0건)

### 분석 시 반영한 현황

- `ctest`: **24/24 Passed** (Green)
- `TVControllerTest` `ControllerTest` 픽스처: **11** `TEST_F` (FR-01: 5, FR-02: 3, FR-03: 3)
- **미구현 테스트:** FR-01-04b(세 자리+확인→6번), FR-04~06 전부, 예외 E1~E2 등
- `MockTunerForController`는 선언만 있고 FR-04에서 활성화 예정

파일 경로: `c:\DEV\TDD_TV_11\docs\test_plan.md`

---

**User**

Report 디렉토리에 보고서를 작성

---

**Cursor**

기존 `Report/01~03` 보고서 형식을 확인한 뒤, `docs/test_plan.md`를 요약·정리한 보고서를 작성했습니다. `ctest`를 재실행해 24/24 Green을 반영했습니다.

### 보고서 개요

| 항목 | 내용 |
|------|------|
| **파일** | `Report/04_Test_Plan_Report.md` |
| **제목** | 04. TV Channel Controller 단위 테스트 계획 보고서 |
| **작성일** | 2026-05-19 |
| **상세본** | `docs/test_plan.md` |

### 주요 섹션

1. **목적** — QA 리드 관점 테스트 계획, 산출물 링크 (01~03 선행 보고서)
2. **테스트 실행 현황** — 24/24 Green, Controller `TEST_F` 11건
3. **TEST_F 범위·우선순위** — FR별 현재/목표, 실행 순서, 현재 TEST_F 목록 표
4. **경계값 (세 자리)** — B1~B7, README vs 구현, `.cursorrules` 명세 충돌 주의
5. **예외·특이** — E1~E3, S1~S8
6. **커버리지** — 90%+ 목표, gcov/lcov 5단계, 환경 권장
7. **요구사항 추적** — 합계 11 → ≥27, FR-04~06 갭 +16
8. **권장 TDD 작업 순서** — B2 Red → FR-04~06 → 커버리지 CI
9. **완료 정의** — 현재 항목 2(ctest Green)만 충족
10. **결론·참고 문서**

경로: `c:\DEV\TDD_TV_11\Report\04_Test_Plan_Report.md`

---

**User**

Prompting 디렉토리에 현재 대화를 추출해줘

---

**Cursor**

`Prompting/04_Test_Plan_prompt.md`에 현재 세션 대화 transcript를 export했습니다.

---

## 세션 산출물 요약

| 순서 | 사용자 요청 | 생성·갱신 파일 |
|------|-------------|----------------|
| 1 | 테스트 계획서 작성 ([P][C][T][F]) | `docs/test_plan.md` |
| 2 | Report 보고서 | `Report/04_Test_Plan_Report.md` |
| 3 | Prompting transcript export | `Prompting/04_Test_Plan_prompt.md` (본 파일) |

## [P][C][T][F] 프롬프트 템플릿 (재사용)

아래 형식은 본 세션에서 사용된 구조화 프롬프트이다.

```text
@include @test/TVControllerTest.cpp @TDD_TV_Requirements.txt

[P] 시니어 QA 리드입니다.
[C] C++17, Google Test, CMake, (가능하면 gcov/lcov)
[T] 테스트 계획서를 작성해줘.
    - TEST_F 기반 단위 테스트 범위/우선순위
    - 경계값 케이스 목록 (세 자리 입력)
    - 예외/특이 케이스 목록
    - 커버리지 목표 (예: 90%+)와 gcov/lcov 측정·개선 전략
[F] Markdown 문서. docs/test_plan.md로 저장
```

### 후속 프롬프트 (동일 세션)

```text
Report 디렉토리에 보고서를 작성
```

```text
Prompting 디렉토리에 현재 대화를 추출해줘
```

## 핵심 분석 결과 (재현용 메모)

| FR | Controller TEST_F | 상태 |
|----|-------------------|------|
| FR-01 | 5 / 목표 10 | 세 자리+확인(B2) 등 미작성 |
| FR-02 | 3 / 4 | 기본 토글 ✅ |
| FR-03 | 3 / 5 | 기본·wrap·빈 목록 ✅ |
| FR-04~06 | 0 / 8+ | API·테스트 미착수 |

**우선순위 실행 순서:** FR-01 보강 → FR-04(Mock `seekCH`) → FR-05 → FR-06 → P1 경계·예외 → 커버리지 90%+

**세 자리 FR-01-04 (README):** `4,5,6` → `45` 적용 → `확인`+`6` → 6번 / `pressOther` → `45` 유지 (`OtherButtonCancelsBuffer` ✅)

**명세 주의:** `.cursorrules` 3자리 이상 → 마지막 두 자리 규칙은 README FR-01-04와 상이 → `ThreeDigits_ThenConfirm_SingleDigit` characterization test로 먼저 고정 권장 (`Report/02` §4.1)

## 관련 보고서·문서

| 문서 | 경로 |
|------|------|
| 테스트 계획서 (상세) | `docs/test_plan.md` |
| 테스트 계획 보고서 | `Report/04_Test_Plan_Report.md` |
| 개발 요구사항 | `TDD_TV_Requirements.txt` |
| 단위 테스트 | `test/TVControllerTest.cpp` |
| 선행 QA 보고서 | `Report/01_TVController_QA_Refactoring_Report.md` |
| 요구사항 분석 | `Report/02_Requirements_Analysis_Report.md` |
| 코드 품질 | `Report/03_TVChannelController_Code_Quality_Report.md` |
| 이전 transcript | `Prompting/01~03_*_prompt.md` |
