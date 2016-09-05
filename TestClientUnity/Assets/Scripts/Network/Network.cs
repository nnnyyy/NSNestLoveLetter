using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

namespace NSNetwork
{
    public class Network
    {
        public enum eErrorType
        {
            ConnectFail,
            BeginReceiveFail,
            EndReceiveFail,
            BeginSendFail,
            EndSendFail,
            MAX
        }

        public class AsyncObject
        {
            public byte[] buffer;
            public Socket clientSocket;

            public AsyncObject(int buffSize)
            {
                buffer = new byte[buffSize];
                Array.Clear(buffer, 0, buffer.Length);
            }
        }

        private Socket socket = null;
        private readonly int BUFFSIZE = 1024;
        private readonly int HEADERSIZE = 2;
        private string ip = "";
        private int port = 0;

        private byte[] savedData = null;
        private LinkedList<Packet> listPacket = new LinkedList<Packet>();
        private object lockObject = new object();
        private bool isSended = false;

        public bool IsSended { get { return isSended; } }
        public delegate void ErrorCallback(eErrorType error);
        public ErrorCallback onErrorCallback;


        public bool HasRecvPacket()
        {
            if (listPacket.Count > 0)
                return true;

            return false;
        }

        public Packet GetPacket()
        {
            if (listPacket.Count <= 0)
                return null;

            Packet packet = listPacket.First.Value;
            lock (lockObject)
            {
                listPacket.RemoveFirst();
            }
            return packet;
        }

        public Network()
        {
            if (socket == null)
            {
                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.IP);                
            }
                

            listPacket.Clear();
        }

        public bool IsConnected()
        {
            if (socket == null)
                return false;

            return socket.Connected;
        }

        public void Connect(string ip, int port)
        {
            if (this.socket == null)
                return;

            bool isConnected = socket.Connected;            
            this.ip = ip;
            this.port = port;

            if (isConnected == false)
            {
                try
                {
                    socket.Connect(this.ip, this.port);
                    isConnected = true;
                    Debug.Log("### Connect Success!! ###");
                }
                catch(SocketException e)
                {
                    Debug.Log("Connect Error : " + e.ErrorCode);
                    isConnected = false;
                    if (onErrorCallback != null)
                        onErrorCallback(eErrorType.ConnectFail);                    
                }
            }

            if (isConnected)
            {
                AsyncObject ao = new AsyncObject(BUFFSIZE);
                ao.clientSocket = socket;

                try
                {
                    socket.BeginReceive(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackBeginReceive, ao);
                }
                catch (Exception ex)
                {
                    Debug.Log("BeginReceive Exception >> " + ex);
                    if (onErrorCallback != null)
                        onErrorCallback(eErrorType.BeginReceiveFail);
                }
            }
        }

        public void Discconnect()
        {
            listPacket.Clear();

            if (socket == null)
                return;

            socket.Disconnect(true);
        }

        public void Send(byte[] sendData)
        {
            if (sendData == null)
                return;

            isSended = true;

            AsyncObject ao = new AsyncObject(1);
            ao.buffer = sendData;
            ao.clientSocket = socket;
            try
            {
                ao.clientSocket.BeginSend(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackEndSend, ao);
            }
            catch (Exception ex)
            {
                Debug.Log("BeginSend Exception >> " + ex);
                if (onErrorCallback != null)
                    onErrorCallback(eErrorType.BeginSendFail);

                isSended = false;
                return;
            }
        }

        void OnCallbackBeginReceive(IAsyncResult ar)
        {
            AsyncObject ao = (AsyncObject)ar.AsyncState;

            int recvBytes = 0;

            try
            {
                recvBytes = ao.clientSocket.EndReceive(ar);
            }
            catch (Exception ex)
            {
                Debug.Log("EndReceive Exception >> " + ex);                
                if (onErrorCallback != null)
                    onErrorCallback(eErrorType.EndReceiveFail);
                return;
            }

            if (recvBytes > 0)
            {
                byte[] msgBytes = null;
                if (savedData != null)
                {
                    msgBytes = new byte[recvBytes + savedData.Length];
                    Array.Copy(savedData, msgBytes, savedData.Length);
                    Array.Copy(ao.buffer, 0, msgBytes, savedData.Length, recvBytes);
                    savedData = null;
                }
                else
                {
                    msgBytes = new byte[recvBytes];
                    Array.Copy(ao.buffer, msgBytes, recvBytes);
                }

                if (msgBytes.Length < HEADERSIZE)
                {
                    savedData = new byte[msgBytes.Length];
                    Array.Copy(msgBytes, savedData, msgBytes.Length);
                }
                else
                {
                    while (true)
                    {
                        short header = BitConverter.ToInt16(msgBytes, 0);
                        if ((msgBytes.Length - HEADERSIZE) >= header)
                        {
                            byte[] packetData = new byte[header];
                            Array.Copy(msgBytes, HEADERSIZE, packetData, 0, header);
                            lock (lockObject)
                            {
                                listPacket.AddLast(new Packet(packetData));
                            }

                            if ((msgBytes.Length - HEADERSIZE) > header)
                            {
                                byte[] tmpData = new byte[(msgBytes.Length - HEADERSIZE) - header];
                                Array.Copy(msgBytes, (HEADERSIZE + header), tmpData, 0, tmpData.Length);
                                msgBytes = tmpData;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            var enc = System.Text.Encoding.GetEncoding(51949);
                            if (enc.GetString(msgBytes).CompareTo("Hello") == 0)
                                break;

                            savedData = new byte[msgBytes.Length];
                            Array.Copy(msgBytes, savedData, msgBytes.Length);
                            break;
                        }
                    }
                }
            }

            try
            {
                ao.clientSocket.BeginReceive(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackBeginReceive, ao);
            }
            catch (Exception ex)
            {
                Debug.Log("BeginReceive Exception >> " + ex);
                if (onErrorCallback != null)
                    onErrorCallback(eErrorType.BeginReceiveFail);
                return;
            }
        }

        void OnCallbackEndSend(IAsyncResult ar)
        {
            isSended = false;
            AsyncObject ao = (AsyncObject)ar.AsyncState;

            int sendBytes = 0;
            try
            {
                sendBytes = ao.clientSocket.EndSend(ar);
            }
            catch (Exception ex)
            {
                Debug.Log("EndSend Exception >> " + ex);
                if (onErrorCallback != null)
                    onErrorCallback(eErrorType.EndSendFail);
                return;
            }

            if (sendBytes > 0)
            {
                //byte[] msgBytes = new byte[sendBytes];
                //Array.Copy(ao.buffer, msgBytes, sendBytes);
            }
        }


    }
}