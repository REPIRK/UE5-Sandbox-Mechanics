#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SandboxSaveGame.h"
#include "SandboxItemData.h"
#include "SandboxWorldManager.generated.h"

/**
 * Manages async loading/saving of world state.
 * Implements Time-Sliced processing to prevent frame drops during mass spawning.
 */
UCLASS()
class SANDBOX_API ASandboxWorldManager : public AActor
{
    GENERATED_BODY()

public:
    ASandboxWorldManager();
    virtual void Tick(float DeltaTime) override;

    // Time budget per frame for spawning (seconds). Default 5ms.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    double MaxFrameTimeBudget = 0.005;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSystem")
    FString SaveSlotName = "SandboxSave01";

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void SaveWorld();

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void LoadWorld();

    UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
    void OnLoadingCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
    void OnLoadingProgress(float Percentage);

private:
    bool bIsLoading = false;
    int32 CurrentLoadIndex = 0;

    UPROPERTY()
    TObjectPtr<USandboxSaveGame> CachedSaveGame;

    // Caches to optimize async loading lookup
    UPROPERTY()
    TMap<int32, UClass*> ClassCache;

    UPROPERTY()
    TMap<int32, USandboxItemData*> DataAssetCache;
};