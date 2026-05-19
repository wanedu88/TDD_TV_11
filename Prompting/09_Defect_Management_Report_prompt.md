# Defect Management Report — Conversation Transcript
_Exported on 5/19/2026 from Cursor (current session)_

---

**User**

@docs/defect_list.md @TDD_TV_Requirements.txt @docs/test_plan.md

[P] QA 리드 엔지니어입니다.
[T] 결함 관리 문서를 defect_report.md로 작성해줘.
    1) 결함 분류 체계:
       Severity (Critical/Major/Minor/Info) x ItemType (5종) 매트릭스 표
    2) 결함 보고서 템플릿 (재현/기대/실제/원인/수정/검증)
    3) 품질 메트릭 수집 계획
       - 테스트 통과율, 커버리지, 단계별 결함 발견율
       - C++: gcov/lcov / Java: JaCoCo / Python: pytest-cov
    4) (선택) GitHub Issues 연동 워크플로우
[F] Markdown. defect_report.md로 저장

---

**Cursor**

`docs/defect_list.md`, `TDD_TV_Requirements.txt`, `docs/test_plan.md`, `Report/06_*`, `.github/workflows/ci.yml` 등을 분석한 뒤 결함 관리 문서를 작성했습니다.

### 문서 구성 (`docs/defect_report.md`)

1. **결함 분류 체계** — Severity 4단계, ItemType 5종, 5×4 매트릭스·처리 정책, Status 정의
2. **결함 보고서 템플릿** — 메타데이터, 재현/기대/실제/원인/수정/검증, DEF-004 예시
3. **품질 메트릭** — QM-01~07, T1~T7 단계별 발견율, gcov/lcov·JaCoCo·pytest-cov, DoD 게이트
4. **GitHub Issues** — 라벨, 워크플로(Mermaid), CI coverage job, Issue template YAML
5. **결함 생명주기** — stateDiagram
6. **교차 링크** — defect_list, test_plan, Report/06·07

### 반영한 현황

- ctest **57/57 Green** (HEAD)
- DEF-001~014: Fixed 5 / Open 8 / Waived 1
- Open 잔여: 명세(DEF-006~007), P1 테스트(DEF-010, 011, 014), 커버리지(DEF-012), 기술 부채

파일 경로: `c:\DEV\TDD_TV_11\docs\defect_report.md`

---

**User**

Report 디렉토리에 보고서 써줘

---

**Cursor**

Report 시리즈 09번으로 이해관계자용 요약 보고서를 작성했습니다.

**파일:** `Report/09_Defect_Management_Report.md`

### Report vs docs

| 문서 | 역할 |
|------|------|
| `Report/09_*` | 현황 요약·분류·템플릿·메트릭·결론·권장 조치 |
| `docs/defect_report.md` | 운영 가이드 전문 (CI YAML, Issue template) |
| `docs/defect_list.md` | DEF-001~014 레지스터 |

`docs/defect_report.md`, `docs/defect_list.md`에 Report/09 교차 링크 추가.

---

**User**

Prompting에 현재 프롬프트를 저장해줘

---

**Cursor**

`Prompting/09_Defect_Management_Report_prompt.md` 저장 (본 파일).

---

## 세션 산출물 요약

| 순서 | 요청 | 산출물 |
|------|------|--------|
| 1 | 결함 관리 문서 | `docs/defect_report.md` |
| 2 | Report | `Report/09_Defect_Management_Report.md` |
| 3 | Prompting | `Prompting/09_Defect_Management_Report_prompt.md` |

## [P][T][F] 템플릿

```text
@docs/defect_list.md @TDD_TV_Requirements.txt @docs/test_plan.md

[P] QA 리드 엔지니어입니다.
[T] 결함 관리 문서를 defect_report.md로 작성해줘.
    1) 결함 분류 체계:
       Severity (Critical/Major/Minor/Info) x ItemType (5종) 매트릭스 표
    2) 결함 보고서 템플릿 (재현/기대/실제/원인/수정/검증)
    3) 품질 메트릭 수집 계획
       - 테스트 통과율, 커버리지, 단계별 결함 발견율
       - C++: gcov/lcov / Java: JaCoCo / Python: pytest-cov
    4) (선택) GitHub Issues 연동 워크플로우
[F] Markdown. defect_report.md로 저장
```

```text
[P] QA 리드 — Report 산출
[T] Report 디렉토리에 보고서 작성 (09번 체계)
[F] Report/09_Defect_Management_Report.md
```

```text
[P] QA 리드 — 결함 레지스터 (선행 작업)
[T] defect_list.md — [ID][Severity][ItemType][Steps][Expected][Actual][Root Cause][Fix Summary]
[F] docs/defect_list.md
```

## ItemType 5종 (참고)

| ItemType | 설명 |
|----------|------|
| Bug | 구현 오류 |
| Missing Implementation | 기능 미구현 |
| Specification Gap | 명세 불일치 |
| Test Gap | 검증 테스트 부재 |
| Technical Debt | 설계·성능·유지보수 개선 |

## 관련 문서

| 문서 | 경로 |
|------|------|
| 결함 관리 보고서 | `Report/09_Defect_Management_Report.md` |
| 결함 관리 가이드 | `docs/defect_report.md` |
| 결함 목록 | `docs/defect_list.md` |
| 결함 분석 | `Report/06_TVChannelController_Defect_Analysis_Report.md` |
| 테스트 계획 | `Report/04_Test_Plan_Report.md`, `docs/test_plan.md` |
| 요구사항 | `TDD_TV_Requirements.txt` |
