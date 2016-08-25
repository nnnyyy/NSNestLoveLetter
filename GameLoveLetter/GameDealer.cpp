#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <random>
#include "GameData.h"
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "GameDealer.h"
#include "Room.h"

using namespace boost::chrono;

CGameDealer::CGameDealer() {
	
}

CGameDealer::~CGameDealer() {

}

LONG card_attribute[] = {
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_ROYAL,
	LOVELETTER_ROYAL,
	LOVELETTER_GOSSIP,
	LOVELETTER_GOSSIP,
	LOVELETTER_COMPANION,
	LOVELETTER_COMPANION,
	LOVELETTER_HERO,
	LOVELETTER_HERO,
	LOVELETTER_WIZARD,
	LOVELETTER_LADY,
	LOVELETTER_PRINCESS
};

CGameDealerLoveLetter::CGameDealerLoveLetter() {
	LONG nSize = sizeof(card_attribute) / sizeof(LONG);
	for (int i = 0; i < nSize; ++i) {
		Card::pointer p(new Card(card_attribute[i]));
		m_vCards.push_back(p);
	}	
}

CGameDealerLoveLetter::~CGameDealerLoveLetter() {

}

void CGameDealerLoveLetter::OnPacket(InPacket& iPacket, CUser::pointer pUser) {

	if (status.bRoundOver || status.bFinalOver) {
		//	라운드가 끝나게 되면 게임 액션 패킷은 받지 않는다.
		return;
	}
	
	LONG nSubType = iPacket.Decode2();

	if (!IsGameRunning()) {
		return;
	}
	 
	switch (nSubType) {
	case CGP_LL_GuardCheck: OnGuardAction(iPacket, pUser); break;
	case CGP_LL_RoyalSubject: OnRoyalSubjectAction(iPacket, pUser); break;
	case CGP_LL_Gossip: OnGossipAction(iPacket, pUser); break;
	case CGP_LL_Companion: OnCompanionAction(iPacket, pUser); break;
	case CGP_LL_Hero: OnHeroAction(iPacket, pUser); break;
	case CGP_LL_Wizard: OnWizardAction(iPacket, pUser); break;
	case CGP_LL_Lady: OnLadyAction(iPacket, pUser); break;
	case CGP_LL_Princess: OnPrincessAction(iPacket, pUser); break;
	}
}

