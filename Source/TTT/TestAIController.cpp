// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyCharacter.h"


// Sets default values
ATestAIController::ATestAIController()
{
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
		
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATestAIController::OnTargetPerceptionUpdated);
}

void ATestAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BehaviorTreeAsset)
	{
		
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

// Called when the game starts or when spawned


void ATestAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (!Actor || !BBComp)
	{
		return;
	}

	// Only react to the player. Sight detects every affiliation, so enemies also see each other.
	const APawn* SeenPawn = Cast<APawn>(Actor);
	if (!SeenPawn || !SeenPawn->IsPlayerControlled())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// Setting TargetActor switches the Behaviour Tree to the chase/shoot branch
		BBComp->SetValueAsObject(TargetActorKey, Actor);
		SetFocus(Actor);
	}
	else
	{
		// Clearing it switches the tree back to the wait branch
		BBComp->ClearValue(TargetActorKey);
		ClearFocus(EAIFocusPriority::Gameplay);

		// The Shoot task finishes instantly, so AbortTask won't stop the weapon - do it here
		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn()))
		{
			Enemy->StopFiring();
		}
	}
}

// Called every frame
void ATestAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

