# DevLog — TTT (GDEV50010 A1)

> Nhật ký dữ kiện: đã làm gì, test nào fail/pass, lỗi gì, sửa ra sao. Phần "Suy nghĩ / phản tư" Minh tự viết — dùng làm tư liệu cho A2.

## Mẫu cho mỗi mục

```
## YYYY-MM-DD — <tính năng>
**Đã làm:**
**Test (TDD):** tên test — RED (fail vì...) → GREEN (pass sau khi...) → REFACTOR
**Lỗi gặp & cách sửa:**
**Quyết định thiết kế:** (chọn gì, bỏ gì, vì sao)
**Suy nghĩ / phản tư (Minh tự viết):**
```

---

## 2026-11-13 — Rà soát code & test cho timer time-attack

**Đã làm:**
- Rà soát các class tự viết trong `Source/TTT/`, cập nhật mô tả trong `CLAUDE.md`.
- Thêm `Docs/MakingATimer.pptx` (slide môn học). `AFPSGameMode` đã làm theo đúng các bước trong slide: delegate `OnTimerUpdated`, `GameDuration`/`RemainingTime`, `FTimerHandle` lặp 1s, `UpdateTimer()`, `HandleGameLoss()`.
- Tạo `Source/TTT/Tests/FPSGameModeTimerTest.cpp` với 4 automation test.

**Test (TDD):**

| Test | RED | GREEN |
|---|---|---|
| `TTT.Timer.StartsAtDuration` | — (code từ slide đã đúng) | Pass |
| `TTT.Timer.CountsDownOneSecondPerTick` | — (code từ slide đã đúng) | Pass |
| `TTT.Timer.EndsRoundAtZero` | Compile fail: chưa có `bIsRoundOver`. Thêm stub thì fail runtime: `Expected 'Round is over' to be true` | Pass sau khi `HandleGameLoss()` set `bIsRoundOver = true` |
| `TTT.Timer.NeverGoesNegativeWithFractionalDuration` | Fail: `GameDuration = 2.5` → `RemainingTime = -0.5` | Pass sau khi clamp `FMath::Max(RemainingTime - 1, 0)` |

- REFACTOR: constructor dùng `RemainingTime = GameDuration` thay cho số `60.0f` hard-code. Chạy lại 4/4 pass.

**Lỗi gặp & cách sửa:**
- **Bug trong game code:** nếu `GameDuration` không phải số nguyên thì timer đếm xuống số âm, UI nhận giá trị âm. Sửa bằng clamp.
- **Lỗi của test harness (không phải lỗi game):**
  1. `FTestWorldWrapper::TickTestWorld()` không làm timer chạy → chuyển sang gọi thẳng `World->GetTimerManager().Tick()`.
  2. `FTimerManager` chỉ tick 1 lần mỗi frame → test phải `++GFrameCounter` trước mỗi tick.
  3. Timer mới tạo ở trạng thái *pending*, tick đầu chỉ kích hoạt → thêm một tick 0s sau khi spawn.
  4. Tick đúng 1.0s chưa làm timer fire (thời gian phải vượt qua mốc hết hạn) → tick 1.01s.
- Chạy test từ dòng lệnh: kết quả không in ra console, phải dùng `-abslog=<file>` rồi đọc log.

**Quyết định thiết kế:**
- Thêm `bIsRoundOver` (public, `BlueprintReadOnly`) để test kiểm tra được trạng thái thua, và UI Blueprint cũng đọc được. Cách khác: kiểm tra game có bị pause không, nhưng test world không có PlayerController nên không kiểm tra được.
- Test chạy timer thật trong test world, thay vì đổi `UpdateTimer()` sang public để gọi tay. Như vậy test được cả việc `SetTimer`/`ClearTimer`, đổi lại phải tự xử lý các chi tiết của `FTimerManager` ở trên.

**Suy nghĩ / phản tư (Minh tự viết):**
- 

---

## Việc còn mở (từ lần rà soát code)

