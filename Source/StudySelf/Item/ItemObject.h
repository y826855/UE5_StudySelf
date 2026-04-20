// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

/**
 * 인스턴싱 되는 아이템 클래스
 */
UCLASS(Blueprintable, BlueprintType)
class STUDYSELF_API UItemObject : public UObject
{
	GENERATED_BODY()
public:
	UItemObject();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 CurrentStack;
};
