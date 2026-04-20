// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PaperSprite.h"
#include "GoogleSheetParserBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class STUDYSELF_API UGoogleSheetParserBase : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "GoogleSheet|Parser")
	virtual void TSVDataParse(const FString& RawTSV) {}
	
protected:
	
	/** 파싱 시작: 외부에서 가져온 TSV 문자열을 쪼개서 메모리에 보관 */
	void BeginParse(const FString& RawTSV);

	/** 메모리 해제: 작업이 끝난 후 임시 배열을 비움 */
	void ReleaseParsedData();
	
	
protected:
	/** 1. 스프라이트 전용 로더 (캐스팅 편의성) */
	TSoftObjectPtr<UPaperSprite> LoadSprite(const FString& FolderPath, const FString& NameFormat, int32 ID);

	/** 2. 클래스/블루프린트 타입 로더 (아이템 사용 효과 클래스 등) */
	UClass* LoadClassResource(const FString& FolderPath, const FString& NameFormat, int32 ID);

	/** 3. 데이터 에셋 로더 */
	UPrimaryDataAsset* LoadDataAsset(const FString& FolderPath, const FString& NameFormat, int32 ID, UClass* AssetClass);

	FString MakeAssetPath(const FString& FolderPath, const FString& NameFormat, int32 ID);

protected:
	// 파싱된 데이터를 클래스 내부에서만 들고 있음 (행-열 2차원 배열)
	TArray<TArray<FString>> ParsedRows;

};
