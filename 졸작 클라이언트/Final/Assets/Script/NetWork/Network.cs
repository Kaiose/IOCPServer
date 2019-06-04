using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.IO;
using UnityEngine;
using System.Collections;
using System.Threading;
using System.Net;
using System.Runtime.InteropServices;


namespace DummyClient
{
    

    public enum NET_STATE
    {
        START,
        CONNECTED,
        DISCONNECT,
        DISCONNECTED,
    }


    public class Network
    {
        [DllImport("ws2_32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern Int32 WSAGetLastError();


        public Queue<Packetinterface> PacketQueue = null;
        public Queue<Packetinterface> readPacketQueue;
        public Queue<Packetinterface> writePacketQueue;
        public Queue<Packetinterface> queue { get { return PacketQueue; } }
        
        //   private NetworkStream stream_;
        //  private TcpClient client_;

        private Socket socket;

    
        private static Byte[] packetBytes;
        private NET_STATE state_;

        private SocketAsyncEventArgs sendArgs;
        private SocketAsyncEventArgs receiveArgs;



        static Int32 totalBytes;

        //   private ContentsProcess ContentsProcee_;

        public class Data
        {
            public Socket socket;
            public const int BuffSize = 1024 * 10;
            public Byte[] buff = new Byte[BuffSize];

        }
        
        public void Swap()
        {
                if (PacketQueue == readPacketQueue)
                {
                        PacketQueue = writePacketQueue;
                    
                }
                else
                {
                        PacketQueue = readPacketQueue;
                    
                }
            
        }

        public Network(){

            // 패킷 큐 생성
            

            readPacketQueue = new Queue<Packetinterface>();
            writePacketQueue = new Queue<Packetinterface>();

            PacketQueue = readPacketQueue;

            //서버의 상태 초기화
            state_ = NET_STATE.START;

            // lock 오브젝트 생성


            //소켓생성
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);


            //// arg 할당
            //sendArgs = new SocketAsyncEventArgs();
            //receiveArgs = new SocketAsyncEventArgs();
     
            //// args 에 대한 함수 등록
            //sendArgs.Completed += new EventHandler<SocketAsyncEventArgs>(sendArgs_func);
            //receiveArgs.Completed += new EventHandler<SocketAsyncEventArgs>(receiveArgs_func);


            // TODO : arg 가 가지고 있을 object 할당

            // args 버퍼 설정

         //   sendArgs.SetBuffer(0, 1024 * 10);
         //   receiveArgs.SetBuffer(0, 1024 * 10);

            // 수신 바이트를 관리할 버퍼


            packetBytes = new Byte[1024 * 10];

            // 수신바이트의 총바이트수와 CurrentOffset 초기화
            totalBytes = 0;
            

            //
        }

      

        public void receiveStart()
        {
            Data data = new Data();
            data.socket = socket;
            
            socket.BeginReceive(data.buff, 0, Data.BuffSize, 0, new AsyncCallback(receive_CallBack), data);

            

        }

        private void receive_CallBack(IAsyncResult ar)
        {
            Data data = (Data)ar.AsyncState;
            Socket localSocket = data.socket;
            Byte[] packetHeader = new Byte[sizeof(Int32)];
            Byte[] packetBody = new Byte[1024]; // 임의 크기로 해놈

            

            Int32 readLen = localSocket.EndReceive(ar);

            if (readLen == 0)
            {
                Debug.Log("Zero Receive Error");
                return;
            }

            //들어온 크기만 큼 packetByte에 복사한다.
            Buffer.BlockCopy(data.buff, 0, packetBytes, totalBytes, readLen);
            totalBytes += readLen;

            
          //  Debug.Log("Receive Size : " + readLen.ToString() + "TotalBytes Size : " + totalBytes.ToString());
            while (totalBytes > 0)
            {
               // Array.Clear(packetHeader, 0, sizeof(Int32));
              //  Array.Clear(packetBody, 0, sizeof(Byte) * 1024);
                Int32 currentOffset = 0;
                Int32 offset = 0;
                Buffer.BlockCopy(packetBytes, currentOffset, packetHeader, 0, sizeof(Int32)); // 
                
                //packet 버퍼에 있는 크기가 헤더 이하이면
                if (totalBytes < BitConverter.ToInt32(packetHeader, 0))
                {

                    Debug.Log("totalBytes is not enough ");
                    break;
                    //localSocket.BeginReceive(data.buff, 0, Data.BuffSize, 0, new AsyncCallback(receive_CallBack), data);

                    //return;
                }
                //packet 버퍼에 있는 크기가 헤더 이상이라면

                //packetHeader 에 packetheader 만큼 복사. 
                

               // Buffer.BlockCopy(packetBytes, currentOffset, packetHeader, 0, sizeof(Int32)); // 
                currentOffset += sizeof(Int32); // packetByte의 offset 변경




                // 패킷 총 길이를 가져옴. + offset 증가
                Int32 packetLen = PacketUtil.decodePacketLen(packetHeader, ref offset);
                packetLen -= offset;
                int remainBytes = totalBytes - currentOffset; // packetData에서 헤더를 제외한 남은 바이트
                if (remainBytes < packetLen)
                {

                    Debug.Log("remainBytes is Not enough");

                    break;
                    //localSocket.BeginReceive(data.buff, 0, Data.BuffSize, 0, new AsyncCallback(receive_CallBack), data);
                    /*
                     * 
                     * 
                     */
                    //return;
                }

                // currentOffset 은 패킷 헤더 이후의 offset
                Buffer.BlockCopy(packetBytes, currentOffset, packetBody, 0, packetLen);
                currentOffset += packetLen;

                Packetinterface rowPacket = PacketUtil.packetAnalyzer(packetBody, packetLen);
              //  Debug.Log("Receive Packet : " + rowPacket.getType().ToString());
                if (rowPacket == null && localSocket.Connected)
                {
                    Debug.Log("잘못된 패킷이 수신되었습니다.");
                }
                // 큐 구분 해야됨.

                if (PacketQueue == readPacketQueue)
                {
                    lock (writePacketQueue)
                    {
                        writePacketQueue.Enqueue(rowPacket);
                    }
                }
                else
                {
                    lock (readPacketQueue)
                    {
                        readPacketQueue.Enqueue(rowPacket);
                    }
                }
                //writePacketQueue

                //Enqueue(rowPacket);

                //packetBytes 를 사용 했으므로 data 및 offset 수정

               // Buffer.BlockCopy(packetBytes, 0, packetBytes, currentOffset, totalBytes - currentOffset);

                Buffer.BlockCopy(packetBytes, currentOffset, packetBytes, 0,totalBytes - currentOffset);

                totalBytes -= currentOffset;


                /*
                 *  생각할 수 있는 것은 Client에서의 Enqueue 와 Dequeue 사이에서 Lock으로 인한 병목현상이다.
                 *  클라이언트의 Network 는 하나의 스레드가 온전히 recv 하고 enqueue 를 실행하지만
                 *  클라이언트의 Unity Script 에서는 하나의 스레드가 실행 시간을 나누어 dequeue를 실행한다
                 *  
                 *  
                 *  11/20 
                 *  
                 *  PacketQueue 에서 Dequeue 하는 부분의 Corutine의 갯수를 증가시켰을때
                 *  전 보다 싱크 의 차이가 줄어드는 것을 확인할 수 있었음.
                 *  즉 병목현상이 맞다고 판단 -> Server와 동일하게 두 개의 버퍼를 만들어 스왚하는 로직을 통해 해소해보자.
                 */
            }
            Array.Clear(data.buff, 0, readLen);
            localSocket.BeginReceive(data.buff, 0, Data.BuffSize, 0, new AsyncCallback(receive_CallBack), data);

        }

    
        private void sendArgs_func(object session, SocketAsyncEventArgs e)
        {
            if(e.LastOperation == SocketAsyncOperation.Send)
            {
                send_CallBack(e);
                return;
            }

            Debug.Log("The Last Operation completed on the socket was not a send");
        }

        private void send_CallBack(SocketAsyncEventArgs e)
        {
            
        }



        public void close()
        {
            state_ = NET_STATE.DISCONNECTED;
            socket.Close(10);
            
        }
        
        public bool Connect(string ip, int port)
        {

            Debug.Log("Reqest Connect ip : " + ip + "port :" + port.ToString());
            var endPoint = new IPEndPoint(IPAddress.Parse(ip), port);
           
            try
            {
                socket.Connect(endPoint);
            }
            catch
            {

                Debug.Log("Connect Failed Error Code : %d" + Marshal.GetLastWin32Error().ToString());
                return false;
            }
            state_ = NET_STATE.CONNECTED;
            
            
            return true;
        }

        
        public bool Reconnect(string ip, int port)
        {


            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            Debug.Log("Reqest ReConnect ip : " + ip + "port :" + port.ToString());
            
            var endPoint = new IPEndPoint(IPAddress.Parse(ip), port);

            

            try
            {
                socket.Connect(endPoint);
            }
            catch
            { 
                Debug.Log("Connect Failed Error Code : %d" + Marshal.GetLastWin32Error().ToString());
                return false;
            }
            state_ = NET_STATE.CONNECTED;


            return true;
        }



        public void ReceiveStart()
        {
            for (int i = 0; i < 5; i++)
            {
     //           new Thread(new ThreadStart(receive)).Start();
            }
        }

        public NET_STATE State()
        {
            return state_;
        }

        public void setContentsProcess(ContentsProcess contentsProcess)
        {
       //     ContentsProcee_ = contentsProcess;
        }

        private bool isConnected()
        {
            return state_ == NET_STATE.CONNECTED ? true : false;
        }

        public void Enqueue(Packetinterface rowPacket)
        {
            lock (PacketQueue)
            {
                PacketQueue.Enqueue(rowPacket);
            }
        }

        public Packetinterface Dequeue()
        {
            lock (PacketQueue)
            {
                Debug.Log("Deque Execute ..");
                  
                return PacketQueue.Dequeue();
            }
        }
        public void sendPacket(Packetinterface packet)
        {
            Debug.Log("Send Packet : " + packet.GetType().ToString());
            try
            {
                
                packet.encode();
                //Header -> data 순으로 바이트변환후 packet내 외부스트림에 저장
                MemoryStream packetBlock = new MemoryStream();
                Int32 packetLen = sizeof(Int32) + (Int32)packet.getStream().Length;
               
                Byte[] packetHeader = BitConverter.GetBytes(packetLen);
                //packetHeader는 Packet의 총길이
               packetBlock.Write(packetHeader, 0, (Int32)packetHeader.Length);


                Byte[] packetData = packet.getStream().ToArray();

                packetBlock.Write(packetData, 0, (Int32)packetData.Length);

                Byte[] packetBytes = packetBlock.ToArray();
                // 블럭을 바이트로 변환
                
                Debug.Log("Send Size : " + packetBytes.Length.ToString());
                socket.Send(packetBytes);
//                stream_.Write(packetBytes,0, (int)packetBlock.Length);

             //   stream_.Flush();
                packetBlock = null;

            }catch(Exception e)
            {
                if (this.isConnected())
                {
                    Debug.Log("잘못된 처리 : send " + e.ToString());
                }
            }
            
        }
    }


}
