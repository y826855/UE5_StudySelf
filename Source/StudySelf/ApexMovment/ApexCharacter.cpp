// Fill out your copyright notice in the Description page of Project Settings.


#include "ApexCharacter.h"

#include "EnhancedInputComponent.h"
#include "StudySelf.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


void AApexCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//sprint
		EnhancedInputComponent->BindAction(DashSprintAction, ETriggerEvent::Started, this, &AApexCharacter::DoSprintStart);
		EnhancedInputComponent->BindAction(DashSprintAction, ETriggerEvent::Completed, this, &AApexCharacter::DoSprintEnd);
		
		//crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AApexCharacter::DoCrouchStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AApexCharacter::DoCrouchEnd);
		
	}
	else
	{
		UE_LOG(LogStudySelf, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AApexCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 현재 상황에 가장 적합한 '원하는 상태'를 계산
	EMovementModeState NewState = GetDesiredState();

	// 2. 상태가 변했다면 전이 로직 실행 (HandleStateTransition)
	if (NewState != CurrentMovementState)
	{
		HandleStateTransition(NewState);
	}

	// 3. 현재 상태 유지 중 필요한 물리 로직 실행 (예: 슬라이딩 감속 등)
	UpdateStatePhysics(DeltaTime);

	// 4. 디버그 메세지 출력 (기존 로직 유지)
	DisplayDebugInfo(DeltaTime);
}

void AApexCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 시작할 때 캐릭터 무브먼트에 설정된 기본 마찰력을 저장
	if (GetCharacterMovement())
	{
		DefaultFriction = GetCharacterMovement()->GroundFriction;
		StandCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	}
}

void AApexCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 착지 시점에 앉기를 누르고 있고 속도가 충분하면 즉시 슬라이딩 전이
	if (bWantCrouch && GetVelocity().Size() > WalkSpeed)
		HandleStateTransition(EMovementModeState::Sliding);
}

void AApexCharacter::SetMovementState(EMovementModeState NewState)
{
	CurrentMovementState = NewState;

	switch (NewState)
	{
	case EMovementModeState::Walking:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		RequestCustomUnCrouch();
		break;
	case EMovementModeState::Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		RequestCustomUnCrouch();
		break;
	case EMovementModeState::Crouching:
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		break;
	default:
		RequestCustomUnCrouch();
		break;
	}
}

EMovementModeState AApexCharacter::GetDesiredState() const
{
	// 1순위: 공중 상태 (최우선)
	if (DetectFalling()) return EMovementModeState::Falling;

	// 2순위: 슬라이딩 유지 및 앉기 판정 (여기에 관성 로직이 포함됨)
	// 이제 슬라이딩 중일 때는 여기서 'Sliding'을 계속 뱉어주므로 아래 Sprinting 체크까지 내려가지 않음
	EMovementModeState CrouchGroupState = DetectCrouchState();
	if (CrouchGroupState != EMovementModeState::Walking)
	{
		return CrouchGroupState;
	}

	// 3순위: 전력 질주
	if (DetectSprint()) return EMovementModeState::Sprinting;

	// 4순위: 기본 걷기
	return EMovementModeState::Walking;
}

void AApexCharacter::HandleStateTransition(EMovementModeState NewState)
{
	if (CurrentMovementState == NewState) return;

	// [Exit Logic] 현재 상태를 벗어날 때
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		StopSliding();
	}

	// [Transition] 만약 앉기 계열에서 일반 이동으로 나간다면 UnCrouch 실행
	bool bWasCrouchGroup = (CurrentMovementState == EMovementModeState::Crouching || CurrentMovementState == EMovementModeState::Sliding);
	bool bIsGoingToCrouchGroup = (NewState == EMovementModeState::Crouching || NewState == EMovementModeState::Sliding);

	if (bWasCrouchGroup && !bIsGoingToCrouchGroup)
	{
		RequestCustomUnCrouch();
	}

	// 상태 변경
	CurrentMovementState = NewState;

	// [Enter Logic] 새로운 상태에 진입할 때
	switch (CurrentMovementState)
	{
	case EMovementModeState::Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		break;

	case EMovementModeState::Crouching:
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		RequestCustomCrouch();
		break;

	case EMovementModeState::Sliding:
		RequestCustomCrouch();
		StartSliding(); // 여기서 물리 설정(마찰력 0 등) 변경
		break;

	case EMovementModeState::Walking:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	}
}

