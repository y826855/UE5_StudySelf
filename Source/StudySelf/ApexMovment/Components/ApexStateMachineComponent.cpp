// Fill out your copyright notice in the Description page of Project Settings.


#include "ApexStateMachineComponent.h"

#include "ApexMovementComponent.h"
#include "ApexMovment/ApexCharacter.h"

UApexStateMachineComponent::UApexStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UApexStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<AApexCharacter>(GetOwner());
	if (OwnerChar)
	{
		// 생성자에서 교체한 커스텀 무브먼트 컴포넌트를 가져옵니다.
		ApexMoveComp = Cast<UApexMovementComponent>(OwnerChar->GetCharacterMovement());
	}
}

void UApexStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1. 매 프레임 "지금 어떤 상태여야 하는가?"를 판단
	EMovementModeState DesiredState = DetermineDesiredState();

	// 2. 상태가 변했다면 전이 로직 실행
	if (DesiredState != CurrentState)
	{
		EMovementModeState OldState = CurrentState;
		CurrentState = DesiredState;
		OnStateChanged(CurrentState, OldState);
	}
}

EMovementModeState UApexStateMachineComponent::DetermineDesiredState() const
{
    if (!ApexMoveComp || !OwnerChar) return EMovementModeState::Walking;

    float Speed = ApexMoveComp->Velocity.Size2D();
    
    // [수정] 무조건 체크하는 게 아니라, 로직 내부에서 필요할 때만 체크하도록 변경
    bool bHasCeiling = false;

    // 1. 공중 상태 (최우선)
    if (ApexMoveComp->IsFalling())
    {
       return EMovementModeState::Falling;
    }

    // 2. [수정된 에펙 규칙] 
    // 사용자가 숙이고 싶어 하거나(bWantCrouch), 이미 숙인 상태(Crouching/Sliding)일 때만 천장을 체크합니다.
    bool bIsAlreadyCrouched = (CurrentState == EMovementModeState::Crouching || CurrentState == EMovementModeState::Sliding);
    
    if (bWantCrouch || bIsAlreadyCrouched)
    {
        // 천장 체크
        bHasCeiling = ApexMoveComp->HasCeilingObstacle();

        // [핵심 방어] 숙이기 키도 안 눌렀고, 천장도 없다면 더 이상 이 구역에 머물 이유가 없음 (탈출!)
        if (!bWantCrouch && !bHasCeiling)
        {
            // 아래의 '3. 일어서기 영역'으로 내려가게 유도
        }
        else
        {
            // 슬라이딩 상태 로직
            if (CurrentState == EMovementModeState::Sliding)
            {
                // 키를 뗐는데 천장도 없다면 슬라이딩 탈출
                if (!bWantCrouch && !bHasCeiling)
                {
                    if (Speed < ApexMoveComp->SprintSpeed) 
                    {
                        return bWantSprint ? EMovementModeState::Sprinting : EMovementModeState::Walking;
                    }
                }

                if (Speed < 200.f) return EMovementModeState::Crouching;
                return EMovementModeState::Sliding;
            }

            // 슬라이딩 진입 조건
            if (bWantCrouch && bWantSprint && Speed > 400.f)
            {
                return EMovementModeState::Sliding;
            }

            // [자동 숙이기 유지] 이미 숙였거나 숙이고 싶을 때, 혹은 천장이 있을 때만 여기 남음
            return EMovementModeState::Crouching;
        }
    }

    // 3. 앉기/천장 영역을 벗어난 경우 (일어서기)
    if (bWantSprint && !ApexMoveComp->Velocity.IsNearlyZero())
    {
       return EMovementModeState::Sprinting;
    }

    return EMovementModeState::Walking;
}

void UApexStateMachineComponent::OnStateChanged(EMovementModeState NewState, EMovementModeState OldState)
{
	if (!ApexMoveComp || !OwnerChar) return;

	// 무브먼트 컴포넌트에 물리 설정 변경 요청
	ApexMoveComp->UpdateMovementPhysics(NewState);

	// 슬라이딩 진입 시 순간 가속(Boost) 부여
	if (NewState == EMovementModeState::Sliding)
	{
		ApexMoveComp->ApplySlidingBoost();
	}

	// 캐릭터에게 상태 변경 알림 (이펙트, 카메라 쉐이크 등)
	OwnerChar->OnMovementStateChanged(NewState);
}

