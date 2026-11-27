// Test helper: records UFeedbackComponent::OnFeedbackPlayed broadcasts.
// Dynamic delegates can only bind to a UFUNCTION on a UObject, so tests need this small class.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FeedbackComponent.h"
#include "FeedbackTestRecorder.generated.h"

UCLASS(Transient)
class UFeedbackTestRecorder : public UObject
{
	GENERATED_BODY()

public:
	TArray<EFeedbackEvent> Events;

	UFUNCTION()
	void Record(EFeedbackEvent Event) { Events.Add(Event); }
};