- [ ] Hai GameMode (`AFPSGameMode` timer, `ATTTGameMode` checkpoint) — level chỉ dùng được một → gộp?
- [ ] `AEnemyCharacter`: `CurrentHP = MaxHP` trong constructor → Blueprint đổi `MaxHP` thì không có tác dụng. Chuyển sang `BeginPlay` (viết test trước).
- [ ] Hệ thống điểm: `AddScore` / `OnItemPickedUp` mới chỉ `UE_LOG`. Dự kiến `ATTTGameState`.
- [ ] `AHealthPickup` cộng armour (`UShooterArmourComponent`), chưa cộng máu.
- [ ] AI: perception chỉ log, chưa ghi vào Blackboard. Cần xác nhận Behavior Tree (Blueprint) có gọi `StartFiring` không.
- [ ] Điều kiện thắng của time-attack: chưa có.

---

## 2026-11-13 — Editor lag / "Video memory has been exhausted"

**Vấn đề:** Timer chạy đúng trong PIE (HUD đếm 60 → 0), nhưng Editor rất lag và báo *"Video memory has been exhausted (498.741 MB over budget)"*, kèm cảnh báo texture streaming pool over budget.

**Dữ kiện máy:** RAM 16 GB (lúc kiểm tra còn trống ~2.4 GB), GPU RTX 3050 Laptop **4 GB VRAM**, CPU i5-11400H (6 nhân / 12 luồng). Lúc chưa Play, riêng Editor đã chiếm 2.6 GB VRAM (đo bằng `nvidia-smi`).

**Chẩn đoán:**
- `Content/` chỉ nặng ~150 MB (texture lớn nhất ~20 MB, của Mannequin template) → không phải do texture của project.
- `stat unit` trong PIE (sau khi đã tắt Ray Tracing): **VRam 3.78 GB / 3.29 GB**, GPU Time 148 ms, Frame 220 ms, chỉ 197 draw call / 192K primitive.
- → Scene rất nhẹ nhưng GPU chậm: nguyên nhân là **tràn VRAM** (dữ liệu bị đẩy sang RAM hệ thống qua PCIe), do các tính năng render cao cấp (Ray Tracing, Lumen, Virtual Shadow Maps) bật mặc định trong template.
- Cảnh báo texture pool là hệ quả: UE tự thu nhỏ pool theo lượng VRAM còn trống.

**Các bước đã sửa (theo thứ tự, tất cả trong `Config/DefaultEngine.ini`):**

| # | Setting | Trước | Sau | Kết quả |
|---|---|---|---|---|
| 1 | `r.RayTracing`, `r.RayTracing.RayTracingProxies.ProjectEnabled` | True | False | Vẫn tràn VRAM (số đo ở trên) |
| 1 | `[SystemSettings] r.Streaming.PoolSize` | mặc định | 1000 | Không hết cảnh báo (pool vẫn bị ép theo VRAM trống) |
| 2 | `r.Shadow.Virtual.Enable` | 1 | 0 | Chưa đo riêng |
| 3 | `r.DynamicGlobalIlluminationMethod` | 1 (Lumen) | 0 (None) | |
| 3 | `r.ReflectionMethod` | 1 (Lumen) | 2 (SSR) | |
| 3 | `r.ScreenPercentage` | ~86% (tự động) | 70 | |
| 3 | `r.Editor.Viewport.ScreenPercentageMode.RealTime` / `r.Editor.Viewport.ScreenPercentage` | tự động | 0 (manual) / 70 | |

**Kết quả cuối:** Editor và PIE **chạy mượt**, hết báo tràn VRAM (Minh xác nhận 2026-11-13).
- Số đo sau khi sửa (`stat unit`): VRam ___ / GPU Time ___ ms / Frame ___ ms ← <Minh điền khi đo lại, để so với 3.78 GB / 148 ms / 220 ms>

