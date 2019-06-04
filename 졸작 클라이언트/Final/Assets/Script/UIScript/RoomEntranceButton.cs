using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using DummyClient;
using UnityEngine.UI;
using System;
public class RoomEntranceButton : MonoBehaviour {

    // Use this for initialization

    public int RoomNumber;

    public Text text;

	void Start () {
        text.text = "Room " + RoomNumber.ToString();
	}
	
	// Update is called once per frame
	void Update () {
		
	}


    public void Click()
    {

        PK_C_REQ_CONNECT_ROOM packet = new PK_C_REQ_CONNECT_ROOM();
        GameManager.getInstance.RoomNumber = RoomNumber;
        Debug.Log(GameManager.getInstance.RoomNumber);
        packet.roomNumber = GameManager.getInstance.RoomNumber;
        packet.accountId = GameManager.getInstance.accountId;
        

        // SceneChange.getInstance.ChangeScene("GameScene");

        GameNetWork.getInstance.sendPacket(packet);

    }
}
