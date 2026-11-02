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
	
	UFUNCTION()
	void OnTargetPerCeptionUpdated(AActor* Actor, FAIStimulus Stimulus);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
