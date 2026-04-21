// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "ApexCharacter.generated.h"


UENUM(BlueprintType)
enum class EMovementModeState : uint8
{
	Walking	    UMETA(DisplayName = "Walking"), // 정지 및 일반 걷기 (Idle + Walk)
	Sprinting	UMETA(DisplayName = "Sprinting"),// 전력질주 (Run)
	Crouching	UMETA(DisplayName = "Crouching"),// 앉기 (Crouch Idle + Crouch Walk)
	Sliding		UMETA(DisplayName = "Sliding"),// 슬라이딩
	Falling		UMETA(DisplayName = "Falling"),// 공중 (점프/추락) - 이것도 미리 넣어두면 편해요
};

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
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	EMovementModeState CurrentMovementState = EMovementModeState::Walking;
    
	/** 이동 속도 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Speed")
	float SprintSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Speed")
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Speed")
	float CrouchSpeed = 300.f;

	/** 슬라이딩 상세 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Sliding")
	float SlidingImpulseForce = 700.f;

	/** 슬라이딩 버튼을 뗐을 때, 이 속도보다 빠르면 슬라이딩을 강제로 유지함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float SlideInertiaThreshold = 400.f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Sliding")
	float SlidingMinSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Sliding")
	float SlidingFriction = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float SlidingBoostCooldown = 2.0f;


	// 런타임에 엔진 설정을 받아올 변수 (수정 불필요하므로 Visible)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|Sliding")
	float DefaultFriction;

	// 가독성을 위한 마찰력 관련 변수 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float DefaultBrakingDeceleration = 2048.f;
	
protected:
	// 슬라이딩 방향 저장 (슬라이딩 중 방향 전환 방지용)
	FVector SlidingDirection;

	bool bWantSprint = false;
	bool bWantCrouch = false;
	bool bCanSlidingBoost = true;

	void SetMovementState(EMovementModeState NewState);
	
	/** 타이머에 의해 호출될 활성화 함수 */
	// 슬라이딩 시작/종료 처리 함수
	void StartSliding();
	void StopSliding();
	void ResetSlidingBoost();

	/** 쿨타임 관리를 위한 타이머 핸들 */
	FTimerHandle SlidingBoostTimerHandle;
	
protected:
	// --- 상황 감지 함수 (Pure Logic) ---
	bool DetectFalling() const;
	bool DetectSprint() const;
	bool ShouldMaintainSliding() const;
    
	// 일반 앉기와 슬라이딩을 모두 포함하는 '숙이기 상태' 판정
	EMovementModeState DetectCrouchState() const;
	// --- 상태 결정 메인 ---
	EMovementModeState GetDesiredState() const;
	
	// 상태가 실제로 바뀔 때 호출되는 핵심 함수 (Enter/Exit 로직 포함)
	void HandleStateTransition(EMovementModeState NewState);

	// 상태별로 매 프레임 실행되어야 할 물리/로직 (Tick 대용)
	void UpdateStatePhysics(float DeltaTime);


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch")
	float CrouchCapsuleHalfHeight = 60.f;  // 앉았을 때 캡슐 절반 높이

	float StandCapsuleHalfHeight;         // 서 있을 때 원래 높이 (자동 저장)

	// 엔진 변수 대신 사용할 우리만의 상태 변수
	bool bIsCustomCrouched = false;

	/** 커스텀 Crouch 실행 함수 */
	void RequestCustomCrouch();
	void RequestCustomUnCrouch();

protected:
	virtual bool IsCrouching() override;
	virtual void DoMove(float Right, float Forward) override;
	virtual void DoJumpStart() override;
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;
	
protected:
	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* CrouchAction;

	/** Sprint Input Action */ //이미 사용중인 이름이라 수정함
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* DashSprintAction;

	/** Crouch */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchEnd();

	/** Sprint */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

private:
	void DisplayDebugInfo(float DeltaTime);
};

