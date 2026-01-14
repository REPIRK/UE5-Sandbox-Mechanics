#include "SandboxUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h" 
#include "SandboxSaveGame.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

// =========================================================================
// SECTION: BUILDING & MATH
// =========================================================================

bool USandboxUtils::CalculatePlacementTransform(
    const UObject* WorldContextObject,
    const FVector CameraLocation,
    const FVector CameraForward,
    float TraceDistance,
    float GridSize,
    bool bAlignToNormal,
    float AdditionalYaw,
    FTransform& OutTransform)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraForward * TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    FQuat YawRotation(FVector::UpVector, FMath::DegreesToRadians(AdditionalYaw));

    if (bHit)
    {
        FVector FinalLocation = HitResult.Location;
        FQuat FinalRotation = FQuat::Identity;

        if (GridSize > 0.0f)
        {
            FinalLocation.X = FMath::GridSnap(FinalLocation.X, GridSize);
            FinalLocation.Y = FMath::GridSnap(FinalLocation.Y, GridSize);
        }

        if (bAlignToNormal)
        {
            FinalRotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).ToQuat();
        }

        FinalRotation = FinalRotation * YawRotation;

        OutTransform.SetLocation(FinalLocation);
        OutTransform.SetRotation(FinalRotation);
        OutTransform.SetScale3D(FVector(1.0f));
        return true;
    }

    FVector FloatingLocation = TraceStart + (CameraForward * (TraceDistance * 0.5f));
    if (GridSize > 0.0f)
    {
        FloatingLocation.X = FMath::GridSnap(FloatingLocation.X, GridSize);
        FloatingLocation.Y = FMath::GridSnap(FloatingLocation.Y, GridSize);
        FloatingLocation.Z = FMath::GridSnap(FloatingLocation.Z, GridSize);
    }

    OutTransform.SetLocation(FloatingLocation);
    OutTransform.SetRotation(YawRotation);
    OutTransform.SetScale3D(FVector(1.0f));

    return false;
}

bool USandboxUtils::IsPlacementValid(
    const UObject* WorldContextObject,
    const FVector ItemLocation,
    const FQuat ItemRotation,
    UStaticMeshComponent* MeshComponent,
    AActor* IgnoredActor)
{
    if (!WorldContextObject || !MeshComponent || !MeshComponent->GetStaticMesh()) return true;

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    FVector Min, Max;
    MeshComponent->GetLocalBounds(Min, Max);
    FVector LocalCenter = (Min + Max) * 0.5f;
    FVector BoxExtent = (Max - Min) * 0.5f;
    BoxExtent *= 0.95f;

    FVector WorldCenter = ItemLocation + ItemRotation.RotateVector(LocalCenter);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(IgnoredActor);
    if (IgnoredActor && IgnoredActor->GetOwner())
    {
        QueryParams.AddIgnoredActor(IgnoredActor->GetOwner());
    }

    bool bHit = World->OverlapBlockingTestByChannel(
        WorldCenter,
        ItemRotation,
        ECC_Visibility,
        FCollisionShape::MakeBox(BoxExtent),
        QueryParams
    );

    return !bHit;
}

// =========================================================================
// SECTION: SETTINGS & OPTIMIZATION
// =========================================================================

void USandboxUtils::ApplyGraphicsQuality(int32 QualityLevel)
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    Settings->SetOverallScalabilityLevel(QualityLevel);

    if (IConsoleVariable* CVarPathTracing = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PathTracing")))
    {
        CVarPathTracing->Set(0, ECVF_SetByCode);
    }

    if (IConsoleVariable* CVarLumenHWRT = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing")))
    {
        int32 HWRTValue = (QualityLevel >= 2) ? 1 : 0;
        CVarLumenHWRT->Set(HWRTValue, ECVF_SetByCode);
    }

    Settings->ApplySettings(false);
}

void USandboxUtils::SetScreenMode(int32 Width, int32 Height, int32 Mode)
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    Settings->SetScreenResolution(FIntPoint(Width, Height));

    EWindowMode::Type WindowMode = EWindowMode::WindowedFullscreen;
    switch (Mode)
    {
    case 0: WindowMode = EWindowMode::Fullscreen; break;
    case 1: WindowMode = EWindowMode::WindowedFullscreen; break;
    case 2: WindowMode = EWindowMode::Windowed; break;
    }
    Settings->SetFullscreenMode(WindowMode);
    Settings->ApplySettings(false);
}