void AApexCharacter::UpdateStatePhysics(float DeltaTime)
{
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		// 예: 슬라이딩 중 경사면에 따른 가속/감속 추가 가능
	}
}

void AApexCharacter::RequestCustomCrouch()
{
	if (bIsCustomCrouched || !GetCapsuleComponent()) return;

	// 1. 캡슐 높이 조절
	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(), CrouchCapsuleHalfHeight);
	
	// 2. 메시 위치 보정: 캡슐이 줄어들면 바닥에서 뜨기 때문에 상대 위치를 낮춤
	// 기본 캡슐 높이가 90이고 메시 Z가 -90이라면, 60으로 줄었을 때 Z를 -60으로 맞춰야 발이 땅에 붙음
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -CrouchCapsuleHalfHeight));

	bIsCustomCrouched = true;
}

void AApexCharacter::RequestCustomUnCrouch()
{
	if (!bIsCustomCrouched || !GetCapsuleComponent()) return;

	// TODO: 나중에 머리 위에 천장이 있는지 체크하는 LineTrace를 여기에 추가 가능

	// 1. 캡슐 높이 복구
	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(), StandCapsuleHalfHeight);
	
	// 2. 메시 위치 복구
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -StandCapsuleHalfHeight));

	bIsCustomCrouched = false;
}

bool AApexCharacter::IsCrouching()
{
	return false;
}

void AApexCharacter::StartSliding()
{
	// 1. 슬라이딩 방향 결정
	SlidingDirection = GetVelocity().GetSafeNormal();

	// 2. 가속 가능 상태일 때만 Impulse 적용
	if (bCanSlidingBoost)
	{
		// 순간 가속 적용
		GetCharacterMovement()->AddImpulse(SlidingDirection * SlidingImpulseForce, true);

		// 가속권 소모 및 쿨타임 타이머 시작
		bCanSlidingBoost = false;
		GetWorldTimerManager().SetTimer(SlidingBoostTimerHandle, this, &AApexCharacter::ResetSlidingBoost, SlidingBoostCooldown, false);
	}
	else
	{
		// 가속이 안 될 때: 에펙의 '데드 슬라이드'처럼 약간의 감속을 주거나 그냥 무시
		GetCharacterMovement()->Velocity *= 0.8f; 
	}

	// 3. 물리 설정 (미끄러짐은 쿨타임과 무관하게 적용)
	GetCharacterMovement()->GroundFriction = SlidingFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = 0.f;

	// 커스텀 숙이기 호출
	RequestCustomCrouch();
}
void AApexCharacter::StopSliding()
{
	// 물리 설정 복구
	GetCharacterMovement()->GroundFriction = DefaultFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
    
	if (!bWantCrouch) RequestCustomUnCrouch();
}

void AApexCharacter::ResetSlidingBoost()
{
	bCanSlidingBoost = true;
	 UE_LOG(LogTemp, Log, TEXT("Sliding Boost Ready!")); // 필요 시 디버그 로그
}
bool AApexCharacter::DetectFalling() const
{
	return GetCharacterMovement()->IsFalling();
}

bool AApexCharacter::DetectSprint() const
{
	// 입력 의지가 있고, 아주 최소한의 움직임이라도 있을 때
	return bWantSprint && GetVelocity().Size() > 10.f;
}

bool AApexCharacter::ShouldMaintainSliding() const
{
	// 1. 버튼을 누르고 있으면 무조건 유지
	if (bWantCrouch) return true;

	// 2. 버튼을 뗐더라도, 현재 속도가 임계값보다 높으면 관성에 의해 유지
	// (슬라이딩 중일 때만 이 관성 법칙이 적용되도록 함)
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		return GetVelocity().Size() > SlideInertiaThreshold;
	}

	return false;
}


