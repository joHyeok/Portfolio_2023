// Fill out your copyright notice in the Description page of Project Settings.


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

		//ûũ �ٿ�δ�
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

			//����Ʈ
			TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) { OnMountComplete(bSuccess); };
			Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

			//�Ϸ�Ǹ�
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
	//1�� ���� ���� �������� ���� �߰��ٿ�ε� ƨ�� ���� �ذ��� ���� �ڵ� 220422
	IFileManager& TempFileManager = IFileManager::Get();

	FString DeleteCachedManifestDir = FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/CachedBuildManifest.txt");
	UE_LOG(LogClass, Log, TEXT("Butter Log : Delete CachedBuildManifest : %s"), *DeleteCachedManifestDir);
	if (!ensure(TempFileManager.Delete(*DeleteCachedManifestDir)))
	{
		UE_LOG(LogClass, Error, TEXT("Unable to delete '%s'"), *DeleteCachedManifestDir);
	}

	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
	Downloader->Initialize("Windows", 8);

	//config���� url ��������
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

	//�Ŵ��佺Ʈ ������Ʈ
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
		//�Ϸ�
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

		//�Ϸ�Ǹ�
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
		//����Ʈ�ϱ�
		Downloader->MountChunks(ChunkDownloadList, MountCompleteCallback);
		return true;


	}
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));
	return false;
}

