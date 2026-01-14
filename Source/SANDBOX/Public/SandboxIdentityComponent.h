#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SandboxItemData.h"
#include "SandboxIdentityComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class SANDBOX_API USandboxIdentityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USandboxIdentityComponent();

    // Ссылка на DataAsset. Если она пуста, предмет не сохранится.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<USandboxItemData> SourceItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float CurrentHealth = 100.0f;

    // Вызывать, когда игрок бьет по предмету
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void TakeDamageFromPlayer(float Amount);
};