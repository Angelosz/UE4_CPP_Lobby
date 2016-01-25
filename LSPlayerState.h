// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "LSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CPP_LOBBYSESSIONS_API ALSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, Category = "LobbySessions")
	void SetName(const FString& Name);
	
};