void CGameDealerLoveLetter::OnGuardAction(InPacket& iPacket, CUser::pointer pUser) {	 
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nTargetIdx		= iPacket.Decode4();	//	누구?
	LONG nCardTypeGuess = iPacket.Decode4();	//	무엇?
	if (m_vPlayers.size() <= nTargetIdx) return;

	Player::pointer pTargetPlayer = m_vPlayers[nTargetIdx];
	if (!pTargetPlayer) {
		return;
	}

	if (nCardTypeGuess == LOVELETTER_GAURD) {
		//	경비병을 직접 지목할 수 없다.
		return;
	}
	
	if (FindTarget(pTurnPlayer) && nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		return;
	}

	if (pTargetPlayer->m_bDead) {
		//	죽은 사람도 지목하지 말자.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GAURD);
	if (!bDrop) {
		//	오류
		return;
	}	

	pTargetPlayer->m_pUser->gamedata.m_nAttackedByGuard++;
	pTurnPlayer->m_pUser->gamedata.m_nUseGuard++;

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType == nCardTypeGuess) {
		pTurnPlayer->m_pUser->gamedata.m_nSuccessUseGuard++;
		Dead(m_vPlayers[nTargetIdx]);
		//	잡았다.		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_GAURD);		
		oPacket.Encode4(TRUE);
		oPacket.Encode4(nTargetIdx);
		pRoom->BroadcastPacket(oPacket);
	}
	else {
		//	아무일도 일어나지 않는다.
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_GAURD);
		oPacket.Encode4(FALSE);		
		oPacket.Encode4(nTargetIdx);
		oPacket.Encode4(nCardTypeGuess);
		pRoom->BroadcastPacket(oPacket);
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::OnRoyalSubjectAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nTargetIdx = iPacket.Decode4();	//	누구에게 물어볼 것인가	
	if (m_vPlayers.size() <= nTargetIdx) return;
	Player::pointer pTargetPlayer = m_vPlayers[nTargetIdx];

	if (FindTarget(pTurnPlayer) && nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		return;
	}

	if (pTargetPlayer->m_bDead) {
		//	죽은 사람도 지목하지 말자.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_ROYAL);
	if (!bDrop) {
		//	오류
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);	

	LONG nSize = m_vPlayers.size();	
	for (int i = 0; i < nSize; ++i) {		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_ROYAL);
		BOOL bMyTurn = IsMyTurn(m_vPlayers[i]);
		oPacket.Encode4(bMyTurn);
		if (bMyTurn) {
			oPacket.Encode4(pTargetPlayer->m_vHandCards[0]->m_nType);
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::OnGossipAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];

	if (pTargetPlayer->m_bDead) {
		//	죽은 자도 타게팅 할 수 없다.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		return;
	}

	//	드롭
	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GOSSIP);
	if (!bDrop) {
		//	오류
		return;
	}	

	pTargetPlayer->m_pUser->gamedata.m_nAttackedByGossip++;
	pTurnPlayer->m_pUser->gamedata.m_nUseGossip++;

	Card::pointer pTargetHandCard = pTargetPlayer->m_vHandCards[0];
	Card::pointer pMyHandCard = pTurnPlayer->m_vHandCards[0];

	BOOL bRet = FALSE;
	LONG nDeadIndex = -1;
	LONG nAliveIndex = -1;
	if (pTargetHandCard->m_nType > pMyHandCard->m_nType) {
		//	내가 죽음
		Dead(pTurnPlayer);
		bRet = TRUE;
		nDeadIndex = status.nCurTurnIndex;
		nAliveIndex = nWho;
	}
	else if (pTargetHandCard->m_nType < pMyHandCard->m_nType) {
		//	상대방이 죽음
		pTurnPlayer->m_pUser->gamedata.m_nSuccessUseGossip++;
		Dead(pTargetPlayer);
		bRet = TRUE;
		nDeadIndex = nWho;
		nAliveIndex = status.nCurTurnIndex;
	}
	else {
		//	비겨서 아무일도 일어나지 않음.
		nAliveIndex = nWho;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_GOSSIP);
	oPacket.Encode4(bRet);
	oPacket.Encode4(nAliveIndex);
	if (bRet) {
		oPacket.Encode4(nDeadIndex);		
	}	
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}

void CGameDealerLoveLetter::OnCompanionAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_COMPANION);
	if (!bDrop) {
		//	오류
		return;
	}

	pTurnPlayer->m_bGuard = TRUE;

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_COMPANION);
	oPacket.Encode4(status.nCurTurnIndex);
	pRoom->BroadcastPacket(oPacket);


	Next();
	Process();
}

void CGameDealerLoveLetter::OnHeroAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	죽은 자도 타게팅 할 수 없다.
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_HERO);
	if (!bDrop) {
		//	오류
		return;
	}

	LONG nDroppedCardType = DropAndGetNewCardByHero(pTargetPlayer);
	BOOL bKill = FALSE;
	if (nDroppedCardType == LOVELETTER_PRINCESS) {
		Dead(pTargetPlayer);
		bKill = TRUE;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_HERO);
	oPacket.Encode4(status.nCurTurnIndex);	// From
	oPacket.Encode4(nWho);					// To
	oPacket.Encode4(bKill);					// Princess Drop Kill
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}

void CGameDealerLoveLetter::OnWizardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	죽은 자도 타게팅 할 수 없다.
		return;
	}	

	if (FindTarget(pTurnPlayer) && nWho == status.nCurTurnIndex) {
		//	타겟이 남아있는데 나에게 쓰는건 안됨
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_WIZARD);
	if (!bDrop) {
		//	오류
		return;
	}	

	// 교환 처리
	ExchangeCard(pTurnPlayer, pTargetPlayer);

	Next();
	Process();
}

void CGameDealerLoveLetter::OnLadyAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_LADY);
	if (!bDrop) {
		//	오류
		return;
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::OnPrincessAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	//	여기 들어오는 자폭 짓은 하지 않을거 같으므로 리턴
	return;

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_PRINCESS);
	if (!bDrop) {
		//	오류
		return;
	}

	Next();
	Process();
}

