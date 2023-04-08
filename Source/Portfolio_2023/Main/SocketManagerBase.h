// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SIOJsonObject.h"
#include "SocketIONative.h"
#include "GameFramework/Actor.h"
#include "SocketManagerBase.generated.h"

UCLASS()
class PORTFOLIO_2023_API ASocketManagerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASocketManagerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		/*스테이트 변화*/
		void Server_ChangeState(USIOJsonObject* iojson);
};
