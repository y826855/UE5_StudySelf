// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PaperSprite.h"
#include "ItemManagerSubSystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, FName, ItemName);

UCLASS(Abstract, Blueprintable)
class STUDYSELF_API UItemManagerSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 인벤토리: <시트의 RowName, 생성된 아이템 오브젝트> */ 
	UPROPERTY(BlueprintReadOnly, Category = "Item|Inventory")
	TMap<FName, UItemObject*> ItemMapConcrete;
	
	/** 인벤토리: <시트의 RowName, 생성된 아이템 오브젝트> */ 
	UPROPERTY(BlueprintReadOnly, Category = "Item|Inventory")
	TMap<FName, UItemObject*> InventoryMap;

	
public:
	// 에디터(BP 자식)에서 테이블을 꽂아줄 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ItemDataTable;

	UPROPERTY(BlueprintAssignable, Category = "Item|Event")
	FOnInventoryUpdated OnInventoryUpdated;
	
	/** ID로 테이블의 행 데이터를 직접 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "Item|Data")
	FItemDataRow GetTableDataByID(FName RowName, bool& bOutSuccess);

	/** ID로 data asset의 스프라이트 값 얻어오기 */
	UFUNCTION(BlueprintPure, Category = "Item|Data")
	TSoftObjectPtr<UPaperSprite> GetSpriteByRowName(FName RowName);
	

	/** 아이템 인벤토리에 RowName으로 아이템을 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Item|Inventory")
	void AddInventoryItemByRowName(FName RowName, int32 Amount);

	/** 인벤토리에 해당 아이템이 있는지 찾습니다. */
	UFUNCTION(BlueprintPure, Category = "Item|Inventory")
	UItemObject* FindInventoryItemObject(FName RowName);

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
