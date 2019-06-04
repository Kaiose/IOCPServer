using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
namespace DummyClient
{

    public class LoginPacketProcess
    {
   
        public void S_ANS_ID_PW_FAIL(Packetinterface rowPacket)
        {
            Debug.Log("로그인 실패");
        }

        
        public void S_ANS_SIGNUP(Packetinterface rowPacket)
        {
            PK_S_ANS_SIGNUP packet = (PK_S_ANS_SIGNUP)rowPacket;
            if (packet.result == false)
            {
                /*
                PK_C_REQ_EXIT ansPacket = new PK_C_REQ_EXIT();
                ansPacket.accountId = GameManager.getInstance.accountId;
                LogInNetwork.getInstance.sendPacket(ansPacket);
                */
            }
        }

        public void S_ANS_SIGNIN(Packetinterface rowPacket)
        {
            PK_S_ANS_SIGNIN packet = (PK_S_ANS_SIGNIN)rowPacket;
            if(packet.result == false)
            {
                /*
                PK_C_REQ_EXIT ansPacket = new PK_C_REQ_EXIT();
                ansPacket.accountId = GameManager.getInstance.accountId;
                LogInNetwork.getInstance.sendPacket(ansPacket);
               */        
            }

            
        }

        public void S_NOTIFY_SERVER(Packetinterface rowPacket) {
            PK_S_NOTIFY_SERVER packet = (PK_S_NOTIFY_SERVER)rowPacket;


            GameManager.getInstance.accountId = packet.accountId;
            GameNetWork.getInstance.Open(packet.ip, packet.port);


            PK_C_REQ_CONNECT ansPacket = new PK_C_REQ_CONNECT();
            ansPacket.accountId = packet.accountId;
            ansPacket.name = GameManager.getInstance.username;
            GameNetWork.getInstance.sendPacket(ansPacket);

        }

        

        public void S_ANS_EXIT(Packetinterface rowPacket)
        {
            
        }

    }
}
