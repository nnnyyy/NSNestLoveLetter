using UnityEngine;
using System.Collections;

namespace NSNetwork
{
    public class Sender
    {
        /// <summary>
        /// 로그인을 시도한다.
        /// </summary>
        public static void Login(string id, string pw)
        {
            CGPLogin login = new CGPLogin(id, pw);
            NetworkUnityEvent.Instance.Send(login);
        }

        /// <summary>
        /// 계정 생성
        /// </summary>
        public static void RegisterUser(string id, string pw, string nick)
        {
            CGPRegisterUser registerUser = new CGPRegisterUser(id, pw, nick);
            NetworkUnityEvent.Instance.Send(registerUser);
        }

        public static void AliveAck() {
            CGPAliveAck aliveAck = new CGPAliveAck();
            NetworkUnityEvent.Instance.Send(aliveAck);
        }

        /// <summary>
        /// 현재 개설된 방 정보를 요청
        /// </summary>
        public static void RoomListRequest()
        {
            CGPRoomListRequest roomListRequest = new CGPRoomListRequest();
            NetworkUnityEvent.Instance.Send(roomListRequest);
        }

        /// <summary>
        /// 방 생성
        /// </summary>
        public static void CreateRoom()
        {
            CGPCreateRoom createRoom = new CGPCreateRoom();
            NetworkUnityEvent.Instance.Send(createRoom);
        }

        /// <summary>
        /// 방 입장
        /// </summary>
        public static void EnterRoom(int sn)
        {
            CGPEnterRoom enterRoom = new CGPEnterRoom(sn);
            NetworkUnityEvent.Instance.Send(enterRoom);
        }

        /// <summary>
        /// 방 퇴장
        /// </summary>
        public static void LeaveRoom()
        {
            CGPLeaveRoom leaveRoom = new CGPLeaveRoom();
            NetworkUnityEvent.Instance.Send(leaveRoom);
        }

        /// <summary>
        /// 게임을 준비/취소한다.
        /// </summary>
        public static void GameReady()
        {
            CGPGameReady gameReady = new CGPGameReady();
            NetworkUnityEvent.Instance.Send(gameReady);
        }

        /// <summary>
        /// 게임을 시작한다.
        /// </summary>
        public static void GameStart()
        {
            CGPGameStart gameStart = new CGPGameStart();
            NetworkUnityEvent.Instance.Send(gameStart);
        }

        /// <summary>
        /// 가드 카드를 사용한다.
        /// </summary>
        /// <param name="userIndex">대상 유저</param>
        /// <param name="cardNumber">예상 카드 넘버</param>
        public static void LLGuardCheck(int userIndex, int cardNumber)
        {
            CGPLLGuardCheck llGuardCheck = new CGPLLGuardCheck(userIndex, cardNumber);
            NetworkUnityEvent.Instance.Send(llGuardCheck);
        }

        /// <summary>
        /// 왕실 신하 카드를 사용한다.
        /// </summary>
        /// <param name="userIndex">대상 유저</param>
        public static void LLRoyalSubject(int userIndex)
        {
            CGPLLRoyalSubject llRoyalSubejct = new CGPLLRoyalSubject(userIndex);
            NetworkUnityEvent.Instance.Send(llRoyalSubejct);
        }

        /// <summary>
        /// 험담가를 사용한다.
        /// </summary>
        /// <param name="userIndex">대상 유저</param>
        public static void LLGossip(int userIndex)
        {
            CGPLLGossip llGossip = new CGPLLGossip(userIndex);
            NetworkUnityEvent.Instance.Send(llGossip);
        }

        /// <summary>
        /// 동료를 사용한다.
        /// </summary>
        public static void LLCompanion()
        {
            CGPLLCompanion llCompanion = new CGPLLCompanion();
            NetworkUnityEvent.Instance.Send(llCompanion);
        }

        /// <summary>
        /// 영웅을 사용한다.
        /// </summary>
        /// <param name="userIndex">대상 유저</param>
        public static void LLHero(int userIndex)
        {
            CGPLLHero llHero = new CGPLLHero(userIndex);
            NetworkUnityEvent.Instance.Send(llHero);
        }

        /// <summary>
        /// 고자를 사용한다.
        /// </summary>
        /// <param name="userIndex">대상 유저</param>
        public static void LLWizard(int userIndex)
        {
            CGPLLWizard llWizard = new CGPLLWizard(userIndex);
            NetworkUnityEvent.Instance.Send(llWizard);
        }

        /// <summary>
        /// 레이디를 사용한다.
        /// </summary>
        public static void LLLady()
        {
            CGPLLLady llLady = new CGPLLLady();
            NetworkUnityEvent.Instance.Send(llLady);
        }

        public static void LLEmotionLaugh(int gameIdx)
        {
            CGPLLEmotionLaugh llEmotion = new CGPLLEmotionLaugh(gameIdx);
            NetworkUnityEvent.Instance.Send(llEmotion);
        }
    }
}