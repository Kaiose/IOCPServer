using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DummyClient
{
    public static class PacketFactory
    {
        public static Packetinterface getPacket(int packetType) 
        {
            switch ((PacketType)packetType)
            {
                case PacketType.E_C_REQ_EXIT: return new PK_C_REQ_EXIT();
                case PacketType.E_S_ANS_EXIT: return new PK_S_ANS_EXIT();
                case PacketType.E_C_REQ_SIGNUP: return new PK_C_REQ_SIGNUP();
                case PacketType.E_S_ANS_SIGNUP: return new PK_S_ANS_SIGNUP();
                case PacketType.E_C_REQ_SIGNIN: return new PK_C_REQ_SIGNIN();
                case PacketType.E_S_ANS_SIGNIN: return new PK_S_ANS_SIGNIN();
                case PacketType.E_S_NOTIFY_SERVER: return new PK_S_NOTIFY_SERVER();
                case PacketType.E_C_REQ_CONNECT: return new PK_C_REQ_CONNECT();
                case PacketType.E_S_ANS_CONNECT: return new PK_S_ANS_CONNECT();
                case PacketType.E_C_REQ_CONNECT_ROOM: return new PK_C_REQ_CONNECT_ROOM();
                case PacketType.E_S_ANS_CONNECT_ROOM: return new PK_S_ANS_CONNECT_ROOM();
                case PacketType.E_S_NOTIFY_OTHER_CLIENT: return new PK_S_NOTIFY_OTHER_CLIENT();
                case PacketType.E_C_REQ_CHAT: return new PK_C_REQ_CHAT();
                case PacketType.E_S_ANS_CHAT: return new PK_S_ANS_CHAT();
                case PacketType.E_C_REQ_MOVE: return new PK_C_REQ_MOVE();
                case PacketType.E_S_ANS_MOVE: return new PK_S_ANS_MOVE();
                case PacketType.E_S_NOTIFY_USER_DATA: return new PK_S_NOTIFY_USER_DATA();
                case PacketType.E_C_REQ_SHOT: return new PK_C_REQ_SHOT();
                case PacketType.E_S_ANS_SHOT: return new PK_S_ANS_SHOT();
                case PacketType.E_C_REQ_EXIT_ROOM: return new PK_C_REQ_EXIT_ROOM();
                case PacketType.E_S_ANS_EXIT_ROOM: return new PK_S_ANS_EXIT_ROOM();
                case PacketType.E_C_REQ_CROSS_OVER_SERVER: return new PK_C_REQ_CROSS_OVER_SERVER();
            }
            return null;
        }
    }
}
