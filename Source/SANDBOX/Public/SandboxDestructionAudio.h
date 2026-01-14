#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h" 
#include "SandboxDestructionAudio.generated.h"

struct FChaosBreakEvent;

/**
 * Component responsible for playing audio when Chaos Geometry Collection breaks.
 * Optimized to prevent audio spam.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SANDBOX_API USandboxDestructionAudio : public UActorComponent
{
    GENERATED_BODY()

public:
    USandboxDestructionAudio();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleBreakEvent(const FChaosBreakEvent& BreakEvent);

    void ShutdownAudioLogic();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> BreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MinTimeBetweenSounds = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxAudioDuration = 10.0f;

private:
    float LastSoundTime = 0.0f;
    FTimerHandle ShutdownTimerHandle;
    bool bIsActive = false;
};