LONG CGameDealerLoveLetter::DropAndGetNewCardByHero(Player::pointer pTargetPlayer) {
	//	항상 1장이 있어야 한다.
	BOOST_ASSERT(pTargetPlayer->m_vHandCards.size() == 1);
	Card::pointer pCardDropped = pTargetPlayer->m_vHandCards[0];
	pTargetPlayer->m_vHandCards.clear();
	pTargetPlayer->m_vGroundCards.push_back(pCardDropped);
	if (pCardDropped->m_nType != LOVELETTER_PRINCESS) {
		GetCardFromDeck(pTargetPlayer);
	}	
	return pCardDropped->m_nType;
}

BOOL CGameDealerLoveLetter::DropCard(Player::pointer pTurnPlayer, LONG nCardType) {
	//	항상 2장이 있어야 한다.
	BOOST_ASSERT(pTurnPlayer->m_vHandCards.size() == 2);

	BOOL bFind = FALSE;
	std::vector<Card::pointer>::iterator iterFind;
	LONG nFindCardIndex = -1;
	Card::pointer pCardDropped;
	for (std::vector<Card::pointer>::iterator iter = pTurnPlayer->m_vHandCards.begin(); iter != pTurnPlayer->m_vHandCards.end(); ++iter) {
		if ((*iter)->m_nType == nCardType) {
			bFind = TRUE;
			pCardDropped = *iter;
			iterFind = iter;
			//pTurnPlayer->m_vHandCards.erase(iter);
			break;
		}
	}

	if (!bFind)
	{
		return FALSE;
	}
	
	if (nCardType == LOVELETTER_HERO || nCardType == LOVELETTER_WIZARD) {
		//	내려놓는 카드가 영웅이나 위자드일 때,
		//	나머지 카드에 귀부인이 있으면 귀부인을 먼저 버려야 함.
		for each (Card::pointer pCard in pTurnPlayer->m_vHandCards)
		{
			if (pCard->m_nType == LOVELETTER_LADY) {
				return FALSE;
			}
		}
	}	

	pTurnPlayer->m_vHandCards.erase(iterFind);
	pTurnPlayer->m_vGroundCards.push_back(pCardDropped);
	return TRUE;
}

void CGameDealerLoveLetter::ExchangeCard(Player::pointer pPlayer1, Player::pointer pPlayer2) {
	BOOST_ASSERT(pPlayer1->m_vHandCards.size() == 1 && pPlayer2->m_vHandCards.size() == 1);
	Card::pointer pCard1 = pPlayer1->m_vHandCards[0];
	Card::pointer pCard2 = pPlayer2->m_vHandCards[0];
	pPlayer1->m_vHandCards.clear();
	pPlayer1->m_vHandCards.push_back(pCard2);
	pPlayer2->m_vHandCards.clear();
	pPlayer2->m_vHandCards.push_back(pCard1);
}

void CGameDealerLoveLetter::Dead(Player::pointer pPlayer) {
	pPlayer->m_bDead = TRUE;
	if (!pPlayer->m_vHandCards.empty()) {
		Card::pointer pCard = pPlayer->m_vHandCards.back();
		pPlayer->m_vHandCards.pop_back();
		BOOST_ASSERT(pPlayer->m_vHandCards.empty());		
		pPlayer->m_vGroundCards.push_back(pCard);
	}	
}

BOOL CGameDealerLoveLetter::FindTarget(Player::pointer pTurnPlayer) {
	for each (Player::pointer p in m_vPlayers)
	{
		if (p != pTurnPlayer && !p->m_bDead && !p->m_bGuard) {
			return TRUE;
		}
	}

	return FALSE;
}

void CGameDealerLoveLetter::Update() {		
	if (status.bRoundOver && 
		duration_cast<milliseconds>(system_clock::now() - status.tRoundOverStart).count() >= GameStatus::WAIT_NEXT_ROUND_TIME ) {
		if (status.bFinalOver) {
			AllReset();
		}
		else {
			InitGame();
			Process();
		}		
		return;
	}
}

