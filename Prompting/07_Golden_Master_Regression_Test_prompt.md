# Golden Master Regression Test — Conversation Transcript
_Exported on 5/19/2026 from Cursor (current session)_

---

**User**

@include/TVChannelController.h @test/TVControllerTest.cpp

[P] 회귀 테스트(Approval/Golden Master) 설계 전문가입니다.
[C] C++17, Google Test, CMake
[T] TexttestFixture 출력 기반 Golden Master를 설계/구현해줘.
    1) expected 출력 파일 생성/보관 전략
    2) Google Test에서 파일 비교 테스트 구현
    3) CMake/ctest에 통합하는 방법
    4) CI에서 자동 실행 구성
[F] 테스트 코드 + CMake 수정안 + 실행 방법

---

**Cursor**

TextTestFixture 스타일 출력 기반 Golden Master 회귀 테스트를 설계·구현했습니다.

### 1) Approved 파일 전략

| 구분 | 경로 | Git |
|------|------|-----|
| Approved (golden) | `test/golden/approved/<scenario>.approved.txt` | 커밋 |
| Received (실패 시) | `test/golden/received/<scenario>.received.txt` | gitignore |

- GTest 이름 = 시나리오명 = 파일 접두사
- `TV_UPDATE_GOLDEN=1`일 때만 approved 덮어쓰기

### 2) GTest 구현

| 파일 | 역할 |
|------|------|
| `test/golden/GoldenMaster.h` | 읽기·CRLF 정규화·비교·update 모드 |
| `test/golden/ControllerTrace.h` | `>` 액션 + `CH=` / `favorites=` / `searched=` 스냅샷 |
| `test/TVControllerGoldenTest.cpp` | FR-01~06 시나리오 14건 |

트랜스크립트 예:

```
# fixture=FakeTuner{1,4,12,56} scenario=fr01_press12_auto
> pressNumber(1)
CH=0
> pressNumber(2)
CH=12
```

### 3) CMake / ctest

- `TVControllerGoldenTest` 실행 파일 추가
- `gtest_discover_tests(... WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})`
- `update_golden` 커스텀 타겟

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
ctest --test-dir build -R TVControllerGolden --output-on-failure
```

Golden 갱신:

```powershell
$env:TV_UPDATE_GOLDEN = "1"
.\build\TVControllerGoldenTest.exe
# 또는 cmake --build build --target update_golden
# 또는 .\scripts\update_golden.ps1
```

### 4) CI

`.github/workflows/ci.yml` — Ubuntu/Windows, `ctest --test-dir build -C Release`

### 실행 결과

| 항목 | 값 |
|------|-----|
| 전체 ctest | **71** (기존 57 + Golden 14) |
| Golden 시나리오 | 14 |
| 상태 | **Green** |

### 추가 산출물

- `.gitignore` — `test/golden/received/`
- `scripts/update_golden.ps1`, `scripts/update_golden.sh`
- `test/golden/README.md`

---

**User**

git hub에 작업한 파일들을 올려줘

---

**Cursor**

GitHub push 시도 — `git status` / `git remote` 확인 중 **사용자 측에서 명령 중단**(미완료). 로컬에서 아래 순서로 push 필요:

```powershell
cd c:\DEV\TDD_TV_11
git add .gitignore CMakeLists.txt .github test/golden test/TVControllerGoldenTest.cpp scripts
git commit -m "Add Golden Master regression tests and CI workflow"
git push -u origin HEAD
```

(remote 미설정 시 `git remote add origin <URL>` 선행)

---

**User**

Report 디렉토리에 보고서를 작성해줘

---

**Cursor**

`Report/07_Golden_Master_Regression_Test_Report.md` 작성 (Report/01~06 형식 준수).

| 섹션 | 내용 |
|------|------|
| §1 | 목적·범위·제약 |
| §2 | ctest 57→71, Green |
| §3 | TextTest 대응, 단위 vs Golden, 트랜스크립트 |
| §4 | Approved 생성·보관·갱신 전략 |
| §5 | 구현·CMake·CI |
| §6 | 시나리오 14건 (FR-01~06) |
| §7~11 | 요구 추적, DoD, 잔여 갭, 결론, 참고 |

---

**User**

Prompting 디렉토리에 대화내용을 추출해줘

---

**Cursor**

`Prompting/07_Golden_Master_Regression_Test_prompt.md` export (본 파일).

---

## 세션 산출물 요약

| 순서 | 사용자 요청 | 생성·갱신 파일 |
|------|-------------|----------------|
| 1 | Golden Master 설계/구현 ([P][C][T][F]) | `test/golden/*`, `test/TVControllerGoldenTest.cpp`, `CMakeLists.txt`, `.github/workflows/ci.yml`, `.gitignore`, `scripts/update_golden.*` |
| 2 | GitHub 업로드 | *(로컬 push 미완료 — 사용자 수동 필요)* |
| 3 | Report 보고서 | `Report/07_Golden_Master_Regression_Test_Report.md` |
| 4 | Prompting transcript | `Prompting/07_Golden_Master_Regression_Test_prompt.md` (본 파일) |

## [P][C][T][F] 프롬프트 템플릿 (재사용)

```text
@include/TVChannelController.h @test/TVControllerTest.cpp

[P] 회귀 테스트(Approval/Golden Master) 설계 전문가입니다.
[C] C++17, Google Test, CMake
[T] TexttestFixture 출력 기반 Golden Master를 설계/구현해줘.
    1) expected 출력 파일 생성/보관 전략
    2) Google Test에서 파일 비교 테스트 구현
    3) CMake/ctest에 통합하는 방법
    4) CI에서 자동 실행 구성
[F] 테스트 코드 + CMake 수정안 + 실행 방법
```

### 후속 프롬프트 (동일 세션)

```text
git hub에 작업한 파일들을 올려줘
```

```text
Report 디렉토리에 보고서를 작성해줘
```

```text
Prompting 디렉토리에 대화내용을 추출해줘
```

## 핵심 구현·테스트 결과 (재현용 메모)

| FR | Golden 시나리오 | FakeTuner |
|----|-----------------|-----------|
| FR-01 | 6 | `{1,4,12,56}` |
| FR-02~03 | 3 | `{1,4,12,56}` |
| FR-04 | 2 | `{1,4,12,56}` |
| FR-05 | 1 | `{1,4,12,56}` |
| FR-06 | 2 | `{4,6,14}` |

**환경 변수:** `TV_UPDATE_GOLDEN`, `TV_GOLDEN_APPROVED_DIR`, `TV_GOLDEN_RECEIVED_DIR`

**ctest:** 71/71 (Tuner 13 + Controller 44 + Golden 14)

**잔여:** GitHub push, gcov 90%, Golden 예외 시나리오 추가(G1)

## 관련 보고서·문서

| 문서 | 경로 |
|------|------|
| Golden Master 보고서 | `Report/07_Golden_Master_Regression_Test_Report.md` |
| Golden 사용 가이드 | `test/golden/README.md` |
| 단위 테스트 보고서 | `Report/05_TVController_Test_Implementation_Report.md` |
| Golden 테스트 | `test/TVControllerGoldenTest.cpp` |
| 승인본 | `test/golden/approved/*.approved.txt` |
| CI | `.github/workflows/ci.yml` |
| 이전 transcript | `Prompting/01~06_*_prompt.md` |
