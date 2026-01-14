#include "SandboxWorldManager.h"
#include "SandboxIdentityComponent.h" 
#include "SandboxItemData.h"          
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASandboxWorldManager::ASandboxWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    // ОПТИМИЗАЦИЯ: Выключаем тик на старте. Включаем только при загрузке.
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASandboxWorldManager::SaveWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 1. Получаем имя текущего уровня (для разделения сохранений по картам)
    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

    // 2. Пытаемся загрузить существующий файл, чтобы НЕ стереть данные других уровней
    USandboxSaveGame* SaveInst = nullptr;
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        SaveInst = Cast<USandboxSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    }

    // Если файла нет, создаем новый
    if (!SaveInst)
    {
        SaveInst = Cast<USandboxSaveGame>(UGameplayStatics::CreateSaveGameObject(USandboxSaveGame::StaticClass()));
    }

    if (!SaveInst) return;

    // --- ОЧИСТКА СТАРЫХ ДАННЫХ ЭТОГО УРОВНЯ ---
    // Удаляем только записи текущего уровня. Остальные уровни остаются.
    SaveInst->Items.RemoveAll([&CurrentLevelName](const FSavedItemCompact& Item)
        {
            return Item.LevelName == CurrentLevelName;
        });

    /*
       УБРАНО: Сохранение позиции игрока.
       Теперь позиция определяется PlayerStart на уровне.
    */

    // --- СБОР НОВЫХ ПРЕДМЕТОВ ---
    // Восстанавливаем таблицу палитры
    TMap<FString, int32> PaletteLookup;
    for (int32 i = 0; i < SaveInst->AssetPalette.Num(); i++)
    {
        PaletteLookup.Add(SaveInst->AssetPalette[i], i);
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor)) continue;

        USandboxIdentityComponent* Identity = Actor->FindComponentByClass<USandboxIdentityComponent>();

        // Сохраняем, если есть Identity и ссылка на DataAsset
        if (Identity && Identity->SourceItemData)
        {
            FString AssetPath = Identity->SourceItemData->GetPathName();
            int32 PaletteIndex = -1;

            if (int32* FoundIdx = PaletteLookup.Find(AssetPath))
            {
                PaletteIndex = *FoundIdx;
            }
            else
            {
                PaletteIndex = SaveInst->AssetPalette.Add(AssetPath);
                PaletteLookup.Add(AssetPath, PaletteIndex);
            }

            FSavedItemCompact CompactItem;
            CompactItem.PaletteIndex = PaletteIndex;
            CompactItem.Transform = Actor->GetActorTransform();

            // Записываем, какому уровню принадлежит предмет
            CompactItem.LevelName = CurrentLevelName;

            SaveInst->Items.Add(CompactItem);
        }
    }

    // Сохраняем прогресс уровней (если он был загружен ранее, он останется)
    // Если нужно обновить MaxUnlockedLevelIndex, это делается в BP_WinZone

    UGameplayStatics::SaveGameToSlot(SaveInst, SaveSlotName, 0);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Saved items for level: %s"), *CurrentLevelName));
    }
}

void ASandboxWorldManager::LoadWorld()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0)) return;

    CachedSaveGame = Cast<USandboxSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!CachedSaveGame) return;

    /*
       УБРАНО: Загрузка позиции игрока.
       Игрок останется на месте спавна.
    */

    UWorld* World = GetWorld();
    if (!World) return;

    // --- ОЧИСТКА МИРА ---
    // Удаляем старые построенные предметы перед загрузкой
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor->FindComponentByClass<USandboxIdentityComponent>())
        {
            Actor->Destroy();
        }
    }

    ClassCache.Empty();
    DataAssetCache.Empty();
    CurrentLoadIndex = 0;

    // Если файл пуст, просто завершаем
    if (CachedSaveGame->Items.Num() == 0)
    {
        bIsLoading = false;
        OnLoadingCompleted();
        return;
    }

    // Включаем Тик для асинхронной загрузки
    bIsLoading = true;
    SetActorTickEnabled(true);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Starting Async Load..."));
    }
}

void ASandboxWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsLoading || !CachedSaveGame)
    {
        SetActorTickEnabled(false);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);
    int32 TotalItems = CachedSaveGame->Items.Num();

    if (TotalItems == 0)
    {
        bIsLoading = false;
        OnLoadingCompleted();
        SetActorTickEnabled(false);
        return;
    }

    double StartTime = FPlatformTime::Seconds();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // --- ЦИКЛ ЗАГРУЗКИ ---
    while (CurrentLoadIndex < TotalItems)
    {
        // Лимит времени на кадр
        if ((FPlatformTime::Seconds() - StartTime) > MaxFrameTimeBudget)
        {
            break;
        }

        // Защита от выхода за пределы массива
        if (!CachedSaveGame->Items.IsValidIndex(CurrentLoadIndex))
        {
            break;
        }

        const FSavedItemCompact& ItemData = CachedSaveGame->Items[CurrentLoadIndex];

        // --- ФИЛЬТР: СПАВНИМ ТОЛЬКО ПРЕДМЕТЫ ТЕКУЩЕГО УРОВНЯ ---
        if (ItemData.LevelName == CurrentLevelName)
        {
            int32 PIndex = ItemData.PaletteIndex;

            if (CachedSaveGame->AssetPalette.IsValidIndex(PIndex))
            {
                UClass* ClassToSpawn = nullptr;
                USandboxItemData* SourceData = nullptr;

                // Проверка кэша
                if (UClass** FoundClass = ClassCache.Find(PIndex))
                {
                    ClassToSpawn = *FoundClass;
                    SourceData = *DataAssetCache.Find(PIndex);
                }
                else
                {
                    FString AssetPath = CachedSaveGame->AssetPalette[PIndex];

                    if (!AssetPath.IsEmpty())
                    {
                        SourceData = Cast<USandboxItemData>(StaticLoadObject(USandboxItemData::StaticClass(), nullptr, *AssetPath));
                        // Проверка на валидность DataAsset и Класса
                        if (SourceData && !SourceData->ActorClassToSpawn.IsNull())
                        {
                            ClassToSpawn = SourceData->ActorClassToSpawn.LoadSynchronous();
                            if (ClassToSpawn)
                            {
                                ClassCache.Add(PIndex, ClassToSpawn);
                                DataAssetCache.Add(PIndex, SourceData);
                            }
                        }
                    }
                }

                // Спавн
                if (ClassToSpawn && SourceData)
                {
                    AActor* NewActor = World->SpawnActor<AActor>(ClassToSpawn, ItemData.Transform, SpawnParams);
                    if (NewActor)
                    {
                        USandboxIdentityComponent* Identity = NewActor->FindComponentByClass<USandboxIdentityComponent>();
                        if (!Identity)
                        {
                            Identity = NewObject<USandboxIdentityComponent>(NewActor);
                            Identity->RegisterComponent();
                        }

                        Identity->SourceItemData = SourceData;
                        Identity->CurrentHealth = 100.0f;
                    }
                }
            }
        }

        CurrentLoadIndex++;
    }

    // Прогресс
    float Percent = (TotalItems > 0) ? (float)CurrentLoadIndex / (float)TotalItems : 1.0f;
    OnLoadingProgress(Percent);

    // Завершение
    if (CurrentLoadIndex >= TotalItems)
    {
        bIsLoading = false;
        CachedSaveGame = nullptr;
        ClassCache.Empty();
        DataAssetCache.Empty();

        OnLoadingCompleted();
        SetActorTickEnabled(false); // Выключаем тик для оптимизации

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Async Load Completed!"));
        }
    }
}