**Đánh đổi:**
- Không còn ánh sáng gián tiếp (GI) → góc khuất tối hơn; nếu tối quá thì tăng intensity Sky Light / Directional Light trong level.
- Phản chiếu chỉ còn SSR (mất phản chiếu khi vật nằm ngoài màn hình).
- Bóng đổ dùng Shadow Maps thường → bóng ở xa kém sắc nét hơn.
- Render ở 70% độ phân giải → hình hơi mềm hơn.

**Quyết định thiết kế:** ưu tiên Editor chạy ổn trên máy dev (4 GB VRAM) hơn là chất lượng hình ảnh, vì A1 chấm phần C++/Blueprint và gameplay time-attack, không chấm đồ họa. Phát hiện quan trọng: template First Person UE 5.8 bật sẵn Ray Tracing + Lumen + VSM, quá nặng cho GPU laptop 4 GB.

**Ghi chú cho sau này:**
- **Quay lại cấu hình cũ:** `git checkout Config/DefaultEngine.ini` (file gốc đang ở commit `42abafc`), hoặc đổi từng dòng trong bảng trên về cột "Trước". Mở lại Editor sau khi đổi.
- **Thử nhanh không sửa file:** gõ cvar vào console trong PIE (vd. `r.ScreenPercentage 85`) — mất khi tắt Editor, tiện để so sánh.
- **Nếu lag trở lại:** còn có thể tắt `r.GenerateMeshDistanceFields` (Lumen đã tắt nên distance field gần như không còn dùng, mà vẫn tốn VRAM); hoặc giảm `r.ScreenPercentage` xuống 60.
- **Khi play test:** chọn *Selected Viewport* thay vì *New Editor Window*, đóng các tab Blueprint/Material trước khi Play, tắt Firefox/Discord/Steam để có thêm RAM.
- **Khi đóng gói bản nộp:** các setting này cũng áp dụng cho bản build. Nếu máy chấm mạnh hơn có thể bật lại Lumen, nhưng phải test lại trên máy đó.
- **Chạy test bằng dòng lệnh** (`UnrealEditor-Cmd.exe`) trong lúc Editor đang mở sẽ tốn thêm RAM và VRAM. Nên dùng Session Frontend trong Editor, hoặc tắt Editor trước khi chạy.

**Suy nghĩ / phản tư (Minh tự viết):**
- 

---

## 2026-11-13 — Enemy AI: Perception → Blackboard → Behaviour Tree → Shoot (XONG — PIE thành công)

**Nguồn:** `Docs/Making the AI Controller Class.pptx`, `Docs/Making The Behaviour Tree and Getting The Enemies to Attack.pptx`, `Docs/Editing NPC Spawner and Setting Up Everthing In Editor.pptx`.

**So với slide — trạng thái trước khi làm:**
- AI Controller (`ATestAIController`): đã có Sight perception + `OnPossess` chạy BT. `OnTargetPerceptionUpdated` chỉ `UE_LOG`, tên hàm viết sai (`OnTargetPerCeption...`).
- NPC Spawner: Minh đã đổi `NPCClass` → `TSubclassOf<AEnemyCharacter>` và `SpawnNPC()` spawn `AEnemyCharacter`.
- Chưa có: ghi Blackboard trong perception, `UBTTask_Shoot`.

**Đã làm (C++):**
- `Source/TTT/BTTask_Shoot.h/.cpp` — task "Shoot": đọc `TargetActor` từ Blackboard → `AEnemyCharacter::StartFiring`; `AbortTask` → `StopFiring`.
- `AEnemyCharacter::GetWeaponTargetLocation()` nhắm vào `CurrentAimTarget` (trước đây luôn bắn thẳng 1000 unit phía trước).
- `ShooterNPCSpawner.h`: forward-declare `AEnemyCharacter` thay vì `#include` trong header (theo coding standard); xóa include `ShooterNPC.h` thừa.
- `ATestAIController::OnTargetPerceptionUpdated`: đổi tên cho đúng chính tả (`PerCeption` → `Perception`), chuyển sang `public` để test gọi được; chỉ phản ứng với pawn `IsPlayerControlled()`; thấy → `SetValueAsObject(TargetActor)` + `SetFocus`; mất dấu → `ClearValue` + `ClearFocus` + `StopFiring()`. Thêm `UPROPERTY TargetActorKey` (mặc định "TargetActor") thay cho chuỗi hard-code.

