using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

namespace NSNetwork
{
    public class NetworkUnityEvent : MonoBehaviour
    {
        static NetworkUnityEvent mInstance = null;
        static public NetworkUnityEvent Instance
        {
            get
            {
                if (mInstance == null)
                {
                    mInstance = NSNest.Common.DontDestroyObject.Create<NetworkUnityEvent>((delObj) =>
                    {
                        mInstance = null;
                    });
                }

                return mInstance;
            }
        }

        
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

        public void Send(SendPacket packet)
        {
            listSend.AddLast(packet);
        }

        void Update()
        {
            if (network == null)
                return;

            if (network.IsConnected() == false)
                return;

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
    }
}

