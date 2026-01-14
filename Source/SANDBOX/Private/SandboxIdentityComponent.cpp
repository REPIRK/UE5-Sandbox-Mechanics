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

    // Если здоровье кончилось, удаляем ссылку на данные.
    // Это сигнал для системы сохранений: "Не сохраняй этот мусор".
    if (CurrentHealth <= 0.0f)
    {
        SourceItemData = nullptr;
    }
}