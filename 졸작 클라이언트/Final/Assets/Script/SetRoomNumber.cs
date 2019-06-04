using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using DummyClient;
public class SetRoomNumber : MonoBehaviour {



	// Use this for initialization
	void Start () {
        string RoomNum = GameManager.getInstance.RoomNumber.ToString();
        string ServerNum = GameNetWork.getInstance.serverNumber.ToString();
        gameObject.GetComponent<Text>().text = "Room : " + RoomNum + " Server : " + ServerNum;
        if(ServerNum == "1")
        {
            gameObject.GetComponent<Text>().color = new Color(255, 0, 0);
        }
        else if (ServerNum == "2")
        {

            gameObject.GetComponent<Text>().color = new Color(0, 255, 0);
        }
        else if (ServerNum == "3")
        {

            gameObject.GetComponent<Text>().color = new Color(0, 0, 255);
        }
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
