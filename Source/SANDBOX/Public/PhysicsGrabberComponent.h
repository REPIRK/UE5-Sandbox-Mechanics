#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "PhysicsGrabberComponent.generated.h"

// Enum representing the current interaction state for the UI/Crosshair
UENUM(BlueprintType)
enum class EGrabState : uint8
{
    Idle        UMETA(DisplayName = "Idle (Dot)"),
    CanGrab     UMETA(DisplayName = "Can Grab (Open Hand)"),
    Holding     UMETA(DisplayName = "Holding (Closed Hand)")
};

// Delegate to notify UI about state changes
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

    // --- INTERACTION API ---

    /** Tries to grab an object or release it if already holding. */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ToggleGrab();

    /** Releases the currently held object. */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ReleaseObject();

    /** Adjusts the distance of the held object (Mouse Wheel). */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ChangeHoldDistance(float AxisValue);

    // --- EVENTS ---

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnGrabStateChanged OnGrabStateChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EGrabState CurrentState;

    // --- CONFIGURATION ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float TraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MinHoldDistance = 100.0f;

    /** Objects with this tag or component tag can be lifted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FName LiftableTag = "LiftableTag";

    // --- PHYSICS SETTINGS ---

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

    // Physics interpolation state
    FVector CurrentTargetLocation;
    FVector CurrentTargetVelocity;
    FRotator RotationOffset;

    /** Updates the trace logic to determine if we can grab something. */
    void UpdateTraceState();

    /** Helper to get player camera data. */
    bool GetPlayerViewPoint(FVector& OutLoc, FVector& OutDir, FRotator& OutRot) const;
};