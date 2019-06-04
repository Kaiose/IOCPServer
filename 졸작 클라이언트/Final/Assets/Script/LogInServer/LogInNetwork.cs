using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;
namespace DummyClient
{
    public class LogInNetwork : MonoBehaviour
    {
        static protected LogInNetwork logInNetwork;
        static public LogInNetwork getInstance { get { return logInNetwork; } }

        public Network network;

        //        private string loginIp_ = "192.168.10.7";//"127.0.0.1"; 192.168.10.15
        private string loginIp_ = "127.0.0.1";
        private int loginPort_ = 9100;


        //public LogInContentsProcess logInContentsProcess;

        private void Awake()
        {
            logInNetwork = this;
            network = new Network();
        }
        
        // Use this for initialization
        void Start()
        {
            //logInContentsProcess = new LogInContentsProcess();
            
            Open();
            //StartCoroutine(Recv());
            DontDestroyOnLoad(this);
        }

        private void OnDestroy()
        {
            disconnect();
        }

        public bool ConnectToServer(string ip, int port)
        {
   
            return network.Connect(ip, port);


        }

        // Update is called once per frame
        void Update()
        {
            //    if (//GameObject.Find("ChattingNetWork").GetComponent<ChattingNetWork>().GetState() == NET_STATE.CONNECTED &&
            //       GameObject.Find("GameNetWork").GetComponent<GameNetWork>().GetState() == NET_STATE.CONNECTED)
            //        Destroy(this);
            //

        }

        public void sendPacket(Packetinterface packet)
        {
            Debug.Log("Login : Send! " + packet.ToString());
            network.sendPacket(packet);
        }

        public void Open()
        {
            if (!ConnectToServer(loginIp_, loginPort_))
            {
                Debug.Log("로그인 서버 연결 실패");
                return;
            }

            network.receiveStart();
            //logInContentsProcess.JobStart();

            //network_.setContentsProcess(new LogInContentsProcess());
        }

        public void disconnect()
        {
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
        
           //     network.receive();

                yield return null;
            }
        }
    }
}