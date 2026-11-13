# CLAUDE.md — GDEV50010 C++ For Engines (UE5 FPS Time-Attack Arena)

> Đây là file ngữ cảnh cho Claude Code khi chạy trong Rider. Những chỗ `<...>` cần bạn điền.

## 1. Ngữ cảnh môn học

- **Sinh viên:** Minh — BUV, Computer Games Design and Programming
- **Module:** GDEV50010 — C++ For Engines
- **Bài đánh giá:**
  - **A1 — Game Project:** game FPS time-attack trong UE5. Yêu cầu chia rõ phần C++ và phần Blueprint, có tài liệu planning, làm theo TDD.
  - **A2 — Reflective Report:** báo cáo phản tư về quá trình làm A1.
- **Tài liệu môn học** (brief, rubric, slide) để ở `Docs/`. Hãy đọc chúng trước khi đề xuất kiến trúc hoặc tính năng:
  - `Docs/MakingATimer.pptx` — slide hướng dẫn làm timer trong GameMode (đã áp dụng vào `AFPSGameMode`)
  - `Docs/A1_Brief.pdf` — `<chưa có, đổi tên cho đúng file>`
  - `Docs/A1_Rubric.pdf` — `<...>`
  - `Docs/A2_Brief.pdf` — `<...>`
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
| Timer time-attack, tính điểm, luật thắng/thua | C++ (`GameMode` / `GameState`) |
| Di chuyển, bắn, máu, sát thương | C++ (Character, Component) |
| Target / enemy spawn, wave | C++ (Actor / Subsystem) |
| Giá trị cân bằng (tốc độ, damage, thời gian) | C++ với `UPROPERTY(EditDefaultsOnly)`, chỉnh trong Blueprint con |
| UI/HUD, hiệu ứng, âm thanh, animation | Blueprint / UMG, nhận dữ liệu qua `BlueprintImplementableEvent` hoặc delegate |

