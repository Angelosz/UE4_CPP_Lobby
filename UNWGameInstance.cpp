// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_LobbySessions.h"
#include "UNWGameInstance.h"

UNWGameInstance::UNWGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnCreateSessionCompleteDelegate = 
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = 
		FOnStartSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnStartOnlineGameComplete);
}
/* Private */
/* Create Session */
void UNWGameInstance::Set_SessionSettings(bool bIsLAN, bool bIsPresence, int32 NumberOfPlayers)
{
	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = bIsLAN;
	SessionSettings->bUsesPresence = bIsPresence;
	SessionSettings->NumPublicConnections = NumberOfPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

	SessionSettings->Set(SETTING_MAPNAME, FString("NewMap"), EOnlineDataAdvertisementType::ViaOnlineService);
}

bool UNWGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId,
								  FName SessionName, bool bIsLAN,
								  bool bIsPresence, int32 NumberOfPlayers)
{
	const auto OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid() && UserId.IsValid())
		{
			Set_SessionSettings(bIsLAN, bIsPresence, NumberOfPlayers);

			OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			return SessionInterface->CreateSession(*UserId, SessionName, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsystem found!"));
	}
	return true;
}

void UNWGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
				FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
	const auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
			if(bWasSuccessful)
			{
				OnStartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				SessionInterface->StartSession(SessionName);
			}
		}
	}
}

void UNWGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, 
				FString::Printf(TEXT("Session Started %s, %d"), *SessionName.ToString(), bWasSuccessful));

	const auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	if(bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "CreatedSession", true, "listen");
	}
}




/* Public */
void UNWGameInstance::StartSession(FName SessionName)
{
	const auto Player = GetFirstGamePlayer();

	HostSession(Player->GetPreferredUniqueNetId(), SessionName, true, true, 2);
}

