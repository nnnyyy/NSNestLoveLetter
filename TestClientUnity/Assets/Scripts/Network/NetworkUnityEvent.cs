﻿using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace NSNetwork
{
    public class NetworkUnityEvent : Singleton<NetworkUnityEvent>
    {        
        public string ip = "52.79.205.198";
        public int port = 7770;
        private Network network = null;

        private LinkedList<SendPacket> listSend = new LinkedList<SendPacket>();

        public NetworkUnityEvent()
        {
            network = new Network();
            network.onErrorCallback = OnError;

            listSend.Clear();
        }

        public void Connect(string ip, int port)
        {
            if (network == null)
                return;

            this.ip = ip;
            this.port = port;
            network.Connect(ip, port);
        }

        public void Disconnect()
        {
            if (network == null)
                return;

            if (network.IsConnected() == true)
                network.Discconnect();
        }

        public void Send(SendPacket packet)
        {
            listSend.AddLast(packet);
        }

        void Update()
        {
            if (network == null)
                return;

            if (network.IsConnected() == false)
            {                
                GlobalData.Instance.ClearData();
                //NetworkUnityEvent.Instance.Connect(NSNest.Common.Const.SERVER_IP, NSNest.Common.Const.SERVER_PORT);
                return;
            }                

            if( network.HasRecvPacket() )
            {
                OnReceive(network.GetPacket());
            }

            if(network.IsSended == false && listSend.Count > 0 )
            {
                var sendPacket = listSend.First.Value;
                listSend.RemoveFirst();
                network.Send(sendPacket.GetData());
            }
        }

        public void OnError(Network.eErrorType errorType)
        {
            Debug.Log( "### Error >> " + errorType + " ###" );
        }

        public void OnReceive(Packet packet)
        {
            Receiver.OnReceive(packet);
        }

        public bool IsConnected()
        {
            return network.IsConnected();
        }

        void OnApplicationQuit()
        {
            Disconnect();
        }
    }
}

