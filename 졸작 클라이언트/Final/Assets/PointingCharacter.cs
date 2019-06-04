using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DummyClient;
public class PointingCharacter : MonoBehaviour {


    private bool isRegist;
    private Transform pointCharacter;

    // Use this for initialization

    private void Awake()
    {

        GameManager.getInstance.pointingCharacter = this;
    }

    void Start () {

        isRegist = false;

        
	}
	
	// Update is called once per frame
	void Update () {
        if (isRegist)
        {
            transform.position = new Vector3(pointCharacter.position.x, pointCharacter.position.y, transform.position.z);

        }
	}

    public void RegistCharacter(Transform character)
    {
        pointCharacter = character;
        isRegist = true;

    }
}
