using UnityEngine;
using System;
using System.Text;
using System.Collections;
using System.Collections.Generic;

static class CVT
{
    public static short ToShort(byte byte1, byte byte2) { return (short)((byte2 << 8) | (byte1 << 0)); }
    public static int ToDword(byte byte1, byte byte2, byte byte3, byte byte4) { return (int)((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0)); }
}

public class Packet
{
}

public class InPacket : Packet
{    
    public short packetId;
    public byte[] buffer;
    private int readPos;

    public InPacket(InPacket _packet)
    {
        packetId = _packet.packetId;
        buffer = new byte[_packet.buffer.Length];
        _packet.buffer.CopyTo(buffer, 0);
        readPos = _packet.readPos;
    }

    public InPacket(byte[] bytes)
    {        
        buffer = bytes;
        this.packetId = CVT.ToShort(buffer[0], buffer[1]);
        readPos += 2;
    }

    public byte readSubID()
    {
        if (buffer.Length < 2)
            return 0;

        this.readPos = 2;
        return this.buffer[1];
    }

    public byte Decode1()
    {
        int sIDX = readPos;
        readPos += sizeof(byte);
        return (byte)buffer[sIDX];
    }    

    public short Decode2()
    {
        int sIDX = readPos;
        readPos += sizeof(short);
        return BitConverter.ToInt16(buffer, sIDX);
    }

    public int Decode4()
    {
        int sIDX = readPos;
        readPos += sizeof(int);
        return BitConverter.ToInt32(buffer, sIDX);
    }    

    public bool DecodeBool()
    {        
        return (Decode1() == 1);
    }

    public string DecodeString_S()//short length
    {
        int sIDX = readPos;
        short len = CVT.ToShort(buffer[sIDX], buffer[++sIDX]);
        readPos += len + sizeof(short);
        System.Text.Encoding enc = System.Text.Encoding.GetEncoding(51949); // euc-kr
        return enc.GetString(buffer, ++sIDX, len);
    }

    public Int64 Decode8()
    {
        int sIDX = readPos;
        readPos += sizeof(Int64);
        return BitConverter.ToInt64(buffer, sIDX);
    }

    public void DecodeBuffer(ref byte[] data, int size)
    {
        int sIDX = readPos;
        readPos += size;
        data = new byte[size];
        Buffer.BlockCopy(buffer, sIDX, data, 0, size);
    }
};


public class OutPacket
{
    protected ArrayList alBuffer;
	private OutPacket() { this.alBuffer = new ArrayList(2); }
    public OutPacket(short PacketID)
		: this()
	{
        this.Encode2(PacketID);
	}
 //   public OutPacket(CMP PacketID)
	//	: this()
	//{
 //       this.Encode2((short)PacketID);
	//}
    
	public int Length { get { return ((short)(this.alBuffer.Count)); } }
	public byte[] Data { get { return this.ToSend(); } }
    public void Encode4Array(int[] DWORDList) { foreach (int i in DWORDList)            Encode4(i); }
    public void Encode4(int DWORD) { Encode1Array(BitConverter.GetBytes(DWORD)); }
    public void Encode2(short WORD) { Encode1Array(BitConverter.GetBytes(WORD)); }
    public void Encode2Array(short[] WORDList) { foreach (short s in WORDList)            Encode4(s); }
    public void Encode1(byte BYTE) { this.alBuffer.Add(BYTE); }
    public void Encode1Array(byte[] BYTEList) { foreach (byte b in BYTEList)            Encode1(b); }
    public void Encode8(Int64 INT64) { Encode1Array(BitConverter.GetBytes(INT64)); }
    public void Encode8Array(float[] INT64List) { foreach (Int64 s in INT64List)            Encode8(s); }

	public void InsertString(string str)
	{
        System.Text.Encoding enc = System.Text.Encoding.GetEncoding(51949); // euc-kr
        Byte[] stringByte = enc.GetBytes(str);
        Encode2((short)stringByte.Length);
        Encode1Array(stringByte);
	}
	public void Clear() { this.alBuffer.Clear(); }
	public override string ToString()
	{
        StringBuilder sb = new StringBuilder(this.Length.ToString());
        sb.Append(Environment.NewLine);
        foreach (byte b in this.alBuffer)
		{
			sb.Append(b); sb.Append(Environment.NewLine);
		}
		return sb.ToString();
	}

	private byte[] ToSend()
	{
        //CEncrypt.Encrypt(ref this.alBuffer);
		ArrayList al = new ArrayList(this.alBuffer);        
        al.InsertRange(0, BitConverter.GetBytes(this.Length));
		byte[] data = new byte[al.Count];
		al.CopyTo(0, data, 0, al.Count);
		return data;
	}

	public byte[] getMerge(byte[] byte1, byte[] byte2)
	{
		byte[] data = new byte[byte1.Length + byte2.Length];

		Buffer.BlockCopy(byte1, 0, data, 0, byte1.Length);
		Buffer.BlockCopy(byte2, 0, data, byte1.Length, byte2.Length);

		return data;
	}
}