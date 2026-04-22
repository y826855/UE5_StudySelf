// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "ApexType.h"
#include "ApexCharacter.generated.h"


/**
 * 
 */
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;


UCLASS()
class STUDYSELF_API AApexCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	AApexCharacter(const FObjectInitializer& ObjectInitializer);
	// 기존 생성자 아래에 이 줄을 추가 (혹은 기존 생성자를 이걸로 대체)
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	
	// --- [1. 컴포넌트] ---
	// 생성자에서 UApexMovementComponent로 교체될 예정입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UApexMovementComponent* ApexMoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UApexStateMachineComponent* StateMachine;

	UCameraComponent* GetFirstPersonCamera();

	// --- [2. 입력 처리 함수] ---
	// 이제 이 함수들은 StateMachine에 "나 이거 하고 싶어"라고 신호만 보냅니다.
	void DoSprintStart();
	void DoSprintEnd();
	void DoCrouchStart();
	void DoCrouchEnd();
	
	virtual void DoJumpStart() override;
	virtual void DoMove(float Right, float Forward) override;


	// --- [3. 입력 액션 (Enhanced Input)] ---
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* DashSprintAction;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex")
	float SlideSteeringDamping = 0.1f; // 슬라이딩 시 입력 감쇄율
public:
	// --- [4. 외부 소통] ---
	/** 상태가 변했을 때 UI나 애니메이션, 이펙트 처리를 위해 호출될 함수 */
	void OnMovementStateChanged(EMovementModeState NewState);

	// 디버깅용 (필요시 유지)
	void DisplayDebugInfo(float DeltaTime);
};

