using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using UnityEngine;
using System.Threading;


namespace DummyClient
{

    class LogInContentsProcess : ContentsProcess
    {
        LoginPacketProcess LoginPacketProcess = null;
        

        private void Start()
        {
           
            LoginPacketProcess = new LoginPacketProcess();
            StartCoroutine(Run());
            DontDestroyOnLoad(this);
        }
        /*
        public LogInContentsProcess()
        {
          //  LoginPacketProcess = new LoginPacketProcess();
            
        }
        */
        public void JobStart()
        {
            //StartCoroutine(Run());
        }

        // 로그인 과 채팅은 상관없는데 게임은함수포인터맵으로 바꾸는게 좋을듯함

        IEnumerator Run()
        {
            Packetinterface packet;

            while (true)
            {

                yield return null;
                if (LogInNetwork.getInstance.network.queue.Count <= 0)
                {
                    LogInNetwork.getInstance.network.Swap();
                    continue;
                }
                packet = LogInNetwork.getInstance.network.queue.Dequeue();
                if (packet == null) continue;
                
                parse(packet);

                
            }
        }

        public override void parse(Packetinterface packet)
        {
            PacketType type = (PacketType)packet.getType();

            Debug.Log("받은 로그인 패킷" + type);

    
                switch (type)
                {
                    case PacketType.E_S_ANS_SIGNIN:
                        LoginPacketProcess.S_ANS_SIGNIN(packet);
                        break;
                    case PacketType.E_S_ANS_SIGNUP:
                        LoginPacketProcess.S_ANS_SIGNUP(packet);
                        break;
                    case PacketType.E_S_NOTIFY_SERVER:
                        LoginPacketProcess.S_NOTIFY_SERVER(packet);
                        break;
                default:
                    Debug.Log("잘못된 패킷이 수신되었습니다. : ");
                    Debug.Log(type);
                    break;
                }
            /*
                if (base.defaultRun(packet) == false)
                {
#if DEBUG
                    Debug.Log("잘못된 패킷이 수신되었습니다. : ");
                    Debug.Log(type);
#endif
                }
                */
        }



    }
}
