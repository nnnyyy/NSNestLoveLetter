#pragma once

class CGameDealer : public Object {
public:
	CGameDealer();
	virtual ~CGameDealer();

	virtual void OnPacket(InPacket& iPacket, CUser::pointer pUser) = 0;
	virtual void Update() = 0;
};

enum {
	LOVELETTER_GAURD = 1,
	LOVELETTER_ROYAL = 2,
	LOVELETTER_GOSSIP = 3,
	LOVELETTER_COMPANION = 4,
	LOVELETTER_HERO = 5,
	LOVELETTER_WIZARD = 6,
	LOVELETTER_LADY = 7,
	LOVELETTER_PRINCESS = 8
};



class CGameDealerLoveLetter : public CGameDealer {
public:

	enum {
		GAMEOVER_DECK_IS_EMPTY = 0,
		GAMEOVER_ALL_DEAD
	};

	class Card : public boost::enable_shared_from_this<Card> {
	public:
		Card(LONG nType) { m_nType = nType; }
		LONG m_nType;	//	카드 종류

		typedef boost::shared_ptr<Card> pointer;
	};

	class Player : public boost::enable_shared_from_this<Player> {
	public:
		ULONG nUserSN;
		LONG m_nIndex;
		BOOL m_bDead;
		BOOL m_bGuard;
		std::vector<Card::pointer> m_vHandCards;
		std::vector<Card::pointer> m_vGroundCards;

		void Init();
		void Encode(OutPacket& oPacket);
		typedef boost::shared_ptr<Player> pointer;		
	};

	class GameStatus {
	public:
		GameStatus() : nPrevRoundWinIndex(-1), nCurTurnIndex(0) {}
		LONG nPrevRoundWinIndex;	//	이전 경기 승리 플레이어		
		LONG nCurTurnIndex;			//	현재 턴 플레이어
		BOOL bRoundOver;			//	현재 라운드가 종료 되었는지
		boost::chrono::system_clock::time_point tRoundOverStart;		//	다음 라운드까지 중간에 티타임을 가지기 위함.
		enum { WAIT_NEXT_ROUND_TIME = 5 * 1000, };

		void EncodeStatus(OutPacket& oPacket);
	};

	CGameDealerLoveLetter();
	virtual ~CGameDealerLoveLetter();

	virtual void OnPacket(InPacket& iPacket, CUser::pointer pUser);				
				void OnGuardAction(InPacket& iPacket, CUser::pointer pUser);
				void OnRoyalSubjectAction(InPacket& iPacket, CUser::pointer pUser);
				void OnGossipAction(InPacket& iPacket, CUser::pointer pUser);
				void OnCompanionAction(InPacket& iPacket, CUser::pointer pUser);
				void OnHeroAction(InPacket& iPacket, CUser::pointer pUser);
				void OnWizardAction(InPacket& iPacket, CUser::pointer pUser);
				void OnLadyAction(InPacket& iPacket, CUser::pointer pUser);
				void OnPrincessAction(InPacket& iPacket, CUser::pointer pUser);
	virtual void Update();	

	void InitGame();
	void Next();
	void Process();
	void GameOver(LONG nReason);
	BOOL CheckDead();
	BOOL CheckGameOver();
	void EncodePlayerInfo(OutPacket& oPacket);
	void EncodePlayerIndexList(OutPacket& oPacket);

protected:
	BOOL GetCardFromDeck(Player::pointer pPlayer);
	BOOL IsMyTurn(Player::pointer pPlayer);
	LONG DropAndGetNewCardByHero(Player::pointer pPlayer);
	BOOL DropCard(Player::pointer pPlayer, LONG nCardType);
	void ExchangeCard(Player::pointer pPlayer1, Player::pointer pPlayer2);
	void Dead(Player::pointer pPlayer);

public:
	typedef boost::shared_ptr<CGameDealerLoveLetter> pointer;
	Object::pointer m_pRoom;

protected:
	typedef std::vector< Player::pointer >::iterator playersIter;
	std::vector< Player::pointer > m_vPlayers;
	std::vector< Card::pointer > m_vCards;

	GameStatus status;

	//	게임에 사용할 카드
	std::vector< Card::pointer > m_vDeck;
	Card::pointer m_pSecretCard;
};