// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFuzzingFlags : uint8
{
	None         = 0      UMETA(Hidden),
	IncludeBase  = 1 << 0 UMETA(DisplayName = "Include Base Properties"),
	IncludeSuper = 1 << 1 UMETA(DisplayName = "Include Super Properties"),
	IncludeNull  = 1 << 2 UMETA(DisplayName = "Include Null Properties"),
	All = (IncludeBase | IncludeSuper | IncludeNull) UMETA(DisplayName = "Include All properties"),
};
ENUM_CLASS_FLAGS(EFuzzingFlags);
