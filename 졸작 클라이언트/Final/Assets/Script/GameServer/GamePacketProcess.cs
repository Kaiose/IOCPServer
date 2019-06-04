using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;

namespace DummyClient
{


    public class GamePacketProcess
    {
        
        public  void S_ANS_EXIT(Packetinterface rowPacket)
        {

            PK_S_ANS_EXIT packet = (PK_S_ANS_EXIT)rowPacket;
            GameManager.getInstance.DeleteCharacter(packet.userNumber);

        }

        public void S_ANS_CONNECT(Packetinterface rowPacket)
        {
            //  PK_S_ANS_CONNECT packet = (PK_S_ANS_CONNECT)rowPacket;
            if (SceneChange.getInstance.GetSceneNumber() == 0)
            {
                Debug.Log(SceneChange.getInstance.GetSceneNumber().ToString());


                LogInNetwork.getInstance.disconnect();
                SceneChange.getInstance.ChangeScene("RoomScene");
            }
            else
            {
                PK_C_REQ_CONNECT_ROOM packet = new PK_C_REQ_CONNECT_ROOM();
                Debug.Log("ReConnect : RoomNUmber : " + GameManager.getInstance.RoomNumber.ToString());
                packet.roomNumber = GameManager.getInstance.RoomNumber;
                packet.accountId = GameManager.getInstance.accountId;
                GameNetWork.getInstance.sendPacket(packet);
            }
        }

        public void S_NOTIFY_CLIENT_DATA(Packetinterface rowPacket)
        {
            PK_S_NOTIFY_OTHER_CLIENT packet = (PK_S_NOTIFY_OTHER_CLIENT)rowPacket;
            Debug.Log("Recive S_NOTIFY_OTHER_CLIENT EnemyNumber : " + packet.userNumber.ToString());
            GameManager.getInstance.CreateEnemyCharacter(packet.userNumber, packet.isDummy);
            GameManager.getInstance.SetCharacterPostition(packet.userNumber, packet.pos_X, packet.pos_Y);

        }

        public void S_NOTIFY_USER_DATA(Packetinterface rowPacket)
        {
            PK_S_NOTIFY_USER_DATA packet = (PK_S_NOTIFY_USER_DATA)rowPacket;
            Debug.Log("Recive S_NOTIFY_USER_DATA");
            packet.userNumber = GameManager.getInstance.MainCharacterNumber;

            GameManager.getInstance.CreateMainCharacter(packet.userNumber);

            GameManager.getInstance.SetCharacterPostition(packet.userNumber, packet.pos_X, packet.pos_Y);
            
        }
        public void S_ANS_CONNECT_ROOM(Packetinterface rowPacket)
        {


            PK_S_ANS_CONNECT_ROOM packet = (PK_S_ANS_CONNECT_ROOM)rowPacket;


            SceneChange.getInstance.ChangeScene("GameScene");
      
            GameManager.getInstance.MainCharacterNumber = packet.userNumber;
            GameManager.getInstance.RoomNumber = packet.roomNumber;
            Debug.Log("S_ANS_CONNECT_ROOM : " + packet.userNumber.ToString());
            PK_C_NOTIFY_CONNECTED_ROOM ansPacket = new PK_C_NOTIFY_CONNECTED_ROOM();
            ansPacket.roomNumber = GameManager.getInstance.RoomNumber;
            ansPacket.userNumber = GameManager.getInstance.MainCharacterNumber;
            GameNetWork.getInstance.sendPacket(ansPacket);


        
        }
        

        public void S_ANS_MOVE(Packetinterface rowPacket)
        {

            PK_S_ANS_MOVE packet = (PK_S_ANS_MOVE)rowPacket;
            //GameManager.getInstance.MoveCharacter(packet.userNumber, (GameManager.Direction)packet.direction);
            GameManager.getInstance.MoveCharacter(packet.userNumber, (GameManager.Direction)packet.direction, packet.pos_X, packet.pos_Y);

        }

        public void S_ANS_CHAT(Packetinterface rowPacket)
        {
            PK_S_ANS_CHAT packet = (PK_S_ANS_CHAT)rowPacket;

            //int userNumber = packet.userNumber;
            string name = packet.name;
            string message = packet.chat;

            GameManager.getInstance.chatting.ReceiveMessage(name, message);

        }
        
        public void S_ANS_SHOT(Packetinterface rowPacket)
        {
            PK_S_ANS_SHOT packet = (PK_S_ANS_SHOT)rowPacket;

            int result = GameManager.getInstance.ShotCharacter(packet.userNumber, (GameManager.Direction)packet.direction, packet.targetNumber);
            if(result == 0)
            {
               
                PK_C_REQ_EXIT_ROOM ansPacket = new PK_C_REQ_EXIT_ROOM();
                ansPacket.roomNumber = GameManager.getInstance.RoomNumber;
                ansPacket.userNumber = packet.targetNumber;

                Thread.Sleep(1000);
                GameNetWork.getInstance.sendPacket(ansPacket);

                
                GameManager.getInstance.RoomNumber = -1;
                GameManager.getInstance.MainCharacterNumber = -1;
                SceneChange.getInstance.ChangeScene("RoomScene");
            }


        }

        public void S_ANS_EXIT_ROOM(Packetinterface rowPacket)
        {
            PK_S_ANS_EXIT_ROOM packet = (PK_S_ANS_EXIT_ROOM)rowPacket;

            GameManager.getInstance.DeleteCharacter(packet.userNumber);


        }


        public void S_NOTIFY_SERVER(Packetinterface rowPacket)
        {
            PK_S_NOTIFY_SERVER packet = (PK_S_NOTIFY_SERVER)rowPacket;

            GameNetWork.getInstance.disconnect();
            if(GameManager.getInstance.accountId != -1)
                GameManager.getInstance.accountId = packet.accountId;

            GameNetWork.getInstance.Reopen(packet.ip, packet.port);
            GameManager.getInstance.RoomNumber = packet.roomNumber;
    
            Debug.Log("receive S_NOTIFY_SERVER , accountId : " + GameManager.getInstance.accountId +" set roomNumber : " + GameManager.getInstance.RoomNumber + " packet roomNumber : " + packet.roomNumber);
            PK_C_REQ_CONNECT ansPacket = new PK_C_REQ_CONNECT();
            ansPacket.accountId = GameManager.getInstance.accountId;
            ansPacket.name = GameManager.getInstance.username;
            GameNetWork.getInstance.sendPacket(ansPacket);

        }




    }
}
