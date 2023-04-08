// Fill out your copyright notice in the Description page of Project Settings.
/*이 코드는 컴파일 에러가 나는 동작하지 않은 코드입니다.
오로지 포트폴리오용 코드로서 한 프로젝트의 일부 코드를 복사한 내용입니다.

Server_ChangeState 함수는 홀덤 게임 진행중 변경된 데이터를 socket으로부터 json으로 받아 처리하는 함수입니다.
json을 파싱하여 나온 프로퍼티에 따라 누구 차례인지, 판돈은 얼마인지, 누가 얼마를 걸었는지 등을 알 수 있고
그에 맞는 동작을 실행합니다.

레벨에는 ATableChairBase라는 액터가 있고 이 액터는 화면상의 프로필위젯, 코인, 카드 액터를 관장하는 클래스입니다.
SocketManager는 ATableChairBase 액터를 가지고 동작을 수행합니다.

초기 개발 당시 if 문으로만 작성하였고 점점 프로퍼티가 늘어나면서 코드가 너무 길어진 감이 있습니다. 
switch문이나 다른 함수나 클래스로 분할하여 사용했으면 어땠을까 라는 아쉬움이 남습니다.
*/


#include "SocketManagerBase.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/FieldPath.h"
#include "Dom/JsonObject.h"
#include "UObject/ReflectedTypeAccessors.h"

#include "../../NFTHoldemGame.h"
#include "../../TH_Socket/Framework/SocketPCTest.h"
#include "TableChairBase.h"
#include "TotalPotBase.h"
#include "CommunityCardLocBase.h"
#include "HoldemCardDeckBase.h"
#include "HoldemCardBase.h"
#include "../../TestParentsChild/T_SpectatorPlayerBase.h"
#include "SoundActor.h"
#include "../Framework/NFTGIBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASocketManagerBase::ASocketManagerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASocketManagerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASocketManagerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASocketManagerBase::GetWorldTCs()
{
	TArray<AActor*> TempTCArray;

	if (TableChairClass)
	{
		//월드의 모든 TC들 가져오기
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TableChairClass, TempTCArray);
	}
	else
	{
		NHLOG_WARNING(TEXT("Check TC Class Ref"));
		return;
	}

	//초기화
	TCArray.Empty();

	//TC가 0개가 아니라면
	if (TempTCArray.Num() != 0)
	{
		//TC배열 만들기
		for (auto Elem : TempTCArray)
		{
			TCArray.Add(Cast<ATableChairBase>(Elem));
		}
	}
	else
	{
		NHLOG_WARNING(TEXT("Check World TC Num"));
		return;
	}

	//플레이어 배열이 0이 아닐경우 인덱스 순서대로 소팅
	if (TCArray.Num() != 0)
	{
		TCArray.Sort([](const ATableChairBase& A, const ATableChairBase& B)
			{
				return A.MyTCIndex < B.MyTCIndex;
			});
	}

	NHLOG_WARNING(TEXT("TCArrayNum is %d"), TCArray.Num());
}

