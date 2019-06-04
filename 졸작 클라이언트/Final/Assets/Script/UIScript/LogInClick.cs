using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DummyClient;
using UnityEngine.UI;

public class LogInClick : MonoBehaviour {


    public InputField ID;
    public InputField PW;
	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (Input.GetKeyDown(KeyCode.Return))
        {
            Click();
        }
	}


    public void Click()
    {
        PK_C_REQ_SIGNIN packet = new PK_C_REQ_SIGNIN();
        packet.name = ID.text;
        packet.password = PW.text;
        
       // packet.name = "kinam";
        //packet.password = "1234";
        
        LogInNetwork.getInstance.sendPacket(packet);

        GameManager.getInstance.username = packet.name;
    }

    public void SignUp()
    {
        PK_C_REQ_SIGNUP packet = new PK_C_REQ_SIGNUP();
        packet.name = ID.text;
        packet.password = PW.text;
        

        //packet.name = "kinam";
        //packet.password = "1234";

        LogInNetwork.getInstance.sendPacket(packet);

        GameManager.getInstance.username = packet.name;

    }
}
