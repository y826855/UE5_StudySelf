// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDataParser.h"

#if WITH_EDITOR 

#include "FAssetGenerationHelper.h"
#include "Item/ItemDataStructure.h"


void UItemDataParser::TSVDataParse(const FString& RawTSV)
{
	// 1. 유효성 체크 (테이블 존재 여부 확인)
	if (!TargetTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFetcher] TargetTable이 지정되지 않았습니다!"));
		return;
	}

	// 2. 파싱 준비 (부모의 ParsedRows 채우기)
	BeginParse(RawTSV);
	if (ParsedRows.Num() == 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemFetcher] 파싱된 데이터가 없습니다."));
		return;
	}

	// 3. 헬퍼를 통한 에셋 일괄 생성/로드
	// 물리적인 파일 준비는 여기서 끝납니다.
	auto DataAssetMap = FAssetGenerationHelper::GenerateAssetMap(
		ParsedRows, 
		AssetFolderPath, 
		AssetNameFormat, 
		UItemDataAsset::StaticClass()
	);

	// 4. 데이터 주입 및 테이블 채우기 (통합 단계)
	// 헬퍼가 준비해준 에셋 맵을 전달하여 데이터 초기화와 테이블 업데이트를 한 번에 수행합니다.
	FillDataTable(DataAssetMap);

	// 5. 마무리 작업 (메모리 해제 및 저장 표시)
	TargetTable->MarkPackageDirty();
	ReleaseParsedData();
    
	UE_LOG(LogTemp, Log, TEXT("[ItemFetcher] 성공: %d개의 에셋 동기화 및 테이블 주입 완료."), DataAssetMap.Num());
}

void UItemDataParser::Initalize(FString spriteFolderPath, FString spriteFileFormat, FString assetFolderPath,
	FString assetNameFormat)
{
	SpriteFolderPath = spriteFolderPath;
	SpriteFileFormat = spriteFileFormat;
	AssetFolderPath = assetFolderPath;
	AssetNameFormat = assetNameFormat;

	UE_LOG(LogTemp, Log, TEXT("초기화 완료: %s"), *AssetNameFormat);
}


void UItemDataParser::FillDataTable(const TMap<int32, UPrimaryDataAsset*>& DataAssetMap)
{
	UE_LOG(LogTemp, Log, TEXT("[ItemParser] FillDataTable 시작. 파싱된 행 개수: %d"), ParsedRows.Num());
	
	// 3. 데이터 루프 처리
	for (const TArray<FString>& Columns : ParsedRows)
	{
		// 최소한의 데이터(ID, Name, Desc, MaxStack)가 있는지 확인
		if (Columns.Num() < 4) continue;

		// 데이터 파싱
		int32 ID = FCString::Atoi(*Columns[0]);
		UE_LOG(LogTemp, Log, TEXT("[Debug] 파싱된 ID 문자열: %s -> 정수 변환: %d"), *Columns[0], ID);
		if (ID <= 0) continue; // 유효하지 않은 ID 스킵

		//이거는 prefix 참조하기
		FName RowName = FName(*FString::Printf(TEXT("Item_%d"), ID));
		UE_LOG(LogTemp, Log, TEXT("[Debug] 생성된 RowName: %s"), *RowName.ToString());
		auto DataAsset = DataAssetMap.Contains(ID) ? SetupItemAsset(DataAssetMap[ID], ID) : nullptr;
		
		// 행 데이터 생성 및 값 할당
		FItemDataRow NewRow;
		NewRow.ItemName = RowName;
		NewRow.DisplayName = Columns[1];
		NewRow.Description = Columns[2];
		NewRow.MaxStack = FCString::Atoi(*Columns[3]);
		NewRow.ItemDataAsset = DataAsset;

		// 5. 데이터 테이블에 행 추가/갱신
		TargetTable->AddRow(RowName, NewRow);
	}
}

UItemDataAsset* UItemDataParser::SetupItemAsset(UPrimaryDataAsset* InAsset, int ID)
{
	// 1. 안전한 형변환
	UItemDataAsset* ItemAsset = Cast<UItemDataAsset>(InAsset);
	if (!ItemAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemFetcher] 에셋 형변환 실패 또는 에셋이 유효하지 않음."));
		return nullptr;
	}
	
	//초기화
	ItemAsset->Icon = LoadSprite(SpriteFolderPath, SpriteFileFormat, ID);

	return ItemAsset;
}


#endif
