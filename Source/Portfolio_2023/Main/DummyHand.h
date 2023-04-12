// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DummyHand.generated.h"

UCLASS()
class PORTFOLIO_2023_API ADummyHand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADummyHand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//더미손 만들기
	void MakeDummyHand();

	//더미손 위치 회전 동기화
	void UpdateDummyHandState();
	UFUNCTION(Server, Reliable)
		void Server_UpdateDummy(FVector Loc, FRotator Rot, bool IsLeft);
	void Server_UpdateDummy_Implementation(FVector Loc, FRotator Rot, bool IsLeft);

	//카메라와 매시 위치 함수
	void SetLocationMeshOnCamera();
	UFUNCTION(Server, Reliable)
		void Server_SetCameraLoc(FVector Loc, FRotator Rot);
	void Server_SetCameraLoc_Implementation(FVector Loc, FRotator Rot);
	UFUNCTION(NetMulticast, Reliable)
		void Net_SetCameraLoc(FVector Loc, FRotator Rot);
	void Net_SetCameraLoc_Implementation(FVector Loc, FRotator Rot);
};