void CGameDealerLoveLetter::InitGame() {	
	//	게임 초기화
	//	순번을 정하고 카드를 섞는다.
	//SetUserSeq();
	status.bRoundOver = FALSE;
	if (status.nPrevRoundWinIndex != -1) {
		status.nCurTurnIndex = status.nPrevRoundWinIndex;
		status.bFinalOver = FALSE;
	}
	else {
		status.nCurTurnIndex = 0;
	}
	

	if (m_vPlayers.empty()) {
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		std::vector < CUser::pointer > &vUsers = pRoom->GetUsers();
		LONG idx = 0;
		for (std::vector < CUser::pointer >::iterator iter = vUsers.begin(); iter != vUsers.end(); ++iter) {
			Player::pointer p(new Player());
			p->Init();
			p->nUserSN = (*iter)->m_nUserSN;
			p->m_bDead = FALSE;
			p->m_pUser = (*iter);
			m_vPlayers.push_back(p);
			p->m_nIndex = idx;
			idx++;
		}
	}
	else {
		for each (Player::pointer p in m_vPlayers)
		{
			p->Init();
		}
	}
	

	//ShuffleCard();
	m_vDeck.clear();
	m_pSecretCard = NULL;
	m_vDeck.resize(m_vCards.size());
	std::copy(m_vCards.begin(), m_vCards.end(), m_vDeck.begin());
	auto engine = std::default_random_engine(std::random_device{}());
	std::shuffle(std::begin(m_vDeck), std::end(m_vDeck), engine);
	
	//DistributeCard();
	//	히든 카드 한장 숨기기
	m_pSecretCard = m_vDeck.back();
	m_vDeck.pop_back();

	LONG nPlayerNum = m_vPlayers.size();
	for (int i = 0; i < nPlayerNum; ++i) {
		m_vPlayers[i]->m_vHandCards.push_back(m_vDeck.back());
		m_vDeck.pop_back();
	}
}
BOOL CGameDealerLoveLetter::CheckDead() {
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	if (player->m_bDead) {
		status.nCurTurnIndex++;
		if (status.nCurTurnIndex >= m_vPlayers.size()) {
			status.nCurTurnIndex = 0;
		}		
		return TRUE;
	}

	return FALSE;
}

BOOL CGameDealerLoveLetter::CheckGameOver() {

	if (m_vDeck.size() <= 0) {
		GameOver(GAMEOVER_DECK_IS_EMPTY);
		return TRUE;
	}

	LONG nDeadCnt = 0;	
	for each (Player::pointer p in m_vPlayers)
	{
		if (p->m_bDead) nDeadCnt++;
	}

	if (m_vPlayers.size() - 1 <= nDeadCnt) {
		GameOver(GAMEOVER_ALL_DEAD);
		return TRUE;
	}

	return FALSE;
}

void CGameDealerLoveLetter::GameOver(LONG nReason) {
	//	이 함수 내에서 더 이상 게임 패킷을 받지 않게 플래그를 설정하고,
	//	누가 이겼는지 게임 결과를 보내도록 한다.
	//	라운드가 남아 있다면 다음 라운드를 진행하고
	//	그것도 다 이겼으면 최종 결과 처리를 한다.	
	Player::pointer pWinner;
	if (nReason == GAMEOVER_DECK_IS_EMPTY) {		
		for each (Player::pointer p in m_vPlayers)
		{
			if (p->m_bDead) continue;
			if (pWinner == NULL) {
				pWinner = p;
				continue;
			}
			else {
				if (pWinner->m_vHandCards[0]->m_nType < p->m_vHandCards[0]->m_nType) {
					pWinner = p;
					continue;
				}
			}
		}
	}

	if (nReason == GAMEOVER_ALL_DEAD) {		
		for each (Player::pointer p in m_vPlayers)
		{
			if (p->m_bDead) continue;
			if (pWinner == NULL) {
				pWinner = p;
				break;
			}			
		}
	}

	if (pWinner) {
		//	이 분이 이 라운드의 승자임
		pWinner->m_nRoundWin++;
		status.bRoundOver = TRUE;
		status.tRoundOverStart = system_clock::now();
		status.nPrevRoundWinIndex = pWinner->m_nIndex;

		if (pWinner->m_nRoundWin >= 3 /* 4명이면 보석 3개 모으기 */) {
			status.bFinalOver = TRUE;			
			for each (Player::pointer p in m_vPlayers)
			{				
				if (pWinner == p) {
					pWinner->m_pUser->gamedata.m_nWin++;					
				}
				else {
					p->m_pUser->gamedata.m_nLose++;
				}
			}
			SendFinalRoundOver(pWinner);
			return;
		}
		else {			
			CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
			OutPacket oPacket(GCP_GameLoveLetter);
			oPacket.Encode2(GCP_LL_RoundResult);
			oPacket.Encode4(pWinner->m_nIndex);
			pRoom->BroadcastPacket(oPacket);
		}		
	}
	else {
		//	말도 안됨.
		//	방폭이나 리셋을 하자.
	}
}

