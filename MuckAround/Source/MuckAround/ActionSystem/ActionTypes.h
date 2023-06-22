// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ActionTypes.generated.h"

// Used to determine the actions current state. NOTE: END will be the start when it is not running
UENUM(BlueprintType)
enum class EActionState : uint8
{
	AS_End            UMETA(DisplayName = "End"),
	AS_Start          UMETA(DisplayName = "Start"),
	AS_Performing     UMETA(DisplayName = "Performing")
};

// Used to determine how the action should end. NOTE: Manual will need to be implemented by the action.
UENUM(BlueprintType)
enum class EActionEndType : uint8
{
	AET_Immediate     UMETA(DisplayName = "Immediate"),
	AET_Manual        UMETA(DisplayName = "Manual"),
	AET_Timer         UMETA(DisplayName = "Timer")
};