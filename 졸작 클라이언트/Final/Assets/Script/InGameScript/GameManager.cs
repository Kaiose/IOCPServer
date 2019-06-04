using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DummyClient;
using System;
public class GameManager : MonoBehaviour {
    static protected GameManager gameManager;
    static public GameManager getInstance { get { return gameManager; } }
    public GameObject MainUnit;
    public GameObject EnemyUnit;
    private GameObject mainCharacter;

    public Chatting chatting;


    public GameObject[] UserList = new GameObject[1001];
    

    public Int32 MainCharacterNumber;
    public Int32 accountId;
    public string username;
    public Int32 RoomNumber;
    public Int32 serverNumber;

    public PointingCharacter pointingCharacter;

    //public UserList userList;
    // Use this for initialization



    public float BoundaryTop;
    public float BoundaryBottom;
    public float BoundaryLeft;
    public float BoundaryRight;



    public enum Direction
    {
        N,
        T,
        TR,
        R,
        RB,
        B,
        BL,
        L,
        LT,
    }


    private void Awake()
    {
        gameManager = this;
    }
    void Start () {
        RoomNumber = -1;
        accountId = -1;
        DontDestroyOnLoad(this);

    }
	
	// Update is called once per frame
	void Update () {
		
	}
    
   
    public void DeleteCharacter(Int32 Number)
    {

        if (RoomNumber == -1) return;

        GameObject unit = UserList[(int)Number];

        Destroy(unit);
    }

    public void CreateEnemyCharacter(Int32 Number, bool isDummy)
    {
        //      Debug.Log("Create Character Number : " + Number.ToString() + "RoomNumber : " );

        if (RoomNumber == -1)
        {
            Debug.Log("Create Failed Character Number : " + Number.ToString() + "RoomNumber is -1 ");
            return;
        }
        if(UserList[(int)Number] != null)
        {
            GameObject.Destroy(UserList[(int)Number]);
        }

        GameObject unit = Instantiate(EnemyUnit);
        UserList[(int)Number] = unit;
        unit.GetComponent<EnemyScript>().number = Number;

        if(isDummy == false)
        {
            unit.GetComponent<SpriteRenderer>().color = new Color(255, 0, 0);   
            unit.GetComponent<EnemyScript>().SelectObject.SetActive(true);
        }

        Debug.Log("Create Success Character Number : " + Number.ToString() + "RoomNumber is" + RoomNumber.ToString());
    }
    public void CreateMainCharacter(Int32 Number)
    {
        Debug.Log("Create main Character Number : " + Number.ToString() + "RoomNumber : " );

        //  GameObject userList = GameObject.Find("UserList");
        GameObject unit = Instantiate(MainUnit);
        UserList[(int)Number] = unit;
        MainCharacterNumber = Number;
        print("instance");


        pointingCharacter.RegistCharacter(unit.transform);
    }
    
    public void MoveCharacter(Int32 Number, Direction direction,float pos_X, float pos_Y)
    {
        //       UserList[(int)Number].GetComponent<EnemyScript>().resetTime();

        if (RoomNumber == -1) return;

        if (UserList[Number] == null)
            return;

      //  Debug.Log("Move Character Number : " + Number.ToString());

        UserList[(int)Number].GetComponent<EnemyScript>().resetTime();
        UserList[(int)Number].GetComponent<EnemyScript>().direction = direction;
        //UserList[Number].GetComponent<EnemyScript>().EnemyWalk();
        //UserList[Number].transform.position = new Vector3(pos_X, pos_Y, -10.0f);
    //    print("SetPostition");
    }

    public int ShotCharacter(Int32 Number, Direction direction, Int32 TargetNumber)
    {

        if (RoomNumber == -1) return -1;
        Debug.Log("Target Number : " + TargetNumber.ToString());

        UserList[(int)Number].GetComponent<EnemyScript>().Shot(direction);

        if (TargetNumber == MainCharacterNumber)
        {
        //    PlayerInputManager.getInstance.Shot((PlayerInputManager.Direction)direction);
            PlayerInputManager.getInstance.Dead();
            return 0;
        }
        else
        {
            UserList[(int)TargetNumber].GetComponent<EnemyScript>().GetDamaged();
            return 1;
        }
    }

    public void SetCharacterPostition(Int32 Number, float pos_X, float pos_Y)
    {


        if (RoomNumber == -1) return;
        if (UserList[Number] == null)
            return;

        UserList[Number].transform.position = new Vector3(pos_X, pos_Y, -10.0f);
       
     //   print("SetPostition");
    }
}
