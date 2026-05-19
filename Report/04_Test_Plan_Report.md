# 04. TV Channel Controller 단위 테스트 계획 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 시니어 QA 리드 관점에서 `TVChannelController` 단위 테스트의 **범위·우선순위·경계값·예외·커버리지 목표**를 정리한 결과이다. 상세 시나리오·CMake/lcov 절차는 별도 산출물 `docs/test_plan.md`에 수록한다.

| 구분 | 설명 |
|------|------|
| 배경 | FR-01~03은 기본 `TEST_F`가 있으나 FR-04~06·세 자리 확인 시나리오 등 갭 존재 |
| 목표 | TDD_TV 요구사항 전체에 대한 `TEST_F` 체계·우선순위·90%+ 커버리지 로드맵 확립 |
| 범위 | `test/TVControllerTest.cpp`, `include/TVChannelController.h` (레거시 `Tuner`/`TVController`/`remoteKey` 제외) |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 테스트 계획서 (상세) | `docs/test_plan.md` | TEST_F 매트릭스, 경계값 B1~B8, gcov/lcov 절차 |
| 개발 요구사항 | `TDD_TV_Requirements.txt` | FR-01~06, Given-When-Then 규칙 |
| 선행 QA 보고서 | `Report/01_TVController_QA_Refactoring_Report.md` | 구현·테스트 현황 |
| 요구사항 분석 | `Report/02_Requirements_Analysis_Report.md` | FR별 갭·명세 충돌 |
| 코드 품질 | `Report/03_TVChannelController_Code_Quality_Report.md` | 리팩토링 우선순위 |

---

## 2. 테스트 실행 현황 (2026-05-19)

```powershell
ctest --test-dir "build" --output-on-failure
```

| 항목 | 값 |
|------|-----|
| 전체 테스트 | 24 |
| 성공 | 24 |
| 실패 | 0 |
| 상태 | **Green** |
| `TVControllerTest` (`ControllerTest` 픽스처) | **11** `TEST_F` |
| `TunerTest` | 13 (Mock 예제 포함) |

현재 Green이나, **요구사항 FR-04~06에 대응하는 Controller 테스트는 0건**이다.

---

## 3. TEST_F 범위 및 우선순위 요약

### 3.1 픽스처 전략

- **픽스처:** `ControllerTest` (`::testing::Test`)
- **기본 Double:** `FakeTuner({1, 4, 12, 56})` — README 선호 채널 예시와 정합
- **FR-04 전용:** `MockTunerForController` (`seekCH` 호출 검증) — 선언만 존재, 미사용
- **스타일:** Given-When-Then 주석, 검증 채널·상태마다 **별도 `TEST_F`**

### 3.2 기능별 우선순위 (P0 = 릴리스 차단)

| FR | 기능 | P0 테스트 (현재/목표) | 상태 |
|----|------|----------------------|------|
| FR-01 | 숫자 버튼 채널 변경 | 5 / 10 | ⚠️ 세 자리+확인(B2) 등 5건 미작성 |
| FR-02 | 선호 채널 토글 | 3 / 4 | ✅ 기본 시나리오, 채널 0 경계 미작성 |
| FR-03 | 다음 선호 채널 | 3 / 5 | ✅ 기본·wrap·빈 목록 |
| FR-04 | 채널 검색 | 0 / 2+ | ❌ API·테스트 미착수 |
| FR-05 | 업/다운 (검색 없음) | 0 / 3 | ❌ |
| FR-06 | 업/다운 (검색 있음) | 0 / 3+ | ❌ |

**실행 순서:** FR-01 보강 → FR-04 → FR-05 → FR-06 → P1 경계·예외 → P2 커버리지 90%+

### 3.3 현재 `TEST_F` 목록 (`TVControllerTest.cpp`)

| TEST_F | FR | 비고 |
|--------|-----|------|
| `PressNumber1ThenConfirm` | FR-01-01 | ✅ |
| `Press1Then2_AutoChange` | FR-01-02 | ✅ |
| `Press1234_TwoStageChange` | FR-01-03 | ✅ |
| `OtherButtonCancelsBuffer` | FR-01-04c | ✅ `4,5,6` 후 Other |
| `Zero7_SingleDigit7` | FR-01-05 | ✅ |
| `FavoriteAdd_NewChannel` | FR-02-01 | ✅ |
| `FavoriteToggle_Remove` | FR-02-02 | ✅ |
| `FavoriteToggleScenario` | FR-02-03 | ✅ |
| `NextFavorite_Normal` | FR-03-01 | ✅ |
| `NextFavorite_WrapAround` | FR-03-02 | ✅ |
| `NextFavorite_EmptyList` | FR-03-03 | ✅ |

