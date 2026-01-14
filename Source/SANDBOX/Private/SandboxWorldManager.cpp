#include "SandboxWorldManager.h"
#include "SandboxIdentityComponent.h" 
#include "SandboxItemData.h"          
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASandboxWorldManager::ASandboxWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    // OPTIMIZATION: Tick disabled by default. Enabled only during loading.
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASandboxWorldManager::SaveWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

    // 1. Try to load existing save to preserve data from other levels
    USandboxSaveGame* SaveInst = nullptr;
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        SaveInst = Cast<USandboxSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    }

    // Create new if invalid
    if (!SaveInst)
    {
        SaveInst = Cast<USandboxSaveGame>(UGameplayStatics::CreateSaveGameObject(USandboxSaveGame::StaticClass()));
    }

    if (!SaveInst) return;

    // --- CLEANUP ---
    // Remove items belonging to the current level before rewriting
    SaveInst->Items.RemoveAll([&CurrentLevelName](const FSavedItemCompact& Item)
        {
            return Item.LevelName == CurrentLevelName;
        });

    // --- COLLECTION ---
    // Reconstruct palette lookup
    TMap<FString, int32> PaletteLookup;
    for (int32 i = 0; i < SaveInst->AssetPalette.Num(); i++)
    {
        PaletteLookup.Add(SaveInst->AssetPalette[i], i);
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor)) continue;

        // Only save actors with Identity component
        USandboxIdentityComponent* Identity = Actor->FindComponentByClass<USandboxIdentityComponent>();

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
            CompactItem.LevelName = CurrentLevelName;

            SaveInst->Items.Add(CompactItem);
        }
    }

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

    UWorld* World = GetWorld();
    if (!World) return;

    // --- CLEANUP SCENE ---
    // Destroy existing constructed items
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

    if (CachedSaveGame->Items.Num() == 0)
    {
        bIsLoading = false;
        OnLoadingCompleted();
        return;
    }

    // Enable tick to start time-sliced loading
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

    // --- TIME-SLICED LOOP ---
    while (CurrentLoadIndex < TotalItems)
    {
        // Check frame budget
        if ((FPlatformTime::Seconds() - StartTime) > MaxFrameTimeBudget)
        {
            break;
        }

        if (!CachedSaveGame->Items.IsValidIndex(CurrentLoadIndex))
        {
            break;
        }

        const FSavedItemCompact& ItemData = CachedSaveGame->Items[CurrentLoadIndex];

        // Filter: Spawn only items for current level
        if (ItemData.LevelName == CurrentLevelName)
        {
            int32 PIndex = ItemData.PaletteIndex;

            if (CachedSaveGame->AssetPalette.IsValidIndex(PIndex))
            {
                UClass* ClassToSpawn = nullptr;
                USandboxItemData* SourceData = nullptr;

                // Check Cache first
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
                        // Load Data Asset
                        SourceData = Cast<USandboxItemData>(StaticLoadObject(USandboxItemData::StaticClass(), nullptr, *AssetPath));

                        // Resolve Soft Class Ptr
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

                // Spawn
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
                        Identity->CurrentHealth = SourceData->DefaultHealth;
                    }
                }
            }
        }

        CurrentLoadIndex++;
    }

    // Report Progress
    float Percent = (TotalItems > 0) ? (float)CurrentLoadIndex / (float)TotalItems : 1.0f;
    OnLoadingProgress(Percent);

    // Completion
    if (CurrentLoadIndex >= TotalItems)
    {
        bIsLoading = false;
        CachedSaveGame = nullptr;
        ClassCache.Empty();
        DataAssetCache.Empty();

        OnLoadingCompleted();
        SetActorTickEnabled(false);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Async Load Completed!"));
        }
    }
}