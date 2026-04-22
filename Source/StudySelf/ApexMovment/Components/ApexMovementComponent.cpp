#include "ApexMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "ApexMovment/ApexType.h"
#include "Camera/CameraComponent.h"

UApexMovementComponent::UApexMovementComponent()
{

}

void UApexMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!CharacterOwner)
		return;

	NormalGroundFriction = GroundFriction;
	NormalBrakingDeceleration = BrakingDecelerationWalking;
	
	// 1. [데이터 박제] 서 있을 때의 당당한 키를 저장합니다.
	if (UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent())
	{
		CharacterCapsule = Capsule;
		NormalHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	}

	if (CharacterOwner)
	{
		FirstPersonCamera = CharacterOwner->FindComponentByClass<UCameraComponent>();
		DefaultCameraRelativeLocation = FirstPersonCamera->GetRelativeLocation(); 
		// 2. [기준점 확보] 메시의 초기 위치를 저장합니다. (Z 보정의 핵심)
		if (CharacterOwner->GetMesh())
			DefaultMeshRelativeLocation = CharacterOwner->GetMesh()->GetRelativeLocation();
	}
		
	// 초기 목표치는 현재 높이로 설정
	TargetHalfHeight = NormalHalfHeight;
}

void UApexMovementComponent::RequestCrouch()
{
	// 목표를 앉은 키로 설정
	TargetHalfHeight = CrouchedHalfHeight;

	// 타이머 시작 (0.01초마다 보간 함수 호출)
	if (!GetWorld()->GetTimerManager().IsTimerActive(T_HeightInterp))
	{
		GetWorld()->GetTimerManager().SetTimer(T_HeightInterp, this, &UApexMovementComponent::UpdateCapsuleHeightInterp, 0.01f, true);
	}

	// [구독형 로직] 숙이기 시도 후 후속 조치 (예: 슬라이딩 판정 등)
	OnCrouchCaptured();
}

void UApexMovementComponent::RequestUnCrouch()
{
	// 천장 체크: 일어서고 싶어도 머리 위에 뭐가 있으면 거부한다!
	if (HasCeilingObstacle())
		return;

	// 목표를 원래 키로 설정
	TargetHalfHeight = NormalHalfHeight;

	if (!GetWorld()->GetTimerManager().IsTimerActive(T_HeightInterp))
	{
		GetWorld()->GetTimerManager().SetTimer(T_HeightInterp, this, &UApexMovementComponent::UpdateCapsuleHeightInterp, 0.01f, true);
	}
}

void UApexMovementComponent::UpdateCapsuleHeightInterp()
{
	float CurrentHalf = CharacterCapsule->GetUnscaledCapsuleHalfHeight();
    
	// 1. 종료 조건
	if (FMath::IsNearlyEqual(CurrentHalf, TargetHalfHeight, 0.1f))
	{
		UpdateCapsulePhysics(TargetHalfHeight);
		UpdateVisualOffsets(NormalHalfHeight - TargetHalfHeight);
		GetWorld()->GetTimerManager().ClearTimer(T_HeightInterp);
		return;
	}

	// 2. 새로운 값 계산
	float NewHalf = FMath::FInterpTo(CurrentHalf, TargetHalfHeight, 0.01f, CrouchInterpSpeed);
    
	// 3. 분리된 함수 호출
	UpdateCapsulePhysics(NewHalf);
	UpdateVisualOffsets(NormalHalfHeight - NewHalf);
}

void UApexMovementComponent::UpdateCapsulePhysics(float NewHalfHeight) const
{
	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	float CurrentHalf = Capsule->GetUnscaledCapsuleHalfHeight();
	float Diff = CurrentHalf - NewHalfHeight;

	// 캡슐 크기 조절
	Capsule->SetCapsuleHalfHeight(NewHalfHeight);
    
	// 지면 고정을 위한 액터 위치 이동
	if (!IsFalling())
		CharacterOwner->AddActorWorldOffset(FVector(0.f, 0.f, -Diff));
}

void UApexMovementComponent::UpdateVisualOffsets(float HeightDifference) const
{
	// 1인칭 시점 카메라가 z축으로 -90만큼 회전되어 있어서 로컬 x축을 이동시킴
	// 1. 메시 보정 (발바닥 고정)
	//if (USkeletalMeshComponent* Mesh = CharacterOwner->GetMesh())
	//{
	//	FVector NewMeshLoc = DefaultMeshRelativeLocation;
	//	NewMeshLoc.X += HeightDifference;
	//	Mesh->SetRelativeLocation(NewMeshLoc);
	//}

	// 2. 카메라 보정 (시야 하강)
	if (FirstPersonCamera)
	{
		FVector NewCamLoc = DefaultCameraRelativeLocation;
		float CameraOffset = HeightDifference * 0.5f; 
		NewCamLoc.X -= CameraOffset; 
       
		FirstPersonCamera->SetRelativeLocation(NewCamLoc);
	}
}

void UApexMovementComponent::OnCrouchCaptured()
{
	// 팀장님이 설계하신 "자동 해제" 혹은 "상태 전환" 로직이 들어갈 자리입니다.
	// 예: 속도가 너무 낮으면 자동으로 RequestUnCrouch() 호출 등
	
}

