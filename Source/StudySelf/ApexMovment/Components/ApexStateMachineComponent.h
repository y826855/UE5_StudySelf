// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ApexMovment/ApexType.h"
#include "ApexStateMachineComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STUDYSELF_API UApexStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UApexStateMachineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- 외부(Character)에서 넣어줄 입력 값 ---
	void SetWantsToSprint(bool bInWantsToSprint) { bWantSprint = bInWantsToSprint; }
	void SetWantsToCrouch(bool bInWantsToCrouch) { bWantCrouch = bInWantsToCrouch; }

	// 현재 상태 반환 (애니메이션 블루프린트 등에서 사용)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	EMovementModeState GetCurrentState() const { return CurrentState; }

private:
	// --- 상태 결정 로직 (내일 구현할 핵심) ---
	EMovementModeState DetermineDesiredState() const;
	
	// 상태가 변경되었을 때 실행할 이벤트 (물리 값 변경 등)
	void OnStateChanged(EMovementModeState NewState, EMovementModeState OldState);

private:
	// 참조 저장
	UPROPERTY()
	class AApexCharacter* OwnerChar;

	UPROPERTY()
	class UApexMovementComponent* ApexMoveComp;

	// 상태 변수
	EMovementModeState CurrentState = EMovementModeState::Walking;

	// 입력 플래그
	bool bWantSprint = false;
	bool bWantCrouch = false;
	
};
