# 02. 요구사항 정리 및 C++ 구현 분석 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 `TDD_TV_11` 프로젝트의 README 기반 요구사항을 개발·검증에 사용할 수 있도록 정리하고, 시니어 C++ QA 관점에서 구현·테스트 함의를 분석한 결과를 기록한다.

| 구분 | 설명 |
|------|------|
| 배경 | README에 기능 시나리오가 서술형으로만 존재하여, TDD·QA 작업 시 추적·검증이 어려움 |
| 목표 | 요구사항 문서화, C++ 구현 규칙 명확화, Google Test 시나리오 체계화 |
| 범위 | `TVChannelController` 및 Tuner 연동 (레거시 `TVController`, `Tuner`, `remoteKey` 미변경) |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 개발 요구사항 | `TDD_TV_Requirements.txt` | FR-01~06 기능·비기능 요구, 완료 정의 |
| 구현 분석 | `docs/requirements_analysis.md` | 기능별 규칙 표, 문자열 주의점, TS 시나리오 목록 |
| 선행 QA 보고서 | `Report/01_TVController_QA_Refactoring_Report.md` | 구현·테스트 현황, 리팩토링 제안 |

---

## 2. 요구사항 정리 요약

### 2.1 기능 요구 (README → FR)

| FR ID | 기능 | README 섹션 | 우선순위 |
|-------|------|-------------|----------|
| FR-01 | 숫자 버튼 채널 변경 | TDD #1 | 필수 |
| FR-02 | 선호 채널 추가/삭제 (토글) | TDD #2 | 필수 |
| FR-03 | 다음 선호 채널 | TDD #3 | 필수 |
| FR-04 | 채널 검색 (`seekCH` 기반 저장) | TDD #4 | 필수 |
| FR-05 | 채널 업/다운 (검색 결과 없음) | TDD #5 | 필수 |
| FR-06 | 채널 업/다운 (검색 결과 있음) | TDD #6 | 필수 |

### 2.2 외부 의존성

- **Tuner** (`setCH`, `getCurrentCH`, `seekCH`): 업체 제공, 본 프로젝트에서 인터페이스·구현 수정 금지.
- **테스트**: `FakeTuner` / `MockTuner`로 대체하여 Controller만 검증.
- **리모컨**: `remoteKey` enum은 레거시; 숫자·업/다운 등은 `TVChannelController` 테스트 헬퍼로 매핑.

### 2.3 채널 도메인

- 유효 범위: **0 ~ 99** (100개).
- Tuner API는 `std::string`, 도메인 로직은 **`int` 정규화 후 연산** 권장 (`docs/requirements_analysis.md` §2).

---

## 3. 구현·테스트 현황 (2026-05-19 기준)

### 3.1 테스트 실행 결과

```powershell
ctest --test-dir "build" --output-on-failure
```

| 항목 | 값 |
|------|-----|
| 전체 테스트 | 24 |
| 성공 | 24 |
| 실패 | 0 |
| 상태 | **Green** |

### 3.2 기능별 구현·테스트 커버리지

| FR | 구현 (`TVChannelController`) | 테스트 (`TVControllerTest` 등) | 비고 |
|----|------------------------------|--------------------------------|------|
| FR-01 | 부분 구현 | 5건 (TS-101~105) | 세 자리 README 시나리오 일부 미검증 |
| FR-02 | 구현 | 3건 (TS-201~203) | 경계 0, 99 미검증 |
| FR-03 | 구현 | 3건 (TS-301~303) | strict greater 엣지 미검증 |
| FR-04 | **미구현** | **없음** | TS-401~403 필요 |
| FR-05 | **미구현** | **없음** | TS-501~602 필요 |
| FR-06 | **미구현** | **없음** | TS-701~802 필요 |

### 3.3 테스트 시나리오 통계 (TS 체계)

`docs/requirements_analysis.md` §4 기준:

| 구분 | 구현·통과 | 미작성/미구현 (대략) |
|------|-----------|----------------------|
| Tuner Mock (TS-001~005) | 5 | 0 |
| 채널 변경 (TS-101~) | 5 | 3+ |
| 선호 (TS-201~) | 3 | 2+ |
| 다음 선호 (TS-301~) | 3 | 1+ |
| 검색·업·다운 (TS-401~) | 0 | **12** |

**요구사항 대비 테스트 커버리지:** FR-01~03은 기본 시나리오 확보, **FR-04~06은 0%**에 가깝다.

---

## 4. QA 주요 발견 사항

### 4.1 세 자리 입력 — README vs 구현

| 구분 | `4`,`5`,`6` 입력 후 동작 |
|------|-------------------------|
| **README** | 45 적용 후, `6`+확인 → 6번; 그 외 버튼 → `6` 무효화 |
| **현재 구현** | 3번째 숫자 시 `buffer.clear()`만 수행 (45 유지, 잔여 digit 없음) |
| **`.cursorrules`** | 3자리 이상 → **마지막 두 자리**가 채널 (예: `123`→`23`) — README와 또 다른 규칙 |

