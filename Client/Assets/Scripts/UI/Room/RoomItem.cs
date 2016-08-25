﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;

public class RoomItem : MonoBehaviour
{
    public GameObject objBase;
    public GameObject objIn;
    public GameObject objOut;

    public UILabel labelRoomSN;
    public UILabel labelPlayerCount;
    public UILabel[] labelPlayers;
    public UILabel[] labelReadys;
    public GameObject btnLeave;
    public GameObject btnReadyStart;
    public UILabel labelReadyStart;

    protected bool isInit = false;
    protected GCPRoomListRet.RoomInfo roomInfo;

    protected readonly int MAX_PLAYER = 4;

    void Init()
    {
        if (isInit == true)
            return;


        isInit = true;
    }

    public void Show(GCPRoomListRet.RoomInfo _roomInfo)
    {
        HideAll();
        roomInfo = _roomInfo;
        if (roomInfo == null)
            return;

        labelRoomSN.text = "[" + _roomInfo.sn + "]";

        if (_roomInfo.sn == GlobalData.Instance.roomSN)
        {
            ShowIn();
            return;
        }

        objBase.SetActive(true);
        objIn.SetActive(false);
        objOut.SetActive(true);

        labelPlayerCount.text = "(" + _roomInfo.userCount + "/4)";
    }

    void ShowIn()
    {
        objBase.SetActive(true);
        objIn.SetActive(true);
        objOut.SetActive(false);

        var players = GlobalData.Instance.roomUsers;
        for(int i = 0; i < MAX_PLAYER; ++i)
        {
            if (i < players.Count)
            {
                labelPlayers[i].text = players[i].nickName;
                labelReadys[i].text = (players[i].readyState > 0) ? "[O]" : "[X]";
            }
            else
            {
                labelPlayers[i].text = "";
                labelReadys[i].text = "[X]";
            }
        }
        
        if(GlobalData.Instance.roomMasterSN == GlobalData.Instance.userSN)
        {
            labelReadyStart.text = "시작";
        }
        else
        {
            labelReadyStart.text = "준비";
        }

    }

    public void HideAll()
    {
        Init();

        objBase.SetActive(false);
        objIn.SetActive(false);
        objOut.SetActive(false);
    }

    public void OnClickLeave()
    {
        if (roomInfo == null)
            return;

        if (GlobalData.Instance.roomSN != roomInfo.sn)
            return;

        Sender.LeaveRoom();
    }

    public void OnClickEnterRoom()
    {
        if (roomInfo == null)
            return;

        Sender.EnterRoom(roomInfo.sn);
    }

    public void OnClickReadyStart()
    {
        if (GlobalData.Instance.roomMasterSN == GlobalData.Instance.userSN)
        {
            Sender.GameStart();
        }
        else
        {
            Sender.GameReady();
        }
    }
}
