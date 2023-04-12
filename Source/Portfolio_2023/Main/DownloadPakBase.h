// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DownloadPakBase.generated.h"

UCLASS()
class PORTFOLIO_2023_API ADownloadPakBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADownloadPakBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*�Ϸ��*/
	void OnDownloadComplete(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Patching")
		/*������ �� �ʱ�ȭ*/
		void InitPatch();

	UFUNCTION(BlueprintCallable, Category = "Patching")
		/*�ٿ�, ����Ʈ*/
		bool PatchGame();
};