---

## 4. 경계값 — 세 자리 숫자 입력 (FR-01-04)

README·`TDD_TV_Requirements.txt` 기준: `4`,`5`,`6` 연속 입력 시 **45 적용** 후, 잔여 `6`은 **확인 시 6번**, **그 외 버튼 시 무효화**.

현재 `pressNumber()`는 3번째 숫자에서 `buffer.clear()`만 수행하며, **확인으로 한 자리 적용(B2)에 대한 테스트가 없다.**

| ID | 입력 시퀀스 | 기대 채널 | TEST_F (계획) | 상태 |
|----|-------------|-----------|---------------|------|
| B1 | `4`→`5`→`6` | `45` | `ThreeDigits_ApplyFirstTwo` | ⚠️ 부분 검증 |
| B2 | B1 + `확인` | `6` | `ThreeDigits_ThenConfirm_SingleDigit` | ❌ |
| B3 | B1 + `pressOther` | `45` 유지 | `OtherButtonCancelsBuffer` | ✅ |
| B4 | `9`→`9`→`9` | `99` | `ThreeDigits_999` | ❌ |
| B5 | `0`→`7`→`5` | `7` (이후 5 무효) | `ThreeDigits_075` | ❌ |
| B6 | `1`→`2`→`3`→`확인` | `3` | `ThreeDigits_123_Confirm` | ❌ |
| B7 | `1`→`0`→`5` | `10` | `ThreeDigits_105` | ❌ |

**두 자리 자동 적용 경계:** `99`, `00`, `07`, `10`, `90` — 각각 별도 `TEST_F` (P1).

> **명세 주의:** `.cursorrules`는 3자리 이상 시 **마지막 두 자리** 규칙을 정의한다. README FR-01-04와 상이하므로, **B2 테스트를 characterization test로 먼저 추가**한 뒤 구현·문서를 단일 규칙으로 정렬할 것 (`Report/02` §4.1 참조).

---

## 5. 예외 및 특이 케이스

### 5.1 예외 (`EXPECT_THROW`)

| ID | 조건 | 기대 | TEST_F | 상태 |
|----|------|------|--------|------|
| E1 | `pressNumber(-1)` | `std::invalid_argument` | `PressNumber_Negative_Throws` | ❌ (구현은 throw, 테스트 없음) |
| E2 | `pressNumber(10)` | 동일 | `PressNumber_Ten_Throws` | ❌ |
| E3 | `FakeTuner::setCH("100")` | `invalid_argument` | `FakeTuner_ChannelOutOfRange` | ❌ |

### 5.2 특이 (no throw)

| ID | 시나리오 | 기대 | 상태 |
|----|----------|------|------|
| S1 | 선호 비어 있을 때 `pressNextFavorite` | 채널 유지 | ✅ |
| S2 | 빈 버퍼에서 `pressConfirm` | no-op | ❌ P1 |
| S4 | 세 자리 후 숫자 재입력 | 새 버퍼 동작 | ❌ P1 |
| S8 | 검색 목록 유무에 따른 FR-05/06 분기 | 상호 배타 | ❌ P0 (FR-04~06 구현 시) |

---

## 6. 커버리지 목표 및 측정 전략

### 6.1 목표

| 메트릭 | 목표 | 대상 |
|--------|------|------|
| Line coverage | **≥ 90%** | `TVChannelController.h` 구현부 |
| Branch coverage | **≥ 85%** | `pressNumber`, `pressNextFavorite`, (예정) 업/다운 |
| Function coverage | **100%** | public API 전부 |

현재 `CMakeLists.txt`에는 **gcov 플래그 미설정**. `ENABLE_COVERAGE` 옵션 추가 후 Debug 빌드 권장 (상세: `docs/test_plan.md` §7).

### 6.2 측정·개선 5단계

| 단계 | 활동 |
|------|------|
| 1 | Baseline `lcov` / `gcovr` 리포트 |
| 2 | FR-04~06 Red→Green 후 재측정 |
| 3 | throw 분기·`buffer.size()>=3`·빈 선호 목록 분기 보강 |
| 4 | CI: line &lt; 90% 시 실패 |
| 5 | Green 유지 리팩토링 후 회귀 |

