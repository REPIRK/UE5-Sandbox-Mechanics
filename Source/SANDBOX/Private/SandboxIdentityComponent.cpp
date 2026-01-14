#include "SandboxIdentityComponent.h"

USandboxIdentityComponent::USandboxIdentityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentHealth = 100.0f;
}

void USandboxIdentityComponent::TakeDamageFromPlayer(float Amount)
{
    if (!SourceItemData) return;

    CurrentHealth -= Amount;

    // If destroyed, nullify data to prevent saving broken objects
    if (CurrentHealth <= 0.0f)
    {
        SourceItemData = nullptr;
    }
}