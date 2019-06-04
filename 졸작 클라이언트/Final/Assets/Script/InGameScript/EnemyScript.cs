using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using DummyClient;
public class EnemyScript : MonoBehaviour {

    bool isDead = false;

    [SerializeField]
    private int Health = 10;
    private int Damage = 10;


    [SerializeField]
    private float Speed;

    private int id;
    public int number;
    public SpriteRenderer spriteRenderer;
    public GameObject SelectObject;
    public Animator animator;
    BoxCollider2D boxCollider;

    public GameManager.Direction direction;


    public float Direction_Maintain_Time;



    

    protected readonly int m_HashWalkPara = Animator.StringToHash("Walk");
    protected readonly int m_HashWalkTopPara = Animator.StringToHash("Walk_T");
    protected readonly int m_HashWalkTopRightPara = Animator.StringToHash("Walk_TR");
    protected readonly int m_HashWalkRightPara = Animator.StringToHash("Walk_R");
    protected readonly int m_HashWalkBottomRightPara = Animator.StringToHash("Walk_BR");
    protected readonly int m_HashWalkBottomPara = Animator.StringToHash("Walk_B");
    protected readonly int m_HashWalkBottomLeftPara = Animator.StringToHash("Walk_BL");
    protected readonly int m_HashWalkLeftPara = Animator.StringToHash("Walk_L");
    protected readonly int m_HashWalkTopLeftPara = Animator.StringToHash("Walk_TL");

    protected readonly int m_HashShotPara = Animator.StringToHash("Shot");
    protected readonly int m_HashShotTopPara = Animator.StringToHash("Shot_T");
    protected readonly int m_HashShotTopRightPara = Animator.StringToHash("Shot_TR");
    protected readonly int m_HashShotRightPara = Animator.StringToHash("Shot_R");
    protected readonly int m_HashShotBottomRightPara = Animator.StringToHash("Shot_BR");
    protected readonly int m_HashShotBottomPara = Animator.StringToHash("Shot_B");
    protected readonly int m_HashShotBottomLeftPara = Animator.StringToHash("Shot_BL");
    protected readonly int m_HashShotLeftPara = Animator.StringToHash("Shot_L");
    protected readonly int m_HashShotTopLeftPara = Animator.StringToHash("Shot_TL");

    protected readonly int m_HashDeadPara = Animator.StringToHash("Dead");
    // Use this for initialization
    void Start() {
        boxCollider = GetComponent<BoxCollider2D>();
        animator = GetComponent<Animator>();


        spriteRenderer = GetComponent<SpriteRenderer>();

        Speed = 1.0f;

        resetTime();

        StartCoroutine(EnemyWalk());
    }

    // Update is called once per frame
    void Update() {
        IsTarget();
        // AngleCalculate();
        AnimationTimeOut();
    }

    public void resetTime()
    {
        Direction_Maintain_Time = 0.2f;
    }

    void AnimationTimeOut()
    {
        Direction_Maintain_Time -= Time.deltaTime;
       // Debug.Log("AnimationTimeOut!!" + Direction_Maintain_Time.ToString());

        //if (Direction_Maintain_Time < 0)
        //{
        ////    Debug.Log("AnimationTimeOut!!" + Direction_Maintain_Time.ToString());
        //    direction = GameManager.Direction.N;
        //    InitializeAnimationPara(0);
        //    resetTime();
        //}
    }

