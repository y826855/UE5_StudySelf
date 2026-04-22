#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMovementModeState : uint8
{
	Walking	    UMETA(DisplayName = "Walking"), // 정지 및 일반 걷기 (Idle + Walk)
	Sprinting	UMETA(DisplayName = "Sprinting"),// 전력질주 (Run)
	Crouching	UMETA(DisplayName = "Crouching"),// 앉기 (Crouch Idle + Crouch Walk)
	Sliding		UMETA(DisplayName = "Sliding"),// 슬라이딩
	Falling		UMETA(DisplayName = "Falling"),// 공중 (점프/추락) - 이것도 미리 넣어두면 편해요
};
