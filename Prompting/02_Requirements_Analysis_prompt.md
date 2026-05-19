# TDD_TV Requirements & Analysis — Conversation Transcript
_Exported on 5/19/2026 from Cursor (current session)_

---

**User**

@readme.md 읽어서 TDD_TV_Requirements.txt  라는 개발 요구 문서를 만들어줘

---

**Cursor**

`README.md`를 바탕으로 프로젝트 루트에 `TDD_TV_Requirements.txt`를 만들었습니다.

### 문서 구성

1. **프로젝트 개요** — 목적, TDD 방식, 채널 범위(0~99)
2. **시스템 구성** — 리모컨 버튼 목록, Tuner 인터페이스(`seekCH`, `setCH`, `getCurrentCH`), Mock/Fake 사용, 수정 금지 대상
3. **기능 요구사항 (FR-01 ~ FR-06)** — README의 6개 TDD 항목을 시나리오별로 정리
   - 숫자 버튼 채널 변경 (5개 시나리오)
   - 선호 채널 토글
   - 다음 선호 채널 (로테이션 포함)
   - 채널 검색
   - 업/다운 (검색 결과 없음/있음)
4. **비기능 요구** — C++17, CMake, GTest, 테스트·리팩토링 규칙
5. **요구사항 추적 매트릭스** — ID별 우선순위
6. **TO-DO** — README의 refactoring 및 미구현 항목(채널 검색, 업/다운)
7. **완료 정의** — 테스트 Green, 레거시 인터페이스 보호 등

파일 위치: `c:\DEV\TDD_TV_11\TDD_TV_Requirements.txt`

`.cursorrules`의 두 자리 채널(`00`~`99`)·세 자리 이상 입력 규칙은 README에 없어 본 문서에는 넣지 않았습니다. README와 규칙을 맞춘 확장판이 필요하면 알려주세요.

---

**User**

@TDD_TV_Requirements.txt  @README.md 
[P] 시니어 C++ QA 엔지니어 관점에서
[C] TDD_TV C++17 프로젝트 (CMake, Google Test)
[T] 요구사항을 C++ 구현 관점에서 재정리해줘.
    1) 기능 별 규칙 표
         채널 변경 / 선호 채널 추가 / 다음 선호 채널 / 채널 검색 / 채널 업 / 채널 다운
     2) 문자열 비교/분기 시 주의점 (std::string find/== 등)
     3) 예외/경계값 조건 ( 채널: 0~99,  세 자리 입력 후 버퍼 처리)
     4) Google Test 기준 테스트 시나리오 목록 (번호)
[F] Markdown (표 + 번호 목록). docs/requirements_analysis.md로 저장

---

**Cursor**

`docs/requirements_analysis.md`에 시니어 C++ QA 관점 분석을 저장했습니다.

### 문서 구성

1. **기능별 규칙 표** (6개)
   - 채널 변경, 선호 채널 추가, 다음 선호 채널, 채널 검색, 채널 업, 채널 다운
   - 각각 규칙 ID, 입력/전제, `Tuner` 호출, C++ 구현 스케치 포함

2. **문자열 비교·분기 주의점**
   - `"7"` vs `"07"`, 사전순 비교 함정, `find` 오탐, `stoi` 예외
   - `parseChannel()` 기반 권장 패턴 및 Mock 검증 방법

3. **예외·경계값**
   - 채널 0~99, `invalid_argument` (TunerTest 기준)
   - 세 자리 입력: README vs 현재 `TVChannelController` **불일치** 명시
   - 경계값 매트릭스 (B-01 ~ B-06)

4. **Google Test 시나리오 (번호)**
   - **TS-001~005** Tuner Mock
   - **TS-101~108** 채널 변경
   - **TS-201~205** 선호
   - **TS-301~305** 다음 선호
   - **TS-401~802** 검색·업·다운 (대부분 미구현으로 표기)
   - 구현됨/미작성 상태와 FR 추적 표 포함

파일 경로: `c:\DEV\TDD_TV_11\docs\requirements_analysis.md`

---

**User**

Report 디렉토리에 보고서를 작성해줘

---

**Cursor**

`Report/02_Requirements_Analysis_Report.md`에 보고서를 작성했습니다.

### 보고서 개요

| 항목 | 내용 |
|------|------|
| **제목** | 02. 요구사항 정리 및 C++ 구현 분석 보고서 |
| **작성일** | 2026-05-19 |
| **근거** | `README.md`, `TDD_TV_Requirements.txt`, `docs/requirements_analysis.md` |

### 주요 섹션

1. **목적** — 요구사항 문서화·C++ QA 분석 배경 및 산출물 목록  
2. **요구사항 정리** — FR-01~06, Tuner/채널 도메인 요약  
3. **구현·테스트 현황** — ctest 24/24 Green, 기능별 커버리지 표  
4. **QA 주요 발견** — 세 자리 입력 명세 충돌, 문자열 표기, FR-04~06 미구현  
5. **기능별 규칙 요약** — CH/FAV/NFAV 등 규칙 ID와 C++ 구현 요약  
6. **권장 TDD 작업 순서** — TS-106 → FR-04~06 → 리팩토링  
7. **결론·참고 문서**

`Report/01_TVController_QA_Refactoring_Report.md`는 구현·리팩토링 중심, **02번**은 이번에 정리한 요구사항·분석·TS 체계 중심으로 구성했습니다.

경로: `c:\DEV\TDD_TV_11\Report\02_Requirements_Analysis_Report.md`

---

**User**

Prompting 디렉토리에 현재까지 대화형 프롬프트를 export transcript 해줘

---

**Cursor**

`Prompting/02_Requirements_Analysis_prompt.md`에 현재 세션 대화 transcript를 export했습니다.

---

## 세션 산출물 요약

| 순서 | 사용자 요청 | 생성·갱신 파일 |
|------|-------------|----------------|
| 1 | README → 개발 요구 문서 | `TDD_TV_Requirements.txt` |
| 2 | C++ QA 관점 요구사항 재정리 | `docs/requirements_analysis.md` |
| 3 | Report 보고서 | `Report/02_Requirements_Analysis_Report.md` |
| 4 | Prompting transcript export | `Prompting/02_Requirements_Analysis_prompt.md` (본 파일) |

## [P][C][T][F] 프롬프트 템플릿 (재사용)

아래 형식은 본 세션에서 효과적이었던 구조화 프롬프트 예시이다.

```text
[P] 시니어 C++ QA 엔지니어 관점에서
[C] TDD_TV C++17 프로젝트 (CMake, Google Test)
[T] 요구사항을 C++ 구현 관점에서 재정리해줘.
    1) 기능 별 규칙 표
         채널 변경 / 선호 채널 추가 / 다음 선호 채널 / 채널 검색 / 채널 업 / 채널 다운
     2) 문자열 비교/분기 시 주의점 (std::string find/== 등)
     3) 예외/경계값 조건 ( 채널: 0~99,  세 자리 입력 후 버퍼 처리)
     4) Google Test 기준 테스트 시나리오 목록 (번호)
[F] Markdown (표 + 번호 목록). docs/requirements_analysis.md로 저장
```

## 관련 보고서·문서

- `Report/01_TVController_QA_Refactoring_Report.md` — 선행 QA·리팩토링 보고서
- `Report/02_Requirements_Analysis_Report.md` — 본 세션 요약 보고서
- `Prompting/01_TVController_QA_Refactoring_Report_prompt.md` — 이전 세션 transcript
