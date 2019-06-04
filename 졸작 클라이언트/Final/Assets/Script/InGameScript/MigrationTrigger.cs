using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using DummyClient;
public class MigrationTrigger : MonoBehaviour {

    public int Direction;
    public int room;

	// Use this for initialization
	void Start () {
        room = GameManager.getInstance.RoomNumber;
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    private void OnTriggerEnter2D(Collider2D collision)
    {
        Debug.Log("Enter zone");

        PK_C_REQ_CROSS_OVER_SERVER packet = new PK_C_REQ_CROSS_OVER_SERVER();
        packet.direction = Direction;
        packet.roomNumber = GameManager.getInstance.RoomNumber;
        packet.userNumber = GameManager.getInstance.MainCharacterNumber;

        GameNetWork.getInstance.sendPacket(packet);
    }
}