**예상 미커버 (현재):** `pressNumber` invalid throw, 3자리 early return, FR-04~06 전체.

### 6.3 환경 권장

| OS | 도구 |
|----|------|
| Linux / WSL | GCC + `lcov` + `genhtml` |
| Windows | MinGW-w64 GCC + `gcovr` (MSVC는 OpenCppCoverage 등 별도) |

---

## 7. 요구사항 추적 매트릭스

| 요구 ID | TEST_F (현재) | TEST_F (목표) | 갭 |
|---------|---------------|---------------|-----|
| FR-01 | 5 | 10 | +5 |
| FR-02 | 3 | 4 | +1 |
| FR-03 | 3 | 5 | +2 |
| FR-04 | 0 | 2+ | +2 |
| FR-05 | 0 | 3 | +3 |
| FR-06 | 0 | 3+ | +3 |
| **합계** | **11** | **≥ 27** | **+16** |

**요구사항 대비 테스트 커버리지 (기능 관점):** FR-01~03 약 60~70%, **FR-04~06 약 0%**.

---

## 8. 권장 작업 순서 (TDD)

| 단계 | 작업 | 산출 |
|------|------|------|
| 1 | `ThreeDigits_ThenConfirm_SingleDigit` 등 FR-01-04b Red | 명세 확정 (README vs `.cursorrules`) |
| 2 | FR-01 경계·예외 P1 (`99`, `00`, E1~E2) | +7 `TEST_F` |
| 3 | FR-04 Red: `ChannelSearch_*` + `pressChannelSearch()` | Mock `seekCH` |
| 4 | FR-05 Red/ Green: `pressChannelUp`/`Down` (검색 없음) | 3 `TEST_F` |
| 5 | FR-06 Red/ Green: 목록 내·외 업/다운 | 3+ `TEST_F` |
| 6 | `ENABLE_COVERAGE` + lcov baseline → 90% 게이트 | CI 스크립트 |
| 7 | Green 유지 리팩토링 (`Report/03` 우선순위 1~2) | SRP 분리 |

**절대 준수:** `Tuner` / `TVController` / `remoteKey` 미변경, Green에서만 리팩토링, 동작 변경과 리팩토링 분리.

---

## 9. 완료 정의 (DoD)

| # | 기준 |
|---|------|
| 1 | FR-01~06 README 시나리오별 `TEST_F` 존재 |
| 2 | `ctest` 100% Green |
| 3 | 세 자리 B1~B7, 예외 E1~E2 통과 |
| 4 | `TVChannelController` line coverage ≥ 90% |
| 5 | Tuner 경유 채널 변경만 (Mock/Fake 검증) |

현재 DoD 충족: **항목 2만 해당** (24/24 Green, FR 전체 미충족).

---

## 10. 결론

1. **테스트 계획 수립 완료:** `docs/test_plan.md`에 `TEST_F` 우선순위, 경계값·예외 목록, gcov/lcov 전략을 문서화했다.
2. **현재 품질:** 11개 Controller `TEST_F`로 FR-01~03 기본 시나리오는 확보되었으나, **FR-04~06 및 FR-01 세 자리 확인 시나리오는 미검증**이다.
3. **최우선 갭:** `ThreeDigits_ThenConfirm_SingleDigit`(B2), FR-04 채널 검색, FR-05/06 업·다운 — 목표 **+16 `TEST_F`**.
4. **커버리지:** CMake 커버리지 옵션·lcov baseline 미적용; 구현 완료 후 **90% line**을 CI 게이트로 권장한다.
5. **다음 단계:** B2 Red 테스트 → 명세 정렬 → FR-04 Mock 테스트 → FR-05/06 순차 TDD.

---

## 11. 참고 문서

| 문서 | 설명 |
|------|------|
| `docs/test_plan.md` | 본 보고서의 상세 테스트 계획서 |
| `TDD_TV_Requirements.txt` | FR-01~06 구조화 요구사항 |
| `test/TVControllerTest.cpp` | Controller 단위 테스트 |
| `include/TVChannelController.h` | 테스트 대상 구현 |
| `Report/01_TVController_QA_Refactoring_Report.md` | QA·리팩토링 현황 |
| `Report/02_Requirements_Analysis_Report.md` | 요구사항·명세 충돌 분석 |
| `Report/03_TVChannelController_Code_Quality_Report.md` | SOLID·리팩토링 방향 |
