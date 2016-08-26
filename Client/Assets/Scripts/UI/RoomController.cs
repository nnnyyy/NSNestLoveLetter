using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.UI
{
    public class RoomController : MonoBehaviour
    {
        public GameObject rootSlot;
        public UILabel labelInfo;
        public UILabel labelRoomListIndex;
        public UILabel labelRoomCreateLeave;

        protected readonly int SLOT_COUNT = 3;
        protected List<GCPRoomListRet.RoomInfo> listRooms;
        protected int roomlistIndex = 0;
        protected int roomListMax = 0;

        protected RoomItem[] rooms;

        void Start()
        {
            AttachCallbacks();
            labelInfo.text = "[" + GlobalData.Instance.userNickname + "] W:" + GlobalData.Instance.cntWin + ", L:" + GlobalData.Instance.cntLose;

            CreateEmptyRooms();
            UpdateCreateInfo();
            UpdateRoomList();
            Sender.RoomListRequest();
        }

        void AttachCallbacks()
        {
            Receiver.OnRoomListRetCallback += OnCallbackRoomListRequest;
            Receiver.OnCreateRoomRetCallback += OnCallbackCreateRoomRet;
            Receiver.OnEnterRoomRetCallback += OnCallbackEnterRoomRet;
            Receiver.OnLeaveRoomRetCallback += OnCallbackLeaveRoomRet;
            Receiver.OnRoomStateCallback += OnCallbackRoomState;
            Receiver.OnGameStartRetCallback += OnCallbackGameStart;
        }

        void OnDestroy()
        {
            Receiver.OnRoomListRetCallback -= OnCallbackRoomListRequest;
            Receiver.OnCreateRoomRetCallback -= OnCallbackCreateRoomRet;
            Receiver.OnEnterRoomRetCallback -= OnCallbackEnterRoomRet;
            Receiver.OnLeaveRoomRetCallback -= OnCallbackLeaveRoomRet;
            Receiver.OnRoomStateCallback -= OnCallbackRoomState;
            Receiver.OnGameStartRetCallback -= OnCallbackGameStart;
        }

        void CreateEmptyRooms()
        {
            float gapY = -190f;
            rooms = new RoomItem[SLOT_COUNT];
            GameObject prafab = Resources.Load<GameObject>("UI/GUI_RoomItem");
            for( int i = 0; i < rooms.Length; ++i )
            {
                GameObject obj = GameObject.Instantiate(prafab) as GameObject;
                obj.transform.parent = rootSlot.transform;
                obj.transform.localScale = Vector3.one;
                obj.transform.localPosition = new Vector3(0f, (gapY * i), 0f);
                obj.name = "Slot_" + i.ToString("D2");
                rooms[i] = obj.GetComponent<RoomItem>();
                rooms[i].Show(null);
            }
        }

        void UpdateCreateInfo()
        {
            if(GlobalData.Instance.IsInRoom() == false)
            {
                labelRoomCreateLeave.text = "방만들기";
            }
            else
            {
                labelRoomCreateLeave.text = "나가기";
            }
        }

        void UpdateRoomList()
        {
            if (listRooms == null || listRooms.Count <= 0)
            {
                roomlistIndex = 0;
                labelRoomListIndex.text = "0/0";
                for (int i = 0; i < rooms.Length; ++i)
                    rooms[i].Show(null);

                return;
            }

            if (roomlistIndex < 0 || roomlistIndex >= roomListMax)
                roomlistIndex = 0;

            labelRoomListIndex.text = (roomlistIndex + 1) + "/" + roomListMax;

            int index = roomlistIndex * SLOT_COUNT;
            for( int i = 0; i < rooms.Length; ++ i )
            {
                int slot = index + i;
                if (slot >= 0 && slot < listRooms.Count)
                    rooms[i].Show(listRooms[slot]);
                else
                    rooms[i].Show(null);
            }
        }

        //**************************************************************************************************************************//
        // Click Funcs
        
        public void OnClickBack()
        {
            NetworkUnityEvent.Instance.Disconnect();
            SceneLoader.Instance.Load("intro");
        }

        public void OnClickCreateRoom()
        {
            if(GlobalData.Instance.IsInRoom() == false)
            {
                Sender.CreateRoom();
            }
            else
            {
                Sender.LeaveRoom();
            }
        }

        public void OnClickRefresh()
        {
            Sender.RoomListRequest();
        }

        public void OnClickPrev()
        {
            if (listRooms == null || listRooms.Count <= 0)
                return;

            if (roomlistIndex > 0)
            {
                --roomlistIndex;
                UpdateRoomList();
            }
        }

        public void OnClickNext()
        {
            if (listRooms == null || listRooms.Count <= 0)
                return;

            if(roomlistIndex < roomListMax)
            {
                ++roomlistIndex;
                UpdateRoomList();
            }
        }

        //**************************************************************************************************************************//
        // Callback Funcs

        void OnCallbackRoomListRequest(GCPRoomListRet roomListRet)
        {
            listRooms = roomListRet.listRooms;
            roomlistIndex = 0;
            roomListMax = (listRooms.Count / SLOT_COUNT) + ((listRooms.Count % SLOT_COUNT > 0) ? 1 : 0);
            UpdateCreateInfo();
            UpdateRoomList();
        }

        void OnCallbackCreateRoomRet(GCPCreateRoomRet createRoomRet)
        {
            if(createRoomRet.result == GCPCreateRoomRet.eReuslt.Success)
            {
                //UpdateCreateInfo();
            }
            else
            {
                DefaultPopup.OpenPopup("방만들기", "방만들기에 실패하였습니다.", DefaultPopup.eType.OK);
            }
        }

        void OnCallbackEnterRoomRet(GCPEnterRoomRet enterRoomRet)
        {
            if(enterRoomRet.result == GCPEnterRoomRet.eResult.Success)
            {
                //UpdateCreateInfo();
                Sender.RoomListRequest();
            }
            else
            {
                DefaultPopup.OpenPopup("입장하기", "게임방 입장에 실패하였습니다.", DefaultPopup.eType.OK);
            }
        }

        void OnCallbackLeaveRoomRet(GCPLeaveRoomRet leaveRoomRet)
        {
            Sender.RoomListRequest();
            UpdateCreateInfo();
            //UpdateRoomList();
        }

        void OnCallbackRoomState(GCPRoomState roomState)
        {
            //UpdateRoomList();
        }

        void OnCallbackGameStart(GCPGameStartRet gameStartRet)
        {
            switch( gameStartRet.result ) 
            {
                case GCPGameStartRet.eResult.Success:
                    // goto game
                    break;

                case GCPGameStartRet.eResult.NotAllReady:
                    DefaultPopup.OpenPopup("GameStart", "Not All Ready", DefaultPopup.eType.OK);
                    break;

                case GCPGameStartRet.eResult.NotEnoughUser:
                    DefaultPopup.OpenPopup("GameStart", "Not Enough User", DefaultPopup.eType.OK);
                    break;

                case GCPGameStartRet.eResult.NotRoomMaster:
                    DefaultPopup.OpenPopup("GameStart", "Not Room Master", DefaultPopup.eType.OK);
                    break;
            }
        }
    }
}