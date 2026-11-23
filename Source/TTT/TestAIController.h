// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "TestAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

UCLASS()
class TTT_API ATestAIController : public AAIController
{
	GENERATED_BODY()

public:
	
	
	// Sets default values for this actor's properties
	ATestAIController();

protected:
	// Called when the game starts or when spawned
	virtual void OnPossess(APawn* InPawn) override;
	
	/** Lets the enemy see the player (sight radius and angle are set in C++). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	// Sight sense config; UPROPERTY so the GC keeps it alive
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Behavior Tree the enemy runs when possessed (e.g. BT_SimpleEnemy). */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	/** Blackboard key written by perception (must match BB_SimpleEnemy) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

	/** How far away the enemy can first spot the player. */
	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight", meta = (ClampMin = 0, Units = "cm"))
	float SightRadius = 1500.0f;

	/** Once spotted, the player must get this far away before the enemy loses track. Kept at least as large as Sight Radius. */
	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight", meta = (ClampMin = 0, Units = "cm"))
	float LoseSightRadius = 2000.0f;

	/** Half-angle of the enemy's view cone: 90 means it sees everything in front of it (180 degrees total). */
	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight", meta = (ClampMin = 0, ClampMax = 180, Units = "deg"))
	float SightAngleDegrees = 90.0f;

	// Applies the designer's sight values before the perception component starts sensing
	virtual void BeginPlay() override;

public:
	/** Perception callback: writes/clears the Blackboard's TargetActor. Public so automation tests can drive it. */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
