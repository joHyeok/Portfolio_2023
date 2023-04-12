// Fill out your copyright notice in the Description page of Project Settings.
/*이 코드는 컴파일 에러가 나는 동작하지 않은 코드입니다.
오로지 포트폴리오용 코드로서 한 프로젝트의 일부 코드를 복사한 내용입니다.
이 코드를 내려받아 사용시 발생하는 버그에 대해 책임지지 않습니다.

InitPatch 함수는 매니페스트를 업데이트합니다.
PatchGame함수는 실제 pak을 다운로드하고 마운트합니다.
*/


#include "DownloadPakBase.h"
#include "ChunkDownloader.h"

// Sets default values
ADownloadPakBase::ADownloadPakBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADownloadPakBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADownloadPakBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADownloadPakBase::OnDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("ButterLandLog : Download complete So Mount"));

		//청크 다운로더
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		FJsonSerializableArrayInt DownloadedChunks;

		for (int32 ChunkID : ChunkDownloadList)
		{
			UE_LOG(LogClass, Warning, TEXT("ButterLandLog : Chunk ID is %d"), ChunkID);
			DownloadedChunks.Add(ChunkID);
		}

		UE_LOG(LogClass, Warning, TEXT("ButterLandLog : Chunks length is %d"), DownloadedChunks.Num());

		if (FCoreDelegates::OnMountPak.IsBound())
		{
			UE_LOG(LogClass, Warning, TEXT("ButterLandLog : OnMountPak is Bound"));

			//마운트
			TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) { OnMountComplete(bSuccess); };
			Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

			//완료되면
			OnPatchComplete.Broadcast(true);
		}
		else
		{
			UE_LOG(LogClass, Warning, TEXT("ButterLandLog : OnMountPak is UnBound"));
		}
	}
	else
	{

		UE_LOG(LogTemp, Display, TEXT("Load process failed"));

		OnPatchComplete.Broadcast(false);
	}
}

void ADownloadPakBase::InitPatch()
{
	//1차 개발 버그 수정으로 인한 추가다운로드 튕김 현상 해결을 위한 코드 220422
	IFileManager& TempFileManager = IFileManager::Get();

	FString DeleteCachedManifestDir = FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/CachedBuildManifest.txt");
	UE_LOG(LogClass, Log, TEXT("Butter Log : Delete CachedBuildManifest : %s"), *DeleteCachedManifestDir);
	if (!ensure(TempFileManager.Delete(*DeleteCachedManifestDir)))
	{
		UE_LOG(LogClass, Error, TEXT("Unable to delete '%s'"), *DeleteCachedManifestDir);
	}

	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
	Downloader->Initialize("Windows", 8);

	//config에서 url 가져오기
	FString CdnBaseUrls;
	FString ConfigSectionName = FString::Printf(TEXT("/Script/Plugins.ChunkDownloader %s"), *DeploymentName);
	GConfig->GetString(*ConfigSectionName, TEXT("CdnBaseUrls"), CdnBaseUrls, GGameIni);

	UE_LOG(LogClass, Warning, TEXT("URL is %s"), *CdnBaseUrls);
	UE_LOG(LogClass, Warning, TEXT("ContentBuildID is %s"), *ContentBuildId);

	if (Downloader->LoadCachedBuild(DeploymentName))
	{
		UE_LOG(LogClass, Warning, TEXT("LoadCacheBuild True!?!?"));
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("LoadCacheBuild True!?!?"));
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("LoadCacheBuild False!?!?"));
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("LoadCacheBuild False!?!?"));
	}

	//매니페스트 업데이트
	TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess)
	{
		bIsDownloadManifestUpToData = bSuccess;
		//if (bSuccess) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("UpdateCompeleteTrue"));
		//else GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("UpdateCompeleteFalse"));

		if (bSuccess)
		{
			UE_LOG(LogClass, Warning, TEXT("UdpateCompleteTrue!?!?"));
		}
		else
		{
			UE_LOG(LogClass, Warning, TEXT("UdpateCompleteFalse!?!?"));
		}
		//완료
		OnPatchReady.Broadcast(bSuccess);
	};
	Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
}

bool ADownloadPakBase::PatchGame()
{
	if (bIsDownloadManifestUpToData)
	{
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		for (int32 ChunkID : ChunkDownloadList)
		{
			int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
			UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
		}

		//완료되면
		TFunction<void(bool)> MountCompleteCallback = [this](bool bSuccess)
		{
			if (bSuccess)
			{
				// call the delegate
				OnPatchComplete.Broadcast(true);
			}
			else
			{
				// call the delegate
				OnPatchComplete.Broadcast(false);
			}
		};
		//마운트하기
		Downloader->MountChunks(ChunkDownloadList, MountCompleteCallback);
		return true;


	}
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));
	return false;
}

