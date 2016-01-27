// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "FSessionSettingsStruct.h"
#include "UNWGameInstance.generated.h"

#define SETTING_PASSWORD FName(TEXT("Password"))

#define LEVEL_SESSION FName(TEXT("Session"))
#define LEVEL_MAINMENU FName(TEXT("MainMenu"))

UCLASS()
class CPP_LOBBYSESSIONS_API UNWGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	FName GameSessionName;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	/* Create Session */
	void Set_SessionSettings(FSessionSettingsStruct& Settings);

	bool HostSession(TSharedPtr<const FUniqueNetId> UserId,
					FName SessionName, FSessionSettingsStruct& Settings);
	
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	virtual void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/* Destroy Session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void SetSearchSettings(bool bIsLAN, bool bIsPresence);

	/* Find Session */
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	void OnFindSessionsComplete(bool bWasSuccessful);

	/* Join Session */
	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


public:
	UNWGameInstance(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Online Session")
	FName PlayerName;

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	FName GetGameSessionName() const;

	/* Session Management */
	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void StartSession(FSessionSettingsStruct Settings, FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void DestroySession(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void DestroyActualSession();

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void FindOnlineGames();

	UFUNCTION(BlueprintCallable, Category = "Online Session")
	void JoinGame(FName SessionName);
};
