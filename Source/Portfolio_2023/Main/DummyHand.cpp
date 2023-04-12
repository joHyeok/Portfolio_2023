// Fill out your copyright notice in the Description page of Project Settings.


#include "Main/DummyHand.h"
#include "DummyHand.h"

// Sets default values
ADummyHand::ADummyHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADummyHand::BeginPlay()
{
	Super::BeginPlay();

	//서버에서 더미 손 만들기 replicated여서 전 클라 전파
	if (GetWorld()->IsServer())
	{
		MakeDummyHand();
	}
}

// Called every frame
void ADummyHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//로컬일때만
	if (IsLocallyControlled())
	{
		//머리 회전
		SetLocationMeshOnCamera();

		//손 회전
		UpdateDummyHandState();
	}
	
}

void ADummyHand::MakeDummyHand()
{
	if (DummyHandClass)
	{
		//왼손
		LeftDummyHand = Cast<ADummyHandBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(
			GetWorld(), DummyHandClass, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
		if (LeftDummyHand)
		{
			LeftDummyHand->HandEnum = EControllerHand::Left;
			UGameplayStatics::FinishSpawningActor(LeftDummyHand, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)));

			LeftDummyHand->AttachToComponent(VROrigin, FAttachmentTransformRules(
				EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false));
		}

		//오른손
		RightDummyHand = Cast<ADummyHandBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(
			GetWorld(), DummyHandClass, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
		if (RightDummyHand)
		{
			RightDummyHand->HandEnum = EControllerHand::Right;
			UGameplayStatics::FinishSpawningActor(RightDummyHand, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)));

			RightDummyHand->AttachToComponent(VROrigin, FAttachmentTransformRules(
				EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false));
		}
	}
}

void ADummyHand::UpdateDummyHandState()
{
	if (!IsLocallyControlled()) return;

	//왼손
	if (LeftController)
	{
		Server_UpdateDummy(LeftController->MotionController->GetSocketLocation(NAME_None)
			, LeftController->MotionController->GetSocketRotation(NAME_None), true);
	}

	//오른손
	if (RightController)
	{
		Server_UpdateDummy(RightController->MotionController->GetSocketLocation(NAME_None),
			RightController->MotionController->GetSocketRotation(NAME_None), false);
	}
}

void ADummyHand::Server_UpdateDummy_Implementation(FVector Loc, FRotator Rot, bool IsLeft)
{
	//왼손
	if (IsLeft)
	{
		if (LeftDummyHand)
		{
			LeftDummyHand->SetActorLocation(Loc);
			LeftDummyHand->SetActorRotation(Rot);
		}
	}
	//오른손
	else
	{
		if (RightDummyHand)
		{
			RightDummyHand->SetActorLocation(Loc);
			RightDummyHand->SetActorRotation(Rot);
		}
	}
}

void ADummyHand::SetLocationMeshOnCamera()
{
	FVector TempLoc = Camera->GetRelativeLocation() + FVector(-13.f, -7.f, -170.f);
	PlayerCameraLoc = Camera->GetRelativeLocation();
	FRotator TempRot = Camera->GetRelativeRotation();

	//서버에 카메라 회전값 뿌리기
	Server_SetCameraLoc(TempLoc, TempRot);
}

void ADummyHand::Server_SetCameraLoc_Implementation(FVector Loc, FRotator Rot)
{
	Net_SetCameraLoc(Loc, Rot);
}

void ADummyHand::Net_SetCameraLoc_Implementation(FVector Loc, FRotator Rot)
{
	Mesh->SetRelativeLocation(Loc);

	//보정
	FRotator TempRot = FRotator(Rot.Pitch, Rot.Yaw + -90.f, Rot.Roll);
	DummyHead->SetRelativeRotation(TempRot);

	DummyHead->SetWorldLocation(Mesh->GetSocketLocation(TEXT("head")) + FVector(0.f, 0.f, -6.f));
}