**Khác với slide (và vì sao):**
1. Nhận diện player: slide dùng `ActorHasTag("Player")`; code dùng `APawn::IsPlayerControlled()`. Lý do: Sight đang detect mọi affiliation nên enemy nhìn thấy nhau; tag phụ thuộc việc nhớ gắn trong Blueprint. `IsPlayerControlled()` luôn đúng dù player là BP nào. (Gắn tag "Player" theo slide vẫn vô hại.)
2. Slide gọi `FinishLatentTask(...)` rồi `return Succeeded` trong `ExecuteTask` → kết thúc task 2 lần. `FinishLatentTask` chỉ dùng khi task trả về `InProgress`. Code chỉ `return Succeeded`.
3. Task kết thúc ngay nên `AbortTask` gần như không được gọi → enemy bắn mãi sau khi mất dấu player. Sửa: controller gọi `StopFiring()` khi xóa `TargetActor`.

**Test (TDD) — `Source/TTT/Tests/TestAIControllerPerceptionTest.cpp`:**

| Test | RED | GREEN |
|---|---|---|
| `TTT.AI.Perception.SetsTargetWhenPlayerSeen` | Compile fail (hàm sai tên + protected) → handler rỗng: `TargetActor is the player: not equal`, `AI focuses the player: not equal` | Pass |
| `TTT.AI.Perception.ClearsTargetWhenPlayerLost` | **Pass sai** với handler rỗng (target chưa từng được set nên "đã xóa") → thêm precondition "target phải được set trước" | Pass |
| `TTT.AI.Perception.IgnoresNonPlayerActors` | Handler rỗng: `Player target kept: not equal` | Pass |

- Test dựng Blackboard bằng code (`UBlackboardData::UpdatePersistentKey<UBlackboardKeyType_Object>("TargetActor")`) và tạo `FAIStimulus` giả (`SensingSucceeded` / `SensingFailed`) để gọi thẳng handler, không cần perception thật.
- `UBTTask_Shoot` không có automation test (cần BT chạy thật) → kiểm tra bằng PIE.
- Không cần sửa `TTT.Build.cs`: `AIModule` đã public-depend `GameplayTasks` (cần cho `UBTTaskNode`).
- Kết quả cuối: **7/7 test pass** (`TTT.AI.*` 3 + `TTT.Timer.*` 4).

**Lỗi gặp & cách sửa (test harness):**
1. `UseBlackboard()` trả về `false` → test world chưa có AI system. Sửa: `World->CreateAISystem()` trong fixture.
2. Blackboard asset tạo bằng code phải gọi `UpdateParentKeys()` + `UpdateKeyIDs()` (asset .uasset tự làm trong `PostLoad`).
3. **RED đầu tiên không đáng tin**: lúc đó fixture còn hỏng (Blackboard null) nên test fail vì fixture chứ chưa chắc vì logic. Sau khi sửa fixture, chạy lại RED bằng cách tạm vô hiệu hóa handler → fail đúng lý do → khôi phục code → GREEN.
4. Lần đầu tạm vô hiệu hóa bằng `return;` bị compiler báo *unreachable code* (C4702 là error) → build fail mà test vẫn chạy trên **binary cũ** và báo pass. Bài học: luôn kiểm tra `Result: Succeeded` của build trước khi tin kết quả test.
5. `ClearsTargetWhenPlayerLost` pass ngay cả khi handler không làm gì → test yếu. Thêm precondition để test chỉ pass khi logic thật sự chạy.

