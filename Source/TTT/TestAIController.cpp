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
	// Perception and the Behaviour Tree are event-driven: nothing to do every frame
	PrimaryActorTick.bCanEverTick = false;
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = SightAngleDegrees;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
		
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATestAIController::OnTargetPerceptionUpdated);
}

void ATestAIController::BeginPlay()
{
	// Blueprint defaults are applied after the constructor, so copy them into the sense config here.
	// Done before Super::BeginPlay so the perception component starts with the right values.
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = FMath::Max(LoseSightRadius, SightRadius);
	SightConfig->PeripheralVisionAngleDegrees = SightAngleDegrees;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	Super::BeginPlay();
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

