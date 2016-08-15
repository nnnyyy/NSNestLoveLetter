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
	case 0: OnGuardAction(iPacket, pUser); break;
	}
}

void CGameDealerLoveLetter::OnGuardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nTargetIdx = iPacket.Decode1();	//	물어볼 타겟 인덱스
	LONG nTypeGuess = iPacket.Decode1();	//	물어볼 타입

	if (nTypeGuess == LOVELETTER_GAURD) {
		//	경비병을 직접 지목할 수 없다.
		return;
	}

	if (nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		return;
	}
}

void CGameDealerLoveLetter::Update() {

}

void CGameDealerLoveLetter::InitGame() {	
	//	게임 초기화
	//	순번을 정하고 카드를 섞는다.

	//SetUserSeq();
	if (status.nPrevRoundWinIndex != -1) {

	}
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
	auto engine = std::default_random_engine{};
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

void CGameDealerLoveLetter::NextTurn() {
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	if (player->m_bDead) {
		status.nCurTurnIndex++;
		if (status.nCurTurnIndex >= m_vPlayers.size()) {
			status.nCurTurnIndex = 0;
			NextTurn();
			return;
		}
	}

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
		oPacket.Encode4(m_vPlayers[i]->m_vHandCards[0]->m_nType);
		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}
}

void CGameDealerLoveLetter::EncodePlayerInfo(OutPacket& oPacket) {
	LONG nSize = m_vPlayers.size();
	oPacket.Encode4(nSize);
	for (int i = 0; i < nSize; ++i) {
		oPacket.Encode4(m_vPlayers[i]->nUserSN);
		oPacket.Encode4(m_vPlayers[i]->m_bDead);
		oPacket.Encode4(m_vPlayers[i]->m_bGuard);
		oPacket.Encode4(m_vPlayers[i]->m_vGroundCards.size());
		if (m_vPlayers[i]->m_vGroundCards.size()) {
			Card::pointer pCard = m_vPlayers[i]->m_vGroundCards.back();
			oPacket.Encode4(pCard->m_nType);
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