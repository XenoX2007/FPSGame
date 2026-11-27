# CLAUDE.md — GDEV50010 C++ For Engines (UE5 FPS Time-Attack Arena)

> Đây là file ngữ cảnh cho Claude Code khi chạy trong Rider. Những chỗ `<...>` cần bạn điền.

## 1. Ngữ cảnh môn học

- **Sinh viên:** Minh — BUV, Computer Games Design and Programming
- **Module:** GDEV50010 — C++ For Engines
- **Bài đánh giá:**
  - **A1 — Game Project (70%):** game FPS time-attack trong UE5. Nộp: Planning & Intent Document (800–1000 từ; bắt buộc Function Specifier Usage, Input to Action Flow, Communication Hierarchy), project UE5 + build + video gameplay, và **Technical Design Document** (2–3 trang: UML C++→BP, code snippet của pattern, Designer Manual). Lưu ý: trong brief "TDD" = *Technical Design Document*; test-driven development là lựa chọn riêng của Minh, không được chấm trực tiếp.
    - Gameplay bắt buộc: timer đếm ngược + **nhặt đủ pickup trước khi hết giờ (điều kiện thắng)**; enemy AI tìm, di chuyển tới, tấn công player; player bắn được enemy.
    - Rubric: C++ giữ data/logic, BP chỉ visual/audio + chỉnh biến; specifier `EditDefaultsOnly`/`BlueprintReadOnly`/`Category`/`ToolTip`; ≥1 pattern (vd Observer bằng dynamic delegate); C++ broadcast delegate → BP con bắt để làm feedback; safe pointer; Phase 4 xóa prototype/debug code.
    - Nộp bài: không có tên sinh viên (loại `CLAUDE.md`, `.claude/`, `.mcp.json` khỏi zip); xóa `Binaries/Intermediate/DerivedDataCache/Saved` trước khi zip.
  - **A2 — Reflective Report (30%):** 1800 từ (tối đa 1980), Harvard; cấu trúc Introduction / Best Practices / Critical Reflection; chỉ Critical Reflection được dùng ngôi thứ nhất.
- **Tài liệu môn học** (brief, rubric, slide) để ở `Docs/`. Hãy đọc chúng trước khi đề xuất kiến trúc hoặc tính năng:
  - `Docs/ControllingOurGameStates.pptx` — slide game state: `StartGameMatch` + override `ReadyToStartMatch`, timer chạy sau khi bấm Start (đã áp dụng vào `AFPSGameMode`)
  - `Docs/A1_Brief.pdf` — brief + rubric A1 (rubric ở Appendix 1)
  - `Docs/A2_Brief.pdf` — brief + rubric A2 (rubric ở Appendix 1)
  - `Docs/TDD/` — tư liệu Technical Design Document (class diagram, data flow)
  - `Docs/Planning.md` — tài liệu planning của Minh
  - `Docs/DevLog.md` — nhật ký làm việc, dùng làm tư liệu cho A2
- Khi rubric và đề xuất của Claude mâu thuẫn nhau, **rubric luôn thắng**. Nếu rubric chưa rõ, Claude phải hỏi lại, không được tự đoán.

## 2. Quy tắc học thuật

- Đây là bài nộp có chấm điểm. Mục tiêu là để Minh **hiểu và tự giải thích được** từng phần code.
- Mỗi khi viết hoặc sửa code, Claude giải thích ngắn: làm gì, vì sao chọn cách này, có lựa chọn nào khác.
- **A2 Reflective Report do Minh tự viết.** Claude chỉ được: góp ý cấu trúc, chỉ ra chỗ lập luận yếu, gợi ý điểm cần phản tư, và tóm tắt `DevLog.md` thành dữ kiện (đã làm gì, gặp lỗi gì, sửa ra sao). Claude không viết thay các đoạn văn.
- Khi một quyết định thiết kế đáng đưa vào báo cáo, Claude nhắc Minh ghi lại vào `Docs/DevLog.md`.

## 3. Thông tin project

- **Engine:** Unreal Engine 5.8 (`EngineAssociation` trong `TTT.uproject`)
- **Tên project / module chính:** `TTT` (file `TTT.uproject`, code ở `Source/TTT/`)
- Project tạo từ template First Person của UE5, có sẵn hai thư mục `Variant_Horror/` và `Variant_Shooter/` của template.
- **IDE:** JetBrains Rider (có plugin Claude Code và RiderLink)
- **Hệ điều hành:** Windows
- **Đường dẫn engine:** `C:\Program Files\Epic Games\UE_5.8`

