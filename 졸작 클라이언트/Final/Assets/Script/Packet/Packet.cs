using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.IO;

namespace DummyClient
{
    public interface Packetinterface
    {
        void encode();
        void decode(byte[] packet, ref int offset);

        int getType();
        MemoryStream getStream();
    }




    public class Packet
    {
       
        protected MemoryStream stream = new MemoryStream();

        public int type;
       

        ~Packet()
        {
            stream = null;
        }
    }
}
