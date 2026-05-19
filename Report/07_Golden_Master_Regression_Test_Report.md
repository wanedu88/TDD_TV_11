# 07. Golden Master 회귀 테스트 구현 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 `TVChannelController`에 대해 **TextTestFixture 스타일 출력 기반 Golden Master(Approval) 회귀 테스트**를 설계·구현한 결과를 정리한다. 단위 테스트(`TVControllerTest.cpp`)가 개별 assertion으로 동작을 검증하는 반면, Golden Master는 **시나리오 전체의 관측 가능한 상태 트랜스크립트**를 승인된 텍스트 파일과 비교하여 회귀를 탐지한다.

| 구분 | 설명 |
|------|------|
| 배경 | Report/05에서 FR-01~06 단위 테스트 57건 Green 달성. 통합 시나리오·출력 회귀 검증 계층 부재 |
| 목표 | Approved 파일 보관 전략, GTest 파일 비교, CMake/ctest 통합, CI 자동 실행 |
| 범위 | `test/golden/`, `test/TVControllerGoldenTest.cpp`, `CMakeLists.txt`, `.github/workflows/ci.yml` |
| 제약 | `Tuner` / `TVController` / `remoteKey` 미변경. `TVChannelController` 로직 변경 없음(테스트 인프라만 추가) |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 단위 테스트 보고서 | `Report/05_TVController_Test_Implementation_Report.md` | 선행 57건 `TEST_F` |
| Golden 사용 가이드 | `test/golden/README.md` | 실행·갱신 절차 |
| 개발 요구사항 | `TDD_TV_Requirements.txt` | FR-01~06 |
| Controller 구현 | `include/TVChannelController.h` | 검증 대상 |

---

## 2. 테스트 실행 결과

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

| 항목 | Report/05 | 현재 |
|------|-----------|------|
| 전체 `ctest` | 57 | **71** |
| `TunerTest` | 13 | 13 |
| `TVControllerTest` (`TEST_F`) | 44 | 44 |
| `TVControllerGoldenTest` | — | **14** |
| 성공 | 57/57 | **71/71** |
| 상태 | Green | **Green** |

Golden Master만 실행:

```powershell
ctest --test-dir build -R TVControllerGolden --output-on-failure
```

---

## 3. 설계 개요

### 3.1 TextTestFixture 대응 개념

| TextTest 개념 | 본 프로젝트 구현 |
|---------------|------------------|
| Test application | `TVControllerGoldenTest` (GTest 실행 파일) |
| Approved output | `test/golden/approved/<scenario>.approved.txt` |
| Received (실패 시) | `test/golden/received/<scenario>.received.txt` (gitignore) |
| `-a` / 승인 갱신 | `TV_UPDATE_GOLDEN=1` 또는 `update_golden` 타겟 |
| Fixture 설명 | 트랜스크립트 헤더 `# fixture=... scenario=...` |

### 3.2 단위 테스트 vs Golden Master

| 계층 | 파일 | 검증 방식 | 적합한 용도 |
|------|------|-----------|-------------|
| 단위 | `test/TVControllerTest.cpp` | `EXPECT_EQ` / Mock `seekCH` | 함수·분기·경계값 |
| 회귀 | `test/TVControllerGoldenTest.cpp` | 전체 출력 문자열 비교 | README 시나리오·통합 흐름 |

두 계층은 **상호 보완**한다. Golden 실패 시 `received/`와 `approved/` diff로 동작 변경 범위를 빠르게 파악할 수 있다.

### 3.3 트랜스크립트 형식

`ControllerTrace`가 액션(`>`)과 스냅샷을 기록한다. `TVController`의 `std::cout` 로그는 사용하지 않는다(요구사항: 로그를 테스트 결과로 간주하지 않음).

```
# fixture=FakeTuner{1,4,12,56} scenario=fr01_press12_auto
> pressNumber(1)
CH=0
> pressNumber(2)
CH=12
```

| 라인 접두사 | 의미 |
|-------------|------|
| `#` | 픽스처·시나리오 메타 |
| `>` | Controller API 호출 |
| `CH=` | `Tuner::getCurrentCH()` |
| `favorites=` | `getFavoriteChannels()` |
| `searched=` | `getSearchedChannels()` |

---

## 4. Approved 파일 생성·보관 전략

### 4.1 디렉터리 구조