## 4. Kiến trúc: chia việc giữa C++ và Blueprint

Nguyên tắc: **C++ chứa logic, dữ liệu và luật chơi. Blueprint chứa phần trình bày và phần tinh chỉnh.**

| Loại việc | Làm ở đâu |
|---|---|
| Timer time-attack, tính điểm, luật thắng/thua, game phase | C++ (`AFPSGameMode`, không dùng GameState riêng — theo giảng viên) |
| Di chuyển, bắn, máu, sát thương | C++ (Character, Component) |
| Target / enemy spawn, wave | C++ (Actor / Subsystem) |
| Giá trị cân bằng (tốc độ, damage, thời gian) | C++ với `UPROPERTY(EditDefaultsOnly)`, chỉnh trong Blueprint con |
| UI/HUD, hiệu ứng, âm thanh, animation | Blueprint / UMG, nhận dữ liệu qua `BlueprintImplementableEvent` hoặc delegate |

Class đang có trong `Source/TTT/` (mô tả theo code hiện tại, cập nhật 2026-11-27):
- `ATTTPlayerController`, `ATTTCameraManager` — có sẵn từ template, chưa sửa.
- `ATTTCharacter` — nhân vật từ template, **đã sửa**: Dash (`DashAction` bind `ETriggerEvent::Started` → `DoDash()`: `LaunchCharacter` theo hướng WASD, không input thì về phía trước; `DashSpeed` 2500 cm/s, `DashCooldown` 1s bằng `FTimerHandle`, `IsDashReady()`; broadcast `OnDashed` (BlueprintAssignable) khi dash thật sự xảy ra). Là class cha của `AShooterCharacter`, `AEnemyCharacter` và `AHorrorCharacter` (template, có Sprint riêng → không khai báo property Sprint ở class cha). Test `TTT.Character.Dash.*`.
- `AFPSGameMode` (kế thừa `AGameMode`, theo slide `ControllingOurGameStates.pptx`) — **GameMode duy nhất** (Blueprint con `Studentwork/MyFPSGameMode`, cũng là `GlobalDefaultGameMode`). Game phase `EGamePhase { PreGame, InGame, PostGame }`: `CurrentPhase` private, chỉ đổi qua `SetGamePhase()` (đồng bộ `bIsRoundOver`, gọi `StartMatch()`/`EndMatch()` của engine có guard vì test world không có GameSession, broadcast `OnGamePhaseChanged`); `GetGamePhase()` BlueprintPure. Match chờ ở `PreGame`/`WaitingToStart` tới khi UI gọi `StartGameMatch()` (BlueprintCallable; chỉ chạy khi đang `PreGame`); `ReadyToStartMatch_Implementation` trả `bManualStartRequested && Super`. Đếm ngược `GameDuration` (60s) bằng `FTimerHandle` lặp 1s, broadcast `OnTimerUpdated`; `RemainingTime` clamp ≥ 0. Thua khi hết giờ hoặc player chết (`SetPlayerDefaults` bind `AShooterCharacter::OnDeath` → `HandlePlayerDeath` → `bPlayerDied = true`) → `HandleGameLoss()` (dừng timer, phase `PostGame`, `OnGameLost`, pause). `RestartRound()` (BlueprintCallable) `OpenLevel` lại level hiện tại; gọi từ `Restart_Button` trong `WBP_StartMenu`. Test `TTT.Timer.*`, `TTT.GameState.*`, `TTT.GameMode.*` trong `Tests/FPSGameModeTimerTest.cpp`.
- `AEnemyCharacter` — enemy kế thừa `ATTTCharacter` + `IShooterWeaponHolder` (tái dùng `AShooterWeapon` của Variant_Shooter). Máu nằm trong `UHealthComponent` (`TakeDamage` → `ApplyDamage`; `OnHealthDepleted` → `HandleHealthDepleted` → `Die()`: broadcast `OnPawnDeath` (BlueprintAssignable), tắt collision, `Destroy`). `BeginPlay` spawn `WeaponClass` gắn socket `HandGrip_R`. `StartFiring(Target)`/`StopFiring()` gọi từ BT. Aim vào `CurrentAimTarget` nếu có, không thì `DefaultAimDistance` phía trước. Không tick. Test `TTT.Enemy.*`.
- `UHealthComponent` (`Public/`) — máu dùng chung cho player và enemy: `MaxHealth` (EditDefaultsOnly), `CurrentHealth` (reset ở `BeginPlay`), `ApplyDamage()` (clamp ≥ 0, trả damage thực nhận), `Heal()` (không khi chết/đầy), `IsDead()`, `GetHealthPercent()`; delegate `OnHealthChanged(Current, Max)`, `OnHealthDepleted(DeadActor)` (chỉ 1 lần). Không tick. Test `TTT.HealthComponent.*`.
- `ATestAIController` — AI controller: `AIPerception` với sense Sight (`SightRadius`/`LoseSightRadius`/`SightAngleDegrees` là `UPROPERTY(EditDefaultsOnly)`, mặc định 1500/2000/90°, áp vào sense config ở `BeginPlay`; phát hiện mọi affiliation; không tick); `OnPossess` chạy `BehaviorTreeAsset`. `OnTargetPerceptionUpdated` (public): chỉ phản ứng với pawn `IsPlayerControlled()`; thấy → set Blackboard `TargetActor` + `SetFocus`; mất dấu → clear + `ClearFocus` + `StopFiring`. Test: `TTT.AI.Perception.*`.
- `APickupBase` (`Public/`, Abstract) — base cho pickup: sphere overlap → `TryPickup(Actor)`; chỉ tiêu thụ khi `ApplyPickup()` (virtual) trả `true`; ẩn + tắt collision, respawn sau `RespawnTime`; `OnRespawn` là `BlueprintNativeEvent` (mặc định gọi `FinishRespawn`).
- `ACollectiblePickup` (`Public/`) — con của `APickupBase` (`bRespawns = false`): chỉ `AShooterCharacter` nhặt được, broadcast `OnCollected(this)`. `AFPSGameMode` đếm tổng lúc `BeginPlay`, nhặt đủ → `HandleGameWin` (`bPlayerWon`, dừng timer, `OnGameWon`). Delegate cho UI: `OnCollectibleProgress(Collected, Total)`, `OnMatchStarted`, `OnGameWon`, `OnGameLost`. Test: `TTT.Pickup.Collectible.*`, `TTT.GameMode.*`.
- `AHealthPickup` (`Public/`) — con của `APickupBase`: chỉ `AShooterCharacter` nhặt được → `FindComponentByClass<UHealthComponent>()->Heal(HealAmount)`. Blueprint: `Studentwork/MyHealthPickup`.
- `AShieldPickup` (`Public/`) — con của `APickupBase`: `FindComponentByClass<UShooterArmourComponent>()` → `AddArmour(ArmourAmount)` nếu chưa đầy. Test: `TTT.Pickup.*`.
- `AShooterCharacter` (`Variant_Shooter/`) — player, từ template, **đã sửa**: có `UShooterArmourComponent`; máu nằm trong `UHealthComponent` (MaxHealth 500; `TakeDamage` → armour → `ApplyDamage`; `OnHealthChanged` → `OnDamaged` cho HUD; `OnHealthDepleted` → `Die()`), delegate `OnDeath` (broadcast trong `Die()`). Sprint: `SprintAction` (Started → `DoSprintStart` đặt `MaxWalkSpeed = SprintSpeed` 900; Completed/Canceled → `DoSprintEnd` trả về `DefaultWalkSpeed` lưu lúc `BeginPlay`). `UCLASS(abstract)` → test phải spawn `BP_ShooterCharacter` (và đặt `MOVE_Walking`, vì pawn không controller có `MOVE_None` → `Launch()` bị bỏ qua). Test `TTT.Character.Sprint.*`.
- `UFeedbackComponent` (`Public/`) — "Sound and Special Effects Component" (giảng viên gợi ý): `HitCue`/`DeathCue`/`DashCue` (`FFeedbackCue { Sound, Effect(Niagara) }`, EditDefaultsOnly). Tự bind ở `BeginPlay`: `UHealthComponent::OnHealthChanged` của owner (máu giảm & > 0 → Hit, về 0 → Death — **không** bind `OnHealthDepleted` vì enemy `Destroy` trong handler đó trước khi component chạy) và `ATTTCharacter::OnDashed`. `PlayFeedback()` phát tại vị trí owner, broadcast `OnFeedbackPlayed(EFeedbackEvent)` cho BP (camera shake…). Owner không biết component. Không tick. Thêm qua Add Component trong BP. Test `TTT.Feedback.*` (listener `Tests/FeedbackTestRecorder.h`).
- `UShooterArmourComponent` (`Variant_Shooter/`) — armour (tuần 4): `MitigateDamage`, `AddArmour`, getter, `StartingArmour` (khởi tạo trong `BeginPlay`), delegate `OnArmourChanged` → `AShooterPlayerController` → UI.
- `UBTTask_Shoot` (`BTTask_Shoot.h`) — BT task "Shoot": đọc `TargetActor` từ Blackboard → `AEnemyCharacter::StartFiring`, trả `Succeeded` ngay; `AbortTask` → `StopFiring`. Dùng trong `BT_SimpleEnemy` (asset Blueprint).
- `AShooterNPCSpawner` (`Variant_Shooter/AI/`) — từ template, **đã sửa**: spawn `AEnemyCharacter` (`NPCClass`), lắng nghe `OnPawnDeath` để respawn sau `RespawnDelay`. `SpawnCount` = tổng số enemy một spawner sinh ra (C++ mặc định 1 → không respawn); `BP_ShooterNPCSpawner` đặt 20. Test `TTT.Spawner.RespawnsAfterEnemyDies`.