    IEnumerator EnemyWalk()
    {
        print(direction.ToString());
        /*
         * direction 이 도달하지 않는 것은 HeartBeat처럼 일정 시간으로 관리하는게 좋을듯함. 
         * 
         */
        while (true)
        {

            yield return null;

            switch (direction)
            {
                case GameManager.Direction.N:
                    InitializeAnimationPara(0);
                    break;

                case GameManager.Direction.T:
                    transform.position = new Vector3(transform.position.x, transform.position.y + Speed * Time.deltaTime, transform.position.z);
                    animator.SetBool(m_HashWalkTopPara, true);
                    if (spriteRenderer.flipX)
                        spriteRenderer.flipX = true;
                    else
                        spriteRenderer.flipX = false;

                    break;

                case GameManager.Direction.TR:
                    transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
                    animator.SetBool(m_HashWalkTopRightPara, true);
                    spriteRenderer.flipX = false;

                    break;

                case GameManager.Direction.R:
                    transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime, transform.position.y, transform.position.z);
                    animator.SetBool(m_HashWalkRightPara, true);

                    spriteRenderer.flipX = false;


                    break;

                case GameManager.Direction.RB:
                    transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
                    animator.SetBool(m_HashWalkBottomRightPara, true);

                    spriteRenderer.flipX = false;

                    break;


                case GameManager.Direction.B:
                    transform.position = new Vector3(transform.position.x, transform.position.y - Speed * Time.deltaTime, transform.position.z);
                    animator.SetBool(m_HashWalkBottomPara, true);
                    if (spriteRenderer.flipX)
                        spriteRenderer.flipX = true;
                    else
                        spriteRenderer.flipX = false;

                    break;


                case GameManager.Direction.BL:

                    transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
                    animator.SetBool(m_HashWalkBottomLeftPara, true);
                    spriteRenderer.flipX = true;

                    break;
                case GameManager.Direction.L:


                    transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime, transform.position.y, transform.position.z);
                    animator.SetBool(m_HashWalkLeftPara, true);

                    spriteRenderer.flipX = true;



                    break;

                case GameManager.Direction.LT:


                    transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);

                    animator.SetBool(m_HashWalkTopLeftPara, true);

                    spriteRenderer.flipX = true;

