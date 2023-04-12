// Fill out your copyright notice in the Description page of Project Settings.
/*이 코드는 컴파일 에러가 나는 동작하지 않은 코드입니다.
오로지 포트폴리오용 코드로서 한 프로젝트의 일부 코드를 복사한 내용입니다.
이 코드를 내려받아 사용시 발생하는 버그에 대해 책임지지 않습니다.
OnConstruction에서 동작하게 하여 에디터 상에서 아트팀이 다룰수 있도록 함.
캐릭터가 패스를 따라 움직일 때 충돌 오브젝트가 없어야하므로 캐릭터의 캡슐 콜리전의 크기만큼 박스트레이스를 사용함
*/

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
	//선 보일지 안보일지
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

	//초기화
	PlayerLocSpline->ClearSplinePoints();

	if (TempArrowArray.Num() != 0)
	{
		for (int i = 0; i < TempArrowArray.Num(); ++i)
		{
			TempArrowArray[i]->DestroyComponent();
		}
		TempArrowArray.Empty();
	}


	////박스 트레이스
	TArray<AActor*> IgnoreList;
	FHitResult OutHitResult;
	for (int j = 0; j < SplinePointLocs.Num(); ++j)
	{
		bool tempbool = false;

		//콜리전이 Default일때도 부딪친다, 오버랩일때는 안부딪친다
		tempbool = UKismetSystemLibrary::BoxTraceSingle(
			GetWorld(), SplinePointLocs[j], SplinePointLocs[j] + FVector(0.f, 0.f, -10000.f), FVector(34.f, 34.f, 0.f),
			GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, IgnoreList, DrawVisibleType,
			OutHitResult, true, FLinearColor::Red, FLinearColor::Green, 7.f);

		//트레이스 정보가 존재 할 때만
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

