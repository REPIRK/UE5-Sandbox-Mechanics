#include "SandboxDestructionAudio.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GeometryCollection/GeometryCollectionComponent.h" 
#include "Chaos/ChaosGameplayEventDispatcher.h"

USandboxDestructionAudio::USandboxDestructionAudio()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USandboxDestructionAudio::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UGeometryCollectionComponent* GeometryCollection = Owner->FindComponentByClass<UGeometryCollectionComponent>();
    if (GeometryCollection)
    {
        GeometryCollection->SetNotifyBreaks(true);
        GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &USandboxDestructionAudio::HandleBreakEvent);
    }
}

void USandboxDestructionAudio::HandleBreakEvent(const FChaosBreakEvent& BreakEvent)
{
    if (!IsValid(this) || !GetWorld()) return;
    if (!BreakSound) return;

    // Timer logic to stop listening after major destruction
    if (!bIsActive)
    {
        bIsActive = true;

        FTimerDelegate TimerDel;
        TimerDel.BindUObject(this, &USandboxDestructionAudio::ShutdownAudioLogic);

        GetWorld()->GetTimerManager().SetTimer(
            ShutdownTimerHandle,
            TimerDel,
            MaxAudioDuration,
            false
        );
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSoundTime < MinTimeBetweenSounds)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(this, BreakSound, BreakEvent.Location);
    LastSoundTime = CurrentTime;
}

void USandboxDestructionAudio::ShutdownAudioLogic()
{
    if (!IsValid(this)) return;

    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;

    UGeometryCollectionComponent* GeometryCollection = Owner->FindComponentByClass<UGeometryCollectionComponent>();

    if (GeometryCollection && GeometryCollection->IsValidLowLevel())
    {
        GeometryCollection->OnChaosBreakEvent.RemoveDynamic(this, &USandboxDestructionAudio::HandleBreakEvent);
        GeometryCollection->SetNotifyBreaks(false);
    }
}