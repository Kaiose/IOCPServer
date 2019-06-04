using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DummyClient;
public class ClickToBackButton : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}


    public void Back()
    {
        PK_C_REQ_EXIT_ROOM packet = new PK_C_REQ_EXIT_ROOM();
        packet.roomNumber = GameManager.getInstance.RoomNumber;
        packet.userNumber = GameManager.getInstance.MainCharacterNumber;

        GameNetWork.getInstance.sendPacket(packet);

        GameManager.getInstance.RoomNumber = -1;
        GameManager.getInstance.MainCharacterNumber = -1;

        SceneChange.getInstance.ChangeScene("RoomScene");
    }
}
