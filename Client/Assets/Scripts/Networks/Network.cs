using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

public class Network : MonoBehaviour
{
    public class AsyncObject
    {
        public byte[] buffer;
        public Socket clientSocket;

        public AsyncObject(int buffSize)
        {
            buffer = new byte[buffSize];
        }
    }

    private Socket socket = null;
    private bool isConnected = false;


    void Start()
    {
        Debug.Log( "### Client Start ###" );
        Connect();
    }

    public void Connect()
    {
        Debug.Log("### Connect ###");
        if (socket == null)
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.IP);

        try
        {
            socket.Connect("localhost", 6814);
            isConnected = true;
            Debug.Log("### Connect Success!! ###");
        }
        catch
        {
            isConnected = false;
            Debug.Log( "### Connect Fail!! ###" );
        }

        if(isConnected)
        {
            AsyncObject ao = new AsyncObject(65535);

            ao.clientSocket = socket;
            socket.BeginReceive(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackBeginReceive, ao);
        }
    }

    public void SendMessage(string somedatas)
    {
        Debug.Log("### SendMessage ###");
        /*
         * 이부분에서 보낼 데이터 파싱을 해야함
         */ 

        AsyncObject ao = new AsyncObject(1);
        ao.buffer = System.Text.Encoding.Unicode.GetBytes(somedatas);
        ao.clientSocket = socket;
        try
        {
            ao.clientSocket.BeginSend(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackSend, ao);
        }
        catch(Exception ex)
        {
            Debug.Log("BeginSend Exception >> " + ex);
            return;
        }
    }

    void OnCallbackBeginReceive(IAsyncResult ar)
    {
        Debug.Log("### OnCallbackBeginReceive ###");
        AsyncObject ao = (AsyncObject)ar.AsyncState;

        int recvBytes = 0;

        try
        {
            recvBytes = ao.clientSocket.EndReceive(ar);
        }
        catch(Exception ex)
        {
            Debug.Log("EndReceive Exception >> " + ex);
            return;
        }

        if(recvBytes > 0)
        {
            /*
             * 받은 메시지에 대한 부분을 여기서 파싱하면 될 듯.
             */

            byte[] msgBytes = new byte[recvBytes];
            Array.Copy(ao.buffer, msgBytes, recvBytes);
            Debug.Log( "Recv Message >> " + recvBytes );
        }

        try
        {
            ao.clientSocket.BeginReceive(ao.buffer, 0, ao.buffer.Length, SocketFlags.None, OnCallbackBeginReceive, ao);
        }
        catch(Exception ex)
        {
            Debug.Log("BeginReceive Exception >> " + ex);
            return;
        }
    }

    void OnCallbackSend(IAsyncResult ar)
    {
        Debug.Log("### OnCallbackSend ###");
        AsyncObject ao = (AsyncObject)ar.AsyncState;

        int sendBytes = 0;
        try
        {
            sendBytes = ao.clientSocket.EndSend(ar);
        }
        catch(Exception ex)
        {
            Debug.Log("EndSend Exception >> " + ex);
            return;
        }

        if(sendBytes > 0)
        {
            byte[] msgBytes = new byte[sendBytes];
            Array.Copy(ao.buffer, msgBytes, sendBytes);

            Debug.Log("Sended Message >> ");
        }
    }


}
