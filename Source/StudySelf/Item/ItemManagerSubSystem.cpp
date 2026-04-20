// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemManagerSubSystem.h"

#include "ItemDataAsset.h"
#include "ItemDataStructure.h"
#include "ItemObject.h"

FItemDataRow UItemManagerSubSystem::GetTableDataByID(FName RowName, bool& bOutSuccess)
{
	bOutSuccess = false;
	if (ItemDataTable)
	{
		FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT(""));
		if (Row)
		{
			bOutSuccess = true;
			return *Row;
		}
	}
	return FItemDataRow();
}

TSoftObjectPtr<UPaperSprite> UItemManagerSubSystem::GetSpriteByRowName(FName RowName)
{
	bool bSuccess;
	FItemDataRow Row = GetTableDataByID(RowName, bSuccess);
    
	if (bSuccess && Row.ItemDataAsset)
	{
		return Row.ItemDataAsset->Icon; 
	}
    
	return nullptr;
}

void UItemManagerSubSystem::AddInventoryItemByRowName(FName RowName, int32 Amount)
{
	bool bSuccess;
	FItemDataRow Row = GetTableDataByID(RowName, bSuccess);
    if (!bSuccess) return;


	// 1. 기존 인벤토리에 있는지 확인
	UItemObject* TargetItem = FindInventoryItemObject(RowName);

	if (TargetItem)
	{
		// 2. 있다면 수량 증가 (MaxStack 체크 로직은 나중에 추가 가능)
		TargetItem->CurrentStack += Amount;

		UE_LOG(LogTemp, Warning, TEXT("Exist"));
	} 
	else
	{
		// 3. 없다면 새로 생성
		TargetItem = NewObject<UItemObject>(this);
		TargetItem->ItemID = RowName;
		TargetItem->CurrentStack += Amount;
        
		// ItemObject 내부에 초기화 함수가 있다고 가정
		// TargetItem->Initialize(RowName, Amount, Row.ItemDataAsset->ActionClass);
        
		InventoryMap.Add(RowName, TargetItem);
		UE_LOG(LogTemp, Warning, TEXT("New Item Object"));
        
		// 획득 액션 실행 (여기서 UI 팝업 등을 트리거)
		// ExecuteAcquireAction(TargetItem);
	}

    OnInventoryUpdated.Broadcast(RowName);
}

UItemObject* UItemManagerSubSystem::FindInventoryItemObject(FName RowName)
{
	if (InventoryMap.Contains(RowName))
	{
		return InventoryMap[RowName];
	}
	return nullptr;
}

void UItemManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 이미 채워져 있다면 중복 실행 방지
	if (ItemMapConcrete.Num() > 0) return;

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is missing!"));
		return;
	}

	TArray<FItemDataRow*> AllRows;
	ItemDataTable->GetAllRows<FItemDataRow>(TEXT("GetAllRows"), AllRows);

	for (FItemDataRow* Row : AllRows)
	{
		if (Row)
		{
			// UItemObject 생성 (Outer를 서브시스템으로 설정)
			UItemObject* NewItem = NewObject<UItemObject>(this);
			NewItem->ItemID = Row->ItemName;
			ItemMapConcrete.Add(Row->ItemName, NewItem);
		}
	}
}