EMovementModeState AApexCharacter::DetectCrouchState() const
{
	// [판단 1] 이미 슬라이딩 중인 경우
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		// 속도가 임계값(SlideInertiaThreshold) 이상이면 무조건 슬라이딩 유지
		// 이때는 bWantCrouch나 bWantSprint 입력을 무시하게 됨
		if (GetVelocity().Size() > SlideInertiaThreshold)
		{
			return EMovementModeState::Sliding;
		}
	}

	// [판단 2] 새로 슬라이딩에 진입하거나 앉기를 유지해야 하는 경우
	if (bWantCrouch)
	{
		// 진입 조건: 달리는 중이거나 충분히 빠를 때
		bool bCanStartSlide = (CurrentMovementState == EMovementModeState::Sprinting) || 
							  (GetVelocity().Size() > SprintSpeed - 100.f);

		return bCanStartSlide ? EMovementModeState::Sliding : EMovementModeState::Crouching;
	}

	// 위 조건들에 해당 없으면 탈출
	return EMovementModeState::Walking;
}

void AApexCharacter::DoMove(float Right, float Forward)
{
	// 슬라이딩 중 입력값을 10%로 줄여서 전달 (미세한 조정은 가능하게)
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		Super::DoMove(Right * 0.1f, Forward * 0.1f);
		return;
	}
	Super::DoMove(Right, Forward);
}

void AApexCharacter::DoJumpStart()
{
	// 1. 슬라이딩이나 앉기 중이라면 엔진의 Crouch 상태를 먼저 풀어야 점프가 가능함
	if (bIsCustomCrouched)
		RequestCustomUnCrouch();

	// 2. 슬라이딩 중 점프라면 '슬라이드 점프' 로직 수행
	if (CurrentMovementState == EMovementModeState::Sliding)
	{
		// 슬라이딩 종료 처리를 HandleStateTransition을 통해 수행 (마찰력 복구 등)
		HandleStateTransition(EMovementModeState::Falling);
        
		// 에펙 느낌을 위해 점프 시 약간의 전진 힘을 더 줄 수도 있음
		GetCharacterMovement()->AddImpulse(SlidingDirection * 200.f, true);
	}

	// 3. 실제 점프 실행 (이제 CanJump()가 true를 반환함)
	Jump();
}
void AApexCharacter::DoCrouchStart()
{
	bWantCrouch = true;
}

void AApexCharacter::DoCrouchEnd()
{
	bWantCrouch = false;
}

void AApexCharacter::DoSprintStart()
{
	bWantSprint = true;
}

void AApexCharacter::DoSprintEnd()
{
	bWantSprint = false;	
}

void AApexCharacter::DisplayDebugInfo(float DeltaTime)
{
	if (!GEngine) return;
	
	// StaticEnum을 사용하면 경로 문제 없이 깔끔하게 이름을 가져옵니다.
	const UEnum* EnumPtr = StaticEnum<EMovementModeState>();
	FString StateName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentMovementState) : TEXT("Invalid");

	// 상태에 따라 색상 지정 (선택 사항)
	FColor DisplayColor = FColor::Cyan;
	if (CurrentMovementState == EMovementModeState::Sprinting) DisplayColor = FColor::Yellow;
	if (CurrentMovementState == EMovementModeState::Sliding) DisplayColor = FColor::Orange;
	if (CurrentMovementState == EMovementModeState::Crouching) DisplayColor = FColor::Green;

	// 화면 왼쪽 상단에 출력 (Key를 1로 고정하면 메세지가 쌓이지 않고 갱신됨)
	GEngine->AddOnScreenDebugMessage(1, DeltaTime, DisplayColor, FString::Printf(TEXT("Current State: %s"), *StateName));
        
	// 속도도 같이 출력해보면 디버깅에 아주 좋습니다.
	GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::White, FString::Printf(TEXT("Velocity: %f"), GetVelocity().Size()));
}


