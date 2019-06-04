using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System;
using UnityEngine;

namespace DummyClient
{
    

    public class GameContentsProcess : ContentsProcess
    {
        GamePacketProcess gamePacketProcess = null;


        private void Start()
        {

            gamePacketProcess = new GamePacketProcess();

            for(int i = 0; i < 5; i++)
                StartCoroutine(Run());


            DontDestroyOnLoad(this);

        }

    
                

        IEnumerator Run()
        {
            Packetinterface packet;

            while (true)
            {
                yield return null;
                if (GameNetWork.getInstance.network.queue.Count <= 0)
                {
                    GameNetWork.getInstance.network.Swap();
                    continue;
                }
                packet = GameNetWork.getInstance.network.queue.Dequeue();
                if (packet == null)
                {
                    Debug.Log("Deque Packet is null");
                    continue;
                }
                parse(packet);
               

            }
        }

        public  override void parse(Packetinterface packet)
        {
        
                PacketType type = (PacketType)packet.getType();

           //     Debug.Log("받은 게임 패킷 : " + type);

                switch (type)
                {
                    case PacketType.E_S_NOTIFY_SERVER:
                    gamePacketProcess.S_NOTIFY_SERVER(packet);
                        return;
                    case PacketType.E_S_ANS_EXIT:
                        gamePacketProcess.S_ANS_EXIT(packet);
                        return;
                    case PacketType.E_S_ANS_CONNECT:
                        gamePacketProcess.S_ANS_CONNECT(packet);
                        return;
                    case PacketType.E_S_ANS_CONNECT_ROOM:
                        gamePacketProcess.S_ANS_CONNECT_ROOM(packet);
                        return;
                    case PacketType.E_S_NOTIFY_OTHER_CLIENT:
                        gamePacketProcess.S_NOTIFY_CLIENT_DATA(packet);
                        return;
                    case PacketType.E_S_ANS_MOVE:
                        gamePacketProcess.S_ANS_MOVE(packet);
                        return;
                    case PacketType.E_S_NOTIFY_USER_DATA:
                        gamePacketProcess.S_NOTIFY_USER_DATA(packet);
                        return;
                    case PacketType.E_S_ANS_CHAT:
                        gamePacketProcess.S_ANS_CHAT(packet);
                        return;
                    case PacketType.E_S_ANS_SHOT:
                        gamePacketProcess.S_ANS_SHOT(packet);
                        return;
                case PacketType.E_S_ANS_EXIT_ROOM:
                    gamePacketProcess.S_ANS_EXIT_ROOM(packet);
                    return;
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

    

