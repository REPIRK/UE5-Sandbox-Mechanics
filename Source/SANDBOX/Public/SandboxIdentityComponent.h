#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SandboxItemData.h"
#include "SandboxIdentityComponent.generated.h"

/**
 * Component identifying an actor as part of the Sandbox Save System.
 * Also handles health/destruction logic.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class SANDBOX_API USandboxIdentityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USandboxIdentityComponent();

    /** Reference to the source DataAsset. Required for saving. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<USandboxItemData> SourceItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float CurrentHealth = 100.0f;

    /** Called when player deals damage to this object. */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void TakeDamageFromPlayer(float Amount);
};