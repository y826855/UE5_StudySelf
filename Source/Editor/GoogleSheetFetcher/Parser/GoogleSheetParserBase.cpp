// Fill out your copyright notice in the Description page of Project Settings.


#include "GoogleSheetParserBase.h"

void UGoogleSheetParserBase::BeginParse(const FString& RawTSV)
{
	ReleaseParsedData();

	if (RawTSV.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[Parser] 전달된 TSV 데이터가 비어있습니다."));
		return;
	}

	// [수정 포인트 1] \r (Carriage Return) 제거
	// 윈도우 스타일의 줄바꿈(\r\n)을 유닉스 스타일(\n)로 통일합니다.
	FString CleanTSV = RawTSV.Replace(TEXT("\r"), TEXT(""));

	TArray<FString> RowStrings;
	// 줄바꿈 기준 분리
	CleanTSV.ParseIntoArray(RowStrings, TEXT("\n"), true);

	// 로그를 찍어서 행이 몇 개로 나뉘었는지 확인해 보세요.
	UE_LOG(LogTemp, Log, TEXT("[Parser] 전체 행 개수(헤더 포함): %d"), RowStrings.Num());

	for (int32 i = 1; i < RowStrings.Num(); ++i) 
	{
		TArray<FString> Columns;
		// 탭 기준 분리
		RowStrings[i].ParseIntoArray(Columns, TEXT("\t"), false);

		if (Columns.Num() > 0)
		{
			ParsedRows.Add(Columns);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[Parser] %d개의 데이터 로드 완료."), ParsedRows.Num());
}

void UGoogleSheetParserBase::ReleaseParsedData()
{
	if (ParsedRows.Num() > 0)
	{
		ParsedRows.Empty();
		UE_LOG(LogTemp, Log, TEXT("[Parser] 임시 데이터 메모리가 해제되었습니다."));
	}
}

// 1. 스프라이트 로더
TSoftObjectPtr<UPaperSprite> UGoogleSheetParserBase::LoadSprite(const FString& FolderPath, const FString& NameFormat, int32 ID)
{
	// 1. 기본 경로 생성 (예: /Game/Items/S_Item_1)
	FString BasePath = MakeAssetPath(FolderPath, NameFormat, ID - 1);
    
	// 2. 에셋 이름 추출 (마지막 / 뒤의 문자열)
	FString AssetName;
	int32 LastSlashIndex;
	if (BasePath.FindLastChar('/', LastSlashIndex))
	{
		AssetName = BasePath.RightChop(LastSlashIndex + 1);
	}

	// 3. 전체 경로 완성: /Game/Items/S_Item_1.S_Item_1
	FString FullPathWithExtension = FString::Printf(TEXT("%s.%s"), *BasePath, *AssetName);

	// 4. SoftObjectPath 생성 시 이 전체 경로를 사용
	return TSoftObjectPtr<UPaperSprite>(FSoftObjectPath(*FullPathWithExtension));
}

// 2. 데이터 에셋 로더
UPrimaryDataAsset* UGoogleSheetParserBase::LoadDataAsset(const FString& FolderPath, const FString& NameFormat, int32 ID, UClass* AssetClass)
{
	if (!AssetClass) return nullptr;

	FString FullPath = MakeAssetPath(FolderPath, NameFormat, ID);
	return Cast<UPrimaryDataAsset>(StaticLoadObject(AssetClass, nullptr, *FullPath));
}

// 3. 블루프린트 클래스 로더
UClass* UGoogleSheetParserBase::LoadClassResource(const FString& FolderPath, const FString& NameFormat, int32 ID)
{
	FString FullPath = MakeAssetPath(FolderPath, NameFormat, ID);

	// 블루프린트 클래스는 메모리 상에서 참조할 때 경로 끝에 _C가 붙어야 함
	if (!FullPath.EndsWith(TEXT("_C")))
	{
		FullPath += TEXT("_C");
	}

	// 클래스 로딩은 StaticLoadClass를 사용
	return StaticLoadClass(UObject::StaticClass(), nullptr, *FullPath);
}

FString UGoogleSheetParserBase::MakeAssetPath(const FString& FolderPath, const FString& NameFormat, int32 ID)
{
	// {0} 포맷을 지원하는 언리얼 방식 경로 조립
	FString FileName = FString::Format(*NameFormat, { ID });
	return FPaths::Combine(*FolderPath, *FileName);
}