void ASocketManagerBase::ConnectHoldemSocketIO(FString SessionId, bool bIsReconnection)
{
	NHLOG_WARNING(TEXT("Connected"));

	//메세지 전달
	SocketIOClient->BindEventToFunction(TEXT("msg"), TEXT("Server_MsgEvent"), this);
	//에러 메세지 전달
	SocketIOClient->BindEventToFunction(TEXT("errorMsg"), TEXT("Server_ErrorMsgEvent"), this);

	//연결
	SocketIOClient->BindEventToFunction(TEXT("connect"), TEXT("Server_Connect"), this);

	//핑
	SocketIOClient->BindEventToFunction(TEXT("ping"), TEXT("Server_Ping"), this);

	//스테이트 변화
	SocketIOClient->BindEventToFunction(TEXT("onChangeState"), TEXT("Server_ChangeState"), this);

	//방 정보 전달
	SocketIOClient->BindEventToFunction(TEXT("sendRoomInfo"), TEXT("Server_SendRoomInfo"), this);

	//플레이어가 방에 추가됨
	SocketIOClient->BindEventToFunction(TEXT("onAddPlayer"), TEXT("Server_OnAddPlayer"), this);

	//방에서 플레이어가 나옴
	SocketIOClient->BindEventToFunction(TEXT("onRemovePlayer"), TEXT("Server_OnRemovePlayer"), this);

	//배열로 방 데이터 전달
	SocketIOClient->BindEventToFunction(TEXT("sendRoomList"), TEXT("Server_SendRoomList"), this);

	//내 정보 받기
	SocketIOClient->BindEventToFunction(TEXT("sendPlayerInfo"), TEXT("Server_SendPlayerInfo"), this);

	//채팅
	SocketIOClient->BindEventToFunction(TEXT("onUserMsg"), TEXT("Server_OnUserMsg"), this);

	//게임 시작
	SocketIOClient->BindEventToFunction(TEXT("onGameStart"), TEXT("Server_OnGameStart"), this);

	//서버 준비 완료
	SocketIOClient->BindEventToFunction(TEXT("onReady"), TEXT("Server_OnReady"), this);


	//서버 준비 완료의 OnReady가 오면 PlayerInfo를 요청함

	//현존 하는 방 요청
	//위젯에서 함
	//ChatEmit_bool(TEXT("getRoomList"), false);
}

