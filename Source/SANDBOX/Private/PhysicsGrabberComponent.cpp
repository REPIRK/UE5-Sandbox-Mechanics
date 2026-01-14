#include "PhysicsGrabberComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

UPhysicsGrabberComponent::UPhysicsGrabberComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsHolding = false;
    CurrentState = EGrabState::Idle; // Старт с Idle
    CurrentHoldDistance = 200.0f;
    SpringStiffness = 40.0f;
    SpringDamping = 4.0f;
}

void UPhysicsGrabberComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        PhysicsHandle = Owner->FindComponentByClass<UPhysicsHandleComponent>();
        if (PhysicsHandle)
        {
            // Твои настройки хэндла
            PhysicsHandle->LinearDamping = 200.0f;
            PhysicsHandle->LinearStiffness = 1500.0f;
            PhysicsHandle->AngularDamping = 10.0f;
            PhysicsHandle->AngularStiffness = 1500.0f;
            PhysicsHandle->InterpolationSpeed = 50.0f;
        }
    }
}

void UPhysicsGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 1. Сначала обновляем UI (проверяем, на что смотрим)
    UpdateTraceState();

    // 2. Если не держим — выходим, физику не считаем
    if (!bIsHolding || !PhysicsHandle) return;

    if (!PhysicsHandle->GetGrabbedComponent())
    {
        ReleaseObject();
        return;
    }

    FVector CameraLoc, CameraDir;
    FRotator CameraRot;
    if (!GetPlayerViewPoint(CameraLoc, CameraDir, CameraRot)) return;

    // --- ТВОЯ РАБОЧАЯ МАТЕМАТИКА ---
    FVector FinalPosition = CameraLoc + (CameraDir * CurrentHoldDistance);

    FVector Displacement = FinalPosition - CurrentTargetLocation;
    FVector SpringForce = Displacement * SpringStiffness;
    FVector DampingForce = CurrentTargetVelocity * SpringDamping;
    FVector Acceleration = SpringForce - DampingForce;

    CurrentTargetVelocity += Acceleration * DeltaTime;
    CurrentTargetLocation += CurrentTargetVelocity * DeltaTime;

    FRotator TargetRotation = CameraRot + RotationOffset;

    PhysicsHandle->SetTargetLocationAndRotation(CurrentTargetLocation, TargetRotation);
}

void UPhysicsGrabberComponent::UpdateTraceState()
{
    EGrabState NewState = EGrabState::Idle;

    // 1. Если держим — состояние Holding
    if (bIsHolding)
    {
        NewState = EGrabState::Holding;
    }
    else
    {
        // 2. Если не держим — пускаем луч, чтобы узнать, можно ли взять
        FVector CamLoc, CamDir;
        FRotator CamRot;
        if (GetPlayerViewPoint(CamLoc, CamDir, CamRot))
        {
            FVector TraceEnd = CamLoc + (CamDir * TraceDistance);
            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(GetOwner());

            bool bHit = GetWorld()->LineTraceSingleByChannel(
                Hit, CamLoc, TraceEnd, ECC_Visibility, Params
            );

            if (bHit && Hit.GetComponent())
            {
                // Проверяем тег и физику
                bool bHasTag = Hit.GetComponent()->ComponentHasTag(LiftableTag) || Hit.GetActor()->ActorHasTag(LiftableTag);
                if (bHasTag && Hit.GetComponent()->IsSimulatingPhysics())
                {
                    NewState = EGrabState::CanGrab;
                }
            }
        }
    }

    // 3. Если состояние изменилось — уведомляем UI
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        if (OnGrabStateChanged.IsBound())
        {
            OnGrabStateChanged.Broadcast(CurrentState);
        }
    }
}

void UPhysicsGrabberComponent::ToggleGrab()
{
    if (bIsHolding)
    {
        ReleaseObject();
        return;
    }

    FVector CamLoc, CamDir;
    FRotator CamRot;
    if (!GetPlayerViewPoint(CamLoc, CamDir, CamRot)) return;

    FVector TraceEnd = CamLoc + (CamDir * TraceDistance);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_Visibility, Params
    );

    if (bHit && Hit.GetComponent())
    {
        // Твои проверки
        bool bHasTag = Hit.GetComponent()->ComponentHasTag(LiftableTag) || Hit.GetActor()->ActorHasTag(LiftableTag);

        if (bHasTag && Hit.GetComponent()->IsSimulatingPhysics())
        {
            if (!PhysicsHandle) return;

            CurrentHoldDistance = (Hit.Location - CamLoc).Size();
            CurrentHoldDistance = FMath::Clamp(CurrentHoldDistance, MinHoldDistance, TraceDistance);

            CurrentTargetLocation = Hit.GetComponent()->GetComponentLocation();
            CurrentTargetVelocity = FVector::ZeroVector;
            RotationOffset = Hit.GetComponent()->GetComponentRotation() - CamRot;

            PhysicsHandle->GrabComponentAtLocationWithRotation(
                Hit.GetComponent(),
                NAME_None,
                Hit.Location,
                Hit.GetComponent()->GetComponentRotation()
            );

            if (GrabVFXTemplate)
            {
                ActiveVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    GrabVFXTemplate,
                    Hit.GetComponent(),
                    NAME_None,
                    FVector::ZeroVector,
                    FRotator::ZeroRotator,
                    EAttachLocation::KeepRelativeOffset,
                    true
                );
            }

            bIsHolding = true;
            // Принудительно обновляем состояние, чтобы UI мгновенно отреагировал
            UpdateTraceState();
        }
    }
}

void UPhysicsGrabberComponent::ReleaseObject()
{
    if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();
        PhysicsHandle->ReleaseComponent();
    }

    if (ActiveVFX)
    {
        ActiveVFX->DestroyComponent();
        ActiveVFX = nullptr;
    }

    bIsHolding = false;
    // Принудительно обновляем состояние
    UpdateTraceState();
}

void UPhysicsGrabberComponent::ChangeHoldDistance(float AxisValue)
{
    if (!bIsHolding) return;
    CurrentHoldDistance += AxisValue * 25.0f;
    CurrentHoldDistance = FMath::Clamp(CurrentHoldDistance, MinHoldDistance, TraceDistance);
}

bool UPhysicsGrabberComponent::GetPlayerViewPoint(FVector& OutLoc, FVector& OutDir, FRotator& OutRot) const
{
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        if (AController* C = Pawn->GetController())
        {
            if (APlayerController* PC = Cast<APlayerController>(C))
            {
                PC->GetPlayerViewPoint(OutLoc, OutRot);
                OutDir = OutRot.Vector();
                return true;
            }
        }
    }
    return false;
}