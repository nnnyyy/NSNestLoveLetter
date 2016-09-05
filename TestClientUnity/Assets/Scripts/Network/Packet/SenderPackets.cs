using UnityEngine;
using System.Collections;

namespace NSNetwork
{
    public class CGPLogin : SendPacket
    {
        public CGPLogin(string id, string pw)
        {
            AddShort((short)eCGP.CGP_Login);
            AddString(id);
            AddString(pw);
            CreateData();
        }
    }

    public class CGPRegisterUser : SendPacket
    {
        public CGPRegisterUser(string id, string pw, string nick)
        {
            AddShort((short)eCGP.CGP_RegisterUser);
            AddString(id);
            AddString(pw);
            AddString(nick);
            CreateData();
        }
    }

    public class CGPAliveAck : SendPacket
    {
        public CGPAliveAck()
        {
            AddShort((short)eCGP.CGP_AliveAck);
            CreateData();
        }
    }
    public class CGPRoomListRequest : SendPacket
    {
        public CGPRoomListRequest()
        {
            AddShort((short)eCGP.CGP_RoomListRequest);
            CreateData();
        }
    }

    public class CGPCreateRoom : SendPacket
    {
        public CGPCreateRoom()
        {
            AddShort((short)eCGP.CGP_CreateRoom);
            CreateData();
        }
    }

    public class CGPEnterRoom : SendPacket
    {
        public CGPEnterRoom(int sn)
        {
            AddShort((short)eCGP.CGP_EnterRoom);
            AddInt(sn);
            CreateData();
        }
    }

    public class CGPLeaveRoom : SendPacket
    {
        public CGPLeaveRoom()
        {
            AddShort((short)eCGP.CGP_LeaveRoom);
            CreateData();
        }
    }

    public class CGPGameReady : SendPacket
    {
        public CGPGameReady()
        {
            AddShort((short)eCGP.CGP_GameReady);
            CreateData();
        }
    }

    public class CGPGameStart : SendPacket
    {
        public CGPGameStart()
        {
            AddShort((short)eCGP.CGP_GameStart);
            CreateData();
        }
    }

    public class CGPLLGuardCheck : SendPacket
    {
        public CGPLLGuardCheck( int userIndex, int cardNumber )
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_GuardCheck);
            AddInt(userIndex);
            AddInt(cardNumber);
            CreateData();
        }
    }

    public class CGPLLRoyalSubject : SendPacket
    {
        public CGPLLRoyalSubject(int userIndex)
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_RoyalSubject);
            AddInt(userIndex);
            CreateData();
        }
    }

    public class CGPLLGossip : SendPacket
    {
        public CGPLLGossip(int userIndex)
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Gossip);
            AddInt(userIndex);
            CreateData();
        }
    }

    public class CGPLLCompanion : SendPacket
    {
        public CGPLLCompanion()
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Companion);
            CreateData();
        }
    }

    public class CGPLLHero : SendPacket
    {
        public CGPLLHero(int userIndex)
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Hero);
            AddInt(userIndex);
            CreateData();
        }
    }

    public class CGPLLWizard : SendPacket
    {
        public CGPLLWizard(int userIndex)
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Wizard);
            AddInt(userIndex);
            CreateData();
        }
    }

    public class CGPLLLady : SendPacket
    {
        public CGPLLLady()
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Lady);
            CreateData();
        }
    }

    public class CGPLLEmotionLaugh : SendPacket
    {
        public CGPLLEmotionLaugh(int _gameIdx)
        {
            AddShort((short)eCGP.CGP_GameLoveLetter);
            AddShort((short)eCGP_LoveLetter.CGP_LL_Emotion);
            AddInt(_gameIdx);
            CreateData();
        }
    }
}