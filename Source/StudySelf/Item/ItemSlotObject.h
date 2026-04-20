// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PaperSprite.h"
#include "ItemSlotObject.generated.h"

/**
 * 아이템 슬롯 데이터를 위한 클래스. 가시용 오브젝트라 보면 될듯
 */

class UItemManagerSubSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSlotUpdated, UItemSlotObject*, SlotObject);

UCLASS(BlueprintType, Blueprintable)
class STUDYSELF_API UItemSlotObject : public UObject
{
	GENERATED_BODY()
public:
	UItemSlotObject();

	UFUNCTION(BlueprintCallable, Category = "Item UI")
	void InitializeFromObject(UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable, Category = "Item UI")
	void InitializeFromData(FName ID, int32 Count);

public:
	/** 1. 핵심 식별자: 아이템 매니저에서 데이터를 찾을 Key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ExposeOnSpawn = true))
	FName ItemID;

	/** 2. 수량: 이 슬롯에 쌓여있는 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ExposeOnSpawn = true))
	int32 ItemCount;

public:
	/** 3. 위젯 편의용 함수: 아이템의 스프라이트 반환 */
	UFUNCTION(BlueprintPure, Category = "Item UI")
	TSoftObjectPtr<UPaperSprite> GetItemSprite() const;

	/** 4. 위젯 편의용 함수: 아이템의 이름 반환 */
	UFUNCTION(BlueprintPure, Category = "Item UI")
	FText GetItemDisplayName() const;

	/** 5. 데이터 업데이트용 함수. */
	UFUNCTION(BlueprintCallable, Category = "Item UI")
	void UpdateSlotData(UItemObject* ItemObject);

public:
	/** 블루프린트에서 'Assign'하거나 'Bind'할 수 있는 이벤트 디스패처 */
	UPROPERTY(BlueprintAssignable, Category = "Item Event")
	FOnItemSlotUpdated OnItemSlotUpdated;
	
private:
	UItemManagerSubSystem* GetItemManager() const;
};
