#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/tr1/random.hpp>
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
	case CGP_LL_GuardCheck:
	case CGP_LL_RoyalSubject:
	case CGP_LL_Gossip: 
	case CGP_LL_Companion:
	case CGP_LL_Hero: 
	case CGP_LL_Wizard:
	case CGP_LL_Lady: 
	case CGP_LL_Princess: OnCardAction(nSubType, iPacket, pUser); break;
	case CGP_LL_Emotion: OnEmotion(iPacket, pUser); break;
	}
}

void CGameDealerLoveLetter::OnCardAction(LONG nPacketSubType, InPacket& iPacket, CUser::pointer pUser) {
	
	_GameArgument gm;
	switch (nPacketSubType) {
	case CGP_LL_GuardCheck:
		gm.nTargetIdx = iPacket.Decode4();
		gm.nTargetCard = iPacket.Decode4();
		GuardAction(gm, pUser);
		break;

	case CGP_LL_RoyalSubject:
		gm.nTargetIdx = iPacket.Decode4();
		RoyalSubjectAction(gm, pUser);
		break;

	case CGP_LL_Gossip:
		gm.nTargetIdx = iPacket.Decode4();
		GossipAction(gm, pUser);
		break;

	case CGP_LL_Companion:
		CompanionAction(gm, pUser);
		break;

	case CGP_LL_Hero:
		gm.nTargetIdx = iPacket.Decode4();
		HeroAction(gm, pUser);
		break;

	case CGP_LL_Wizard:
		gm.nTargetIdx = iPacket.Decode4();
		WizardAction(gm, pUser);
		break;

	case CGP_LL_Lady:
		LadyAction(gm, pUser);
		break;

	case CGP_LL_Princess:
		PrincessAction(gm, pUser);
		break;
	default:
		BOOST_ASSERT(FALSE && "Error Action Type");
	}	
}