Class dự kiến thêm (cập nhật khi thay đổi):
- (không có) — `ATTTGameState` đã bỏ: giảng viên xác nhận timer/điểm/phase nằm trong GameMode.

## 5. Quy ước code UE5

- Theo [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine): tiền tố `A` / `U` / `F` / `E` / `I`, `bIsSomething` cho bool, PascalCase.
- Dùng `TObjectPtr<>` cho con trỏ UObject trong header; luôn kèm `UPROPERTY()` để GC theo dõi.
- Header chỉ forward-declare; `#include` đặt trong `.cpp`. File `.generated.h` phải là include cuối cùng trong header.
- Ưu tiên Component và delegate hơn kế thừa sâu.
- Không đặt logic nặng trong `Tick`. Ưu tiên timer (`FTimerHandle`) và event.
- Comment ngắn bằng tiếng Anh trong code. Giải thích cho Minh thì dùng tiếng Việt.

## 6. Build và test (test-driven development)

**Build** (kiểm tra lỗi compile; tắt Unreal Editor trước, hoặc dùng Live Coding trong Editor thay cho lệnh này):
```bat
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TTTEditor Win64 Development -Project="C:\Users\Minh\Downloads\FPS-UE5\FPSGame\TTT.uproject" -WaitMutex
```

