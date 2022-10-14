// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PythonBridge.generated.h"


UCLASS(Abstract, Blueprintable)
class EBLT_API UPythonBridge final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Python")
	static const UPythonBridge* Get();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Python")
	FString GenerateStringFromRegex(const FString& Regex) const;
};
