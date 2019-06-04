using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DummyClient
{

    enum PacketType
    {
        E_C_REQ_EXIT = 90,
        E_S_ANS_EXIT = 91,
        PACKET_TYPE_ERROR = 99,
        E_C_REQ_SIGNUP = 100,
        E_S_ANS_SIGNUP = 101,
        E_C_REQ_SIGNIN = 102,
        E_S_ANS_SIGNIN = 103,
        E_S_NOTIFY_SERVER = 104,
        E_C_REQ_CONNECT = 105,
        E_S_ANS_CONNECT = 106,
        E_C_REQ_CONNECT_ROOM = 107,
        E_S_ANS_CONNECT_ROOM = 108,
        E_S_NOTIFY_OTHER_CLIENT = 109,
        E_C_REQ_CHAT = 110,
        E_S_ANS_CHAT = 111,
        E_C_REQ_MOVE = 112,
        E_S_ANS_MOVE = 113,


        E_C_NOTIFY_CONNECTED_ROOM = 114,
        E_S_NOTIFY_USER_DATA = 115,
        E_C_REQ_SHOT = 116,
        E_S_ANS_SHOT = 117,
        
        E_C_REQ_EXIT_ROOM = 118,
        E_S_ANS_EXIT_ROOM = 119,

        E_C_REQ_CROSS_OVER_SERVER = 121,
        //121 : T_ASK_HOLD_ROOM
        //122 : T_ANS_HOLD_ROOM
    }
};
