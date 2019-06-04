using System;
using System.IO;
using System.Text;
using UnityEngine;
namespace DummyClient
{
    // 패킷의 구성 성분을 만든다
    public static class PacketUtil
    {
        //-------------------------------------------------------------------//
        //--- encodeing 부분

            //Memorystream 에서 write 가 성공하면 알아서 offset 변경됨 때문에 0 으로 해도 상관없음.
        public static void encodeHeader(MemoryStream packet, int headerType)
        {
            PacketUtil.encode(packet, headerType);
        }

        public static void encode(MemoryStream packet, bool value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(bool));
        }

        public static void encode(MemoryStream packet, Byte value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(Byte));
        }
        public static void encode(MemoryStream packet, Double value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(Double));
        }

        public static void encode(MemoryStream packet, Single value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(Single));
        }

        public static void encode(MemoryStream packet, Char value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(Char));
        }

        public static void encode(MemoryStream packet, Char[] value)
        {
            int len = (int)value.Length;
            PacketUtil.encode(packet, len);
            for(int i = 0; i < len; i++)
            {
                PacketUtil.encode(packet,value[i]);
            }
            
        }

        
        public static void encode(MemoryStream packet, Int32 value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(Int32));
           
        }

        public static void encode(MemoryStream packet, UInt32 value)
        {
            packet.Write(BitConverter.GetBytes(value), 0, sizeof(UInt32));
        }

        
        public static void encode(MemoryStream packet, string str)
        {
            PacketUtil.encode(packet, Encoding.UTF8.GetBytes(str).Length);
            packet.Write(Encoding.UTF8.GetBytes(str), 0, Encoding.UTF8.GetBytes(str).Length);
        }
        //-------------------------------------------------------------------//
        //---decoding 부분

        public static bool decodeBool(Byte[] data, ref Int32 offset)
        {
            bool val = BitConverter.ToBoolean(data, offset);
            offset += sizeof(bool);
            return val;
        }

        public static Char[] decodeCharArray(Byte[] data , ref Int32 offset)
        {
            Int32 strLen = PacketUtil.decodeInt32(data, ref offset);

            Char[] val = new char[strLen];

            for(int i = 0; i < strLen; i++)
            {
                val[i] = (char)PacketUtil.decodeByte(data, ref offset);
            }

            return val;
        }

        public static Int32 decodePacketLen(Byte[] data, ref Int32 offset)
        {
            return PacketUtil.decodeInt32(data, ref offset);
        }

        
        public static Byte decodeByte(Byte[] data, ref Int32 offset)
        {
            Byte val = data[offset];
            offset += sizeof(Byte);
            return val;
        }

        public static Char decodeInt8(Byte[] data, ref Int32 offset)
        {
            Char val = BitConverter.ToChar(data, offset);
            offset += sizeof(Char);
            return val;
        }

        public static Int16 decodeInt16(Byte[] data, ref Int32 offset)
        {
            Int16 val = BitConverter.ToInt16(data, offset);
            offset += sizeof(Int16);
            return val;
        }

        public static UInt16 decodeUInt16(Byte[] data, ref Int32 offset)
        {
            UInt16 val = BitConverter.ToUInt16(data, offset);
            offset += sizeof(UInt16);
            return val;
        }

        public static Int32 decodeInt32(Byte[] data, ref Int32 offset)
        {
            Int32 val = BitConverter.ToInt32(data, offset);
            offset += sizeof(Int32);
            return val;
        }

        public static UInt32 decodeUInt32(Byte[] data, ref Int32 offset)
        {
            UInt32 val = BitConverter.ToUInt32(data, offset);
            offset += sizeof(UInt32);
            return val;
        }


        public static Double decodedouble(Byte[] data, ref Int32 offset)
        {
            Double val = BitConverter.ToDouble(data, offset);
            offset += sizeof(UInt64);
            return val;
        }

        public static Single decodefloat(Byte[] data, ref Int32 offset)
        {
            Single val = BitConverter.ToSingle(data, offset);
            offset += sizeof(Single);
            return val;
        }
        public static string decodestring(Byte[] data, ref Int32 offset)
        {
            Int32 strLen = PacketUtil.decodeInt32(data, ref offset);
            string str = System.Text.Encoding.UTF8.GetString(data, offset, strLen);
            offset += strLen;
            return str;
        }

        

        public static Packetinterface packetAnalyzer(Byte[] packetByte, int totalOffset)
        {
            Int32 offset = 0;
            Int32 packetType = PacketUtil.decodeInt32(packetByte, ref offset);

            Packetinterface packet = PacketFactory.getPacket(packetType);
            if (packet == null)
            {
                Debug.Log("packet Analyzer failed");
                return null;
            }

            // 데이터가 있으면 decoding 해서 넘기기
            if (offset < totalOffset)
            {
                packet.decode(packetByte, ref offset);
            }
            return packet;
        }
    }
}