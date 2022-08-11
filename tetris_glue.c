#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tetris_api.h"
#include "tetris_glue.h"
#include <time.h>


#if WIN32

#include <conio.h>
#include <windows.h>
#include <stdio.h>

#endif


#define TETRIS_MUX_MAX_CONN_NUM    16


TETRIS_INIT_DB_T *TETRIS_MuxArr[TETRIS_MUX_MAX_CONN_NUM];

static int TETRIS_GLUE_init(int ConnID);


#if WIN32

 
static void sleep(unsigned int mseconds)
{
  /* Need #include <time.h> */
  clock_t goal = mseconds + clock();
  while (goal > clock());
}

/*********************************************************
 * Function: TETRIS_GLUE_GotoXY
 *********************************************************/
static void TETRIS_GLUE_GotoXY(int ConnID, const int x, const int y)
{
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD position = {x, y};
  SetConsoleCursorPosition(handle, position);
}




/*********************************************************
 * Function: TETRIS_GLUE_print
 *********************************************************/
#define TETRIS_GLUE_print printf
/*static void TETRIS_GLUE_print(int ConnID, const char *format, ...)
{
  va_list args;

  va_start(args, format);

  printf(format, args);
}
*/

/*********************************************************
 * Function: TETRIS_GLUE_cls
 *********************************************************/
static void TETRIS_GLUE_cls(int ConnID)
{
  system("cls");
}

/*********************************************************
 * Function: TETRIS_GLUE_getch
 *********************************************************/
static char TETRIS_GLUE_getch(int ConnID, const int timeout)
{
  char ch=0;
/*  fd_set readfds;
  struct timeval timeout_val;

  timeout_val.tv_sec = 0;
  timeout_val.tv_usec = timeout * 1000;

  FD_ZERO(&readfds );
  FD_SET(fileno(stdin), &readfds);

  select(1, &readfds, NULL, NULL, &timeout_val);
*/

  if (timeout)
  {
    sleep(timeout);

    if (kbhit())
    {
      ch = getch();
    }
  }
  else
  {
    ch = getch();
  }

  if (ch == -32)
  {
    ch = getch();
  }

  return ch;
}


/*********************************************************
 * Function: TETRIS_GLUE_prepare
 *********************************************************/
static unsigned long TETRIS_GLUE_prepare(int ConnID)
{
  return 0;
}


/*********************************************************
 * Function: TETRIS_GLUE_finish
 *********************************************************/
static void TETRIS_GLUE_finish(int ConnID)
{

}


/*********************************************************
 * Function: main
 *********************************************************/
int main()
{
  return TETRIS_GLUE_init(0);
}


#endif /* WINDOWS */




#if VXWORKS_APP




/*********************************************************
 * Function: TETRIS_GLUE_GotoXY
 *********************************************************/
static void TETRIS_GLUE_GotoXY(int ConnID, const int x, const int y)
{
  unsigned char buffer[9] = {27,'[',0,0,';',0,0,'H',0};

  buffer[2] = (char) y/10 + '0';
  buffer[3] = (char) y%10 + '0';
  buffer[5] = (char) x/10 + '0';
  buffer[6] = (char) x%10 + '0';

  /* Go to X Y : */
  TERM_MUX_Print_Str(ConnID, buffer, 1);
}


/*********************************************************
 * Function: TETRIS_GLUE_print
 *********************************************************/
static void TETRIS_GLUE_print(int ConnID, const char *format, ...)
{
  va_list args;
  char str[256] = {0};

  va_start(args, format);

  vsprintf(str, format, args);
  
  TERM_MUX_Print_Str(ConnID, (unsigned char *)str, 1);
}

/*********************************************************
 * Function: TETRIS_GLUE_cls
 *********************************************************/
static void TETRIS_GLUE_cls(int ConnID)
{
  unsigned char reset_vt100_buffer[] = {27,'c',0};
  unsigned char clear_screen_buffer[] = {27,'[','2','J',27,'[','0',';','0','H',0};
  unsigned char set_screen_scrolling_buffer[] = {27,'[','0','1',';','2','5','r',0};

  TERM_MUX_Print_Str(ConnID, reset_vt100_buffer, 1);
  /* Without delay PROCOMM doesn't work properly;
  HyperTerminal doesn't require any delay */
  TASK_DELAY (10);

  TERM_MUX_Print_Str(ConnID, clear_screen_buffer, 0);
  TERM_MUX_Print_Str(ConnID, set_screen_scrolling_buffer, 1);
}

/*********************************************************
 * Function: TETRIS_GLUE_getch
 *********************************************************/
