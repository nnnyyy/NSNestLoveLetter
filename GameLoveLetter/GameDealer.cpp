#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <random>
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "GameDealer.h"
#include "Room.h"

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
	LONG nSubType = iPacket.Decode2();

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

	if (nCardTypeGuess == LOVELETTER_GAURD) {
		//	경비병을 직접 지목할 수 없다.
		return;
	}

	if (nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		return;
	}

	if (m_vPlayers[nTargetIdx]->m_bDead) {
		//	죽은 사람도 지목하지 말자.
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GAURD);
	if (!bDrop) {
		//	오류
		return;
	}if (CheckDead()) {
		NextTurn();
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType == nCardTypeGuess) {
		m_vPlayers[nTargetIdx]->m_bDead = TRUE;
		Card::pointer pCard = m_vPlayers[nTargetIdx]->m_vHandCards.back();
		m_vPlayers[nTargetIdx]->m_vHandCards.pop_back();
		m_vPlayers[nTargetIdx]->m_vGroundCards.push_back(pCard);
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
		pRoom->BroadcastPacket(oPacket);
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;		
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnRoyalSubjectAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nTargetIdx = iPacket.Decode4();	//	누구에게 물어볼 것인가	

	if (nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		return;
	}

	if (m_vPlayers[nTargetIdx]->m_bDead) {
		//	죽은 사람도 지목하지 말자.
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
			oPacket.Encode4(m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType);
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnGossipAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GOSSIP);
	if (!bDrop) {
		//	오류
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
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

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnHeroAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_HERO);
	if (!bDrop) {
		//	오류
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnWizardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_WIZARD);
	if (!bDrop) {
		//	오류
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
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

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnPrincessAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_PRINCESS);
	if (!bDrop) {
		//	오류
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

BOOL CGameDealerLoveLetter::DropCard(Player::pointer pTurnPlayer, LONG nCardType) {
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

void CGameDealerLoveLetter::Update() {
	
}

void CGameDealerLoveLetter::InitGame() {	
	//	게임 초기화
	//	순번을 정하고 카드를 섞는다.

	//SetUserSeq();
	if (status.nPrevRoundWinIndex != -1) {

	}
	//	첫번째 턴인 유저
	status.nCurTurnIndex = 0;

	m_vPlayers.clear();
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	std::vector < CUser::pointer > &vUsers = pRoom->GetUsers();
	for (std::vector < CUser::pointer >::iterator iter = vUsers.begin(); iter != vUsers.end(); ++iter) {
		Player::pointer p(new Player());
		p->nUserSN = (*iter)->m_nUserSN;
		p->m_bDead = FALSE;
		m_vPlayers.push_back(p);
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

void CGameDealerLoveLetter::NextTurn() {

	//	죽을 사람이 다 죽으면 게임을 종료한다.
	//if( !CheckDeadCount() ) { GameOver(); return; }

	if (CheckDead()) {
		NextTurn();
		return;
	}
	
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	//	카드 한장 뽑기
	player->m_bGuard = FALSE;	//	내 턴이 돌아오면 가드가 풀린다.
	BOOL bSucceed = GetCardFromDeck(player);
	if (!bSucceed) {
		//	카드가 없다.
		//	GameOver(); 
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

void CGameDealerLoveLetter::GameOver() {
	//	이 함수 내에서 더 이상 게임 패킷을 받지 않게 플래그를 설정하고,
	//	누가 이겼는지 게임 결과를 보내도록 한다.
	//	라운드가 남아 있다면 다음 라운드를 진행하고
	//	그것도 다 이겼으면 최종 결과 처리를 한다.
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
	BOOST_ASSERT(pPlayer->m_vHandCards.size() == 2);
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