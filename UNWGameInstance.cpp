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

	SessionSettings->Set(SETTING_PARTYNAME, Settings.PartyName, EOnlineDataAdvertisementType::ViaOnlineService);
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
		SessionSettings->Set(SETTING_PASSWORD, Settings.Password, EOnlineDataAdvertisementType::ViaOnlineService);
	}
	
	MapName = FName(*Settings.MapName);
	SessionSettings->Set(SETTING_MAPNAME, Settings.MapName, EOnlineDataAdvertisementType::ViaOnlineService);
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
		UGameplayStatics::OpenLevel(GetWorld(), MapName, true, "listen");
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
	OnSearchCompleted.Broadcast(bWasSuccessful);

	const auto OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		auto SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		}
	}
}

/* Join Session */
bool UNWGameInstance::Get_SearchResultOf(FString OwningUserId, FOnlineSessionSearchResult& SearchResult)
{
	for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
	{
		if (Result.Session.OwningUserId->ToString() == OwningUserId)
		{
			SearchResult = Result;
			return true;
		}
	}
	return false;
}

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

TArray<FSessionSearchResultStruct> UNWGameInstance::GetSearchResultStructs() const
{
	TArray<FSessionSearchResultStruct> Results;
	for (auto result : SessionSearch->SearchResults)
	{
		FString OwningUserId = result.Session.OwningUserId->ToString();
		FString PartyName;
		result.Session.SessionSettings.Get(SETTING_PARTYNAME, PartyName);
		auto NumberOfOpenConnections = result.Session.SessionSettings.NumPublicConnections;
		auto NumberOfMaxConnections = result.Session.NumOpenPrivateConnections;
		auto bHasPassword = result.Session.SessionSettings.Settings.Contains(SETTING_PASSWORD);
		FString MapName;
		result.Session.SessionSettings.Get(SETTING_MAPNAME, MapName);

		FSessionSearchResultStruct resultStruct(OwningUserId, PartyName, NumberOfOpenConnections, NumberOfMaxConnections, bHasPassword, MapName);
		Results.Add(resultStruct);
	}

	return Results;
}

bool UNWGameInstance::CheckPasswordForSessionOf(FString SessionOwnerUserId, FString Password)
{
	FOnlineSessionSearchResult SearchResult;
	if (Get_SearchResultOf(SessionOwnerUserId, SearchResult))
	{
		FString SessionPassword;
		if (SearchResult.Session.SessionSettings.Get(SETTING_PASSWORD, SessionPassword))
		{
			if (SessionPassword == Password)
			{
				return true;
			}
		}
	}

	return false;
}

void UNWGameInstance::JoinGame(FName SessionName, FString OwningUserId)
{
	const auto Player = GetFirstGamePlayer();

	FOnlineSessionSearchResult SearchResult;

	if (Get_SearchResultOf(OwningUserId, SearchResult))
	{
		FString hola = SearchResult.Session.OwningUserId->ToString();
		if (SearchResult.Session.OwningUserId != Player->GetPreferredUniqueNetId())
		{
			JoinSession(Player->GetPreferredUniqueNetId(), SessionName, SearchResult);
		}
	}
}