**Test** bằng UE Automation:
- Test đặt trong `Source/TTT/Tests/`. Dùng `IMPLEMENT_SIMPLE_AUTOMATION_TEST` hoặc Automation Spec (`BEGIN_DEFINE_SPEC`).
- Tên test theo dạng `TTT.<Feature>.<Case>`, ví dụ `TTT.Timer.EndsRoundAtZero`.
- Chạy test:
```bat
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\Minh\Downloads\FPS-UE5\FPSGame\TTT.uproject" -ExecCmds="Automation RunTests TTT;Quit" -unattended -nullrhi -nosound -abslog="<file .log>"
```
- Kết quả nằm trong log, tìm dòng `Test Completed. Result={Success|Fail}`. Output console không in kết quả test.
- Ghi chú khi test timer: test world không tự tick `FTimerManager`; test phải gọi `World->GetTimerManager().Tick()` và tăng `GFrameCounter` (xem `Tests/FPSGameModeTimerTest.cpp`).

**Quy trình TDD bắt buộc:** viết test trước → chạy cho fail → viết code tối thiểu để test pass → refactor. Claude ghi rõ mình đang ở bước nào. Sau mỗi tính năng, Claude nhắc Minh ghi vào DevLog: test nào, đã fail/pass thế nào.

## 7. Giới hạn của Claude trong project này

- **Không đọc được** file `.uasset` / `.umap` (Blueprint, level). Khi cần biết Blueprint làm gì, Claude hỏi Minh hoặc đọc mô tả trong `Docs/`.
- **Không quét** các thư mục `Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, `.vs/`, `.idea/`.
- **Không tự sửa** `*.uproject`, `*.Build.cs`, `*.Target.cs`, `Config/*.ini` nếu chưa hỏi Minh.
- Trước khi đổi tên class hoặc xóa `UPROPERTY` đang được Blueprint dùng, Claude phải cảnh báo, vì việc này có thể làm hỏng Blueprint.

## 8. Cách trả lời

- Trả lời bằng **tiếng Việt**. Thuật ngữ kỹ thuật giữ nguyên tiếng Anh.
- Ngắn gọn: code trước, giải thích sau.
- Khi có nhiều cách làm, nêu tối đa 2–3 cách, mỗi cách một dòng ưu/nhược, rồi đề xuất một cách.
