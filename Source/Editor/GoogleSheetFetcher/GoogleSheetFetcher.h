// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
#include "GoogleSheetFetcher.generated.h"

/**
 * 구글 시트 데이터 TSV로 받아오는 클래스 
 */

// 성공/실패 시 결과를 블루프린트에 전달하기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSheetDataReceived, bool, bSuccess, const FString&, ResponseString);

UCLASS(BlueprintType)
class STUDYSELFEDITOR_API UGoogleSheetFetcher : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	// 블루프린트에서 호출할 비동기 함수
	/**
	 * @param SheetID  시트의 고유 ID
	 * @param PageID   시트 탭의 gid (기본값 "0")
	 * @param Range    가져올 데이터 범위 (예: "A1:G100")
	 */
	UFUNCTION(BlueprintCallable, Category = "GoogleSheet")
	void FetchSheetDataByParts(FString SheetID, FString PageID, FString Range);

	// 완료 시 응답을 보낼 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "GoogleSheet")
	FOnSheetDataReceived OnDataReceived;

private:
	// 실제 응답이 왔을 때 호출될 내부 콜백 함수
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

};

