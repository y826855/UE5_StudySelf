// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSlotObject.h"

#include "ItemManagerSubSystem.h"
#include "ItemDataStructure.h"
#include "ItemObject.h"

UItemSlotObject::UItemSlotObject()
{
	ItemID = NAME_None;
	ItemCount = 0; 
}

void UItemSlotObject::InitializeFromObject(UItemObject* ItemObject)
{
	if (!ItemObject) return;
	ItemID = ItemObject->ItemID;
	ItemCount = ItemObject->CurrentStack;
}

void UItemSlotObject::InitializeFromData(FName ID, int32 Count)
{
	ItemID = ID;
	ItemCount = Count; 
}

TSoftObjectPtr<UPaperSprite> UItemSlotObject::GetItemSprite() const
{
	auto ItemManager = GetItemManager();
	if (ItemManager && !ItemID.IsNone())
		return ItemManager->GetSpriteByRowName(ItemID);

	return nullptr;
}

FText UItemSlotObject::GetItemDisplayName() const
{
	auto ItemManager = GetItemManager();
	bool bIsSuccess = false;
	auto RowData = ItemManager->GetTableDataByID(ItemID, bIsSuccess);
	if (!bIsSuccess) return FText::GetEmpty();
		
	return FText::FromString(RowData.DisplayName);
}

void UItemSlotObject::UpdateSlotData(UItemObject* ItemObject)
{
	if(!ItemObject) return;
	ItemID = ItemObject->ItemID;
	ItemCount = ItemObject->CurrentStack;

	OnItemSlotUpdated.Broadcast(this);
}

UItemManagerSubSystem* UItemSlotObject::GetItemManager() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	return World->GetGameInstance()->GetSubsystem<UItemManagerSubSystem>();
}
 