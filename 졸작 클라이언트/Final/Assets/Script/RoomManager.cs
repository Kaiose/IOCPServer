using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
public class RoomManager : MonoBehaviour {

    public Int32 RoomNumber;
    public int UserSize;

    public Text RoomNumberText;

	// Use this for initialization
	void Start () {
        RoomNumber = GameManager.getInstance.RoomNumber;
	}
	
	// Update is called once per frame
	void Update () {



	}
}