```
test/golden/
├── GoldenMaster.h          # 비교·갱신 유틸
├── ControllerTrace.h       # 트랜스크립트 빌더
├── README.md
├── approved/               # Golden (버전 관리 대상)
│   └── <scenario>.approved.txt
└── received/               # 실패 시만 생성 (gitignore)
    └── <scenario>.received.txt
```

### 4.2 명명 규칙

- GTest 이름 = 파일 접두사: `TEST(TVControllerGolden, fr01_press12_auto)` → `fr01_press12_auto.approved.txt`
- 확장자 `.approved.txt`는 TextTest 관례와 동일한 **승인본** 의미

### 4.3 생성·갱신 워크플로

1. **최초 생성**: `TV_UPDATE_GOLDEN=1`로 실행 → `approved/`에 파일 생성
2. **일반 CI/로컬**: `TV_UPDATE_GOLDEN` 미설정 → 기존 `approved`와 바이트 비교 (CRLF→LF 정규화)
3. **의도적 변경 후**: diff 검토 → `TV_UPDATE_GOLDEN=1` → `approved` 커밋

```powershell
$env:TV_UPDATE_GOLDEN = "1"
.\build\TVControllerGoldenTest.exe
# 또는
cmake --build build --target update_golden
# 또는
.\scripts\update_golden.ps1
```

### 4.4 환경 변수

| 변수 | 기본값 | 용도 |
|------|--------|------|
| `TV_UPDATE_GOLDEN` | `0` | `1`/`true`/`yes` 시 approved 덮어쓰기 |
| `TV_GOLDEN_APPROVED_DIR` | `test/golden/approved` | 승인본 루트 오버라이드 |
| `TV_GOLDEN_RECEIVED_DIR` | `test/golden/received` | 실패 산출물 경로 |

CMake 빌드 시 `GOLDEN_APPROVED_DIR` / `GOLDEN_RECEIVED_DIR`이 컴파일 정의로 주입된다.

---

## 5. 구현 상세

### 5.1 핵심 모듈

| 파일 | 역할 |
|------|------|
| `test/golden/GoldenMaster.h` | `readFile`, `normalizeNewlines`, `assertGolden`, `updateMode()` |
| `test/golden/ControllerTrace.h` | `pressNumber`, `pressChannelSearch` 등 → 텍스트 스냅샷 |
| `test/TVControllerGoldenTest.cpp` | FR별 시나리오 14건, `runScenario` / `runSearchListScenario` |

### 5.2 CMake / ctest 통합

`CMakeLists.txt` 변경 요약:

- `add_executable(TVControllerGoldenTest ...)`
- `gtest_discover_tests(TVControllerGoldenTest WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})`
- `add_custom_target(update_golden ...)` — 승인본 일괄 재생성

`.gitignore`에 `test/golden/received/` 추가 — 실패 아티팩트는 저장소에 포함하지 않음.

### 5.3 CI 구성

`.github/workflows/ci.yml`:

- 트리거: `push` / `pull_request` (`main`, `master`)
- 매트릭스: `ubuntu-latest`, `windows-latest`
- 단계: Configure → Build (Release) → `ctest --test-dir build -C Release --output-on-failure`

Golden 테스트는 별도 job 없이 **전체 ctest에 포함**되어 PR마다 회귀가 검증된다.

---

## 6. 시나리오 목록 (14건)

### 6.1 FR-01: 숫자 버튼 (6건)

| 시나리오 | 검증 요약 |
|----------|-----------|
| `fr01_press1_confirm` | `1` + 확인 → CH=1 |
| `fr01_press12_auto` | `1`,`2` → CH=12 |
| `fr01_press1234` | `1,2,3,4` → 12 후 34 |
| `fr01_three_digits_456_confirm` | `4,5,6` + 확인 → 6 |
| `fr01_three_digits_456_other` | `4,5,6` + Other → 45 유지 |
| `fr01_zero7` | `0`,`7` → CH=7 |

### 6.2 FR-02~03: 선호 채널 (3건)

| 시나리오 | 검증 요약 |
|----------|-----------|
| `fr02_favorite_toggle_scenario` | 12,8,37,8,6 토글 → favorites=[6,12,37] |
| `fr03_next_favorite_from6` | 선호 {1,4,12,56}, CH=6 → 12 |
| `fr03_next_favorite_wrap56` | CH=56 → wrap 1 |

