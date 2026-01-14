#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SandboxItemData.generated.h"

UENUM(BlueprintType)
enum class ESandboxItemCategory : uint8
{
    Construction    UMETA(DisplayName = "Construction"),
    Furniture       UMETA(DisplayName = "Furniture"),
    Destructible    UMETA(DisplayName = "Destructible"),
    Misc            UMETA(DisplayName = "Misc")
};

/**
 * Primary Data Asset representing a spawnable item.
 * Uses Soft References for memory optimization.
 */
UCLASS(BlueprintType, Blueprintable)
class SANDBOX_API USandboxItemData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Config")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Config")
    TObjectPtr<UTexture2D> Thumbnail;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Config")
    ESandboxItemCategory Category;

    /** Soft Ref to Actor Class to avoid hard loading. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Config")
    TSoftClassPtr<AActor> ActorClassToSpawn;

    /** Soft Ref to Static Mesh for ghost preview. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> GhostMesh;

    /** Default Health for physics objects. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Stats")
    float DefaultHealth = 500.0f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};