// Fill out your copyright notice in the Description page of Project Settings.


#include "ApexCharacter.h"

#include "EnhancedInputComponent.h"
#include "Components/ApexMovementComponent.h"
#include "Components/ApexStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AApexCharacter::AApexCharacter(const FObjectInitializer& ObjectInitializer)
/** [핵심] 기본 무브먼트 컴포넌트를 우리가 만든 UApexMovementComponent로 교체 */
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UApexMovementComponent>(CharacterMovementComponentName))
{
	// 1. 상태 머신 컴포넌트 생성
	StateMachine = CreateDefaultSubobject<UApexStateMachineComponent>(TEXT("StateMachine"));

	// 2. 교체된 무브먼트 컴포넌트 캐싱
	ApexMoveComp = Cast<UApexMovementComponent>(GetCharacterMovement());

	// 3. 기본 설정 (필요시)
	PrimaryActorTick.bCanEverTick = true;
}

void AApexCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DisplayDebugInfo(DeltaTime);
}

void AApexCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기화 시 필요한 로직이 있다면 여기서 수행
}

void AApexCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Sprint 입력 바인딩
		if (DashSprintAction)
		{
			EnhancedInputComponent->BindAction(DashSprintAction, ETriggerEvent::Started, this, &AApexCharacter::DoSprintStart);
			EnhancedInputComponent->BindAction(DashSprintAction, ETriggerEvent::Completed, this, &AApexCharacter::DoSprintEnd);
		}

		// Crouch 입력 바인딩
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AApexCharacter::DoCrouchStart);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AApexCharacter::DoCrouchEnd);
		}
	}
}



// --- [입력 처리: 상태 머신으로 신호 전달] ---

UCameraComponent* AApexCharacter::GetFirstPersonCamera()
{
	
	return nullptr;
}

void AApexCharacter::DoSprintStart()
{
	if (StateMachine)
	{
		StateMachine->SetWantsToSprint(true);
	}
}

void AApexCharacter::DoSprintEnd()
{
	if (StateMachine)
	{
		StateMachine->SetWantsToSprint(false);
	}
}

void AApexCharacter::DoCrouchStart()
{
	if (StateMachine)
	{
		StateMachine->SetWantsToCrouch(true);
	}
}

void AApexCharacter::DoCrouchEnd()
{
	if (StateMachine)
	{
		StateMachine->SetWantsToCrouch(false);
	}
}

void AApexCharacter::DoJumpStart()
{
	// 점프 시 슬라이딩 상태라면 강제로 해제하거나, 
	// 상태 머신에서 점프를 감지하여 처리하도록 설계할 수 있습니다.
	Super::DoJumpStart();
}

void AApexCharacter::DoMove(float Right, float Forward)
{
	if (StateMachine && StateMachine->GetCurrentState() == EMovementModeState::Sliding)
		Super::DoMove(Right * SlideSteeringDamping, Forward * SlideSteeringDamping);
	else	
		Super::DoMove(Right, Forward);
}

void AApexCharacter::OnMovementStateChanged(EMovementModeState NewState)
{
	// 상태 머신에서 상태가 바뀔 때 이 함수를 호출해줍니다.
	// 여기서 사운드, 이펙트, 카메라 쉐이크 등을 처리합니다.
	
	switch (NewState)
	{
	case EMovementModeState::Sliding:
		// 슬라이딩 시작 효과 (예: 카메라 흔들림)
		break;
	case EMovementModeState::Walking:
		// 일어서기 완료 시 효과
		break;
	}
}

void AApexCharacter::DisplayDebugInfo(float DeltaTime)
{
	if (!GEngine || !StateMachine || !ApexMoveComp) return;

	// 1. 현재 상태 및 상태별 색상 설정
	EMovementModeState CurrentState = StateMachine->GetCurrentState();
	FColor StateColor;

	switch (CurrentState)
	{
	case EMovementModeState::Walking:   StateColor = FColor::White;  break;
	case EMovementModeState::Sprinting: StateColor = FColor::Green;  break;
	case EMovementModeState::Crouching: StateColor = FColor::Yellow; break;
	case EMovementModeState::Sliding:   StateColor = FColor::Orange; break;
	case EMovementModeState::Falling:   StateColor = FColor::Red;    break;
	default:                            StateColor = FColor::Cyan;   break;
	}

	// 2. 상태 이름 가져오기
	const UEnum* EnumPtr = StaticEnum<EMovementModeState>();
	FString StateName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentState) : TEXT("Invalid");

	// 3. 속도 정보 계산 (평면 속도와 전체 속도)
	FVector Velocity = ApexMoveComp->Velocity;
	float Speed2D = Velocity.Size2D(); // 수평 속도
	float Speed3D = Velocity.Size();   // 전체 속도

	// 4. 화면 출력 (Key를 -1에서 특정 번호(예: 100)로 바꾸면 메시지가 겹치지 않고 한 줄에서 갱신됩니다)
	// 상태 표시
	GEngine->AddOnScreenDebugMessage(100, 0.f, StateColor, FString::Printf(TEXT("State: %s"), *StateName));
    
	// 속도 표시
	GEngine->AddOnScreenDebugMessage(101, 0.f, FColor::Cyan, FString::Printf(TEXT("Speed: %.2f (Z: %.2f)"), Speed2D, Velocity.Z));
}