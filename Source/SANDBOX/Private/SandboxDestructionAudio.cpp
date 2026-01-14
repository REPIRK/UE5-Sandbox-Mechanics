#include "SandboxDestructionAudio.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GeometryCollection/GeometryCollectionComponent.h" 
#include "Chaos/ChaosGameplayEventDispatcher.h" // Для типов событий

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
    // SAFETY CHECK 1: Проверяем, жив ли компонент и мир
    if (!IsValid(this) || !GetWorld()) return;

    // SAFETY CHECK 2: Если звука нет, нечего делать
    if (!BreakSound) return;

    // Таймер отключения (оптимизация)
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

    // Воспроизводим звук в точке удара
    UGameplayStatics::PlaySoundAtLocation(this, BreakSound, BreakEvent.Location);
    LastSoundTime = CurrentTime;
}

void USandboxDestructionAudio::ShutdownAudioLogic()
{
    // SAFETY CHECK 3: Проверка перед отключением
    if (!IsValid(this)) return;

    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;

    UGeometryCollectionComponent* GeometryCollection = Owner->FindComponentByClass<UGeometryCollectionComponent>();

    // Проверяем, жив ли компонент физики
    if (GeometryCollection && GeometryCollection->IsValidLowLevel())
    {
        GeometryCollection->OnChaosBreakEvent.RemoveDynamic(this, &USandboxDestructionAudio::HandleBreakEvent);
        GeometryCollection->SetNotifyBreaks(false);
    }
}