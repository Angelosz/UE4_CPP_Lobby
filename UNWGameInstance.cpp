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

	OnJoinSessionCompleteDelegate =
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnJoinSessionComplete);
}

/* Private */
/* Create Session */
void UNWGameInstance::Set_SessionSettings(FSessionSettingsStruct& Settings)
{
	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = Settings.bIsLAN;
	SessionSettings->bUsesPresence = Settings.bIsPresence;
	SessionSettings->NumPublicConnections = Settings.NumberOfPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

	if(Settings.bHasPassword)
	{
		SessionSettings->Set(SETTING_PASSWORD, Settings.Password, EOnlineDataAdvertisementType::DontAdvertise);
	}

	SessionSettings->Set(SETTING_MAPNAME, FString("NewMap"), EOnlineDataAdvertisementType::ViaOnlineService);
}

bool UNWGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId,
								  FName SessionName, FSessionSettingsStruct& Settings)
{
	const auto OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		const auto SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid() && UserId.IsValid())
		{
			Set_SessionSettings(Settings);

			OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			GameSessionName = SessionName;
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
		UGameplayStatics::OpenLevel(GetWorld(), LEVEL_SESSION, true, "listen");
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
				UGameplayStatics::OpenLevel(GetWorld(), LEVEL_MAINMENU, true);
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

/* Join Session */
bool UNWGameInstance::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	auto bSuccess = false;
	
	auto OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid() && UserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			bSuccess = SessionInterface->JoinSession(*UserId, SessionName, SearchResult);
		}
	}

	return bSuccess;
}

void UNWGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Joined Session: %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));

	auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			const auto PlayerController = GetFirstLocalPlayerController();

			FString TravelURL;

			if(PlayerController && SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Joined Session: %s, %d"), *TravelURL));

				PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute);
			}
		}
	}
}


/* Public */
FName UNWGameInstance::GetGameSessionName() const
{
	return GameSessionName;
}

/* Session Management */
void UNWGameInstance::StartSession(FSessionSettingsStruct Settings, FName SessionName)
{
	const auto Player = GetFirstGamePlayer();

	HostSession(Player->GetPreferredUniqueNetId(), SessionName, Settings);
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

void UNWGameInstance::DestroyActualSession()
{
	const auto OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			SessionInterface->DestroySession(GameSessionName);
		}
	}
}

void UNWGameInstance::FindOnlineGames()
{
	const auto Player = GetFirstGamePlayer();

	FindSessions(Player->GetPreferredUniqueNetId(), true, true);
}

void UNWGameInstance::JoinGame(FName SessionName)
{
	const auto Player = GetFirstGamePlayer();

	FOnlineSessionSearchResult SearchResult;

	if(SessionSearch->SearchResults.Num() > 0)
	{
		for (auto i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			if(SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId())
			{
				SearchResult = SessionSearch->SearchResults[i];

				JoinSession(Player->GetPreferredUniqueNetId(), SessionName, SearchResult);
				break;
			}
		}
	}
}
