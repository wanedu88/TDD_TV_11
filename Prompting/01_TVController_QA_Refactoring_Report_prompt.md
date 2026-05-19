# Cursorrules for Gilded Rose project
_Exported on 5/19/2026 at 11:43:33 GMT+9 from Cursor (3.4.20)_

---

**User**

[P] 당신은 레거시 코드 QA/리팩토링을 돕는 시니어 C++ 엔지니어입니다.
[C] Gilded Rose C++17 프로젝트에서 Cursor AI가 항상 따라야 하는 규칙을
    프로젝트 루트의 .cursorrules로 작성하려고 합니다.
[T] 아래 요구를 만족하는 .cursorrules 내용을 작성해줘.
    - 기술 스택: C++17 + CMake + Google Test (+ gcov/lcov)
    - 절대 규칙:  TVController 구조체, Tuner 구조체, remotekey 구조체 수정 금지, channel은 00-99, , 세자리 이상 입력 시 마지막 두자리가 channel 
    - 테스트 규칙: Given-When-Then, TEST_F, channel이 10 이하일 때 00-09로 입력, channel 1개당 별도 케이스 작성
    - 리팩토링 규칙: 테스트 Green 상태에서만 진행, 매직 넘버 상수화 권장
[F] .cursorrules 파일에 그대로 붙여넣을 수 있는 텍스트


---

**Cursor**

요청하신 `.cursorrules` 작성 작업으로 이해했습니다. Cursor 규칙 작성 가이드를 먼저 확인한 뒤, 바로 붙여넣을 수 있는 형태로 정리하겠습니다.

아래 내용을 프로젝트 루트의 `.cursorrules`에 그대로 붙여넣으면 됩니다.

```text
# Gilded Rose C++17 Project Rules

You are a senior C++ engineer helping with legacy-code QA, characterization testing, and safe refactoring.

## Tech Stack

- Use C++17.
- Use CMake for build configuration.
- Use Google Test for unit tests.
- Use gcov/lcov for coverage when coverage is requested.

## Absolute Rules

- Do not modify the `TVController` struct.
- Do not modify the `Tuner` struct.
- Do not modify the `remotekey` struct.
- Preserve existing public behavior unless a test explicitly defines the new behavior.
- `channel` must always be treated as a two-digit value from `00` to `99`.
- When three or more digits are entered for a channel, the channel is determined by the last two digits only.
  - Example: `123` means channel `23`.
  - Example: `100` means channel `00`.

## Test Rules

- Write tests in Given-When-Then style.
- Use Google Test `TEST_F` fixtures for related behavior.
- When entering channels from `0` to `9`, input them as `00` to `09`.
  - Example: channel `1` must be tested by entering `0`, then `1`.
- Write a separate test case for each channel value being verified.
- Prefer small, behavior-focused tests over broad tests with many assertions.
- Add or update tests before changing production code.

## Refactoring Rules

- Refactor only when all tests are Green.
- Do not mix behavior changes and refactoring in the same step.
- Keep refactoring small and reversible.
- Prefer extracting named constants for magic numbers.
  - Example: use named constants for channel bounds, digit counts, and formatting width.
- Preserve legacy behavior unless tests clearly specify otherwise.
```

---

**User**

파일 생성

---

**Cursor**

프로젝트 루트에 `.cursorrules` 파일을 생성하겠습니다. 먼저 기존 파일이 있는지만 확인한 뒤 없으면 바로 추가하겠습니다.

기존 `.cursorrules`는 없어서 새 파일로 생성합니다. 앞서 정리한 규칙을 그대로 프로젝트 루트에 추가하겠습니다.

생성했습니다: `C:\DEV\TDD_TV_11\.cursorrules`

요청하신 C++17/CMake/Google Test 규칙, 구조체 수정 금지, 채널 입력 규칙, 테스트/리팩토링 규칙을 포함했습니다.

---

**User**

Report, Prompting, docs 디렉토리 3개를 생성해줘

---

**Cursor**

프로젝트 루트에 `Report`, `Prompting`, `docs` 세 디렉터리를 만들겠습니다. 먼저 루트 위치를 확인한 뒤 한 번에 생성하겠습니다.

프로젝트 루트가 확인됐습니다. 이제 요청하신 세 디렉터리를 생성하겠습니다.

생성했습니다.

- `Report`
- `Prompting`
- `docs`

---

**User**

Report 디렉토리에 보고서를 작성해줘

---

**Cursor**