Class đang có trong `Source/TTT/` (mô tả theo code hiện tại, cập nhật 2026-11-13):
- `ATTTPlayerController`, `ATTTCameraManager` — có sẵn từ template, chưa sửa.
- `ATTTCharacter` — nhân vật người chơi từ template, **đã sửa**: implement `II_Scorable2` (`AddScore_Implementation` hiện chỉ `UE_LOG`, chưa cộng điểm thật), có thêm `DashAction`.
- `ATTTGameMode` — từ template, **đã sửa**: `BeginPlay` tìm mọi `ACheckpoint` trong level và bind `ItemPickedUp` → `OnItemPickedUp` (hiện chỉ `UE_LOG`).
- `AFPSGameMode` — luật time-attack: đếm ngược `GameDuration` (60s) bằng `FTimerHandle` lặp 1s, broadcast `OnTimerUpdated(RemainingTime)` cho UI; hết giờ → `HandleGameLoss()` (set `bIsRoundOver = true`, pause game). `RemainingTime` được clamp ≥ 0. Có test `TTT.Timer.*` trong `Tests/FPSGameModeTimerTest.cpp`. Chưa có điều kiện thắng. Lưu ý: đang song song với `ATTTGameMode` — cần xác định level dùng GameMode nào.
- `AEnemyCharacter` — enemy kế thừa `ATTTCharacter` + `IShooterWeaponHolder` (tái dùng `AShooterWeapon` của Variant_Shooter). Spawn `WeaponClass` gắn vào socket `HandGrip_R` ở `BeginPlay`; `StartFiring(Target)`/`StopFiring()` (dự kiến gọi từ Behavior Tree trong Blueprint); `TakeDamage` trừ `CurrentHP`, ≤ 0 → `Die()` (broadcast `OnPawnDeath`, tắt collision, `Destroy`). Aim vào `CurrentAimTarget` nếu có, không thì 1000 unit phía trước.
- `ATestAIController` — AI controller: `AIPerception` với sense Sight (radius 1500/2000, góc 90°, phát hiện mọi affiliation); `OnPossess` chạy `BehaviorTreeAsset`. `OnTargetPerceptionUpdated` (public): chỉ phản ứng với pawn `IsPlayerControlled()`; thấy → set Blackboard `TargetActor` + `SetFocus`; mất dấu → clear + `ClearFocus` + `StopFiring`. Test: `TTT.AI.Perception.*`.
- `II_Scorable2` (file `I_Scorable2.h`) — interface với `AddScore()` (`BlueprintNativeEvent`). Tên chưa đúng chuẩn UE; hỏi Minh trước khi đổi tên vì Blueprint có thể đang tham chiếu.
- `ACheckpoint` (`Private/`) — Box trigger: actor nào implement `II_Scorable2` đi vào thì gọi `Execute_AddScore`, rồi broadcast `ItemPickedUp`. Logic overlap đang viết trong header.
- `APickupBase` (`Public/`, Abstract) — base cho pickup: sphere overlap → `TryPickup(Actor)`; chỉ tiêu thụ khi `ApplyPickup()` (virtual) trả `true`; ẩn + tắt collision, respawn sau `RespawnTime`; `OnRespawn` là `BlueprintNativeEvent` (mặc định gọi `FinishRespawn`).
- `AHealthPickup` (`Public/`) — con của `APickupBase`: `Cast<AShooterCharacter>` → `Heal(HealAmount)`. Blueprint: `Studentwork/MyHealthPickup`.
- `AShieldPickup` (`Public/`) — con của `APickupBase`: `FindComponentByClass<UShooterArmourComponent>()` → `AddArmour(ArmourAmount)` nếu chưa đầy. Test: `TTT.Pickup.*`.
- `AShooterCharacter` (`Variant_Shooter/`) — player, từ template, **đã sửa**: có `UShooterArmourComponent`; thêm `Heal()`, `GetCurrentHP()`, `GetMaxHP()`. `UCLASS(abstract)` → test phải spawn `BP_ShooterCharacter`.
- `UShooterArmourComponent` (`Variant_Shooter/`) — armour (tuần 4): `MitigateDamage`, `AddArmour`, getter, `StartingArmour` (khởi tạo trong `BeginPlay`), delegate `OnArmourChanged` → `AShooterPlayerController` → UI.
- `UBTTask_Shoot` (`BTTask_Shoot.h`) — BT task "Shoot": đọc `TargetActor` từ Blackboard → `AEnemyCharacter::StartFiring`, trả `Succeeded` ngay; `AbortTask` → `StopFiring`. Dùng trong `BT_SimpleEnemy` (asset Blueprint).
- `AShooterNPCSpawner` (`Variant_Shooter/AI/`) — từ template, **đã sửa**: spawn `AEnemyCharacter` (`NPCClass`), lắng nghe `OnPawnDeath` để respawn.
- `AFirstActor`, `AMyActor` — actor thử nghiệm, chưa có logic.

Class dự kiến thêm (cập nhật khi thay đổi):
- `ATTTGameState` — thời gian còn lại, điểm
- `UTTTHealthComponent` — máu, sát thương
- `<...>`

## 5. Quy ước code UE5

- Theo [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine): tiền tố `A` / `U` / `F` / `E` / `I`, `bIsSomething` cho bool, PascalCase.
- Dùng `TObjectPtr<>` cho con trỏ UObject trong header; luôn kèm `UPROPERTY()` để GC theo dõi.
- Header chỉ forward-declare; `#include` đặt trong `.cpp`. File `.generated.h` phải là include cuối cùng trong header.
- Ưu tiên Component và delegate hơn kế thừa sâu.
- Không đặt logic nặng trong `Tick`. Ưu tiên timer (`FTimerHandle`) và event.
- Comment ngắn bằng tiếng Anh trong code. Giải thích cho Minh thì dùng tiếng Việt.

## 6. Build và test (TDD)

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
