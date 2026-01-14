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

    // НОВОЕ ПОЛЕ: К какому уровню принадлежит этот предмет
    UPROPERTY()
    FString LevelName;
};

UCLASS()
class SANDBOX_API USandboxSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform PlayerTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AssetPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSavedItemCompact> Items;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
    int32 MaxUnlockedLevelIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    float MusicVolume = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    FString LanguageCode = "en";
};