static char TETRIS_GLUE_getch(int ConnID, const int timeout)
{
  char ch=0;
  unsigned long key;
  unsigned long queue_id;

  if (ConnID < TETRIS_MUX_MAX_CONN_NUM)
  {
    queue_id = TETRIS_MuxArr[ConnID]->queue_id;
    
    if (QUE_RECEIVE(queue_id, QUE_WAIT, (timeout/TETRIS_GLUE_QUEUE_MSEC_IN_TICK), (void *)&key, sizeof(key)))
    {
      switch (key)
      {
        case 10:
        case 13:
          ch = TETRIS_KEY_ENTER;
          break;
        case 27:
          ch = TETRIS_KEY_ESC;
          break;
        case 32:
          ch = TETRIS_KEY_SPACE;
          break;
        case 43:
        case 44:
          ch = TETRIS_KEY_PLUS;
          break;
        case 45:
          ch = TETRIS_KEY_MINUS;
          break;
        case 193:
          ch = TETRIS_KEY_ARROW_UP;
          break;
        case 194:
          ch = TETRIS_KEY_ARROW_DOWN;
          break;
        case 195:
          ch = TETRIS_KEY_ARROW_RIGHT;
          break;
        case 196:
          ch = TETRIS_KEY_ARROW_LEFT;
          break;
      }       
    }
    else
    {
      if (errno == S_objLib_OBJ_TIMEOUT)
        ch = TETRIS_KEY_TIMEOUT;
    }
  }

  return ch;
}


/*********************************************************
 * Function: TETRIS_GLUE_post_char
 *********************************************************/
static void TETRIS_GLUE_post_char(unsigned long ConnID, unsigned char ch)
{
  unsigned long key;
  unsigned long queue_id;

  if (ConnID < TETRIS_MUX_MAX_CONN_NUM)
  {
    queue_id = TETRIS_MuxArr[ConnID]->queue_id;
    key = ch;
    
    QUE_SEND(queue_id, (void *)&key, sizeof(key));
  }
}


/*********************************************************
 * Function: TETRIS_GLUE_prepare
 * Abstract: Prepare the message queue in order to get the 
 *           charachters.
 *********************************************************/
static unsigned long TETRIS_GLUE_prepare(int ConnID)
{
  char que_name[20];
  unsigned long queue_id = 0;

  if (ConnID < TETRIS_MUX_MAX_CONN_NUM)
  {
    sprintf(que_name, TETRIS_GLUE_QUEUE_NAME, ConnID);
    queue_id = QUE_CREATE(que_name, TETRIS_GLUE_QUEUE_SIZE, TETRIS_GLUE_QUEUE_MSG_SIZE, TETRIS_GLUE_QUEUE_OPTION);

    TETRIS_MuxArr[ConnID]->queue_id = queue_id;
    
    if (queue_id)
    {
      TERM_MUX_ChangeCliPostFunc(ConnID, TETRIS_GLUE_post_char);
    }
  }

  return queue_id;
}

/*********************************************************
 * Function: TETRIS_GLUE_finish
 * Abstract: Prepare the message queue in order to get the 
 *           charachters.
 *********************************************************/
static void TETRIS_GLUE_finish(int ConnID)
{
  unsigned long queue_id;

  if (ConnID < TETRIS_MUX_MAX_CONN_NUM)
  {
    TERM_MUX_ChangeCliPostFunc(ConnID, NULL);

    queue_id = TETRIS_MuxArr[ConnID]->queue_id;
    QUE_DELETE(queue_id);
  }
}

#endif /* VXWORKS_APP */




/*********************************************************
 * Function: TETRIS_GLUE_getch
 *********************************************************/
int TETRIS_GLUE_init(int ConnID)
{
  TETRIS_INIT_DB_T *db=NULL;
  

  if (ConnID < TETRIS_MUX_MAX_CONN_NUM)
  {
    if (TETRIS_MuxArr[ConnID])
    {
      /* Already exist */
      db = TETRIS_MuxArr[ConnID];
    }
    else
    {
      /* New game */
      db = (TETRIS_INIT_DB_T *)TETRIS_MALLOC(sizeof(TETRIS_INIT_DB_T));
      if (db)
      {
        db->size_x = 10;
        db->size_y = 20;
        db->max_tetris_length = 500;
        db->gotoxy = TETRIS_GLUE_GotoXY;
        db->print = TETRIS_GLUE_print;
        db->cls = TETRIS_GLUE_cls;
        db->getch = TETRIS_GLUE_getch;
        db->connId = ConnID;
        db->queue_id = 0;
        db->self = 0;
        TETRIS_MuxArr[ConnID] = db;
      }
      else
      {
        return -1;
      }
    }
    
    TETRIS_GLUE_prepare(ConnID);
    
    if (TETRIS_start(db) != TETRIS_PAUSE_GAME_E)
    {
      TETRIS_FREE(db);
      db = NULL;
    }
    
    TETRIS_GLUE_finish(ConnID);
    
    TETRIS_MuxArr[ConnID] = db;
  }

  return 0;
}

