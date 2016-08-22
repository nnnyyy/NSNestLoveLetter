using UnityEngine;
using System.Collections;

namespace NSNetwork
{
    public class Sender
    {
        public static void Login(string id, string pw)
        {
            CGPLogin login = new CGPLogin(id, pw);
            NetworkUnityEvent.Instance.Send(login);
        }

        public static void RoomListRequest()
        {
            CGPRoomListRequest roomListRequest = new CGPRoomListRequest();
            NetworkUnityEvent.Instance.Send(roomListRequest);
        }

        public static void CreateRoom()
        {
            CGPCreateRoom createRoom = new CGPCreateRoom();
            NetworkUnityEvent.Instance.Send(createRoom);
        }

        public static void LeaveRoom()
        {
            CGPLeaveRoom leaveRoom = new CGPLeaveRoom();
            NetworkUnityEvent.Instance.Send(leaveRoom);
        }

        public static void GameReady()
        {
            CGPGameReady gameReady = new CGPGameReady();
            NetworkUnityEvent.Instance.Send(gameReady);
        }

        // Game Lover Letter
        public static void LLGuardCheck(int userIndex, int cardNumber)
        {
            CGPLLGuardCheck llGuardCheck = new CGPLLGuardCheck(userIndex, cardNumber);
            NetworkUnityEvent.Instance.Send(llGuardCheck);
        }

        public static void LLRoyalSubject(int userIndex)
        {
            CGPLLRoyalSubject llRoyalSubejct = new CGPLLRoyalSubject(userIndex);
            NetworkUnityEvent.Instance.Send(llRoyalSubejct);
        }

        public static void LLGossip(int userIndex)
        {
            CGPLLGossip llGossip = new CGPLLGossip(userIndex);
            NetworkUnityEvent.Instance.Send(llGossip);
        }

        public static void LLCompanion()
        {
            CGPLLCompanion llCompanion = new CGPLLCompanion();
            NetworkUnityEvent.Instance.Send(llCompanion);
        }

        public static void LLHero(int userIndex)
        {
            CGPLLHero llHero = new CGPLLHero(userIndex);
            NetworkUnityEvent.Instance.Send(llHero);
        }

        public static void LLWizard(int userIndex)
        {
            CGPLLWizard llWizard = new CGPLLWizard(userIndex);
            NetworkUnityEvent.Instance.Send(llWizard);
        }

        public static void LLLady()
        {
            CGPLLLady llLady = new CGPLLLady();
            NetworkUnityEvent.Instance.Send(llLady);
        }
    }
}