void CGameDealerLoveLetter::GuardAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		LogAdd(boost::str(boost::format("GuardAction - 1 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	LONG nTargetIdx = _arg.nTargetIdx;	//	누구?
	LONG nCardTypeGuess = _arg.nTargetCard;	//	무엇?
	if (m_vPlayers.size() <= nTargetIdx) return;

	Player::pointer pTargetPlayer = m_vPlayers[nTargetIdx];
	if (!pTargetPlayer) {
		LogAdd(boost::str(boost::format("GuardAction - 2 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	if (nCardTypeGuess == LOVELETTER_GAURD) {
		//	경비병을 직접 지목할 수 없다.
		LogAdd(boost::str(boost::format("GuardAction - 3 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	if (FindTarget(pTurnPlayer) && nTargetIdx == status.nCurTurnIndex) {
		//	내 자신을 지목할 수 없다.
		LogAdd(boost::str(boost::format("GuardAction - 4 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	if (pTargetPlayer->m_bDead) {
		//	죽은 사람도 지목하지 말자.
		LogAdd(boost::str(boost::format("GuardAction - 5 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		LogAdd(boost::str(boost::format("GuardAction - 6 : %d , %d") % pTurnPlayer->m_nIndex % pTurnPlayer->m_bCPU));
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GAURD);
	if (!bDrop) {
		//	오류
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom->IsCPUGame()) {
		pTargetPlayer->m_pUser->gamedata.m_nAttackedByGuard++;
		pTurnPlayer->m_pUser->gamedata.m_nUseGuard++;
	}	

	BOOL bSucceed = FALSE;	
	if (m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType == nCardTypeGuess) {
		if (!pRoom->IsCPUGame()) {
			pTurnPlayer->m_pUser->gamedata.m_nSuccessUseGuard++;
		}
		Dead(m_vPlayers[nTargetIdx]);
		//	잡았다.
		bSucceed = TRUE;

	}

	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_GAURD);
	oPacket.Encode4(pTurnPlayer->m_nIndex);
	oPacket.Encode4(pTargetPlayer->m_nIndex);
	oPacket.Encode4(nCardTypeGuess);
	oPacket.Encode4(bSucceed);
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}

void CGameDealerLoveLetter::RoyalSubjectAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nTargetIdx = _arg.nTargetIdx;	//	누구에게 물어볼 것인가	
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
		oPacket.Encode4(pTurnPlayer->m_nIndex);
		oPacket.Encode4(pTargetPlayer->m_nIndex);
		BOOL bMyTurn = IsMyTurn(m_vPlayers[i]);
		oPacket.Encode4(bMyTurn);
		if (bMyTurn) {
			oPacket.Encode4(pTargetPlayer->m_vHandCards[0]->m_nType);
		}

		SendPacket(m_vPlayers[i], oPacket);		
	}

	Next();
	Process();
}
void CGameDealerLoveLetter::GossipAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		LogAdd("GossipAction Error - 1");
		return;
	}

	LONG nWho = _arg.nTargetIdx;
	if (m_vPlayers.size() <= nWho) {
		LogAdd("GossipAction Error - 2");
		return;
	}
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

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom->IsCPUGame()) {
		pTargetPlayer->m_pUser->gamedata.m_nAttackedByGossip++;
		pTurnPlayer->m_pUser->gamedata.m_nUseGossip++;
	}

	Card::pointer pTargetHandCard = pTargetPlayer->m_vHandCards[0];
	Card::pointer pMyHandCard = pTurnPlayer->m_vHandCards[0];

	LONG nRet = 0;	//	0 무승부 -1 Target Win 1 Turn Win
	LONG nDeadCard = -1;
	LONG nAliveIndex = -1;
	if (pTargetHandCard->m_nType > pMyHandCard->m_nType) {
		//	내가 죽음
		Dead(pTurnPlayer);
		nRet = -1;
		nDeadCard = pMyHandCard->m_nType;
		nAliveIndex = nWho;
	}
	else if (pTargetHandCard->m_nType < pMyHandCard->m_nType) {
		//	상대방이 죽음
		if (!pRoom->IsCPUGame()) {
			pTurnPlayer->m_pUser->gamedata.m_nSuccessUseGossip++;
		}		
		Dead(pTargetPlayer);
		nRet = 1;
		nDeadCard = pTargetHandCard->m_nType;
		nAliveIndex = status.nCurTurnIndex;
	}
	else {
		//	비겨서 아무일도 일어나지 않음.
		nAliveIndex = nWho;
	}
	
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_GOSSIP);
	oPacket.Encode4(pTurnPlayer->m_nIndex);
	oPacket.Encode4(pTargetPlayer->m_nIndex);
	oPacket.Encode4(nRet);
	if (nRet != 0) {
		oPacket.Encode4(nDeadCard);
	}
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}
void CGameDealerLoveLetter::CompanionAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
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

void CGameDealerLoveLetter::HeroAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nWho = _arg.nTargetIdx;
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	죽은 자도 타게팅 할 수 없다.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
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

	LONG nNewCard = 0;
	if (!bKill) {
		nNewCard = status.nCurTurnGetCardIndex;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	LONG nSize = m_vPlayers.size();
	for (int i = 0; i < nSize; ++i) {
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_HERO);
		oPacket.Encode4(pTurnPlayer->m_nIndex);
		oPacket.Encode4(pTargetPlayer->m_nIndex);
		oPacket.Encode4(nDroppedCardType);
		BOOL bTargetPlayer = FALSE;
		if (m_vPlayers[i] == pTargetPlayer) {
			bTargetPlayer = TRUE;
		}
		oPacket.Encode4(bTargetPlayer);
		if (bTargetPlayer) {
			oPacket.Encode4(nNewCard);
		}

		SendPacket(m_vPlayers[i], oPacket);		
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::WizardAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	LONG nWho = _arg.nTargetIdx;
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	죽은 자도 타게팅 할 수 없다.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
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

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);

	LONG nSize = m_vPlayers.size();
	for (int i = 0; i < nSize; ++i) {
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_WIZARD);
		oPacket.Encode4(pTurnPlayer->m_nIndex);
		oPacket.Encode4(pTargetPlayer->m_nIndex);
		BOOL bSrcOrTarget = FALSE;
		if (pTurnPlayer == m_vPlayers[i] || pTargetPlayer == m_vPlayers[i]) {
			bSrcOrTarget = TRUE;
		}
		oPacket.Encode4(bSrcOrTarget);
		if (bSrcOrTarget) {
			oPacket.Encode4(pTargetPlayer->m_vHandCards[0]->m_nType);
			oPacket.Encode4(pTurnPlayer->m_vHandCards[0]->m_nType);
		}

		SendPacket(m_vPlayers[i], oPacket);
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::LadyAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	턴이 아닌 사람에게 액션 패킷이 오면 무효처리 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_LADY);
	if (!bDrop) {
		//	오류
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_LADY);
	oPacket.Encode4(status.nCurTurnIndex);
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}

void CGameDealerLoveLetter::PrincessAction(_GameArgument _arg, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (!pTurnPlayer->m_bCPU && pTurnPlayer->nUserSN != pUser->m_nUserSN) {
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

void CGameDealerLoveLetter::OnEmotion(InPacket& iPacket, CUser::pointer pUser) {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_Emotion);
	oPacket.Encode4(iPacket.Decode4());
	pRoom->BroadcastPacket(oPacket);
}

LONG CGameDealerLoveLetter::DropAndGetNewCardByHero(Player::pointer pTargetPlayer) {
	//	항상 1장이 있어야 한다.
	BOOST_ASSERT(pTargetPlayer->m_vHandCards.size() == 1);
	Card::pointer pCardDropped = pTargetPlayer->m_vHandCards[0];
	pTargetPlayer->m_vHandCards.clear();
	pTargetPlayer->m_vGroundCards.push_back(pCardDropped);
	if (pCardDropped->m_nType != LOVELETTER_PRINCESS) {
		BOOL bRet = GetCardFromDeck(pTargetPlayer);
		if (!bRet) return -1;
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
			SendGameInitInfo();
			Process();
		}		
		return;
	}

	if (status.IsReservedCPUProcess() && status.IsOverCPUDelay()) {
		ProcessCPU(status.pCPUTurn);
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
		status.nRoundOverCnt = m_vPlayers.size() == 4 ? 4 : 5;
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

		std::vector< CRoom::CPUInfo > &vCPUs = pRoom->GetCPUs();
		for (std::vector< CRoom::CPUInfo >::iterator iter = vCPUs.begin(); iter != vCPUs.end(); ++iter) {
			Player::pointer p(new Player());
			p->Init();
			p->nUserSN = (*iter).nSN;
			p->m_bDead = FALSE;
			p->m_pUser = NULL;
			m_vPlayers.push_back(p);
			p->m_nIndex = idx;
			p->m_bCPU = TRUE;
			idx++;
		}
	}
	else {
		for each (Player::pointer p in m_vPlayers)
		{
			p->Init();
		}
	}

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

		if (pWinner->m_nRoundWin >= status.nRoundOverCnt) {
			status.bFinalOver = TRUE;			
			for each (Player::pointer p in m_vPlayers)
			{				
				if (pWinner == p) {
					if(!pWinner->m_bCPU)
						pWinner->m_pUser->gamedata.m_nWin++;					
				}
				else {
					if (!p->m_bCPU)
						p->m_pUser->gamedata.m_nLose++;
				}
			}
			SendFinalRoundOver(pWinner);
			return;
		}
		else {	
			std::map<LONG, LONG> mLastCard;
			for each (Player::pointer p in m_vPlayers) {
				if (p->m_bDead) continue;
				mLastCard[p->m_nIndex] = p->m_vHandCards[0]->m_nType;
			}

			CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
			OutPacket oPacket(GCP_GameLoveLetter);
			oPacket.Encode2(GCP_LL_RoundResult);
			oPacket.Encode4(nReason);
			oPacket.Encode4(pWinner->m_nIndex);
			if (nReason == GAMEOVER_DECK_IS_EMPTY) {
				oPacket.Encode4(mLastCard.size());
				std::map<LONG, LONG>::iterator iter = mLastCard.begin();
				for (; iter != mLastCard.end(); ++iter) {
					oPacket.Encode4(iter->first);
					oPacket.Encode4(iter->second);
				}
			}
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
		LogAdd(boost::str(boost::format("GetCardFromDeck Failed : %d") % player->m_nIndex));
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	LONG nSize = m_vPlayers.size();	
	for (int i = 0; i < nSize; ++i) {		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_Status);
		status.EncodeStatus(oPacket);
		SendPacket(m_vPlayers[i], oPacket);
	}

	//	현재 턴 플레이어가 CPU 라면 CPU 행동으로 들어간다.
	if (player->m_bCPU) {
		status.ReservCPUProcess(player);		
	}
}

void CGameDealerLoveLetter::ProcessCPU(Player::pointer p) {
	BOOST_ASSERT(p->m_vHandCards.size() == 2);
	status.pCPUTurn = NULL;	

	std::vector<Card::pointer> vTemp(2);
	std::copy(p->m_vHandCards.begin(), p->m_vHandCards.end(), vTemp.begin());
	auto engine = std::default_random_engine(std::random_device{}());
	std::shuffle(std::begin(vTemp), std::end(vTemp), engine);

	Card::pointer pCardUse = vTemp[0];
	if (pCardUse->m_nType == LOVELETTER_PRINCESS) {
		pCardUse = vTemp[1];
	}

	LogAdd(boost::str(boost::format("ProcessCPU : %d , %d") % p->m_nIndex % pCardUse->m_nType));

	std::vector<Player::pointer> vTargets;
	if (pCardUse->m_nType == LOVELETTER_GAURD) {		
		Player::pointer pTarget = NULL;
		for (std::vector< Player::pointer >::iterator iter = m_vPlayers.begin(); iter != m_vPlayers.end(); ++iter) {
			Player::pointer pTemp = (*iter);
			if (pTemp == p || pTemp->m_bDead || pTemp->m_bGuard) continue;
			vTargets.push_back(pTemp);
		}

		if (vTargets.size() == 0) pTarget = p;
		else {
			std::shuffle(std::begin(vTargets), std::end(vTargets), engine);
			pTarget = vTargets[0];
		}
		_GameArgument gm;						
		std::tr1::mt19937 rng(time(0));
		std::tr1::uniform_int_distribution<> six(2, 8);		
		gm.nTargetIdx = pTarget->m_nIndex;
		gm.nTargetCard = six(rng);
		GuardAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_ROYAL) {
		Player::pointer pTarget = NULL;
		for (std::vector< Player::pointer >::iterator iter = m_vPlayers.begin(); iter != m_vPlayers.end(); ++iter) {
			Player::pointer pTemp = (*iter);
			if (pTemp == p || pTemp->m_bDead || pTemp->m_bGuard) continue;
			vTargets.push_back(pTemp);
		}

		if (vTargets.size() == 0) pTarget = p;
		else {
			std::shuffle(std::begin(vTargets), std::end(vTargets), engine);
			pTarget = vTargets[0];
		}
		_GameArgument gm;
		gm.nTargetIdx = pTarget->m_nIndex;
		RoyalSubjectAction(gm, NULL);
	}	
	else if (pCardUse->m_nType == LOVELETTER_GOSSIP) {
		Player::pointer pTarget = NULL;
		for (std::vector< Player::pointer >::iterator iter = m_vPlayers.begin(); iter != m_vPlayers.end(); ++iter) {
			Player::pointer pTemp = (*iter);
			if (pTemp == p || pTemp->m_bDead || pTemp->m_bGuard) continue;
			vTargets.push_back(pTemp);
		}

		if (vTargets.size() == 0) pTarget = p;
		else {
			std::shuffle(std::begin(vTargets), std::end(vTargets), engine);
			pTarget = vTargets[0];
		}
		_GameArgument gm;
		gm.nTargetIdx = pTarget->m_nIndex;
		GossipAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_COMPANION) {
		Player::pointer pTarget = p;
		
		_GameArgument gm;		
		CompanionAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_HERO) {
		Player::pointer pTarget = NULL;
		for (std::vector< Player::pointer >::iterator iter = m_vPlayers.begin(); iter != m_vPlayers.end(); ++iter) {
			Player::pointer pTemp = (*iter);
			if (pTemp->m_bDead || pTemp->m_bGuard) continue;
			vTargets.push_back(pTemp);
		}

		if (vTargets.size() == 0) pTarget = p;
		else {
			std::shuffle(std::begin(vTargets), std::end(vTargets), engine);
			pTarget = vTargets[0];
		}
		_GameArgument gm;
		gm.nTargetIdx = pTarget->m_nIndex;
		HeroAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_WIZARD) {
		Player::pointer pTarget = NULL;
		for (std::vector< Player::pointer >::iterator iter = m_vPlayers.begin(); iter != m_vPlayers.end(); ++iter) {
			Player::pointer pTemp = (*iter);
			if (pTemp == p || pTemp->m_bDead || pTemp->m_bGuard) continue;
			vTargets.push_back(pTemp);
		}

		if (vTargets.size() == 0) pTarget = p;
		else {
			std::shuffle(std::begin(vTargets), std::end(vTargets), engine);
			pTarget = vTargets[0];
		}
		_GameArgument gm;
		gm.nTargetIdx = pTarget->m_nIndex;
		WizardAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_LADY) {
		Player::pointer pTarget = p;

		_GameArgument gm;
		LadyAction(gm, NULL);
	}
	else if (pCardUse->m_nType == LOVELETTER_PRINCESS) {
		Player::pointer pTarget = p;

		_GameArgument gm;
		PrincessAction(gm, NULL);
	}
}

void CGameDealerLoveLetter::SendPacket(Player::pointer p, OutPacket& oPacket) {
	if (p->m_bCPU) return;
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	CUser::pointer pUser = pRoom->GetUser(p->nUserSN);
	pUser->SendPacket(oPacket);
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
	if (m_vDeck.size() <= 0) {
		return FALSE;
	}
	status.nCurTurnGetCardIndex = m_vDeck.back()->m_nType;
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
	oPacket.Encode4(nCurTurnGetCardIndex);
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

void CGameDealerLoveLetter::SendGameInitInfo() {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	LONG nSize = m_vPlayers.size();
	for (int i = 0; i < nSize; ++i) {
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_InitStatus);
		status.EncodeStatus(oPacket);
		EncodePlayerInfo(oPacket);				
		oPacket.Encode4(m_vPlayers[i]->m_vHandCards[0]->m_nType);
		SendPacket(m_vPlayers[i], oPacket);		
	}
}

BOOL CGameDealerLoveLetter::IsGameRunning() {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom) return FALSE;
	
	return pRoom->IsGameRunning();
}

void CGameDealerLoveLetter::StopGame() {
	AllReset();
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_Aborted);
	pRoom->BroadcastPacket(oPacket);
}