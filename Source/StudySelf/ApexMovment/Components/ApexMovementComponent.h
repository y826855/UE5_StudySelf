// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ApexMovementComponent.generated.h"

// 상태 구분을 위한 전방 선언
enum class EMovementModeState : uint8;

UCLASS()
class STUDYSELF_API UApexMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UApexMovementComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UCameraComponent* FirstPersonCamera;

	UPROPERTY()
	UCapsuleComponent* CharacterCapsule;
	
	FVector DefaultCameraRelativeLocation;
	
public:
	// --- [1. 외부 명령 인터페이스 (Request)] ---
	// 팀장님이 설계하신 대로 입력을 통해 직접 호출하는 함수들
	void RequestCrouch();
	void RequestUnCrouch();

	/** 상태 머신에 의해 호출되는 물리 설정 함수 (속도, 마찰력 등) */
	void UpdateMovementPhysics(EMovementModeState NewState);

	/** 슬라이딩 시작 시 가속 부여 */
	void ApplySlidingBoost();

	/** 머리 위 장애물 체크 (원래 키로 돌아갈 수 있는지 확인) */
	bool HasCeilingObstacle() const;

private:
	// --- [2. 내부 동작 및 보간 (Timeline 대체 로직)] ---
	/** 캡슐 높이를 목표치로 부드럽게 보간 (타이머 호출용) */
	void UpdateCapsuleHeightInterp();

	/** 숙이기/슬라이딩 진입 성공 시 자동 해제 조건을 감시하거나 후속 조치를 취하는 함수 */
	void OnCrouchCaptured();

	/** 슬라이딩 부스트 쿨타임 리셋 */
	void ResetSlideBoost() { bCanSlideBoost = true; }

private:
	float NormalGroundFriction;
	float NormalBrakingDeceleration;
	float NormalHalfHeight;
	float TargetHalfHeight;

	// 메시의 초기 상대 위치 (Z값 보정용 기준점)
	FVector DefaultMeshRelativeLocation;

	// 타이머 핸들
	FTimerHandle T_HeightInterp;
	FTimerHandle SlideBoostTimer;

	bool bCanSlideBoost = true;

public:
	// --- [4. 에디터 노출 설정값 (Apex 전용)] ---

	// [General]
	UPROPERTY(EditAnywhere, Category = "Movement|Apex|General")
	float CrouchInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|General")
	float NormalWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|General")
	float SprintSpeed = 900.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|General")
	float CrouchWalkSpeed = 300.f;

	// [Sliding]
	UPROPERTY(EditAnywhere, Category = "Movement|Apex|Sliding")
	float SlideFriction = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|Sliding")
	float SlideBrakingDeceleration = 500.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|Sliding")
	float SlideBoostForce = 500.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Apex|Sliding")
	float SlideBoostCooldown = 1.5f;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateCapsulePhysics(float NewHalfHeight) const;
	void UpdateVisualOffsets(float HeightDifference) const;
	
private:
//DEBUG
	UPROPERTY(EditAnywhere, Category = "Movement|Apex|Debug")
	float bShowDebugCapsule = false;
	void DrawMovementDebug();
};