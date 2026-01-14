#include "SandboxItemData.h"

FPrimaryAssetId USandboxItemData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("SandboxItemData", GetFName());
}