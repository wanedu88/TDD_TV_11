# Golden Master (Approval) Tests

TextTest-style output regression for `TVChannelController`.

## Layout

| Path | Role |
|------|------|
| `approved/<scenario>.approved.txt` | Committed expected transcript (golden) |
| `received/<scenario>.received.txt` | Written on mismatch (gitignored) |
| `../TVControllerGoldenTest.cpp` | Scenario scripts |
| `ControllerTrace.h` | Action → text transcript builder |
| `GoldenMaster.h` | Compare / update helper |

## Transcript format

```
# fixture=FakeTuner{1,4,12,56} scenario=fr01_press12_auto
> pressNumber(1)
CH=0
> pressNumber(2)
CH=12
```

## Run

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Golden tests only:

```powershell
ctest --test-dir build -R TVControllerGolden --output-on-failure
```

## Update goldens (after intentional behavior change)

```powershell
$env:TV_UPDATE_GOLDEN = "1"
.\build\TVControllerGoldenTest.exe
# or
cmake --build build --target update_golden
# or
.\scripts\update_golden.ps1
```

Then review `test/golden/approved/*.approved.txt` and commit.

## Environment

| Variable | Default | Purpose |
|----------|---------|---------|
| `TV_UPDATE_GOLDEN` | `0` | `1` → overwrite approved files |
| `TV_GOLDEN_APPROVED_DIR` | `test/golden/approved` | Approved root |
| `TV_GOLDEN_RECEIVED_DIR` | `test/golden/received` | Failure artifacts |