void CGameDealerLoveLetter::Next() {
	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}
}

void CGameDealerLoveLetter::Process() {

	//	죽을 사람이 다 죽으면 게임을 종료한다.
	if( CheckGameOver() ) { return; }

	if (CheckDead()) {
		Process();
		return;
	}
	
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	//	카드 한장 뽑기
	player->m_bGuard = FALSE;	//	내 턴이 돌아오면 가드가 풀린다.
	BOOL bSucceed = GetCardFromDeck(player);
	if (!bSucceed) {
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	LONG nSize = m_vPlayers.size();	
	for (int i = 0; i < nSize; ++i) {		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_Status);
		status.EncodeStatus(oPacket);
		EncodePlayerInfo(oPacket);
		BOOL bMyTurn = IsMyTurn(m_vPlayers[i]);
		oPacket.Encode4(bMyTurn);		
		if (!m_vPlayers[i]->m_bDead) {
			oPacket.Encode4(m_vPlayers[i]->m_vHandCards[0]->m_nType);
			if (bMyTurn) {
				oPacket.Encode4(m_vPlayers[i]->m_vHandCards[1]->m_nType);
			}
		}
		else {
			//	죽었을 경우
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	//	현재 턴인 플레이어의 2번째 카드를 그 플레이어에게만 전송한다.
	//	나머지 플레이어에겐 뒷면을 제공 ( 클라 액션용 )
}

void CGameDealerLoveLetter::EncodePlayerInfo(OutPacket& oPacket) {
	LONG nSize = m_vPlayers.size();
	oPacket.Encode4(nSize);
	for (int i = 0; i < nSize; ++i) {
		oPacket.Encode4(m_vPlayers[i]->nUserSN);
		oPacket.Encode4(m_vPlayers[i]->m_bDead);
		oPacket.Encode4(m_vPlayers[i]->m_bGuard);
		LONG nGroundCardSize = m_vPlayers[i]->m_vGroundCards.size();
		oPacket.Encode4(nGroundCardSize);		
		if (nGroundCardSize) {
			for (int i2 = 0; i2 < nGroundCardSize; ++i2) {
				Card::pointer pCard = m_vPlayers[i]->m_vGroundCards[i2];
				oPacket.Encode4(pCard->m_nType);
			}						
		}
	}
}

void CGameDealerLoveLetter::EncodePlayerIndexList(OutPacket& oPacket) {
	LONG nSize = m_vPlayers.size();
	oPacket.Encode4(nSize);
	for (int i = 0; i < nSize; ++i) {
		oPacket.Encode4(m_vPlayers[i]->nUserSN);
		oPacket.Encode4(i);
	}	
}

BOOL CGameDealerLoveLetter::GetCardFromDeck(Player::pointer pPlayer) {
	pPlayer->m_vHandCards.push_back(m_vDeck.back());
	BOOST_ASSERT(pPlayer->m_vHandCards.size() == 2 || pPlayer->m_vHandCards.size() == 1);
	m_vDeck.pop_back();	
	return TRUE;
}

BOOL CGameDealerLoveLetter::IsMyTurn(Player::pointer pPlayer) {
	return m_vPlayers[status.nCurTurnIndex] == pPlayer;
}

void CGameDealerLoveLetter::GameStatus::EncodeStatus(OutPacket& oPacket) {
	oPacket.Encode4(nCurTurnIndex);
}

void CGameDealerLoveLetter::Player::Init() {
	m_vHandCards.clear();
	m_vGroundCards.clear();
	m_bDead = FALSE;
	m_bGuard = FALSE;
}

void CGameDealerLoveLetter::Player::Encode(OutPacket& oPacket) {
	oPacket.Encode1(m_vGroundCards.size());
	oPacket.Encode1(m_bDead);
	oPacket.Encode1(m_bGuard);
}

void CGameDealerLoveLetter::AllReset() {
	//	
	status.Reset();
	m_vPlayers.clear();

}

void CGameDealerLoveLetter::SendFinalRoundOver(Player::pointer pWinner) {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_FinalResult);
	oPacket.Encode4(pWinner->m_nIndex);
	pRoom->BroadcastPacket(oPacket);
	pRoom->ResetReady();
	pRoom->BroadcastRoomState();
}

BOOL CGameDealerLoveLetter::IsGameRunning() {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom) return FALSE;
	
	return pRoom->IsGameRunning();
}