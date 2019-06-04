using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using DummyClient;
public class UserList : MonoBehaviour {

    public GameObject[] UserPool = new GameObject[201];

	// Use this for initialization
	void Start () {
      //  GameManager gameManager = GameObject.Find("ScriptManager").GetComponent<GameManager>();

       // gameManager.userList = this;
    }
	
	// Update is called once per frame
	void Update () {
		
	}


    public void SetUser(int Number, GameObject Target)
    {
        UserPool[Number] = Target;
    }
}
