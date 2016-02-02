// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SessionSearchResultStruct.generated.h"

USTRUCT(BlueprintType)
struct CPP_LOBBYSESSIONS_API FSessionSearchResultStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Session Search Result")
		FString OwningUserId;

	UPROPERTY(BlueprintReadOnly, Category = "Session Search Result")
		FString PartyName;

	UPROPERTY(BlueprintReadOnly, Category = "Session Search Result")
		int32 NumberOfOpenConnections;

	UPROPERTY(BlueprintReadOnly, Category = "Session Search Result")
		bool bHasPassword;

	FSessionSearchResultStruct()
	{
		OwningUserId = "User Id";
		PartyName = "Party Name";
		NumberOfOpenConnections = 0;
		bHasPassword = false;
	}

	FSessionSearchResultStruct(FString _OwningUserId, FString _PartyName, int32 _NumberOfOpenConnections, bool _bHasPassword)
		: OwningUserId(_OwningUserId), PartyName(_PartyName), NumberOfOpenConnections(_NumberOfOpenConnections), bHasPassword(_bHasPassword) {}
};
