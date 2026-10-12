// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "I_Scorable2.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Checkpoint.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndGameSignature,FString, EndMessage);
class UBoxComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemPickedUp);

UCLASS()
class ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckpoint();

	UPROPERTY(EditAnywhere)
	UBoxComponent* BoxCollider;
	
	UFUNCTION()
	void CheckpointOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		
	
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("Checkpoint Overlapped"));
		
		if (OtherActor)
		{
			if (UKismetSystemLibrary::DoesImplementInterface(OtherActor,  UI_Scorable2::StaticClass()))
			{
				II_Scorable2::Execute_AddScore(OtherActor);
			}
		}
		
		if (ItemPickedUp.IsBound())
		{
			ItemPickedUp.Broadcast();
		}
	}
	UPROPERTY(BlueprintAssignable)
	FItemPickedUp ItemPickedUp;
	

protected:
	UFUNCTION()
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