                    break;
            }

            BoundaryCheck();

        }

    }

    public void BoundaryCheck()
    {

        if (transform.position.x <= GameManager.getInstance.BoundaryLeft)
        {
            transform.position = new Vector3(GameManager.getInstance.BoundaryLeft, transform.position.y);
        }
        else if (transform.position.x >= GameManager.getInstance.BoundaryRight)
        {
            transform.position = new Vector3(GameManager.getInstance.BoundaryRight, transform.position.y);
        }


        if (transform.position.y <= GameManager.getInstance.BoundaryBottom)
        {
            transform.position = new Vector3(transform.position.x, GameManager.getInstance.BoundaryBottom);
        }
        else if (transform.position.y >= GameManager.getInstance.BoundaryTop)
        {
            transform.position = new Vector3(transform.position.x, GameManager.getInstance.BoundaryTop);

        }

    }

    //  public void EnemyWalk()
    //  { 
    //      print(direction.ToString());
    //      /*
    //       * direction 이 도달하지 않는 것은 HeartBeat처럼 일정 시간으로 관리하는게 좋을듯함. 
    //       * 
    //       */
    //      switch (direction)
    //      {
    //          case GameManager.Direction.N:
    //              return;

    //          case GameManager.Direction.T:
    //              transform.position = new Vector3(transform.position.x, transform.position.y + Speed * Time.deltaTime, transform.position.z);
    //              animator.SetBool(m_HashWalkTopPara, true);
    //              if (spriteRenderer.flipX)
    //                  spriteRenderer.flipX = true;
    //              else
    //                  spriteRenderer.flipX = false;

    //              break;

    //          case GameManager.Direction.TR:
    //              transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
    //              animator.SetBool(m_HashWalkTopRightPara, true);
    //              spriteRenderer.flipX = false;

    //              break;

    //          case GameManager.Direction.R:
    //              transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime, transform.position.y, transform.position.z);
    //              animator.SetBool(m_HashWalkRightPara, true);

    //              spriteRenderer.flipX = false;


    //              break;

    //          case GameManager.Direction.RB:
    //              transform.position = new Vector3(transform.position.x + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
    //              animator.SetBool(m_HashWalkBottomRightPara, true);

    //              spriteRenderer.flipX = false;

    //              break;


    //         case GameManager.Direction.B:
    //              transform.position = new Vector3(transform.position.x, transform.position.y - Speed * Time.deltaTime, transform.position.z);
    //              animator.SetBool(m_HashWalkBottomPara, true);
    //              if (spriteRenderer.flipX)
    //                  spriteRenderer.flipX = true;
    //              else
    //                  spriteRenderer.flipX = false;

    //              break;


    //          case GameManager.Direction.BL:

    //              transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);
    //              animator.SetBool(m_HashWalkBottomLeftPara, true);
    //              spriteRenderer.flipX = true;

    //              break;
    //          case GameManager.Direction.L:


    //              transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime, transform.position.y, transform.position.z);
    //              animator.SetBool(m_HashWalkLeftPara, true);

    //              spriteRenderer.flipX = true;



    //              break;

    //          case GameManager.Direction.LT:


    //              transform.position = new Vector3(transform.position.x - Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.y + Speed * Time.deltaTime / Mathf.Sqrt(2), transform.position.z);

    //              animator.SetBool(m_HashWalkTopLeftPara, true);

    //              spriteRenderer.flipX = true;

    //              break;
    //    }



    ////      AnimationTimeOut();
    //  }


    public void Shot(GameManager.Direction dir)
    {
        Debug.Log("Enemy Shot" + dir.ToString());
        switch (dir)
        {
            case GameManager.Direction.N:
                return;

            case GameManager.Direction.T:
                animator.SetTrigger(m_HashShotTopPara);
                spriteRenderer.flipX = false;
                return;

            case GameManager.Direction.TR:

                animator.SetTrigger(m_HashShotTopRightPara);
                spriteRenderer.flipX = false;
                return;

    
            case GameManager.Direction.R:

                animator.SetTrigger(m_HashShotRightPara);

                spriteRenderer.flipX = false;

                return;

            case GameManager.Direction.RB:

                animator.SetTrigger(m_HashShotBottomRightPara);
                spriteRenderer.flipX = false;
                return;

   
            case GameManager.Direction.B:
                print(" Input Right Shot Button" + dir.ToString());

                animator.SetTrigger(m_HashShotBottomPara);
                spriteRenderer.flipX = false;

                return;
           case GameManager.Direction.BL:

                spriteRenderer.flipX = true;

                animator.SetTrigger(m_HashShotBottomLeftPara);

                return;

             case GameManager.Direction.L:
                spriteRenderer.flipX = true;


                animator.SetTrigger(m_HashShotLeftPara);

                return;

            case GameManager.Direction.LT:

                spriteRenderer.flipX = true;


                animator.SetTrigger(m_HashShotTopLeftPara);
                return;

        }
        
    }

    public bool GetDead()
    {
        return isDead;
    }

    public void SetId(int number)
    {
        id = number;
    }
    public int GetId(int number)
    {
        return id;
    }

    public void GetDamaged()
    {
        Health -= Damage;
        if (Health <= 0)
        {
            isDead = true;
            SelectObject.SetActive(false);
            animator.SetTrigger(m_HashDeadPara);
        }
    }

    PlayerInputManager.Direction AngleCalculate()
    {
        float y = -PlayerInputManager.getInstance.transform.position.y + transform.position.y;
        float x = -PlayerInputManager.getInstance.transform.position.x + transform.position.x;
        float Angle = Mathf.Atan2(y, x);
        
        float result = Angle;

        print(result);
        print("x :" + x.ToString() + "y : " + y.ToString());
        if ( x >= 0 && y >= 0)
        {
            if (result > 1.125f)
            {

                return PlayerInputManager.Direction.T;
            }
            if (result < 0.375f)
            {

                return PlayerInputManager.Direction.R;
            }


            return PlayerInputManager.Direction.TR;
                
        }
        else if( x>= 0 && y < 0)
        {

            if (result > -0.375f)
            {
                return PlayerInputManager.Direction.R;
            }

            if (result < -1.125f)
            {
                return PlayerInputManager.Direction.B;
            }
            return PlayerInputManager.Direction.RB;

        }
        else if( x < 0 && y>= 0)
        {

            if (result > 2.625f)
            {
                return PlayerInputManager.Direction.L;

            }

            if (result < 1.875f)
            {
                return PlayerInputManager.Direction.T;
            }

            return PlayerInputManager.Direction.LT;
        }
        else if( x < 0 && y < 0)
        {


            if (result > -1.875f)
            {
                return PlayerInputManager.Direction.B;

            }

            if (result < -2.625f)
            {
                return PlayerInputManager.Direction.L;
            }

            return PlayerInputManager.Direction.BL;

        }
        else
        {
            return PlayerInputManager.Direction.N;
        }
        
    }


    void IsTarget()
    {

        if (Input.GetMouseButtonDown(0)){

           
            Vector3 vector = Input.mousePosition;
            Vector3 mousePos = Camera.main.ScreenToWorldPoint(vector);
            if (Mathf.Abs(boxCollider.bounds.center.x - mousePos.x) <= boxCollider.bounds.extents.x &&
                Mathf.Abs(boxCollider.bounds.center.y - mousePos.y) <= boxCollider.bounds.extents.y)
            {
                SelectObject.SetActive(true);
            }

        }

        if (Input.GetMouseButtonDown(1))
        {

            Vector3 vector = Input.mousePosition;
            Vector3 mousePos = Camera.main.ScreenToWorldPoint(vector);
            if (Mathf.Abs(boxCollider.bounds.center.x - mousePos.x) <= boxCollider.bounds.extents.x &&
                Mathf.Abs(boxCollider.bounds.center.y - mousePos.y) <= boxCollider.bounds.extents.y)
            {
                SelectObject.SetActive(true);
                PlayerInputManager.Direction dir = AngleCalculate();
                PlayerInputManager.getInstance.Shot(AngleCalculate());
                PK_C_REQ_SHOT packet = new PK_C_REQ_SHOT();
                packet.userNumber = GameManager.getInstance.MainCharacterNumber;
                packet.roomNumber = GameManager.getInstance.RoomNumber;
                packet.direction = (int)dir;
                packet.targetNumber = number;
                GameNetWork.getInstance.sendPacket(packet);

                GetDamaged();
            }

        }


    }




    void Dead()
    {

        direction = GameManager.Direction.N;
        animator.SetBool(m_HashWalkPara, false);
        animator.SetTrigger(m_HashDeadPara);

    }

    void InitializeAnimationPara(int HashPara)
    {

        if (HashPara == m_HashWalkTopPara)
        {
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkTopRightPara)
        {
            animator.SetBool(m_HashWalkPara, false);
            animator.SetBool(m_HashWalkTopPara, false);

            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkRightPara)
        {
            animator.SetBool(m_HashWalkPara, false);
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);

            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkBottomRightPara)
        {
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);

            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkBottomPara)
        {
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);

            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkBottomLeftPara)
        {
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);

            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        if (HashPara == m_HashWalkLeftPara)
        {
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);

            animator.SetBool(m_HashWalkTopLeftPara, false);
        }

        if (HashPara == m_HashWalkTopLeftPara)
        {
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);

        }
        if (HashPara == 0)
        {
            //     animator.SetBool(m_HashShotPara, false);
            animator.SetBool(m_HashWalkTopPara, false);
            animator.SetBool(m_HashWalkTopRightPara, false);
            animator.SetBool(m_HashWalkRightPara, false);
            animator.SetBool(m_HashWalkBottomRightPara, false);
            animator.SetBool(m_HashWalkBottomPara, false);
            animator.SetBool(m_HashWalkBottomLeftPara, false);
            animator.SetBool(m_HashWalkLeftPara, false);
            animator.SetBool(m_HashWalkTopLeftPara, false);
        }
        ////     animator.SetBool(m_HashShotPara, false);
        //     animator.SetBool(m_HashShotTopPara, false);
        //     animator.SetBool(m_HashShotTopRightPara, false);
        //     animator.SetBool(m_HashShotRightPara, false);
        //     animator.SetBool(m_HashShotBottomRightPara, false);
        //     animator.SetBool(m_HashShotBottomPara, false);
        //     animator.SetBool(m_HashShotBottomLeftPara, false);
        //     animator.SetBool(m_HashShotLeftPara, false);
        //     animator.SetBool(m_HashShotTopLeftPara, false);
    }
}

