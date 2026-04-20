// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Editor/GoogleSheetFetcher/Parser/GoogleSheetParserBase.h"
#include "Item/ItemDataAsset.h"
#include "ItemDataParser.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class STUDYSELFEDITOR_API UItemDataParser : public UGoogleSheetParserBase
{
	GENERATED_BODY()

public:

	
	virtual void TSVDataParse(const FString& RawTSV) override;

	UFUNCTION(BlueprintCallable, Category = "Config")
	void Initalize(FString spriteFolderPath, FString spriteFileFormat, FString assetFolderPath, FString assetNameFormat);
	
protected:

	/** 작업 대상이 될 데이터 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UDataTable* TargetTable;

	/** 스프라이트 리소스가 위치한 폴더 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SpriteFolderPath = TEXT("/Game/Icons/");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SpriteFileFormat = TEXT("Item_Icon");
	
	/** 데이터 에셋 로드 경로 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Path")
	FString AssetFolderPath = TEXT("/Game/Items/DataAssets/");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Path")
	FString AssetNameFormat = TEXT("DA_Item_{0}");
	
private:
	void FillDataTable(const TMap<int32, UPrimaryDataAsset*>& DataAssetMap);
	UItemDataAsset* SetupItemAsset(UPrimaryDataAsset* InAsset, int ID);
};
