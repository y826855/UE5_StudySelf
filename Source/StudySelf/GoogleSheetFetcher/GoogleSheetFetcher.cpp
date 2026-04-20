// Fill out your copyright notice in the Description page of Project Settings.


#include "GoogleSheetFetcher.h"

#if WITH_EDITOR 

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UGoogleSheetFetcher::FetchSheetDataByParts(FString SheetID, FString PageID, FString Range)
{
	if (SheetID.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[HTTP] SheetID가 비어있습니다."));
		return;
	}

	// 1. 기본 베이스 URL 조립 (TSV 포맷 지정)
	FString TargetURL = FString::Printf(TEXT("https://docs.google.com/spreadsheets/d/%s/export?format=tsv"), *SheetID);

	// 2. PageID(gid) 추가
	if (!PageID.IsEmpty())
	{
		TargetURL += FString::Printf(TEXT("&gid=%s"), *PageID);
	}

	// 3. Range 추가
	if (!Range.IsEmpty())
	{
		TargetURL += FString::Printf(TEXT("&range=%s"), *Range);
	}

	// HTTP 요청 생성 및 전송 로직 (이전과 동일)
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGoogleSheetFetcher::OnProcessRequestComplete);
    
	Request->SetURL(TargetURL);
	Request->SetVerb(TEXT("GET"));
    
	UE_LOG(LogTemp, Log, TEXT("[HTTP] 조립된 URL로 요청 시작: %s"), *TargetURL);
	Request->ProcessRequest();
}

void UGoogleSheetFetcher::OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	// 1. 결과값 초기화
	bool bSuccess = false;
	FString ResponseString = TEXT("");

	// 2. 연결 및 응답 객체 유효성 체크
	if (bConnectedSuccessfully && Response.IsValid())
	{
		// 3. HTTP 상태 코드 확인 (200 OK인지)
		if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			// 성공! 데이터 꺼내기
			ResponseString = Response->GetContentAsString();
			bSuccess = true;

			UE_LOG(LogTemp, Log, TEXT("[GoogleSheet] 통신 성공! 데이터 길이: %d"), ResponseString.Len());
		}
		else
		{
			// 서버 에러 (URL이 잘못됐거나 권한이 없는 경우 등)
			UE_LOG(LogTemp, Error, TEXT("[GoogleSheet] 서버 응답 에러 코드: %d"), Response->GetResponseCode());
		}
	}
	else
	{
		// 네트워크 연결 실패 (인터넷 끊김 등)
		UE_LOG(LogTemp, Error, TEXT("[GoogleSheet] 네트워크 연결 실패!"));
	}

	// 4. 블루프린트 델리게이트를 통해 결과 방송
	// 브로드캐스트를 하면 이 객체를 지켜보던 위젯이나 다른 클래스가 반응합니다.
	OnDataReceived.Broadcast(bSuccess, ResponseString);
}

#endif
