#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "SandboxUtils.generated.h"

/**
 * Helper library for Sandbox mechanics, Math, System Settings and Time control.
 * Optimized for UE 5.4+
 */
UCLASS()
class SANDBOX_API USandboxUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // =========================================================================
    // SECTION: BUILDING & MATH
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Math", meta = (WorldContext = "WorldContextObject"))
    static bool CalculatePlacementTransform(
        const UObject* WorldContextObject,
        const FVector CameraLocation,
        const FVector CameraForward,
        float TraceDistance,
        float GridSize,
        bool bAlignToNormal,
        float AdditionalYaw,
        FTransform& OutTransform
    );

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Collision", meta = (WorldContext = "WorldContextObject"))
    static bool IsPlacementValid(
        const UObject* WorldContextObject,
        const FVector ItemLocation,
        const FQuat ItemRotation,
        UStaticMeshComponent* MeshComponent,
        AActor* IgnoredActor
    );

    // =========================================================================
    // SECTION: SETTINGS & OPTIMIZATION
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Settings")
    static void ApplyGraphicsQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Settings")
    static void SetScreenMode(int32 Width, int32 Height, int32 Mode);

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Settings")
    static void SetUpscalingMode(int32 Mode);

    UFUNCTION(BlueprintPure, Category = "Sandbox|Settings")
    static int32 GetCurrentUpscalingMode();

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Settings")
    static void SetVSyncEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Settings")
    static void SetFrameRateLimit(float Limit);

    // =========================================================================
    // SECTION: TIME & AUDIO
    // =========================================================================

    /**
     * Sets global time dilation AND adjusts audio pitch.
     * Prevents deep bass sound in slow motion for music.
     * NOW ACCEPTS CURRENT VOLUME TO PREVENT RESETTING IT.
     */
    UFUNCTION(BlueprintCallable, Category = "Sandbox|Time", meta = (WorldContext = "WorldContextObject"))
    static void SetGameSpeed(
        const UObject* WorldContextObject,
        float Speed,
        USoundMix* SoundMix,
        USoundClass* MusicClass,
        float CurrentMusicVolume,   // <--- NEW ARGUMENT
        USoundClass* SFXClass,
        float CurrentSFXVolume      // <--- NEW ARGUMENT
    );

    UFUNCTION(BlueprintCallable, Category = "Sandbox|Audio", meta = (WorldContext = "WorldContextObject"))
    static void SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* SoundClass, USoundMix* SoundMix, float Volume);
    UFUNCTION(BlueprintCallable, Category = "Sandbox|Audio")
    static void GetSavedAudioSettings(float& OutMusicVolume, float& OutSFXVolume);
    /**
     * Changes the current game language.
     * @param CultureCode "en", "ru", "es", "zh", etc.
     */
    UFUNCTION(BlueprintCallable, Category = "Sandbox|System")
    static void SetLanguage(FString CultureCode);

    /**
     * Gets current language code.
     */
    UFUNCTION(BlueprintPure, Category = "Sandbox|System")
    static FString GetCurrentLanguage();
};