void USandboxUtils::SetUpscalingMode(int32 Mode)
{
    static IConsoleVariable* CVarScreenPerc = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    if (!CVarScreenPerc) return;

    float TargetPercentage = 100.0f;
    switch (Mode)
    {
    case 0: TargetPercentage = 100.0f; break;
    case 1: TargetPercentage = 66.6f;  break;
    case 2: TargetPercentage = 58.0f;  break;
    case 3: TargetPercentage = 50.0f;  break;
    default: TargetPercentage = 100.0f; break;
    }
    CVarScreenPerc->Set(TargetPercentage, ECVF_SetByCode);
}

int32 USandboxUtils::GetCurrentUpscalingMode()
{
    static IConsoleVariable* CVarScreenPerc = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    if (!CVarScreenPerc) return 0;

    float CurrentVal = CVarScreenPerc->GetFloat();
    if (CurrentVal >= 99.0f) return 0;
    else if (CurrentVal >= 60.0f) return 1;
    else if (CurrentVal >= 55.0f) return 2;
    else return 3;
}

void USandboxUtils::SetVSyncEnabled(bool bEnabled)
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    Settings->SetVSyncEnabled(bEnabled);
    Settings->ApplySettings(false);
}

void USandboxUtils::SetFrameRateLimit(float Limit)
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    Settings->SetFrameRateLimit(Limit);
    Settings->ApplySettings(false);
}

// =========================================================================
// SECTION: TIME & AUDIO FIX
// =========================================================================

void USandboxUtils::SetGameSpeed(
    const UObject* WorldContextObject,
    float Speed,
    USoundMix* SoundMix,
    USoundClass* MusicClass,
    float CurrentMusicVolume, // <--- »—ѕќЋ№«”≈ћ
    USoundClass* SFXClass,
    float CurrentSFXVolume)   // <--- »—ѕќЋ№«”≈ћ
{
    float ClampedSpeed = FMath::Clamp(Speed, 0.1f, 2.0f);
    UGameplayStatics::SetGlobalTimeDilation(WorldContextObject, ClampedSpeed);

    if (!SoundMix) return;

    float SFXPitch = 1.0f;
    float MusicPitch = 1.0f;

    if (ClampedSpeed < 1.0f)
    {
        SFXPitch = FMath::GetMappedRangeValueClamped(FVector2D(0.1f, 1.0f), FVector2D(0.4f, 1.0f), ClampedSpeed);
        MusicPitch = FMath::GetMappedRangeValueClamped(FVector2D(0.1f, 1.0f), FVector2D(0.75f, 1.0f), ClampedSpeed);
    }
    else
    {
        SFXPitch = FMath::GetMappedRangeValueClamped(FVector2D(1.0f, 2.0f), FVector2D(1.0f, 1.4f), ClampedSpeed);
        MusicPitch = FMath::GetMappedRangeValueClamped(FVector2D(1.0f, 2.0f), FVector2D(1.0f, 1.1f), ClampedSpeed);
    }

    if (SFXClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            WorldContextObject,
            SoundMix,
            SFXClass,
            CurrentSFXVolume, // <--- —юда ставим реальную громкость, а не 1.0f
            SFXPitch,
            0.5f,
            true
        );
    }

    if (MusicClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            WorldContextObject,
            SoundMix,
            MusicClass,
            CurrentMusicVolume, // <--- —юда ставим реальную громкость
            MusicPitch,
            0.5f,
            true
        );
    }

    UGameplayStatics::PushSoundMixModifier(WorldContextObject, SoundMix);
}

void USandboxUtils::SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* SoundClass, USoundMix* SoundMix, float Volume)
{
    if (!SoundClass || !SoundMix) return;

    UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, SoundClass, Volume, 1.0f, 0.0f, true);
    UGameplayStatics::PushSoundMixModifier(WorldContextObject, SoundMix);
}
void USandboxUtils::GetSavedAudioSettings(float& OutMusicVolume, float& OutSFXVolume)
{
    // «начени€ по умолчанию (если файла нет)
    OutMusicVolume = 1.0f;
    OutSFXVolume = 1.0f;

    // »м€ слота должно совпадать с тем, что мы использовали в меню (Settings)
    FString SlotName = TEXT("Settings");

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, 0);

        // ѕытаемс€ привести к нашему классу
        if (USandboxSaveGame* MySave = Cast<USandboxSaveGame>(LoadedSave))
        {
            OutMusicVolume = MySave->MusicVolume;
            OutSFXVolume = MySave->SFXVolume;
        }
    }
}

// language
void USandboxUtils::SetLanguage(FString CultureCode)
{
    if (FInternationalization::Get().SetCurrentCulture(CultureCode))
    {
        // ќчищаем кэш шрифтов и текстов, чтобы UI обновилс€ мгновенно
        FTextLocalizationManager::Get().RefreshResources();
    }
}

FString USandboxUtils::GetCurrentLanguage()
{
    return FInternationalization::Get().GetCurrentCulture()->GetName();
}