**Checklist Editor (Minh làm — Claude không mở được .uasset):**
- [ ] `BB_SimpleEnemy`: key `TargetActor`, kiểu Object, Base Class = Actor (tên key phải đúng y hệt)
- [ ] `BT_SimpleEnemy`: Root → Selector
  - [ ] Nhánh trái: Sequence + Blackboard decorator (`TargetActor` **Is Set**, Observer Aborts = Both) → MoveTo (key `TargetActor`) → Shoot
  - [ ] Nhánh phải: Wait + Blackboard decorator (`TargetActor` **Is Not Set**, Observer Aborts = Both)
  - [ ] Behaviour Tree Details: Blackboard Asset = `BB_SimpleEnemy`
- [ ] `BP_TestAIController` (Blueprint con của `TestAIController`): Behavior Tree Asset = `BT_SimpleEnemy`; kiểm tra AI Sight Config (Radius 1500, Lose Sight 2000, 90°, detect Enemies/Neutrals/Friendlies)
- [ ] `BP_EnemyCharacter`: mesh + Anim Class giống ShooterNPC; Weapon Class = vũ khí của NPC; Auto Possess AI = Placed in World or Spawned; AI Controller Class = `BP_TestAIController`
- [ ] Player BP (ShooterCharacter): tag "Player" (theo slide, không bắt buộc với code hiện tại)
- [ ] Level: NavMeshBoundsVolume phủ hết level, nhấn **P** thấy màu xanh
- [ ] Level: đặt `BP_ShooterNPCSpawner`, NPC Class = `BP_EnemyCharacter`
- [ ] PIE: enemy đứng chờ → thấy player thì đuổi + bắn → mất dấu thì ngừng bắn, quay về Wait

**Kết quả PIE:** xem "PIE lần 1" và "PIE lần 2" bên dưới.

**Suy nghĩ / phản tư (Minh tự viết):**
- 

### PIE lần 1 — Enemy AI (2026-11-13)

**Quan sát:** enemy spawn và đuổi theo player, nhưng chỉ bắn khi đã áp sát; player không có súng.

**Nguyên nhân 1 — enemy chỉ bắn khi áp sát (2 lỗi chồng nhau):**
- BT: Sequence chạy *Move To → Shoot* theo thứ tự trái→phải; Move To chỉ xong khi tới sát target (Acceptable Radius mặc định nhỏ) → Shoot chỉ chạy lúc đó. (Slide dựng cây theo thứ tự này.)
- C++: `AEnemyCharacter::OnSemiWeaponRefire()` để trống `{}` (copy từ interface). Súng semi-auto (`bFullAuto = false`) bắn 1 phát mỗi lần `StartFiring`, rồi gọi `OnSemiWeaponRefire()` để chủ súng quyết định bắn tiếp → enemy chỉ bắn 1 phát mỗi vòng lặp BT. `AShooterNPC` của template thì có gọi lại `Weapon->StartFiring()`.

**Đã sửa:**
- C++: `OnSemiWeaponRefire()` gọi lại `Weapon->StartFiring()` khi vẫn còn `CurrentAimTarget` (`StopFiring()` xóa target → ngừng). Không có automation test (cần súng thật với mesh/projectile) → kiểm bằng PIE.
- BT (Minh làm): đổi thứ tự thành *Shoot → Move To*, tăng Acceptable Radius của Move To → enemy bắn trong lúc tiến lại và dừng ở khoảng cách bắn.

**Nguyên nhân 2 — player không có súng:**
- Level `Lvl_FirstPerson` dùng `Studentwork/MyFPSGameMode` (parent `AFPSGameMode`) với pawn `BP_ShooterCharacter` → nhân vật *có thể* cầm súng, nhưng chỉ nhận súng khi chạm `BP_ShooterPickup`; level chưa có pickup nào.
- Sửa (Minh làm): đặt `BP_ShooterPickup` vào level, chọn Weapon Type trong `DT_WeaponData`.

**Kết quả PIE lần 2 (2026-11-13):** ✅ Thành công — enemy bắn ngay khi thấy player trong lúc tiến lại; player nhặt súng từ `BP_ShooterPickup` và bắn được. <Minh bổ sung chi tiết/ảnh chụp nếu muốn>