### 6.3 FR-04~06: 검색·업/다운 (5건)

| 시나리오 | FakeTuner | 검증 요약 |
|----------|-----------|-----------|
| `fr04_channel_search` | {1,4,12,56} | searched 4채널 |
| `fr04_search_then_up_from6` | 동일 | 검색 후 CH=6 → Up → 12 |
| `fr05_up_down_no_search` | {1,4,12,56} | 6↔7, 99→0, 0→99 |
| `fr06_up_down_with_search` | {4,6,14} | README FR-06 업/다운·목록 외 15 |
| `fr06_up_down_cycle` | {4,6,14} | 6 → Up 14 → Down 6 |

---

## 7. 요구사항 추적

| FR | Golden 시나리오 수 | 단위 `TEST_F` (Report/05) | 비고 |
|----|-------------------|---------------------------|------|
| FR-01 | 6 | 8 | 대표 시나리오 위주 |
| FR-02 | 1 | 6 | 통합 토글 시나리오 |
| FR-03 | 2 | 6 | from6, wrap56 |
| FR-04 | 2 | 6 | Mock 검증은 단위만 |
| FR-05 | 1 (통합) | 10 | `fr05_up_down_no_search` |
| FR-06 | 2 | 8 | `{4,6,14}` 픽스처 |

Golden Master는 **대표 통합 시나리오**를 담당하고, 경계·예외·Mock 상호작용은 기존 단위 테스트가 담당한다.

---

## 8. DoD 및 품질 기준

| 항목 | 상태 | 비고 |
|------|------|------|
| Approved 보관 전략 문서화 | ✅ | 본 보고서 §4, `test/golden/README.md` |
| GTest 파일 비교 구현 | ✅ | `GoldenMaster.h` |
| ctest 통합 | ✅ | 71 tests discovered |
| CI 자동 실행 | ✅ | `.github/workflows/ci.yml` |
| `ctest` Green | ✅ | 71/71 |
| Production 코드 무변경 | ✅ | Controller 헤더 변경 없음 |

---

## 9. 잔여 갭 및 권장 후속 작업

| ID | 내용 | 우선순위 |
|----|------|----------|
| G1 | Report/05 P1 예외·경계 시나리오 Golden 추가 (`pressNumber` invalid 등) | P2 |
| G2 | CI에 `update_golden` 실수 방지 — `TV_UPDATE_GOLDEN` 미설정 고정 | ✅ (기본) |
| G3 | Golden diff를 PR 코멘트로 노출 (선택, `diff` 아티팩트 업로드) | P3 |
| G4 | Line coverage 90% (Report/05 항목 4) | P1 |

---

## 10. 결론

1. **회귀 계층 추가**: TextTest 스타일 Golden Master 14건으로 README·FR 대표 시나리오의 **출력 기반 회귀 검증**을 도입하였다.
2. **운영 절차 확립**: `approved/` 커밋, `received/` gitignore, `TV_UPDATE_GOLDEN` 갱신, `update_golden` 타겟·스크립트로 승인본 lifecycle을 정의하였다.
3. **빌드·CI 통합**: `TVControllerGoldenTest`가 `ctest`에 자동 등록되며, Ubuntu/Windows CI에서 단위 테스트와 함께 실행된다.
4. **실행 결과**: `ctest` **71/71 Passed** (기존 57 + Golden 14).
5. **다음 단계**: GitHub push 후 CI Green 확인, 필요 시 G1 추가 시나리오·gcov 90% 측정.

---

## 11. 참고 문서

| 문서 | 설명 |
|------|------|
| `test/TVControllerGoldenTest.cpp` | Golden 시나리오 정의 |
| `test/golden/GoldenMaster.h` | 비교·갱신 로직 |
| `test/golden/ControllerTrace.h` | 트랜스크립트 빌더 |
| `test/golden/approved/*.approved.txt` | 승인된 Golden 파일 (14개) |
| `test/golden/README.md` | 실행·갱신 가이드 |
| `scripts/update_golden.ps1` / `update_golden.sh` | 승인본 재생성 스크립트 |
| `CMakeLists.txt` | 타겟·`update_golden` 정의 |
| `.github/workflows/ci.yml` | CI 워크플로 |
| `Report/05_TVController_Test_Implementation_Report.md` | 선행 단위 테스트 보고서 |
