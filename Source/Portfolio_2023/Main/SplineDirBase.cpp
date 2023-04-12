// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineDirBase.h"

// Sets default values
ASplineDirBase::ASplineDirBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASplineDirBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EDrawDebugTrace::Type DrawVisibleType;
	//�� ������ �Ⱥ�����
	if (IsVisibleLine)
	{
		DrawVisibleType = EDrawDebugTrace::ForDuration;
	}
	else
	{
		DrawVisibleType = EDrawDebugTrace::None;
	}

	int TempIndex = (int)(Spline->GetSplineLength() / 100.f);

	SplinePointLocs.Empty();
	for (int i = 1; i < TempIndex; ++i)
	{
		SplinePointLocs.Add(Spline->GetLocationAtDistanceAlongSpline(i * 100.f, ESplineCoordinateSpace::World));
	}

	//�ʱ�ȭ
	PlayerLocSpline->ClearSplinePoints();

	if (TempArrowArray.Num() != 0)
	{
		for (int i = 0; i < TempArrowArray.Num(); ++i)
		{
			TempArrowArray[i]->DestroyComponent();
		}
		TempArrowArray.Empty();
	}


	////�ڽ� Ʈ���̽�
	TArray<AActor*> IgnoreList;
	FHitResult OutHitResult;
	for (int j = 0; j < SplinePointLocs.Num(); ++j)
	{
		bool tempbool = false;

		//�ݸ����� Default�϶��� �ε�ģ��, �������϶��� �Ⱥε�ģ��
		tempbool = UKismetSystemLibrary::BoxTraceSingle(
			GetWorld(), SplinePointLocs[j], SplinePointLocs[j] + FVector(0.f, 0.f, -10000.f), FVector(34.f, 34.f, 0.f),
			GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, IgnoreList, DrawVisibleType,
			OutHitResult, true, FLinearColor::Red, FLinearColor::Green, 7.f);

		//Ʈ���̽� ������ ���� �� ����
		if (tempbool)
		{
			if (OutHitResult.GetActor()->ActorHasTag(TEXT("Land")))
			{
				PlayerLocSpline->AddSplinePoint(OutHitResult.Location + FVector(0.f, 0.f, PlayerEyeHeight),
					ESplineCoordinateSpace::World, true);
			}
			else
			{
				UArrowComponent* NewArrow = NewObject<UArrowComponent>(this, UArrowComponent::StaticClass(), NAME_None);
				if (NewArrow)
				{
					NewArrow->RegisterComponent();
					NewArrow->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
					//NewArrow->AttachTo(Root);
					NewArrow->SetWorldTransform(
						FTransform(FRotator(90.f, 0.f, 0.f), FVector(OutHitResult.Location), FVector(150.f, 1.f, 1.f)));
					TempArrowArray.Add(NewArrow);
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void ASplineDirBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASplineDirBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

