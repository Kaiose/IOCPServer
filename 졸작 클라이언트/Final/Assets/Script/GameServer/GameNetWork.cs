using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;

namespace DummyClient
{

    public class GameNetWork : MonoBehaviour
    {
        static protected GameNetWork gameNetWork;
        static public GameNetWork getInstance { get { return gameNetWork; } }

        public Network network;

        public string name_;

        public int serverNumber;

        //public GameContentsProcess gameContentsProcess;
        private void Awake()
        {
            gameNetWork = this;
            network = new Network();
        }
        private void Start()
        {
            // gameContentsProcess = new GameContentsProcess();
            serverNumber = 1;
            DontDestroyOnLoad(this);
     
        }

        public void sendPacket(Packetinterface packet)
        {
            network.sendPacket(packet);
        }

        private void OnDestroy()
        {
            disconnect();
        }
        
        public bool ConnectToServer(string ip, int port)
        {
            if (port == 9900)
                serverNumber = 1;
            else if (port == 9800)
                serverNumber = 2;
            else if (port == 9700)
                serverNumber = 3;
            return network.Connect(ip, port);
        }


        public bool ReConnectToServer(string ip, int port)
        {
            if (port == 9900)
                serverNumber = 1;
            else if (port == 9800)
                serverNumber = 2;
            else if (port == 9700)
                serverNumber = 3;
            return network.Reconnect(ip, port);
        }


        public void Open(string ip, int port)
        {
            if (!ConnectToServer(ip, port))
            {
                Debug.Log("Game Server Connect Failed");
            }
            network.receiveStart();
            // network.ReceiveStart();
            // gameContentsProcess.JobStart();

            //StartCoroutine(Recv());

        }


        public void Reopen(string ip, int port)
        {
            if (!ReConnectToServer(ip, port))
            {
                Debug.Log("Game Server Connect Failed");
            }
            network.receiveStart();
            // network.ReceiveStart();
            // gameContentsProcess.JobStart();

            //StartCoroutine(Recv());

        }


        public NET_STATE GetState()
        {
            return network.State();
        }

        public void disconnect()
        {
            Debug.Log("Gamenetwork PK_C_REQ_EXIT\n");

            PK_C_REQ_EXIT exitPacket = new PK_C_REQ_EXIT();
            exitPacket.accountId = GameManager.getInstance.accountId;
            exitPacket.roomNumber = GameManager.getInstance.RoomNumber;
            exitPacket.userNumber = GameManager.getInstance.MainCharacterNumber;

            network.sendPacket(exitPacket);

            network.close();
        }

     
        IEnumerator Recv()
        {
            while (true)
            {
               // network.receive();
                yield return null;
            }
        }



    }
}
