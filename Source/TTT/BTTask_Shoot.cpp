// Behaviour Tree task: makes the possessed AEnemyCharacter fire at the Blackboard's TargetActor.

#include "BTTask_Shoot.h"
#include "AIController.h"
#include "EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Shoot::UBTTask_Shoot()
{
	NodeName = TEXT("Shoot");
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyCharacter* EnemyNPC = Cast<AEnemyCharacter>(AIController->GetPawn());
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!EnemyNPC || !BBComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	EnemyNPC->StartFiring(TargetActor);

	// Instant task: return Succeeded so the tree keeps looping (MoveTo -> Shoot).
	// No FinishLatentTask here - that is only for tasks that returned InProgress.
	return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_Shoot::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (AEnemyCharacter* EnemyNPC = Cast<AEnemyCharacter>(AIController->GetPawn()))
		{
			EnemyNPC->StopFiring();
		}
	}
	return Super::AbortTask(OwnerComp, NodeMemory);
}
