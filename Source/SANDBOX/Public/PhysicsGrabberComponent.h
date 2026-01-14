#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "PhysicsGrabberComponent.generated.h"

// 1. Enum для состояний прицела
UENUM(BlueprintType)
enum class EGrabState : uint8
{
    Idle        UMETA(DisplayName = "Idle (Dot)"),
    CanGrab     UMETA(DisplayName = "Can Grab (Open Hand)"),
    Holding     UMETA(DisplayName = "Holding (Closed Hand)")
};

// 2. Делегат для уведомления UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabStateChanged, EGrabState, NewState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SANDBOX_API UPhysicsGrabberComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsGrabberComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ToggleGrab();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ReleaseObject();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ChangeHoldDistance(float AxisValue);

    // --- STATE FOR UI ---

    // Событие, на которое подпишется BP_FlyCam
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnGrabStateChanged OnGrabStateChanged;

    // Текущее состояние для чтения
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EGrabState CurrentState;

    // --- CONFIG ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float TraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MinHoldDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FName LiftableTag = "LiftableTag";

    // --- PHYSICS SETTINGS (Твои настройки) ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SpringStiffness = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SpringDamping = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> GrabVFXTemplate;

private:
    UPROPERTY()
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> ActiveVFX;

    bool bIsHolding;
    float CurrentHoldDistance;

    // Твои переменные для пружины
    FVector CurrentTargetLocation;
    FVector CurrentTargetVelocity;
    FRotator RotationOffset;

    bool GetPlayerViewPoint(FVector& OutLoc, FVector& OutDir, FRotator& OutRot) const;

    // Новая функция для проверки (чтобы не писать спагетти в BP)
    void UpdateTraceState();
};