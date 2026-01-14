#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SandboxSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FSavedItemCompact
{
    GENERATED_BODY()

    UPROPERTY()
    int32 PaletteIndex = -1;

    UPROPERTY()
    FTransform Transform;

    /** Determines which level this item belongs to. */
    UPROPERTY()
    FString LevelName;
};

/**
 * Main SaveGame class containing player progress and world state.
 */
UCLASS()
class SANDBOX_API USandboxSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    FTransform PlayerTransform;

    /** Optimized list of asset paths to reduce file size. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TArray<FString> AssetPalette;

    /** List of all spawned items in the world. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TArray<FSavedItemCompact> Items;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 MaxUnlockedLevelIndex = 0;

    // --- SETTINGS ---

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    float MusicVolume = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    FString LanguageCode = "en";
};