bool UApexMovementComponent::HasCeilingObstacle() const
{
	if (!CharacterOwner || !CharacterOwner->GetCapsuleComponent()) return false;

	// 현재 내 반높이 (보간 중일 수 있으므로 실시간 값 사용)
	float CurrentHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	
	// 레이캐스트 거리: "원래 키 - 현재 키" 만큼 위로 쏜다.
	float TraceDistance = (NormalHalfHeight - CurrentHalfHeight) * 2.0f;
	
	// 시작점: 현재 캡슐의 천장(머리 끝)
	FVector Start = CharacterOwner->GetActorLocation() + (CharacterOwner->GetActorUpVector() * CurrentHalfHeight);
	// 끝점: 원래 내가 서 있어야 할 머리 끝 위치 (+ 여유분 5.0f)
	FVector End = Start + (CharacterOwner->GetActorUpVector() * (TraceDistance + 5.0f));

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	// 캡슐 반지름만큼 두껍게 스윕(Sweep)하여 끼임 방지
	FCollisionShape SweepSphere = FCollisionShape::MakeSphere(CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius());

	return GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, SweepSphere, QueryParams);
}

void UApexMovementComponent::UpdateMovementPhysics(EMovementModeState NewState)
{
	// 이전 상태와 동일하다면 굳이 다시 계산하지 않음 (최적화)
	// if (CurrentApexState == NewState) return; 

	switch (NewState)
	{
	case EMovementModeState::Walking:
		MaxWalkSpeed = NormalWalkSpeed;
		GroundFriction = NormalGroundFriction;
		BrakingDecelerationWalking = NormalBrakingDeceleration;
		RequestUnCrouch();
		break;

	case EMovementModeState::Sprinting:
		MaxWalkSpeed = SprintSpeed;
		GroundFriction = NormalGroundFriction;
		BrakingDecelerationWalking = NormalBrakingDeceleration;
		RequestUnCrouch();
		break;

	case EMovementModeState::Crouching:
		MaxWalkSpeed = CrouchWalkSpeed;
		GroundFriction = NormalGroundFriction;
		BrakingDecelerationWalking = NormalBrakingDeceleration;
		// 일반 앉기 상태이므로 캡슐이 작아져야 함
		RequestCrouch(); 
		break;

	case EMovementModeState::Sliding:
		// [에펙 핵심] 슬라이딩 시 최고 속도 제한을 풀어서 관성을 유지함
		MaxWalkSpeed = SprintSpeed; 
		GroundFriction = SlideFriction; // 매우 낮은 마찰력 (미끄러짐)
		BrakingDecelerationWalking = SlideBrakingDeceleration; // 서서히 멈춤
        
		// 슬라이딩도 결국 숙인 상태이므로 캡슐 축소 요청
		RequestCrouch(); 
		break;

	case EMovementModeState::Falling:
		RequestUnCrouch();
		// [중요] 공중에서는 마찰력을 계산하지 않으므로 속도 제한만 유지
		// 슬라이딩 점프 시 가속도를 보존하기 위해 MaxWalkSpeed를 낮추지 않음
		break;
	}
}

void UApexMovementComponent::ApplySlidingBoost()
{
	if (bCanSlideBoost)
	{
		// 진행 방향으로 가속 부여
		FVector BoostDir = GetCharacterOwner()->GetActorForwardVector();
		AddImpulse(BoostDir * SlideBoostForce, true);

		bCanSlideBoost = false;
		GetWorld()->GetTimerManager().SetTimer(SlideBoostTimer, this, &UApexMovementComponent::ResetSlideBoost, 2.0f);
	}
}

void UApexMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bShowDebugCapsule) return;
	DrawMovementDebug();
}

void UApexMovementComponent::DrawMovementDebug()
{
	// 에디터가 아니거나, 캐릭터가 없으면 즉시 리턴
	if (!CharacterOwner || !GetWorld()) return;

	FVector CapsuleLoc = CharacterCapsule->GetComponentLocation();
	float HalfHeight = CharacterCapsule->GetUnscaledCapsuleHalfHeight();
	float Radius = CharacterCapsule->GetUnscaledCapsuleRadius();

	// 1. 현재 실시간 캡슐 (주황/초록) - 1프레임만 유지
	FColor CapsuleColor = (HalfHeight < NormalHalfHeight - 1.f) ? FColor::Orange : FColor::Green;
	DrawDebugCapsule(GetWorld(), CapsuleLoc, HalfHeight, Radius, CharacterCapsule->GetComponentQuat(), 
					 CapsuleColor, false, -1.f, 0, 1.5f);

	// 2. 원래 키 가이드라인 (하얀색 실선)
	DrawDebugCapsule(GetWorld(), CapsuleLoc, NormalHalfHeight, Radius, CharacterCapsule->GetComponentQuat(), 
					 FColor::White, false, -1.f, 0, 0.5f);

	// 3. 화면 텍스트 정보
	if (GEngine)
	{
		FString DebugText = FString::Printf(TEXT("Height: %.1f / Target: %.1f"), HalfHeight, TargetHalfHeight);
		GEngine->AddOnScreenDebugMessage(1, 0.f, CapsuleColor, DebugText);
	}
}
