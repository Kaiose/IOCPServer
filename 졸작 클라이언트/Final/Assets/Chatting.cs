using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using DummyClient;

public class Chatting : MonoBehaviour {


    public GameObject viewPort;

    public GameObject Content;
    public Text message;

    public InputField inputField;

    public Text[] messageArray;

    public float contentsHeight;
    public float messageHeight;

    int sizeofArray;

    public float viewPortStartPosition_X;
    public float viewPortStartPosition_Y;
	// Use this for initialization
	void Start () {

        GameManager.getInstance.chatting = this;

        contentsHeight = Content.GetComponent<RectTransform>().rect.height;
        messageHeight = message.GetComponent<RectTransform>().rect.height;

        sizeofArray = (int)(contentsHeight / messageHeight);

        messageArray = new Text[sizeofArray];
     
        viewPortStartPosition_X = viewPort.transform.position.x;
        viewPortStartPosition_Y = viewPort.transform.position.y;


        float viewPortHeight = viewPort.GetComponent<RectTransform>().rect.height;

        float messageBoxInterval = message.GetComponent<RectTransform>().rect.height;

        for (int i = 0; i < sizeofArray; i++)
        {
            messageArray[i] = Instantiate(message);

            messageArray[i].GetComponent<Transform>().parent = Content.transform;

            messageArray[i].transform.position = new Vector3(viewPortStartPosition_X, viewPortStartPosition_Y - viewPortHeight + messageBoxInterval * i, 0);
            

        }
	}
	
	// Update is called once per frame
	void Update () {

        if (inputField.isFocused == false)
        {
            PlayerInputManager.getInstance.Wait = false;
        }
        else
        {
            PlayerInputManager.getInstance.Wait = true;
        }

        if (Input.GetKeyDown(KeyCode.Return))
        {
            
            if (inputField.IsActive() && inputField.text != "")
            {

                PK_C_REQ_CHAT packet = new PK_C_REQ_CHAT();
                packet.roomNumber = GameManager.getInstance.RoomNumber;
                
                packet.userNumber = GameManager.getInstance.MainCharacterNumber;
                packet.chat = inputField.text;
                GameNetWork.getInstance.sendPacket(packet);

                Debug.Log("RoomNumber : " + packet.roomNumber.ToString() + "User Number : " + packet.userNumber.ToString());

                ReceiveMessage(GameManager.getInstance.username,inputField.text);
                inputField.text = "";
            }

            inputField.Select();

            //                inputField.DeactivateInputField();

            //inputField.ActivateInputField();

        }
    }


    public void ReceiveMessage(string name, string msg)
    {

        Debug.Log(name + " : " + msg);

        for ( int i = sizeofArray - 1; i > 0; i--)
        {
            messageArray[i].text = messageArray[i - 1].text;

        }
        string combine = name + " : " + msg;
        messageArray[0].text = combine;


    }
}
