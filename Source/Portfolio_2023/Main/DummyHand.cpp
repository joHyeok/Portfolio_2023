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

	//�������� ���� �� ����� replicated���� �� Ŭ�� ����
	if (GetWorld()->IsServer())
	{
		MakeDummyHand();
	}
}

// Called every frame
void ADummyHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//�����϶���
	if (IsLocallyControlled())
	{
		//�Ӹ� ȸ��
		SetLocationMeshOnCamera();

		//�� ȸ��
		UpdateDummyHandState();
	}
	
}

void ADummyHand::MakeDummyHand()
{
	if (DummyHandClass)
	{
		//�޼�
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

		//������
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

	//�޼�
	if (LeftController)
	{
		Server_UpdateDummy(LeftController->MotionController->GetSocketLocation(NAME_None)
			, LeftController->MotionController->GetSocketRotation(NAME_None), true);
	}

	//������
	if (RightController)
	{
		Server_UpdateDummy(RightController->MotionController->GetSocketLocation(NAME_None),
			RightController->MotionController->GetSocketRotation(NAME_None), false);
	}
}

void ADummyHand::Server_UpdateDummy_Implementation(FVector Loc, FRotator Rot, bool IsLeft)
{
	//�޼�
	if (IsLeft)
	{
		if (LeftDummyHand)
		{
			LeftDummyHand->SetActorLocation(Loc);
			LeftDummyHand->SetActorRotation(Rot);
		}
	}
	//������
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

	//������ ī�޶� ȸ���� �Ѹ���
	Server_SetCameraLoc(TempLoc, TempRot);
}

void ADummyHand::Server_SetCameraLoc_Implementation(FVector Loc, FRotator Rot)
{
	Net_SetCameraLoc(Loc, Rot);
}

void ADummyHand::Net_SetCameraLoc_Implementation(FVector Loc, FRotator Rot)
{
	Mesh->SetRelativeLocation(Loc);

	//����
	FRotator TempRot = FRotator(Rot.Pitch, Rot.Yaw + -90.f, Rot.Roll);
	DummyHead->SetRelativeRotation(TempRot);

	DummyHead->SetWorldLocation(Mesh->GetSocketLocation(TEXT("head")) + FVector(0.f, 0.f, -6.f));
}

