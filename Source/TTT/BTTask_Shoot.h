// Behaviour Tree task: makes the possessed AEnemyCharacter fire at the Blackboard's TargetActor.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Shoot.generated.h"

/**
 * Shows up in the Behaviour Tree editor as "Shoot".
 * Reads TargetActor from the Blackboard and calls AEnemyCharacter::StartFiring.
 */
UCLASS()
class TTT_API UBTTask_Shoot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Shoot();

protected:
	/** Blackboard key holding the actor to shoot at (must match BB_SimpleEnemy) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetActorKey = TEXT("TargetActor");

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