**권장:** TS-106(3자리 후 `6`+확인) 테스트를 먼저 추가하여 **단일 진실 공급원**을 확정한 뒤 구현 또는 문서를 정렬한다.

### 4.2 채널 문자열 표기 불일치

- 테스트: `EXPECT_EQ("1", ...)`, `EXPECT_EQ("7", ...)`.
- `.cursorrules`: `00`~`09` 두 자리 입력·표기.
- **위험:** `==` 문자열 비교, 사전순 비교(`"9" > "12"`)로 업/다운·선호 로직 오류 가능.

**권장:** `parseChannel()` 헬퍼로 int 비교 후 assertion; `setCH` 호출 시 포맷 정책 팀 합의.

### 4.3 미구현 기능 (FR-04~06)

README 및 `TDD_TV_Requirements.txt`에 명시된 다음 항목은 Controller·테스트 모두 부재한다.

1. **채널 검색:** `seekCH()` 반복 → `searchedChannels_` 저장.
2. **채널 업/다운 (검색 없음):** 0~99 mod 순환 (99→0, 0→99).
3. **채널 업/다운 (검색 있음):** 저장 목록 내 next/prev + 목록 밖 현재 채널 시 wrap (FR-06-02).

구현 시 `std::upper_bound` / `std::lower_bound` 및 검색 목록 유무 분기가 필요하다.

---

## 5. 기능별 구현 규칙 (요약)

상세 표·코드 스케치는 `docs/requirements_analysis.md` §1 참조.

| 기능 | 핵심 규칙 ID | C++ 구현 요약 |
|------|--------------|---------------|
| 채널 변경 | CH-01~05 | `vector<int>` 버퍼, 2자리 시 `applyBuffer()`, 1자리는 확인 시 적용 |
| 선호 추가 | FAV-01~02 | `std::set<int>`, `stoi(getCurrentCH())` 토글, `setCH` 없음 |
| 다음 선호 | NFAV-01~03 | `upper_bound` + wrap, 빈 목록 no-op |
| 채널 검색 | SRCH-01~02 | `seekCH()` 루프, 목록 멤버 추가 |
| 채널 업 | UP-A/B | 검색 ∅ → mod+1; 검색 O → 목록 내 next |
| 채널 다운 | DN-A/B | 검색 ∅ → mod-1; 검색 O → 목록 내 prev |

---

## 6. 권장 작업 순서 (TDD)

`01_TVController_QA_Refactoring_Report.md` 및 본 분석을 통합한 실행 순서:

| 단계 | 작업 | 산출 |
|------|------|------|
| 1 | 명세 충돌 해소 | TS-106 등 characterization test |
| 2 | FR-04 테스트 작성 | TS-401~403 (Red) |
| 3 | FR-04 구현 | `pressChannelSearch()`, `searchedChannels_` |
| 4 | FR-05 테스트·구현 | TS-501~602 |
| 5 | FR-06 테스트·구현 | TS-701~802 |
| 6 | 경계·포맷 보강 | TS-107, 108, 204, 205 등 |
| 7 | 리팩토링 | 상수화, 버퍼 로직 분리 (Green 유지) |

**절대 준수**

- `TVController`, `Tuner`, `remoteKey` 수정 금지.
- 테스트 Green 상태에서만 리팩토링.
- 동작 변경과 리팩토링 분리.

---

## 7. 결론

1. **요구사항 문서화 완료:** `TDD_TV_Requirements.txt`로 FR-01~06 및 완료 정의를 추적 가능하게 정리했다.
2. **C++ QA 분석 완료:** `docs/requirements_analysis.md`에 규칙 표·문자열 함정·TS 번호 체계를 수록했다.
3. **현재 품질 상태:** 24개 테스트 Green이나, README 전체 요구의 약 **50%**(FR-04~06)는 미구현·미검증이다.
4. **최우선 리스크:** 세 자리 입력·채널 문자열 표기·명세 3원 불일치(README / 구현 / `.cursorrules`).
5. **다음 단계:** TS-106으로 CH-04 명세 고정 → FR-04~06 Red 테스트 → `TVChannelController` 확장.

---

## 8. 참고 문서

| 문서 | 설명 |
|------|------|
| `README.md` | 원본 기능 시나리오 |
| `TDD_TV_Requirements.txt` | 구조화된 개발 요구사항 |
| `docs/requirements_analysis.md` | C++ 구현·Google Test 상세 분석 |
| `Report/01_TVController_QA_Refactoring_Report.md` | 기존 QA·리팩토링 보고서 |
| `.cursorrules` | 프로젝트 절대 규칙·테스트 스타일 |
