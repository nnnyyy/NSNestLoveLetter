#pragma once
#include "LogMan.h"
using namespace boost::chrono;

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
		LONG m_nType;	//	ī�� ����

		typedef boost::shared_ptr<Card> pointer;
	};

	class Player : public boost::enable_shared_from_this<Player> {
	public:
		Player() : m_nRoundWin(0), m_bCPU(FALSE) {}
		CUser::pointer m_pUser;
		ULONG nUserSN;
		LONG m_nIndex;
		BOOL m_bDead;
		BOOL m_bGuard;
		std::vector<Card::pointer> m_vHandCards;
		std::vector<Card::pointer> m_vGroundCards;
		LONG m_nRoundWin;
		BOOL m_bCPU;

		void Init();
		void Encode(OutPacket& oPacket);
		typedef boost::shared_ptr<Player> pointer;		
	};

	class GameStatus {
	public:
		GameStatus() : nPrevRoundWinIndex(-1), nCurTurnIndex(0), bFinalOver(FALSE), bRoundOver(FALSE) {}
		LONG nPrevRoundWinIndex;	//	���� ��� �¸� �÷��̾�		
		LONG nCurTurnIndex;			//	���� �� �÷��̾�
		LONG nCurTurnGetCardIndex;	//	���� �� �÷��̾ ���� ī�� ����
		BOOL bRoundOver;			//	���� ���尡 ���� �Ǿ�����
		LONG nRoundOverCnt;			//	���� ��� ����Ǳ� ���� ī��Ʈ. 4�� 4�� 3�� 5��
		BOOL bFinalOver;
		boost::chrono::system_clock::time_point tRoundOverStart;		//	���� ������� �߰��� ƼŸ���� ������ ����.
		boost::chrono::system_clock::time_point tCPUStart;
		enum { WAIT_NEXT_ROUND_TIME = 10 * 1000, };

		void EncodeStatus(OutPacket& oPacket);
		void Reset() {
			nPrevRoundWinIndex = -1;
			nCurTurnIndex = 0;
			bRoundOver = FALSE;	
			pCPUTurn = NULL;
		}

		Player::pointer pCPUTurn;

		void ReservCPUProcess(Player::pointer p) { 			
			pCPUTurn = p; 
			tCPUStart = system_clock::now();
		}

		BOOL IsReservedCPUProcess() {
			if (pCPUTurn) return TRUE;
			return FALSE;
		}

		BOOL IsOverCPUDelay() {
			return (duration_cast<milliseconds>(system_clock::now() - tCPUStart).count() >= 4000);
		}
	};

	CGameDealerLoveLetter();
	virtual ~CGameDealerLoveLetter();

	struct _GameArgument {
		_GameArgument() : nTargetIdx(-1), nTargetCard(-1) {}

		LONG nTargetIdx;
		LONG nTargetCard;
	};

	void GuardAction(_GameArgument _arg, CUser::pointer pUser);
	void RoyalSubjectAction(_GameArgument _arg, CUser::pointer pUser);
	void GossipAction(_GameArgument _arg, CUser::pointer pUser);
	void CompanionAction(_GameArgument _arg, CUser::pointer pUser);
	void HeroAction(_GameArgument _arg, CUser::pointer pUser);
	void WizardAction(_GameArgument _arg, CUser::pointer pUser);
	void LadyAction(_GameArgument _arg, CUser::pointer pUser);
	void PrincessAction(_GameArgument _arg, CUser::pointer pUser);

	virtual void OnPacket(InPacket& iPacket, CUser::pointer pUser);		
				void OnCardAction(LONG nPacketSubType, InPacket& iPacket, CUser::pointer pUser);				
				void OnEmotion(InPacket& iPacket, CUser::pointer pUser);
	virtual void Update();	

	void InitGame();
	void Next();
	void Process();
	void ProcessCPU(Player::pointer p);
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
	BOOL FindTarget(Player::pointer pTurnPlayer);

	void AllReset();
	void SendFinalRoundOver(Player::pointer pWinner);
public:
	void SendGameInitInfo();

	BOOL IsGameRunning();
	void StopGame();

public:
	typedef boost::shared_ptr<CGameDealerLoveLetter> pointer;
	Object::pointer m_pRoom;

protected:
	typedef std::vector< Player::pointer >::iterator playersIter;
	std::vector< Player::pointer > m_vPlayers;
	std::vector< Card::pointer > m_vCards;

	GameStatus status;

	//	���ӿ� ����� ī��
	std::vector< Card::pointer > m_vDeck;
	Card::pointer m_pSecretCard;

	void SendPacket(Player::pointer p, OutPacket& oPacket);

protected:
};