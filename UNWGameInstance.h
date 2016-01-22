// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "UNWGameInstance.generated.h"


UCLASS()
class CPP_LOBBYSESSIONS_API UNWGameInstance : public UGameInstance
{
	GENERATED_BODY()
private:
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	/* Create Session */
	void Add_CreateSessionCompleteDelegate_Handle(IOnlineSessionPtr SessionInterface);
	void Set_SessionSettings(bool bIsLAN, bool bIsPresence, int32 NumberOfPlayers);

	bool HostSession(TSharedPtr<const FUniqueNetId> UserId,
					FName SessionName,
					bool bIsLan,
					bool bIsPresence,
					int32 NumberOfPlayers);
	
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	virtual void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
	/* End Create Session */

public:
	UNWGameInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void StartSession(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void DestroySession(FName SessionName);
};
