#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IWebSocket.h" // For IWebSocket
#include "WebSocketsModule.h" // For IWebSocketsModule
#include "UnrealWebSocketClient.generated.h" // Generated header must be last


//new 9_4 Dynamic multicast delegate for BP Subscription
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageReceived, const FString&, Message);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LGSCOREJAN12026_API UUnrealWebSocketClient : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUnrealWebSocketClient();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// WebSocket URL for your Spring Boot application
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSockets")
	FString WebSocketUrl = TEXT("ws://localhost:8050/ws"); // Adjust port if necessary

	// Our WebSocket connection object
	TSharedPtr<IWebSocket> WebSocket;

	// Function to connect to the WebSocket server
	UFUNCTION(BlueprintCallable, Category = "WebSockets")
	void ConnectToWebSocket();

	// Function to close the WebSocket connection
	UFUNCTION(BlueprintCallable, Category = "WebSockets")
	void DisconnectFromWebSocket();

	// Function to send a game state update message
	UFUNCTION(BlueprintCallable, Category = "WebSockets")
	void SendGameStateUpdate(FString GameStateName, FString EventType, FString Message);

	//new 9_4 Delegate Declarations for BP
	// This delegate will be broadcast when a message is received, allowing Blueprints to react
	UPROPERTY(BlueprintAssignable, Category = "WebSockets")
	FOnWebSocketMessageReceived OnMessageReceivedDelegate; // <-- Renamed to avoid confusion with the internal OnMessage function
	

	
protected:
	// WebSocket Delegates
	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessage(const FString& Message);
};