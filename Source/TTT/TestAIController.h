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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI ")
	UAIPerceptionComponent* AIPerceptionComp;
	
	UAISenseConfig_Sight* SightConfig;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard key written by perception (must match BB_SimpleEnemy) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

public:
	/** Perception callback: writes/clears the Blackboard's TargetActor. Public so automation tests can drive it. */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
