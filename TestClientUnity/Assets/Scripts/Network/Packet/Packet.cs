using UnityEngine;
using System;
using System.Collections;

namespace NSNetwork
{
    public class Packet
    {
        protected byte[] data;

        public byte[] GetData()
        {
            return data;
        }

        public Packet(byte[] data)
        {
            this.data = data;
        }

        public Packet()
        {
        }
    }

    public class ReceivePacket
    {
        public eGCP type;
        protected int byteIndex;

        public char GetChar(byte[] data)
        {
            char retVal = (char)data[byteIndex++];
            return retVal;
        }

        public short GetShort(byte[] data)
        {
            short retVal = BitConverter.ToInt16(data, byteIndex);
            byteIndex += sizeof(short);
            return retVal;
        }

        public int GetInt(byte[] data)
        {
            int retVal = BitConverter.ToInt32(data, byteIndex);
            byteIndex += sizeof(int);
            return retVal;
        }

        public long GetLong(byte[] data)
        {
            long retVal = BitConverter.ToInt64(data, byteIndex);
            byteIndex += sizeof(long);
            return retVal;
        }
     
        public string GetString(byte[] data)
        {
            string retVal = "";
            short size = GetShort(data);
            var enc = System.Text.Encoding.GetEncoding(51949);
            retVal = enc.GetString(data, byteIndex, size);
            byteIndex += size;
            return retVal;
        }
    }
    
    public class SendPacket : Packet
    {
        ArrayList al = new ArrayList();

        public void AddChar(byte val)       { al.Add(new byte[] { val }); }
        public void AddShort(short val)     { al.Add(BitConverter.GetBytes(val)); }
        public void AddInt(int val)         { al.Add(BitConverter.GetBytes(val)); }
        public void AdDLong(long val)       { al.Add(BitConverter.GetBytes(val)); }
        public void AddString(string val)
        {
            var enc = System.Text.Encoding.GetEncoding(51949);
            byte[] bytes = enc.GetBytes(val);
            AddShort((short)bytes.Length);
            al.Add(bytes);
        }

        public void CreateData()
        {
            short header = 0;
            for( int i = 0; i < al.Count; ++i )
                header += (short)(((byte[])al[i]).Length);

            data = new byte[sizeof(short) + header];
            Array.Copy(BitConverter.GetBytes(header), data, sizeof(short));

            int index = sizeof(short);
            for( int i = 0; i < al.Count; ++i )
            {
                byte[] tmp = (byte[])al[i];
                Array.Copy(tmp, 0, data, index, tmp.Length);
                index += tmp.Length;
            }
        }
    }
}
