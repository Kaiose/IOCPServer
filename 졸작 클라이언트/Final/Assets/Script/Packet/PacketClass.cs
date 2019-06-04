using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DummyClient
{
    public class PK_C_REQ_EXIT : Packet, Packetinterface
    {

        public int accountId;
        public int roomNumber;
        public int userNumber;
        
        public PK_C_REQ_EXIT()
        {
            type = (int)PacketType.E_C_REQ_EXIT;

        }

        int Packetinterface.getType(){
            return type;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, accountId);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            accountId = PacketUtil.decodeInt32(packet, ref offset);
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }
    }

    public class PK_S_ANS_EXIT : Packet, Packetinterface
    {

    
        public int userNumber;

        public PK_S_ANS_EXIT()
        {
            type = (int)PacketType.E_S_ANS_EXIT;

        }

        int Packetinterface.getType()
        {
            return type;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
      
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }
    }

    public class PK_C_REQ_SIGNUP : Packet, Packetinterface
    {
        public string name;
        public string password;
        public PK_C_REQ_SIGNUP()
        {
            type = (int)PacketType.E_C_REQ_SIGNUP;
        }


        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, name);
            PacketUtil.encode(stream, password);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            name = PacketUtil.decodestring(packet, ref offset);
            password = PacketUtil.decodestring(packet, ref offset);

        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    public class PK_S_ANS_SIGNUP : Packet, Packetinterface
    {
        public bool result;
        public PK_S_ANS_SIGNUP()
        {
            type = (int)PacketType.E_S_ANS_SIGNUP;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, result);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            result = PacketUtil.decodeBool(packet, ref offset);
        }



        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };


    public class PK_C_REQ_SIGNIN : Packet, Packetinterface
    {
        public string name;
        public string password;
        public PK_C_REQ_SIGNIN()
        {
            type = (int)PacketType.E_C_REQ_SIGNIN;
        }


        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, name);
            PacketUtil.encode(stream, password);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            name = PacketUtil.decodestring(packet, ref offset);
            password = PacketUtil.decodestring(packet, ref offset);
        }


        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    public class PK_S_ANS_SIGNIN : Packet, Packetinterface
    {

        public bool result;
        public PK_S_ANS_SIGNIN()
        {
            type = (int)PacketType.E_S_ANS_SIGNIN;
        }



        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, result);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            result = PacketUtil.decodeBool(packet, ref offset);
        }


        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };


    public class PK_C_REQ_CHAT : Packet, Packetinterface {

    	public int roomNumber;
        public int userNumber;
        public string chat;
	    public PK_C_REQ_CHAT()
        {
            type = (int)PacketType.E_C_REQ_CHAT;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, chat);

        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            chat = PacketUtil.decodestring(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }
        int Packetinterface.getType()
        {
            return type;
        }

    };

    public class PK_S_ANS_CHAT : Packet, Packetinterface {
        public string name;
        public int userNumber;
        public string chat;
        public PK_S_ANS_CHAT()
        {
            type = (int)PacketType.E_S_ANS_CHAT;
        }


        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, name);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, chat);

        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            name = PacketUtil.decodestring(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            chat = PacketUtil.decodestring(packet, ref offset);

        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }

        int Packetinterface.getType()
        {
            return type;
        }
    };

    
    //E_S_NOTIFY_SERVER = 104,
    public class PK_S_NOTIFY_SERVER : Packet, Packetinterface
    {
        public int accountId;
        public int roomNumber;
        public string ip;
        public int port;
        public	PK_S_NOTIFY_SERVER()
        {
            type = (int)PacketType.E_S_NOTIFY_SERVER;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, accountId);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, ip);
            PacketUtil.encode(stream, port);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            accountId = PacketUtil.decodeInt32(packet, ref offset);
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            ip = PacketUtil.decodestring(packet, ref offset);
            port = PacketUtil.decodeInt32(packet, ref offset);
        }
   


        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };


    //E_C_REQ_CONNECT = 105,

    public class PK_C_REQ_CONNECT : Packet, Packetinterface
    {

        public int accountId;
        public string name;
        public PK_C_REQ_CONNECT()
        {
            type = (int)PacketType.E_C_REQ_CONNECT;
        }


        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, accountId);
            PacketUtil.encode(stream, name);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            accountId = PacketUtil.decodeInt32(packet, ref offset);
            name = PacketUtil.decodestring(packet, ref offset);
        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };


    //E_S_ANS_CONNECT = 106,
    public class PK_S_ANS_CONNECT : Packet, Packetinterface
    {
    
        public PK_S_ANS_CONNECT()
        {
            type = (int)PacketType.E_S_ANS_CONNECT;
        }


        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };
    //E_C_REQ_CONNECT_ROOM = 107,
    public class PK_C_REQ_CONNECT_ROOM : Packet, Packetinterface
    {
        public int accountId;       
    	public int roomNumber;
        
        public PK_C_REQ_CONNECT_ROOM()
        {
            type = (int)PacketType.E_C_REQ_CONNECT_ROOM;
        }



        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, accountId);
            PacketUtil.encode(stream, roomNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            accountId = PacketUtil.decodeInt32(packet, ref offset);
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    //E_S_ANS_CONNECT_ROOM = 108,

    public class PK_S_ANS_CONNECT_ROOM : Packet, Packetinterface
    {
        public int roomNumber;      
        public int userNumber;
        public PK_S_ANS_CONNECT_ROOM()
        {
            type = (int)PacketType.E_S_ANS_CONNECT_ROOM;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    //E_S_NOTIFY_OTHER_CLIENT = 109,

    public class PK_S_NOTIFY_OTHER_CLIENT : Packet, Packetinterface
    {
    	public int userNumber;
        public float pos_X;
        public float pos_Y;
        public bool isDummy;
	    public PK_S_NOTIFY_OTHER_CLIENT()
        {
            type = (int)PacketType.E_S_NOTIFY_OTHER_CLIENT;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, pos_X);
            PacketUtil.encode(stream, pos_Y);
            PacketUtil.encode(stream, isDummy);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            pos_X = PacketUtil.decodefloat(packet, ref offset);
            pos_Y = PacketUtil.decodefloat(packet, ref offset);
            isDummy = PacketUtil.decodeBool(packet, ref offset);
        }

        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };
    //E_C_REQ_MOVE = 112,

    public class PK_C_REQ_MOVE : Packet, Packetinterface
    {
    	public int roomNumber;
        public int userNumber;
        public int direction;
        public float pos_X;
        public float pos_Y;
        
        public PK_C_REQ_MOVE()
        {
            type = (int)PacketType.E_C_REQ_MOVE;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, direction);
            PacketUtil.encode(stream, pos_X);
            PacketUtil.encode(stream, pos_Y);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            direction = PacketUtil.decodeInt32(packet, ref offset);
            pos_X = PacketUtil.decodefloat(packet, ref offset);
            pos_Y = PacketUtil.decodefloat(packet, ref offset);
        }

       
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };
    //E_S_ANS_MOVE = 113,


    public class PK_S_ANS_MOVE : Packet, Packetinterface
    {
     
	    public int userNumber;
        public int direction;
        public float pos_X;
        public float pos_Y;
        public PK_S_ANS_MOVE()
        {
            type = (int)PacketType.E_S_ANS_MOVE;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, direction);
            PacketUtil.encode(stream, pos_X);
            PacketUtil.encode(stream, pos_Y);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            direction = PacketUtil.decodeInt32(packet, ref offset);
            pos_X = PacketUtil.decodefloat(packet, ref offset);
            pos_Y = PacketUtil.decodefloat(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    public class PK_C_NOTIFY_CONNECTED_ROOM : Packet, Packetinterface
    {

        public int roomNumber;
        public int userNumber;

        public PK_C_NOTIFY_CONNECTED_ROOM()
        {
            type = (int)PacketType.E_C_NOTIFY_CONNECTED_ROOM;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {

            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    }


    public class PK_S_NOTIFY_USER_DATA : Packet, Packetinterface
    {

        public int userNumber;
        public float pos_X;
        public float pos_Y;

        public PK_S_NOTIFY_USER_DATA()
        {
            type = (int)PacketType.E_S_NOTIFY_USER_DATA;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, pos_X);
            PacketUtil.encode(stream, pos_Y);
        }

        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            pos_X = PacketUtil.decodefloat(packet, ref offset);
            pos_Y = PacketUtil.decodefloat(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    }


    public class PK_C_REQ_SHOT : Packet, Packetinterface
    {
        public int roomNumber;
        public int userNumber;
        public int direction;
        public int targetNumber;

        public PK_C_REQ_SHOT()
        {
            type = (int)PacketType.E_C_REQ_SHOT;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, direction);
            PacketUtil.encode(stream, targetNumber);
        }

        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            direction = PacketUtil.decodeInt32(packet, ref offset);
            targetNumber = PacketUtil.decodeInt32(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };

    public class PK_S_ANS_SHOT : Packet, Packetinterface
    {

        public int userNumber;
        public int direction;
        public int targetNumber;

        public PK_S_ANS_SHOT()
        {
            type = (int)PacketType.E_S_ANS_SHOT;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, direction);

            PacketUtil.encode(stream, targetNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            direction = PacketUtil.decodeInt32(packet, ref offset);
            targetNumber = PacketUtil.decodeInt32(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    };





    public class PK_C_REQ_EXIT_ROOM : Packet, Packetinterface
    {

        public int roomNumber;
        public int userNumber;

        public PK_C_REQ_EXIT_ROOM()
        {
            type = (int)PacketType.E_C_REQ_EXIT_ROOM;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {

            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    }

    public class PK_S_ANS_EXIT_ROOM : Packet, Packetinterface
    {
        
        public int userNumber;

        public PK_S_ANS_EXIT_ROOM()
        {
            type = (int)PacketType.E_S_ANS_EXIT_ROOM;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, userNumber);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {

            userNumber = PacketUtil.decodeInt32(packet, ref offset);

        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    }

    public class PK_C_REQ_CROSS_OVER_SERVER : Packet, Packetinterface
    {

        public int roomNumber;
        public int userNumber;
        public int direction;
        public PK_C_REQ_CROSS_OVER_SERVER()
        {
            type = (int)PacketType.E_C_REQ_CROSS_OVER_SERVER;
        }

        void Packetinterface.encode()
        {
            PacketUtil.encodeHeader(stream, type);
            PacketUtil.encode(stream, roomNumber);
            PacketUtil.encode(stream, userNumber);
            PacketUtil.encode(stream, direction);
        }
        void Packetinterface.decode(byte[] packet, ref int offset)
        {

            roomNumber = PacketUtil.decodeInt32(packet, ref offset);
            userNumber = PacketUtil.decodeInt32(packet, ref offset);
            direction = PacketUtil.decodeInt32(packet, ref offset);
        }
        MemoryStream Packetinterface.getStream()
        {
            return stream;
        }


        int Packetinterface.getType()
        {
            return type;
        }
    }
}
