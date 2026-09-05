#include "UnrealWebSocketClient.h"
#include "WebSocketsModule.h" // Needed for FWebSocketsModule
#include "Misc/Guid.h" // For generating unique IDs, if needed
#include "Serialization/JsonSerializer.h" // For JSON serialization
#include "Serialization/JsonWriter.h"

// Sets default values for this component's properties
UUnrealWebSocketClient::UUnrealWebSocketClient()
{
PrimaryComponentTick.bCanEverTick = false; // We don't need Tick for this example
}


void UUnrealWebSocketClient::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.0f,
			FColor::Cyan,
			TEXT("WEBSOCKET COMPONENT BEGINPLAY DAMMIT")
		);
	}

	ConnectToWebSocket();
}

void UUnrealWebSocketClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
Super::EndPlay(EndPlayReason);
DisconnectFromWebSocket();
}


// Called every frame
void UUnrealWebSocketClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction); // <--- Corrected this line!
	// No tick logic needed for this component, as PrimaryComponentTick.bCanEverTick is false
}

//9_4 this will tell us its secrets!  
void UUnrealWebSocketClient::ConnectToWebSocket()
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	const FString CleanUrl = WebSocketUrl.TrimStartAndEnd();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("CONNECTING TO [%s] LEN=%d"),
				*CleanUrl,
				CleanUrl.Len()
			)
		);
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(CleanUrl);

	WebSocket->OnConnected().AddUObject(
		this,
		&UUnrealWebSocketClient::OnConnected
	);

	WebSocket->OnConnectionError().AddUObject(
		this,
		&UUnrealWebSocketClient::OnConnectionError
	);

	WebSocket->OnClosed().AddUObject(
		this,
		&UUnrealWebSocketClient::OnClosed
	);

	WebSocket->OnMessage().AddUObject(
		this,
		&UUnrealWebSocketClient::OnMessage
	);

	WebSocket->Connect();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Attempting to connect to WebSocket: [%s], Len=%d"),
		*CleanUrl,
		CleanUrl.Len()
	);
}

//new 9_4 Dont forget to bring your towel...
void UUnrealWebSocketClient::DisconnectFromWebSocket()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();

		UE_LOG(
			LogTemp,
			Log,
			TEXT("Disconnected from WebSocket.")
		);
	}
}

void UUnrealWebSocketClient::SendGameStateUpdate(FString GameStateName, FString EventType, FString Message)
	{
if (WebSocket.IsValid() && WebSocket->IsConnected())
{
TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

// Match the DTO structure in Spring Boot: GameStateUpdateMessage
JsonObject->SetStringField(TEXT("gameStateName"), GameStateName);
JsonObject->SetStringField(TEXT("eventType"), EventType);
JsonObject->SetStringField(TEXT("message"), Message);
// You might add an ID or timestamp here too if your DTO had it
// JsonObject->SetStringField(TEXT("id"), FGuid::NewGuid().ToString());
// JsonObject->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());

FString OutputString;
TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

WebSocket->Send(OutputString);
UE_LOG(LogTemp, Log, TEXT("Sent WebSocket message: %s"), *OutputString);
}
else
{
UE_LOG(LogTemp, Warning, TEXT("WebSocket not connected. Cannot send message, crybaby!!."));
}
}

void UUnrealWebSocketClient::OnConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("WebSocket Connected FINALLY!"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.0f,
			FColor::Green,
			TEXT("WEB SOCKET CONNECTED FINALLY!")
		);
	}
}

void UUnrealWebSocketClient::OnConnectionError(const FString& Error)
{
	UE_LOG(
		LogTemp,
		Error,
		TEXT("WebSocket Connection Error: %s"),
		*Error
		);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.0f,
			FColor::Red,
			FString::Printf(
				TEXT("WEB SOCKET ERROR: %s"),
				*Error
			)
		);
	}
}

void UUnrealWebSocketClient::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
UE_LOG(LogTemp, Warning, TEXT("WebSocket Connection Closed. Status: %d, Reason: %s, WasClean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
}

void UUnrealWebSocketClient::OnMessage(const FString& Message)
{
// This is where you would handle incoming messages from Spring Boot, if any
UE_LOG(LogTemp, Log, TEXT("Received WebSocket Message: %s"), *Message);

	//9_4_Broadcast the Delegate enabling BPs to Respond!  Critical 
	OnMessageReceivedDelegate.Broadcast(Message);
}





