using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

//	주석 풀어도 안돌아감. ( 다 지우고 참고 해야될 부분만 남겨놓음 )
//	Update랑 TryReceive 랑 ProcessPacket 참고하면 될 듯함.
//	TryReceive 에서 수신한 패킷 조립하고 ProcessPacket에서 처리하는 구조~
/*
public class NetworkManager : Singleton<NetworkManager>
{
    

    // Update is called once per frame
    void Update()
    {
        if (clientSocket.bAlertWndShowFlag)
        {
            clientSocket.bAlertWndShowFlag = false;
            if(IsRemainConnectableServer()) {

                string sMaintenanceStart, sMaintenanceEnd;

                //  연결 첫 시도가 끝난 상태에서, 본섭 점검이 진행되었을 때 공지
                if (curConnServerIndex == 1 && 
                    FindObjectOfType<StartScene>() != null &&
                    FindObjectOfType<StartScene>().CheckMaintenance(out sMaintenanceStart, out sMaintenanceEnd) )
                {
                    UIManager.o.OpenMessageBox("", string.Format("서버 점검 중입니다.\r\n\r\n{0}~\r\n{1}", sMaintenanceStart, sMaintenanceEnd), (ButtonID id, UIMessageBox uiMessageBox) =>
                    {
                        ABManager.o.LoadLevel("Start");
                    }, MessageBoxType.okey);
                }

                doConnect = true;
                return;
            }

            UIManager.o.OpenMessageBox("", "아래 이유로 게임을 이용할 수 없습니다.\r\n1) 게임월드가 점검 중일 경우\r\n2)스타플래닛 모바일 점검 중일 경우\r\n네트워크 연결이 원활하지 않은 경우\r\n자세한 내용은 홈페이지를 확인 해주세요.", (ButtonID id, UIMessageBox uiMessageBox) =>
            {
                Application.Quit();

            }, MessageBoxType.okey);
            return;
        }

        if (networkState == DISCONNECTED)
        {
            if (doConnect)
            {
                doConnect = false;
                if (doDelayReConnect)
                {
                    // WIFI <=> LTE 변환시 기기상황마다 1~2초 소요가되어 잠시 후에 시도 한다. (바로 시도하여 실패시 네트워크 오류 메시지)
                    System.Threading.Thread.Sleep(2500);

                    Init();
                    Debug.Log("Connect Start : " + sDelayedConnIP + " " + nDelayedConnPort);
                    clientSocket.Connect(sDelayedConnIP, nDelayedConnPort);
                    networkState = CONNECTING;
                    if (tryCount++ >= 5)
                    {
                        doDelayReConnect = false;
                        tryCount = 0;
                        networkState = FAILED;
                        if (OnConnectFailed != null)
                            OnConnectFailed();
                    }

                }
                else if (!IsRemainConnectableServer())
                {
                    tryCount = 0;
                    networkState = FAILED;
                    if (OnConnectFailed != null)
                        OnConnectFailed();
                }
                else
                {
                    Init();

                    if (bTransfer)
                    {
                        bTransfer = false;
                        clientSocket.Connect(ip, port);
                        sDelayedConnIP = ip;
                        nDelayedConnPort = port;
                        ++tryCount;
                    }
                    else
                    {
                        Debug.Log("Connect Start : " + liIpPort[curConnServerIndex].ip + " " + liIpPort[curConnServerIndex].port);
                        if (FindObjectOfType<StartScene>() != null)
                        {
                            FindObjectOfType<StartScene>().StartServerConnectStateTextAnimation(curConnServerIndex + 1, liIpPort.Count);
                        }
                        clientSocket.Connect(liIpPort[curConnServerIndex].ip, liIpPort[curConnServerIndex].port);
                        sDelayedConnIP = liIpPort[curConnServerIndex].ip;
                        nDelayedConnPort = liIpPort[curConnServerIndex].port;
                        ++curConnServerIndex;
                        ++tryCount;
                    }
                    networkState = CONNECTING;
                }
            }
        }

        if (networkState == CONNECTING)
        {
            if (clientSocket.connected)
            {
                networkState = CONNECTED;
                if (OnConnected != null)
                {
                    OnConnected();
                    if (doDelayReConnect)
                    {
                        OnConnected = () =>
                        {
                            LoginType type = Global.o.GetLastLoginType();
                            string token = Global.o.GetAutoLoginToken();

                            OutPacket oPacket = new OutPacket(CMP.CMP_DelayedCloseLogin);
                            oPacket.Encode1((byte)type);
                            oPacket.InsertString(SPMHelper.GetDevieceID());
                            oPacket.InsertString(token);
                            NetworkManager.o.SendPacket(oPacket);
                            return;
                        };
                        tryCount = 0;
                        doDelayReConnect = false;
                    }                    
                }
            }
            else if (!clientSocket.connecting)
            {
                networkState = DISCONNECTED;
                doDelayReConnect = false;
            }
        }

        if (networkState == CONNECTED)
        {            
            TryReceive();
        }
    }

    public void Close()
    {
        tryCount = 0;
        if (clientSocket != null)
            clientSocket.Close();
        networkState = DISCONNECTED;
    }

    private void TryReceive()
    {
        recvBuffer.Prepare();

        int bytesRead = clientSocket.Receive(recvBuffer.buffer, recvBuffer.index, recvBuffer.size);        
        if (bytesRead < 0)
        {
            clientSocket.Close();
            networkState = DISCONNECTED;
            if (OnDisconnected != null)
                OnDisconnected();
        }
        else if (bytesRead > 0)
        {
            recvBuffer.Forward(bytesRead);
            inputStream.ReadComplete();

            UInt32 packetLen = 0;
            if (!inputStream.ReadUInt32(ref packetLen))
            {
                MDebug.o.AddLog(string.Format("inputStream.ReadUInt32 Error"));
                return;
            }
            bytesRead -= 4;

            //MDebug.o.AddLog(string.Format("packetLen {0} / {1} ", packetLen, bytesRead));

            byte[] bytes = new byte[packetLen];
            int PacketIndex = 0;

            while (true)
            {
                int nRead = bytesRead;
                if (packetLen < bytesRead)
                    nRead = Convert.ToInt32(packetLen);

                if (!inputStream.ReadData(ref bytes, PacketIndex, nRead))
                {
                    return;
                }
                inputStream.ReadComplete();

                PacketIndex += bytesRead;

                if (PacketIndex < packetLen)
                {
                    recvBuffer.Prepare();
                    bytesRead = clientSocket.Receive(recvBuffer.buffer, recvBuffer.index, recvBuffer.size);
                    if (bytesRead < 0)
                    {
                        clientSocket.Close();
                        networkState = DISCONNECTED;
                        if (OnDisconnected != null)
                            OnDisconnected();
                        return;
                    }
                    else if (bytesRead > 0)
                    {
                        recvBuffer.Forward(bytesRead);
                        inputStream.ReadComplete();

                    }
                }
                else
                {
                    CEncrypt.Decrypt(ref bytes);

                    InPacket packet = new InPacket(bytes);
                    ProcessPacket(packet);
                    recvSeq++;

                    if (nRead < bytesRead)
                    {
                        if (!inputStream.ReadUInt32(ref packetLen))
                        {
                            return;
                        }
                        bytesRead -= 4;

                        bytes = new byte[packetLen];
                        PacketIndex = 0;
                    }
                    else
                        break;
                }
            }

        }
    }

    public void ProcessPacket( InPacket _packet ) {

        LMP type = (LMP)_packet.packetId;        
        switch (type)
        {
            case LMP.LMP_AliveReq:
                {
                    OutPacket oPacket = new OutPacket(CMP.CMP_AliveAck);
                    SendPacket(oPacket);
                }
                break;
            case LMP.LMP_MobileLoginReq:
                CUserLocal.o.OnLoginRequest(_packet);
                break;
            case LMP.LMP_AccountListResult:
                CUserLocal.o.OnAccountListResult(_packet);
                break;
            case LMP.LMP_SelectSPMAccountResult:
                CUserLocal.o.OnSelectSPMAccountResult(_packet);
                break;
            case LMP.LMP_CharacterListResult:
                CUserLocal.o.OnCharacterListResult(_packet);
                break;
            case LMP.LMP_CharacterSelectResult:
                CUserLocal.o.OnCharacterSelectResult(_packet);
                break;
            case LMP.LMP_StarTicketInfo:
                CUserLocal.o.OnStarTicketInfo(_packet);
                break;
            case LMP.LMP_AutoMobileLoginReq:
                CUserLocal.o.OnAutoLoginResult(_packet);
                break;
            case LMP.LMP_UIStarPlanetMiniGameResult:
                ManagerBase.o.OnUIStarPlanetMiniGameResult(_packet);
                break;
            case LMP.LMP_MiniRoom:
                ManagerBase.o.OnPacketBase(_packet);
                break;
            case LMP.LMP_Message:
                WvsContext.o.ProcessPacket(_packet);
                break;
            case LMP.LMP_MigrateCommand:
                OnMigrateOut(_packet);
                break;
            case LMP.LMP_MigrateComplete:
                OnMigrateComplete(_packet);
                break;                
            case LMP.LMP_MobileStarPlanetMatching_Error:
                ManagerBase.o.OnStarPlanetMatchingError(_packet);
                break;
            case LMP.LMP_MobileStarPlanetMatching_RegisterAck:
                ManagerBase.o.OnStarPlanetMatchingRegisterAck(_packet);
                break;
            case LMP.LMP_MobileStarPlanetMatching_UnRegisterAck:
                ManagerBase.o.OnStarPlanetMatchingUnRegisterAck(_packet);
                break;
            case LMP.LMP_MobileStarPlanetMatching_OpenUI:
                ManagerBase.o.OnStarPlanetMatchingOpenUI(_packet);
                break;
            case LMP.LMP_MobileStarPlanetMatching_WaitAssent:
                ManagerBase.o.OnStarPlanetMatchingWaitAssent(_packet);
                break;
            case LMP.LMP_MobileStarPlanetMatching_Transfer:
                ManagerBase.o.OnStarPlanetMatchingTransfer(_packet);
                break;
            case LMP.LMP_MobileUIStarPlanetQueue:
                ManagerBase.o.OnStarPlanetMatchingQueue(_packet);
                break;
            case LMP.LMP_MobileUIStarPlanetQueueErr:
                ManagerBase.o.OnStarPlanetMatchingQueueErr(_packet);
                break;
            case LMP.LMP_MobileProgressMessageFont:
                ManagerBase.o.OnProgressMessageFont(_packet);
                break;
            case LMP.LMP_SetCharacterID:
                CUserLocal.o.OnSetCharacterID(_packet);
                break;

            case LMP.LMP_GetTodayMileageResult:
                int nInGameMileage = _packet.Decode4();
                int nMobileMileage = _packet.Decode4();
                int nToday = _packet.Decode4();
                Debug.Log(string.Format("{0} {1} {2}", nInGameMileage, nMobileMileage, nToday));
                CUserLocal.o.m_nInGameCharMileage = nInGameMileage;
                CUserLocal.o.m_nMobileCharMileage = nMobileMileage;
                CUserLocal.o.m_nCharMileage = nInGameMileage + nMobileMileage;
                CUserLocal.o.m_nTodayEarnedMileage = nToday;                
                break;            
            case LMP.LMP_ScriptMessage:
            case LMP.LMP_ScriptItemScreenMessage:
                string message = _packet.DecodeString_S();
                //UIManager.o.NoticeMessage(message, 2.5f);
                UIManager.o.OpenMessageBox("", message, null, MessageBoxType.okey);
                break;

            case LMP.LMP_MobileStarPlanetMatching_AbleToPlayWithAI:
                ManagerBase.o.OnAIMatchEnable();
                break;

            case LMP.LMP_StarPlanetInfoResultByCharacter:
                CUserLocal.o.OnStarPlanetInfoResult(_packet);
                break;            
    
            case LMP.LMP_MileageNotice:
                {
                    string messageMileageNotice = _packet.DecodeString_S();
                    UIManager.o.OpenMessageBox("",messageMileageNotice,null,MessageBoxType.okey);
                }
                break;           
         
            case LMP.LMP_BroadcastMsg:
                {
                    int nType = (int)_packet.Decode1();
                    bool bEnable = _packet.DecodeBool();
                    if (bEnable)
                    {
                        string msg = _packet.DecodeString_S();
                        Lobby lobby = FindObjectOfType<Lobby>();
                        if (lobby != null)
                        {                            
                            CUserLocal.o.ReserveSlideMessage(msg);  
                        }
                        else
                        {
                            CUserLocal.o.ReserveSlideMessage(msg);                            
                        }
                    }
                    else
                    {
                        Lobby lobby = FindObjectOfType<Lobby>();
                        if (lobby != null)
                        {                            
                            CUserLocal.o.ReserveSlideMessage("");
                        }
                        else
                        {
                            CUserLocal.o.ReserveSlideMessage("");
                        }
                    }             
                }
                break;
            case LMP.LMP_DataCRCRequest:
                {
                    int nPos = _packet.Decode4();
                    int nLength = _packet.Decode4();
                    int nRandCRC = _packet.Decode4();

                    DateTime dt = DateTime.Now;
                    int nDataCRC = APKSecurityMan.o.GetAPKFileCRC(nPos, nLength, nRandCRC);
                    string sSigncodeCookie = APKSecurityMan.o.GetXigncodeCookie();
                    TimeSpan ts = DateTime.Now - dt;

                    OutPacket oPacket = new OutPacket(CMP.CMP_DataCRCRespones);
                    oPacket.Encode4(nDataCRC);
                    oPacket.InsertString(sSigncodeCookie);
                    oPacket.Encode4((int)(ts.TotalMilliseconds * 1000));
                    SendPacket(oPacket);
                }
                break;
            case LMP.LMP_DelayedCloseLoginResult:
                {
                    CUserLocal.o.OnDelayedCloseLoginResult(_packet);
                }
                break;

            case LMP.LMP_CheckUpdateResult:        
                {
                    StartScene startScene = FindObjectOfType<StartScene>();
                    if (startScene != null)
                    {
                        startScene.OnCheckUpdateResult(_packet.DecodeBool());
                    }
                }
                break;

            case LMP.LMP_RefreshRoundInfo:
                {
                    long nRound = _packet.Decode4();
                    UIManager.o.OpenMessageBox("", "새로운 라운드가 시작되었습니다.", null, MessageBoxType.okey);
                }
                break;

            case LMP.LMP_MobileReviewEventParticipate:
                {
                    int nParticipate = _packet.Decode4();
                    Debug.Log("[LMP_MobileReviewEventParticipate] " + nParticipate);
                    if (nParticipate == 0)
                    {
                        //  이벤트 참여 기록이 없다면, 참여 시킴.
                        CUserLocal.o.bReviewEventParticipate = false;
                    }
                }
                break;

            case LMP.LMP_LobbyMessage:
                {
                    string sMsg = _packet.DecodeString_S();
                    Global.o.ReservMessageForLobby(sMsg);                    
                }
                break;

            case LMP.LMP_MobileEventModify:
                {
                    CUserLocal.o.OnMobileEventModify(_packet);
                }
                break;


            case LMP.LMP_GameRecordInfo:
                {
                    CUserLocal.o.gameRecord.Decode(_packet);                    
                }
                break;


            case LMP.LMP_StarCoin:
                {
                    CUserLocal.o.OnStarCoin(_packet);
                }
                break;
            case LMP.LMP_StarCoinRewardResult:
                {
                    byte nType = _packet.Decode1();
                    int nStarCoin = _packet.Decode4();

                    string sMsg;
                    if (nStarCoin > 0)
                        sMsg = string.Format("스타코인 {0}개가 지급되었습니다.", nStarCoin);
                    else
                        sMsg = string.Format("오류가 발생하였습니다. 잠시 후 다시 이용해주세요");

                    UIManager.o.OpenMessageBox("", sMsg, null, MessageBoxType.okey);
                }
                break;

            case LMP.LMP_MobileStarPlanetFriendPlay_RegisterAck: ManagerBase.o.OnStarPlanetFriendPlay_RegisterAck(_packet); break;
            case LMP.LMP_MobileStarPlanetFriendPlay_RegisterCancelMatchProc: ManagerBase.o.OnStarPlanetFriendPlay_CancelMatchProc(_packet); break;
            case LMP.LMP_MobileStarPlanetFriendPlay_UpdateRoomInfo: ManagerBase.o.OnStarPlanetFriendPlay_UpdateRoomInfo(_packet); break;
            case LMP.LMP_MobileStarPlanetFriendPlay_MiniGameRetult: ManagerBase.o.OnStarPlanetFriendPlay_GameResult(_packet); break;
            case LMP.LMP_MobileStarPlanetFriendPlay_AgreeState: ManagerBase.o.OnStarPlanetFriendPlay_AgreeState(_packet); break;
            case LMP.LMP_MobileStarPlanetFriendPlay_Aborted: ManagerBase.o.OnStarPlanetFriendPlay_Aborted(_packet); break;
            default:
                break;
        }

    }

    public void SendPacket( OutPacket _packet )
    {
        //Debug.Log(_packet.ToString());
        if (clientSocket == null || clientSocket.connected == false )
            return;

        clientSocket.Send(_packet.Data);
    }
}
*/