void ASocketManagerBase::Server_ChangeState(USIOJsonObject* iojson)
{
	if (iojson->HasField(TEXT("game")))
	{
		//NHLOG_WARNING(TEXT("Yes Has game Field"));
		NHLOG_WARNING(TEXT("game Change state %s"), *iojson->GetField(TEXT("game"))->AsString());

		//game 타입의 오브젝트
		USIOJsonObject* GameJsonObject = iojson->GetField(TEXT("game"))->AsObject();

		//game의 property
		FString GamePropertyType = GameJsonObject->GetStringField(TEXT("property"));

		//사용자가 사용할 액션
		if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::actionOption))
		{
			TArray<FString> ActionArray;
			//사용할 수 있는 액션 전달
			for (auto Elem : GameJsonObject->GetArrayField(TEXT("value")))
			{
				ActionArray.Add(Elem->AsString());
			}

			ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (!PC)
			{
				NHLOG_WARNING(TEXT("Check PC is not Valid"));
				return;
			}

			//내 액션 쓰기
			PC->SetMyAction(ActionArray);

			//bet widget 액션 블록 체크
			for (auto Elem : SortTCArray)
			{
				Elem->CheckMyTurnAndAction();
			}
		}
		//현재 누구 차례?
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::playerIndex))
		{
			//게임 카운트만큼 뺀다
			//플레이어라면 카운트가 0이라서 상관 없음
			//옵저버라면 카운트가 존재해서 몇번째 인덱스로 가야하는지 판단하기
			NHLOG_WARNING(TEXT("GamePlayCount is %d"), GamePlayCount_Observer);
			int CurrentPlayerTurnIndex = (int)GameJsonObject->GetNumberField(TEXT("value"));
			//관전자용 계산으로 더이상 카운트를 세지 않음
			//(int)GameJsonObject->GetNumberField(TEXT("value")) - GetMoveArrayCount_Observer(GamePlayCount_Observer);

			//카운트 만큼 뺐는데 음수면 6더하기
			if (CurrentPlayerTurnIndex < 0)
			{
				CurrentPlayerTurnIndex += 6;
			}

			//NHLOG_WARNING(TEXT("game : Current Turn is %d"), CurrentPlayerTurnIndex);

			if (SortTCArray.Num() != 6)
			{
				NHLOG_WARNING(TEXT("Check SortTCArray"));
				return;
			}

			for (auto Elem : SortTCArray)
			{
				NHLOG_WARNING(TEXT("SortTC의 순서 player id is %d"), Elem->GetPlayerUID());
			}

			NHLOG_WARNING(TEXT("CurrentPlayerTurnIndex is %d"), CurrentPlayerTurnIndex);

			//현재 차례 TC에게는 너 차례라고 전달, 아니면 너 차례 아니니까 초기화 할 거 하라고 전달
			for (int i = 0; i < SortTCArray.Num(); i++)
			{
				if (CurrentPlayerTurnIndex == i)
				{
					//NHLOG_SCREEN(TEXT("This is %d Turn"), i);

					SortTCArray[i]->CheckMyTurn();
				}
				else
				{
					//NHLOG_SCREEN(TEXT("%d is Init Turn"), i);
					SortTCArray[i]->InitTurn();
				}
			}

			//위에서 Turn Type을 제대로 정하고 나서 위젯 로직 실행
			// 안그러면 전 플레이어가 MyTurn이라는 타입을 갖고 있어서 꼬이는듯..
			for (int i = 0; i < SortTCArray.Num(); i++)
			{
				//SortTCArray[i]->CheckBetWidgetVisible();
			}

		}
		//게임의 상태 전달
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::status))
		{
			FString GameStatus = GameJsonObject->GetStringField(TEXT("value"));
			NHLOG_WARNING(TEXT("GameStatus %s"), *GameStatus);

			ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (!PC)
			{
				NHLOG_WARNING(TEXT("Check PC is not Valid"));
				return;
			}

			//현재 게임 상태 쓰기
			PC->SetWidgetCurrentGameStatus(GameStatus);
			for (auto Elem : SortTCArray)
			{
				if (Elem->GetBetBubbleType() != EBetType::Fold)
				{
					//버블 제거 : 폴드가 아닐때만
					Elem->HideBetBubble();
				}

				if (GameStatus != TEXT("pre-flop")) // 22/10/26 한결 : 첫 입장 때 BB Sb의 코인이 바로 토탈 팟으로 가버리므로이 경우는 제외, 스폰만 함
				{
					Elem->MoveTotalPot();
				}

				if (GameStatus == TEXT("pre-flop"))
				{
					FCard TempCard = FCard();
					Elem->TakeCard(TempCard);
					Elem->TakeCard(TempCard);
				}

				//턴 넘어갈 때 턴 머니 초기화
				Elem->SetTurnMoneyForWin(0);
			}

			//재시작 : 존재하는 카드들 전부 리셋
			if (GameStatus == TEXT("restart"))
			{
				UsePlayerReSort_Observer = true;
				AfterHandRankShowAddMoney = false;

				FTimerHandle WaitHandle;
				float WaitTime = 4.f; //시간을 설정하고
				PC->ResetTotalPotCost();

				GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
					{

						//커뮤니티 카드 제거
						for (auto Elem : CommunityCardArray)
						{
							if (Elem)
							{
								Elem->Destroy();
							}
						}
						CommunityCardArray.Empty();

						//커뮤Loc bool 값 초기화
						for (auto Elem : CommunityLocActorArray)
						{
							Elem->IsCardSpawned = false;
						}

						for (auto Elem : SortTCArray)
						{
							//핸드카드 제거
							Elem->DestroyHandCards();

							//버블 제거
							Elem->HideBetBubble();

							//프로필 초기화
							Elem->InitProfileWidget();

							//게임끝나면 타이머 초기화
							Elem->InitProfileTimer();

							Elem->ResetWidgets();

							//코인 제거
							Elem->AllCoinDestroy();

							//토탈 팟 초기화 
							Elem->ResetTotalPot();

							Elem->ElseCoinDestroy();


							//show down을 위해 사용했던 배열들, 변수들 초기화
							Elem->InitCardArraysForShowDown();

							//승리 텍스트 collapsed
							Elem->SetVisible_WinText(ESlateVisibility::Collapsed);

							if (!IsValid(SpawnedObserver))
							{
								Elem->InitLocal(); //옵저버가 없다면 로컬세팅, 있다면 패스
								NHLOG_WARNING(TEXT("Init Local"));
							}

							//이번판 총 판돈 금액 초기화
							Elem->SetMyCurrentTurnMoney(0);

							//핸드랭크 버블 초기화
							Elem->HideHandRankBubble();

							//더하기 위젯 감추기
							Elem->CheckHiddemAddMoney = true;
							Elem->HiddenAddMoney(true);

							//턴 머니 초기화
							Elem->SetTurnMoneyForWin(0);

							Elem->SetVisibleOfTotalPot(true);
						}
					}), WaitTime, false);
			}
		}
		//현재 게임의 판돈
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::stake))
		{
			int CurrentGameStake = (int)GameJsonObject->GetNumberField(TEXT("value"));
			NHLOG_WARNING(TEXT("GameStake %d"), CurrentGameStake);

			ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (!PC)
			{
				NHLOG_WARNING(TEXT("PC is not valid"));
				return;
			}

			//위젯에 현재 판의 판돈쓰기
			PC->SetCurrentRaisePot_Widget(CurrentGameStake);
		}
		//게임의 판돈
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::totallStake))
		{
			int GameTotalStake = (int)GameJsonObject->GetNumberField(TEXT("value"));
			NHLOG_WARNING(TEXT("GameTotalStake %d"), GameTotalStake);

			//총 금액 쓰기
			if (WorldTotalPot)
			{
				WorldTotalPot->SetWorldTotalPotText(GameTotalStake);
			}
			else
			{
				NHLOG_WARNING(TEXT("CheckWorldTotalPot"));
			}

			ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (!PC)
			{
				NHLOG_WARNING(TEXT("PC is not valid"));
				return;
			}

			//위젯에 총 게임 판돈 쓰기
			PC->SetTotalRaisePot_Widget(GameTotalStake);
		}
		//레이즈 유무
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::isRaise))
		{
			bool IsRaise = GameJsonObject->GetBoolField(TEXT("value"));
			NHLOG_WARNING(TEXT("Raise %s"), (IsRaise == true ? TEXT("true") : TEXT("false")));
		}
		//커뮤니티 카드
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::communityCards))
		{
			TArray<FCard> CommunityCards;

			for (auto Elem : GameJsonObject->GetArrayField(TEXT("value")))
			{
				NHLOG_WARNING(TEXT("CommunityCard %s"), *Elem->AsString());

				TArray<FString> ParseArray;
				Elem->AsString().ParseIntoArray(ParseArray, TEXT(","));

				TArray<FString> ParseArray_Small;
				ParseArray[0].ParseIntoArray(ParseArray_Small, TEXT("\""));
				//문자
				FString CardShape = ParseArray_Small[1];

				TArray<FString> ParseArray_Small_2;
				ParseArray[1].ParseIntoArray(ParseArray_Small_2, TEXT("]"));
				int32 CardNumber = FCString::Atoi(*ParseArray_Small_2[0]);

				ECardShape TempShapeEnum = ECardShape::None;
				if (CardShape == TEXT("s")) TempShapeEnum = ECardShape::Spade;
				else if (CardShape == TEXT("h")) TempShapeEnum = ECardShape::Heart;
				else if (CardShape == TEXT("c")) TempShapeEnum = ECardShape::Clover;
				else if (CardShape == TEXT("d")) TempShapeEnum = ECardShape::Diamond;

				FCard TempCard;
				TempCard.CardSuit = TempShapeEnum;
				TempCard.CardPower = CardNumber;

				//파싱한 카드 저장
				CommunityCards.Add(TempCard);
			}

			//가져온 카드 수만큼 돌면서 덱에게 카드 스폰하라고 하기
			for (int i = 0; i < CommunityCards.Num(); i++)
			{
				//카드 인덱스가 이미 배치 되어 있다면
				if (!CommunityLocActorArray[i]->IsCardSpawned)
				{
					FVector TempCardLoc = CommunityLocActorArray[i]->GetActorLocation();
					FTransform TempCardTrans = FTransform(CommunityLocActorArray[i]->GetActorRotation(), TempCardLoc, FVector(2.75f, 2.8875f, 2.75f));

					//카드 스폰 후 움직임
					//스폰한 카드 저장
					CommunityCardArray.Add(
						WorldCardDeckActor->SpawnCardAndMove(TempCardTrans, CommunityCards[i], true, false));

					//스폰했다고 true 처리
					CommunityLocActorArray[i]->IsCardSpawned = true;
				}
			}

			for (auto Elem : SortTCArray)
			{
				//로컬 일때만 하기
				if (Elem->GetPlayerType() == EPlayerType::LocalPlayer)
				{
					//TC한테 최고 랭크 표시하라고 하기
					Elem->SetHandRankBubble(CommunityCards);
				}

				//커뮤니티 카드 TC에게 알려주기 : showdown할 때 쓰라고
				Elem->SetCommunityCardArray(CommunityCardArray);
			}

		}
		//핸드카드
		else if (GamePropertyType == NHDataBase::GetEnumNameToString(EGameJsonType::handCards))
		{
			TArray<FCard> HandCards;

			for (auto Elem : GameJsonObject->GetArrayField(TEXT("value")))
			{
				NHLOG_WARNING(TEXT("Handcard %s"), *Elem->AsString());

				TArray<FString> ParseArray;
				Elem->AsString().ParseIntoArray(ParseArray, TEXT(","));

				TArray<FString> ParseArray_Small;
				ParseArray[0].ParseIntoArray(ParseArray_Small, TEXT("\""));
				//문자
				FString CardShape = ParseArray_Small[1];

				TArray<FString> ParseArray_Small_2;
				ParseArray[1].ParseIntoArray(ParseArray_Small_2, TEXT("]"));
				int32 CardNumber = FCString::Atoi(*ParseArray_Small_2[0]);

				ECardShape TempShapeEnum = ECardShape::None;
				if (CardShape == TEXT("s")) TempShapeEnum = ECardShape::Spade;
				else if (CardShape == TEXT("h")) TempShapeEnum = ECardShape::Heart;
				else if (CardShape == TEXT("c")) TempShapeEnum = ECardShape::Clover;
				else if (CardShape == TEXT("d")) TempShapeEnum = ECardShape::Diamond;

				FCard TempCard;
				TempCard.CardSuit = TempShapeEnum;
				TempCard.CardPower = CardNumber;

				//파싱한 카드 저장
				HandCards.Add(TempCard);
			}
			//모든 TC
			for (auto Elem : SortTCArray)
			{
				Elem->SetHandCard(HandCards);
			}

		}
	}
	else if (iojson->HasField(TEXT("player")))
	{
		//NHLOG_WARNING(TEXT("Yes Hasnt player Field"));
		NHLOG_WARNING(TEXT("player Change state %s"), *iojson->GetField(TEXT("player"))->AsString());

		//player 타입의 오브젝트
		USIOJsonObject* PlayerJsonObject = iojson->GetField(TEXT("player"))->AsObject();

		//player의 id
		int PlayerID = (int)PlayerJsonObject->GetNumberField(TEXT("id"));
		//NHLOG_WARNING(TEXT("player : id %d"), PlayerID);

//player의 property
		FString PlayerPropertyType = PlayerJsonObject->GetStringField(TEXT("property"));

		//이름
		if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::name))
		{
			FString PlayerName = PlayerJsonObject->GetStringField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : name %s"), *PlayerName);

			//플레이어 전체 배열의 이름도 변경
			for (auto Elem : AllPlayerInfoArray)
			{
				//id가 같다면 이름 변경
				if (Elem.PlayerUID == PlayerID)
				{
					NHLOG_WARNING(TEXT("AllPlayerInfo Change Name id %d, name %s"), PlayerID, *PlayerName);
					Elem.PlayerName = PlayerName;
				}
			}

			for (auto Elem : SortTCArray)
			{
				//UID가 같다면
				if (Elem->GetPlayerUID() == PlayerID)
				{
					//TC에 변수 쓰기
					Elem->SetPlayerNickName(PlayerName);

					//닉네임 변경
					Elem->ChangeNickName(PlayerName);
					break;
				}
			}
		}
		//가진 돈
		else if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::money))
		{
			int PlayerMoney = (int)PlayerJsonObject->GetNumberField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : Current Money is %d"), PlayerMoney);

			//가진 돈 변경이라면 Bet위젯에 써주기
			if (LocalPlayerID == PlayerID)
			{
				ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
				if (!PC)
				{
					NHLOG_WARNING(TEXT("PC is not valid"));
					return;
				}

				//현재 내가 가진 돈 위젯에 쓰기
				PC->SetCurrentMyMoney_Widget(PlayerMoney);
			}

			for (auto Elem : SortTCArray)
			{
				//UID가 같다면
				if (Elem->GetPlayerUID() == PlayerID)
				{
					//win이 왔을 때 addMoney표시를 위해 설정
					Elem->SetTurnMoneyForWin(PlayerMoney);

					//더하기 금액 쓰기
					Elem->SetAddMoney(PlayerMoney);

					//true라면 핸드랭크를 하고 난 이후이고 돈이 왔다면 얻은것임
					if (AfterHandRankShowAddMoney)
					{
						Elem->HiddenAddMoney(false); //프레임과 돈이 보이게
					}

					//가진돈 변경
					Elem->ChangePlayerMoney(PlayerMoney);

					break;
				}
			}
		}
		//플레이어 상태
		else if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::status))
		{
			FString PlayerStatus = PlayerJsonObject->GetStringField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : PlayerStatus %s"), *PlayerStatus);

			ASocketPCTest* PC = Cast<ASocketPCTest>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (!PC)
			{
				NHLOG_WARNING(TEXT("PC is not valid"));
				return;
			}

			//win으로 왔을 때
			if (PlayerStatus == TEXT("win"))
			{
				//전원이 폴드인지 판단하기 위해서
				int CheckAllPlayerFold = 0;

				for (auto Elem : SortTCArray)
				{
					//UID가 같다면
					if (Elem->GetPlayerUID() == PlayerID)
					{
						//add money를 보이게 하기 위해서 false처리
						Elem->CheckHiddemAddMoney = false;


						//Money가 있다면 표시
						//if (Elem->GetTurnMoneyForWin() != 0)
						//{
						//	Elem->SetAddMoney(Elem->GetTurnMoneyForWin());
						//}
					}

					//폴드가 아니라면 
					if (Elem->GetPlayerStatus() != TEXT("fold") && Elem->GetPlayerType() != EPlayerType::None)
					{
						//증가하기
						CheckAllPlayerFold++;
					}
				}

				//전원 폴드 상태가 1이라면 나 빼고 전원 폴드임
				//더하기 돈 표시해줘야함
				if (CheckAllPlayerFold == 1)
				{
					for (auto Elem : SortTCArray)
					{
						//UID가 같다면
						if (Elem->GetPlayerUID() == PlayerID)
						{
							//add money 보이게 함
							NHLOG_WARNING(TEXT("Show Add Money Work Win Proccess"));
							Elem->HiddenAddMoney(Elem->CheckHiddemAddMoney); //프레임과 돈이 보이게
							Elem->HiddenWinText(Elem->CheckHiddemAddMoney); //텍스트만 보이게
						}
					}
				}
			}

			if (PlayerID == LocalPlayerID)
			{
				PC->SetVisibleRaiseOverlay(false);//Status가 변하면 뭐든간에 RaiseOverlay의 visible을 꺼줌
				NHLOG_WARNING((TEXT("Local Observer")));
				//내 상태가 옵저버로 변했다면
				if (PlayerStatus == TEXT("observer"))
				{
					NHLOG_WARNING((TEXT("Block BetWidget")));

					//플레이어에서 옵저버가 되었으므로 카운트수를 다시 0부터 세줌
					GamePlayCount_Observer = 0;

					//옵저버가 되면 id 저장 배열 초기화 : 이전 데이터가 남아 있을지 모르니까
					SavePlayerIDArray_Observer.Empty();

					//버튼 다 블록
					PC->VisibleBetWidgetAction(ESlateVisibility::Visible);

					for (auto Elem : SortTCArray)
					{
						if (Elem->MyTCIndex == 0)
						{
							Elem->HidePlayerWidget();
						}
					} //옵저버로 변했으므로 내 위젯을 꺼줌
				}
			}
			for (auto Elem : SortTCArray)
			{
				//UID가 같다면
				if (Elem->GetPlayerUID() == PlayerID)
				{
					if (Elem->GetPlayerUID() == LocalPlayerID)
					{
						PC->SetVisibleRaiseOverlay(false);//Status가 변하면 뭐든간에 RaiseOverlay의 visible을 꺼줌

					}
					//버블 타입 변경
					Elem->SetBubbleTypeAndPopUp(PlayerStatus);

					//Status TC에 쓰기
					Elem->SetPlayerStatus(PlayerStatus);

					NFT_GI->PlaySoundEffect_GI(ESoundEffectType::BetAction);
					NFT_GI->PlayVibration(0.4f, 0.1f);

					//if (PlayerStatus == "observer") //플레이어의 상태가 옵저버로 변했다면
					//{
					//	Elem->InitObsever();
					//}
					//강조 체크
					//Elem->CheckImpactCard();
					break;
				}
			}

		}
		//현재 베팅에 건 돈
		else if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::stake))
		{
			//이 판에서 건 내 총 판돈 금액
			int CurrentPlayerStake = (int)PlayerJsonObject->GetNumberField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : PlayerStake %d"), CurrentPlayerStake);

			for (auto Elem : SortTCArray)
			{
				//UID가 같다면
				if (Elem->GetPlayerUID() == PlayerID)
				{
					//건 돈이 0이라면 판 초기화 이므로 초기화로 새로 쓰기
					if (CurrentPlayerStake == 0)
					{
						//이번판에 건 돈 변수 쓰기
						Elem->SetMyCurrentTurnMoney(CurrentPlayerStake);
					}
					else
					{
						//버블에 건 돈 변경 : 현재 더해져 온 돈 - 전에 낸 돈
						Elem->SetBubbleMoney(CurrentPlayerStake - Elem->GetMyCurrentTurnMoney());

						//이번판에 건 돈 변수 쓰기
						Elem->SetMyCurrentTurnMoney(CurrentPlayerStake);
						break;
					}

				}
			}
		}
		//플레이어 연결 상태
		else if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::isConnect))
		{
			bool IsPlayerConnect = (int)PlayerJsonObject->GetBoolField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : Player is Connect %d"), IsPlayerConnect == true ? TEXT("true") : TEXT("false"));
		}
		//쇼 다운때 공개되는 패와 기타 데이터
		else if (PlayerPropertyType == NHDataBase::GetEnumNameToString(EPlayerJsonType::handRank))
		{
			//핸드랭크 오면 머니값 보이게 하려고 true로 설정하기
			AfterHandRankShowAddMoney = true;

			USIOJsonValue* HandRankJsonValue = PlayerJsonObject->GetField(TEXT("value"));
			NHLOG_WARNING(TEXT("player : HandRank %s"), *HandRankJsonValue->AsString());

			USIOJsonValue* HandRankValue = PlayerJsonObject->GetField(TEXT("value"));

			int32 HandRankInt = (int32)HandRankJsonValue->AsObject()->GetNumberField(TEXT("rank"));

			//랭크 이름
				//이름 value
			USIOJsonValue* HandRankNameValue = HandRankValue->AsObject()->GetField(TEXT("name"));

			//실제 랭크 이름
			FString RankName = HandRankNameValue->AsObject()->GetStringField(TEXT("en"));

			NHLOG_WARNING(TEXT("id %d rank name is %s"), PlayerID, *RankName);

			for (auto Elem : SortTCArray)
			{
				if (Elem->GetPlayerUID() == LocalPlayerID)
				{
					if (Elem->GetHandRankFromBubble() != HandRankInt && LocalPlayerID == PlayerID) //로컬랭크와 서버랭크가 다를 때
					{
						SendChatSocketIO(FString(TEXT("+++++++++++++++++++++++++Error Rank++++++++++++++++")));
						NHLOG_ERROR(TEXT("+++++++++++++++++++++++++Error Rank++++++++++++++++"));
						NHLOG_WARNING(TEXT("id %d LocalRank %d ServerRank %d"), LocalPlayerID, Elem->GetHandRankFromBubble(), HandRankInt);
					}
				}
				if (Elem->GetPlayerUID() == PlayerID)
				{
					//승리 랭크 이름 써주기
					Elem->SetWinText(RankName);
				}


			}

			//이 플레이어의 랭크
			int HandRank_Rank = (int)HandRankValue->AsObject()->GetNumberField(TEXT("rank"));

			//이 플레이어의 카드 정보들 : 5장
			TArray<USIOJsonValue*> CardValues = HandRankValue->AsObject()->GetArrayField(TEXT("cards"));


		//최고 카드 5장 배열
			TArray<FCard> HandRankCardArray;

			//cards
			for (auto Elem : CardValues)
			{
				//각 카드 배열로 나누기
				TArray<FString> ParseArray;
				Elem->AsString().ParseIntoArray(ParseArray, TEXT(","));

				//문자
				TArray<FString> ParseArray_Small;
				ParseArray[0].ParseIntoArray(ParseArray_Small, TEXT("\""));
				FString CardShape = ParseArray_Small[1];

				//숫자
				TArray<FString> ParseArray_Small_2;
				ParseArray[1].ParseIntoArray(ParseArray_Small_2, TEXT("]"));
				int32 CardNumber = FCString::Atoi(*ParseArray_Small_2[0]);

				ECardShape TempShapeEnum = ECardShape::None;
				if (CardShape == TEXT("s")) TempShapeEnum = ECardShape::Spade;
				else if (CardShape == TEXT("h")) TempShapeEnum = ECardShape::Heart;
				else if (CardShape == TEXT("c")) TempShapeEnum = ECardShape::Clover;
				else if (CardShape == TEXT("d")) TempShapeEnum = ECardShape::Diamond;

				FCard TempCard;
				TempCard.CardSuit = TempShapeEnum;
				TempCard.CardPower = CardNumber;

				//배열에 쓰기
				HandRankCardArray.Add(TempCard);
			}

			//CompareArray - 비교
			TArray<int> CompareIntArray;
			TArray<USIOJsonValue*> CompareArray = HandRankValue->AsObject()->GetArrayField(TEXT("compareArray"));

			//, 로 나누기
			TArray<FString> ParseArray;
			CompareArray[0]->AsString().ParseIntoArray(ParseArray, TEXT(","));

			//int 배열에 넣기
			for (auto Elem : ParseArray)
			{
				CompareIntArray.Add(FCString::Atoi(*Elem));
			}

			//핸드카드
			TArray<USIOJsonValue*> HandCardValues = HandRankValue->AsObject()->GetArrayField(TEXT("handCards"));

			////핸드 카드 2장 배열
			TArray<FCard> HandRankHandCardArray;

			//cards
			for (auto Elem : HandCardValues)
			{
				//각 카드 배열로 나누기
				TArray<FString> ParseHandArray;
				Elem->AsString().ParseIntoArray(ParseHandArray, TEXT(","));

				//문자
				TArray<FString> ParseArray_Small;
				ParseHandArray[0].ParseIntoArray(ParseArray_Small, TEXT("\""));
				FString CardShape = ParseArray_Small[1];

				//숫자
				TArray<FString> ParseArray_Small_2;
				ParseHandArray[1].ParseIntoArray(ParseArray_Small_2, TEXT("]"));
				int32 CardNumber = FCString::Atoi(*ParseArray_Small_2[0]);

				ECardShape TempShapeEnum = ECardShape::None;
				if (CardShape == TEXT("s")) TempShapeEnum = ECardShape::Spade;
				else if (CardShape == TEXT("h")) TempShapeEnum = ECardShape::Heart;
				else if (CardShape == TEXT("c")) TempShapeEnum = ECardShape::Clover;
				else if (CardShape == TEXT("d")) TempShapeEnum = ECardShape::Diamond;

				FCard TempCard;
				TempCard.CardSuit = TempShapeEnum;
				TempCard.CardPower = CardNumber;

				//배열에 쓰기
				HandRankHandCardArray.Add(TempCard);

			}

			for (auto Elem : SortTCArray)
			{
				//일치하는 UID만
				if (Elem->GetPlayerUID() == PlayerID)
				{
					//핸드카드 텍스처 바꾸기
					Elem->ChangeMyHandCardTexture(HandRankHandCardArray);

					//best 카드 쓰기
					Elem->SetBestCardArray(HandRankCardArray);

					//게임끝나면 타이머 초기화
					//혁 - 머지 하면서 이쪽으로 변경함
					Elem->InitProfileTimer();
				}
			}
		}
	}
	else
	{
		NHLOG_WARNING(TEXT("Check json No game or player field"));
	}
}
