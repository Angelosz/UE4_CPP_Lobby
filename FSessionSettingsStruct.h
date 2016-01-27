// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FSessionSettingsStruct.generated.h"
/**
*
*/
USTRUCT(BlueprintType)
struct CPP_LOBBYSESSIONS_API FSessionSettingsStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Settings")
	bool bIsLAN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Settings")
	bool bIsPresence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Settings")
	int32 NumberOfPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Settings")
	bool bHasPassword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Settings")
	FString Password;

	FSessionSettingsStruct()
	{
		bIsLAN = false;
		bIsPresence = false;
		NumberOfPlayers = 4;
		bHasPassword = false;
		Password = FString(TEXT("password"));
	}
};
