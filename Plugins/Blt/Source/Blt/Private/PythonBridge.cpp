// Fill out your copyright notice in the Description page of Project Settings.

#include "PythonBridge.h"


const UPythonBridge* UPythonBridge::Get()
{
	TArray<UClass*> PythonBridgeClasses;
	GetDerivedClasses(StaticClass(), PythonBridgeClasses);
	
	const uint32& BridgeCount = PythonBridgeClasses.Num();
	if (BridgeCount == 0u)
		return nullptr;
	
	return Cast<UPythonBridge>(PythonBridgeClasses[BridgeCount - 1u]->GetDefaultObject());
}