`Report` 디렉터리에 프로젝트 보고서 파일을 작성하겠습니다. 먼저 소스/테스트 구조와 README를 확인해서 현재 프로젝트 상태에 맞는 내용으로 구성하겠습니다.

README와 CMake 설정을 확인했습니다. 이제 `include`와 `test`의 핵심 파일을 읽어서 보고서에 현재 구조, 테스트 상태, 리팩토링 주의점을 반영하겠습니다.

핵심 구현과 테스트 구조를 확인했습니다. 보고서 신뢰도를 위해 현재 빌드된 테스트 결과도 확인한 뒤, `Report` 아래 Markdown 보고서로 작성하겠습니다.

테스트는 24개 모두 통과했습니다. 이제 현재 요구사항, 코드 구조, 테스트 결과, 리스크와 개선 제안을 담은 보고서를 `Report` 디렉터리에 작성하겠습니다.

`Report` 디렉터리에 보고서를 작성했습니다.

파일: `Report/TVController_QA_Refactoring_Report.md`

현재 프로젝트 구조, 테스트 실행 결과 `24/24 Passed`, QA 리스크, `.cursorrules` 기준과 현재 구현의 차이, 리팩토링 제안까지 포함했습니다.

---

**User**

리포트 문서 이름을 01_*로 변경해줘

---

**Cursor**

보고서 파일명을 `01_` 접두어가 붙은 이름으로 변경하겠습니다. 기존 파일명을 확인한 뒤 같은 디렉터리에서 rename 처리하겠습니다.

변경했습니다.

`Report/TVController_QA_Refactoring_Report.md` → `Report/01_TVController_QA_Refactoring_Report.md`

---

**User**

Prompting 디렉토리에 01_* 파일이름으로 현재까지 대화형 프롬프트를 추출해줘

---

**Cursor**

현재 대화에서 사용된 사용자 프롬프트들을 추출해 `Prompting` 디렉터리에 `01_` 접두어 파일로 정리하겠습니다. 먼저 디렉터리 상태를 확인한 뒤 Markdown 파일을 생성하겠습니다.

`Prompting` 디렉터리가 비어 있어 새 파일로 작성합니다. 지금까지의 요청 흐름을 재사용 가능한 대화형 프롬프트 기록 형태로 정리하겠습니다.

생성했습니다.

`Prompting/01_interactive_prompts.md`

현재까지의 사용자 프롬프트를 순서대로 추출하고, `[P]`, `[C]`, `[T]`, `[F]` 형식에 대한 활용 메모도 함께 정리했습니다.

---

**User**

Prompting 디렉토리에 대화형 프롬프트를 export transcript 해줘

---

**Cursor**

`Prompting` 디렉터리에 현재 대화 내용을 transcript 형식으로 별도 export 하겠습니다. 기존 프롬프트 추출 파일과 구분되도록 `02_` 접두어로 대화 흐름 전체를 정리하겠습니다.

현재 요청에 맞춰 transcript 파일을 새로 작성하겠습니다. 사용자 요청과 응답 요지를 함께 남겨 이후 보고서/프롬프트 재현에 사용할 수 있게 구성합니다.

생성했습니다.

`Prompting/02_interactive_prompt_transcript.md`

현재까지의 대화형 프롬프트 흐름을 transcript 형식으로 정리했고, 생성된 산출물 목록도 함께 포함했습니다.

---

**User**

Report  디렉토리에 01_* 이름으로 보고서를 작성해줘

---

**Cursor**

`Report` 디렉터리에 `01_` 접두어 보고서가 있는지 먼저 확인하겠습니다. 이미 있으면 해당 파일을 기준으로 유지하고, 없으면 새로 작성하겠습니다.

현재 검색 결과로는 `01_` 보고서가 확인되지 않습니다. 디렉터리 목록을 직접 확인한 뒤 새 보고서를 생성하겠습니다.

`Report` 디렉터리가 비어 있어 새 파일을 작성합니다. 보고서 내용은 현재 프로젝트 구조와 테스트 상태를 다시 확인해서 `01_` 이름으로 생성하겠습니다.

핵심 요구사항과 현재 구현을 확인했습니다. 보고서에 테스트 상태를 포함하기 위해 기존 CTest도 다시 실행합니다.

테스트 24개가 모두 통과하는 것을 확인했습니다. 이제 `Report/01_*` 이름의 보고서를 작성하겠습니다.

작성했습니다.

`Report/01_TVController_QA_Refactoring_Report.md`

현재 테스트 결과도 다시 확인했고, `24/24 Passed` 상태를 보고서에 반영했습니다.

