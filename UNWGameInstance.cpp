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

	OnDestroySessionCompleteDelegate =
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnDestroySessionComplete);

	OnFindSessionsCompleteDelegate =
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnFindSessionsComplete);
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

/* Destroy Session */
void UNWGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("Session Destroyed %s, %d"), *SessionName.ToString(), bWasSuccessful));

	const auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			if(bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), "MainMenu", true);
			}
		}
	}
}


/* Find Session */
void UNWGameInstance::SetSearchSettings(bool bIsLAN, bool bIsPresence)
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = bIsLAN;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->PingBucketSize = 100;

	if(bIsPresence)
	{
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
	}
}

void UNWGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid() && UserId.IsValid())
		{
			SetSearchSettings(bIsLAN, bIsPresence);

			auto SearchSettingsRef = SessionSearch.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			SessionInterface->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

void UNWGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Find Sessions, %d"), bWasSuccessful));

	const auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			if(bWasSuccessful)
			{
				auto NumberOfSessionsFound = SessionSearch->SearchResults.Num();

				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Number of Sessions Found: %d"), NumberOfSessionsFound));

				if (NumberOfSessionsFound > 0)
				{
					for (auto i = 0; i < NumberOfSessionsFound; i++)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
														 FString::Printf(TEXT("Session Found: %s, %d"),
																		 *(SessionSearch->SearchResults[i].Session.OwningUserName), i + 1));
					}
				}
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Finding End")));
			}
		}
	}
}

/* Public */
void UNWGameInstance::StartSession(FName SessionName)
{
	const auto Player = GetFirstGamePlayer();

	HostSession(Player->GetPreferredUniqueNetId(), SessionName, true, true, 4);
}

void UNWGameInstance::DestroySession(FName SessionName)
{
	const auto OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			SessionInterface->DestroySession(SessionName);
		}
	}
}

void UNWGameInstance::FindOnlineGames()
{
	const auto Player = GetFirstGamePlayer();

	FindSessions(Player->GetPreferredUniqueNetId(), true, true);
}
