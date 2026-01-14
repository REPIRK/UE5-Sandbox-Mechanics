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

    // Мягкая ссылка на класс (Blueprint), чтобы не грузить его сразу
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Config")
    TSoftClassPtr<AActor> ActorClassToSpawn;

    // Мягкая ссылка на меш для "Призрака"
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> GhostMesh;

    // Стандартное здоровье для новых предметов
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Stats")
    float DefaultHealth = 500.0f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};