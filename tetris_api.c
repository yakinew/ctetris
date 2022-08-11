

#include <stdlib.h>

#include "tetris_api.h"


/*************************************************************************
* 
**************************************************************************/

#define TETRIS_KEY_ROTATE      TETRIS_KEY_ARROW_UP
#define TETRIS_KEY_DROP_DOWN  TETRIS_KEY_SPACE



#define TETRIS_SIGN_CYAN    '*'
#define TETRIS_SIGN_PINK    '*'
#define TETRIS_SIGN_ORANGE  '*'
#define TETRIS_SIGN_YELLOW  '*'
#define TETRIS_SIGN_GREEN   '*'
#define TETRIS_SIGN_PINK    '*'
#define TETRIS_SIGN_RED     '*'

#define TETRIS_SIGN_EMPTY   ' '
#define TETRIS_SIGN_FULL_LINE   '-'
#define TETRIS_SIGN_BORDER  '#'

#define TETRIS_SIGN_NULL   '\0'

#define TETRIS_MAX_BUF       256

#define TETRIS_MAX_X       20
#define TETRIS_MIN_X       10
#define TETRIS_MAX_Y       20
#define TETRIS_MIN_Y       7

#define TETRIS_DELAY_MIN              10
#define TETRIS_DELAY_MAX              1000
#define TETRIS_DELAY_FOREVER          0
#define TETRIS_DEFAULT_DELAY          500
#define TETRIS_LEVEL_START            0
#define TETRIS_LEVEL_MAX              9
#define TETRIS_LEVEL_LINES_FOR_NEXT   20
#define TETRIS_SPEED_FACTOR           9
#define TETRIS_SPEED_START            1000





/*************************************************************************
* 
**************************************************************************/

typedef enum {
  TETRIS_DIRECTION_UP_E,
  TETRIS_DIRECTION_RIGHT_E,
  TETRIS_DIRECTION_DOWN_E,
  TETRIS_DIRECTION_LEFT_E,
  TETRIS_DIRECTION_ALL_E
} TETRIS_DIRECTION_E;


typedef enum {
  TETRIS_UPDATE_SHAPE      = 0x0001,
  TETRIS_UPDATE_SCORE      = 0x0002,
  TETRIS_UPDATE_SCREEN     = 0x0004,
  TETRIS_UPDATE_NEXT_SHAPE = 0x0008,
  TETRIS_UPDATE_LEVEL      = 0x0010,
  TETRIS_UPDATE_LINES      = 0x0020,
  TETRIS_UPDATE_BORDER     = 0x0040,
  TETRIS_UPDATE_ALL        = 0x00FF
} TETRIS_UPDATE_E;





/*************************************************************************
* 
**************************************************************************/

typedef struct {
  int letter;
  unsigned short display[TETRIS_DIRECTION_ALL_E];
  int color;
} TETRIS_PIECE_T;


typedef struct {
  short x;
  short y;
  TETRIS_DIRECTION_E direction;
  int piece;
} TETRIS_TETRIMON_T;



typedef struct {
  short x;
  short y;
  char  sign;
  char  change;
} TETRIS_ELEMENT_T;



typedef struct {
  TETRIS_INIT_DB_T  db;
  int               score;
  int               level;
  TETRIS_TETRIMON_T prev;
  TETRIS_TETRIMON_T curr;
  TETRIS_TETRIMON_T next;
  char **           screen;
  int               delay; /*In msec */
  TETRIS_STATUS_E   status;
  int            speed; /* In msec */
  int            update;
  int            *update_lines;
  int            update_num;
  int            lines;
  int            margin;
} TETRIS_DB_T;


/*************************************************************************
* Internal database:
**************************************************************************/



TETRIS_PIECE_T Pieces[] = {
  {'I', {0x00F0, 0x4444, 0x0F00, 0x2222}, TETRIS_SIGN_RED},
  {'J', {0x0E20, 0x44C0, 0x8E00, 0x6440}, TETRIS_SIGN_RED},
  {'L', {0x2E00, 0x4460, 0x0E80, 0xC440}, TETRIS_SIGN_RED},
  {'O', {0xCC00, 0xCC00, 0xCC00, 0xCC00}, TETRIS_SIGN_RED},
  {'S', {0x4620, 0x06C0, 0x8C40, 0x6C00}, TETRIS_SIGN_RED},
  {'T', {0x4640, 0x0E40, 0x4C40, 0x4E00}, TETRIS_SIGN_RED},
  {'Z', {0x2640, 0x0C60, 0x4C80, 0xC600}, TETRIS_SIGN_RED}
};





/*************************************************************************
* Function: TETRIS_ChooseTetrimon
* Abstract: Choose a tetrimon.
**************************************************************************/
void TETRIS_ChooseTetrimon(TETRIS_TETRIMON_T *item, TETRIS_DB_T *s)
{
  /* Set posiition */
  item->x = s->db.size_x+14;
  item->y = s->db.size_y/2+5;

  /* Set directiona dn piece type */  
  item->direction = TETRIS_DIRECTION_UP_E;
  item->piece = rand() % sizeof(Pieces) / sizeof(TETRIS_PIECE_T);

  if ((Pieces[item->piece].display[item->direction] & 0xF000) == 0)
  {
    item->y--;

    if ((Pieces[item->piece].display[item->direction] & 0xFF00) == 0)
      item->y--;
  }
}



/*************************************************************************
* Function: TETRIS_DisplayTetrimon
* Abstract: Display the current tetrimon.
**************************************************************************/
void TETRIS_DisplayTetrimon(const TETRIS_TETRIMON_T *curr, const TETRIS_TETRIMON_T *prev, int transparent, TETRIS_DB_T *s)
{
  unsigned short mask = 0x8000;
  int i, j;
  int x, y;
  char c;

#define TETRIS_DisplayTetrimon_X_OFFSET 2
#define TETRIS_DisplayTetrimon_Y_OFFSET 1

  if (prev)
  {
    for (i=0; i<4; i++)
    {
      for (j=0; j<4; j++)
      {
        if (Pieces[prev->piece].display[prev->direction] & mask)
        {
          x = prev->x + j;
          y = prev->y + i;

          s->db.gotoxy(s->db.connId, x+TETRIS_DisplayTetrimon_X_OFFSET, y+TETRIS_DisplayTetrimon_Y_OFFSET);
          s->db.print(
#if TETRIS_MULTIPLE_CONN
            s->db.connId, 
#endif
            "%c", TETRIS_SIGN_EMPTY);
        }

        mask >>= 1;
      }
    }
  }

  if (curr)
  {
    mask = 0x8000;

    for (i=0; i<4; i++)
    {
      for (j=0; j<4; j++)
      {
        if ((Pieces[curr->piece].display[curr->direction] & mask) || (!transparent))
        {
          x = curr->x + j;
          y = curr->y + i;
          c = (Pieces[curr->piece].display[curr->direction] & mask) ? Pieces[curr->piece].color : TETRIS_SIGN_EMPTY;

          s->db.gotoxy(s->db.connId, x+TETRIS_DisplayTetrimon_X_OFFSET, y+TETRIS_DisplayTetrimon_Y_OFFSET);
          s->db.print(
#if TETRIS_MULTIPLE_CONN
            s->db.connId, 
#endif
            "%c", c);
        }

        mask >>= 1;
      }
    }
  }
}




/*************************************************************************
* Function: TETRIS_Display
* Abstract: Display the board.
**************************************************************************/
void TETRIS_Display(TETRIS_DB_T *s)
{
  int i, y;
  char buf[TETRIS_MAX_BUF] = {0};

  if (s->update == TETRIS_UPDATE_ALL)
  {
    s->db.cls(s->db.connId);

    for (i=0; i<s->db.size_y; i++)
    {
      s->db.gotoxy(s->db.connId, 1, i+1);

      buf[0] = TETRIS_SIGN_BORDER;
      buf[1] = '\0';
      strncat(buf, s->screen[i], s->db.size_x);
      buf[s->db.size_x+1] = TETRIS_SIGN_BORDER;
      buf[s->db.size_x+2] = '\r';
      buf[s->db.size_x+3] = '\n';
      buf[s->db.size_x+4] = '\0';
      s->db.print(
  #if TETRIS_MULTIPLE_CONN
        s->db.connId, 
  #endif
        buf);
    }

    s->db.gotoxy(s->db.connId, 1, i+1);

    memset(buf, TETRIS_SIGN_BORDER, s->db.size_x+2);
    buf[s->db.size_x+2] = '\0';
    s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      buf);
  }
  else if (s->update & TETRIS_UPDATE_SCREEN)
  {
    /* Only in this case need to update the lines without the border */
    for (i=0; i<s->update_num; i++)
    {
      y = s->update_lines[i];
      s->db.gotoxy(s->db.connId, 2, y+1);
      snprintf(buf, s->db.size_x+1, "%s", s->screen[y]);
      buf[s->db.size_x] = '\0';
      s->db.print(
#if TETRIS_MULTIPLE_CONN
        s->db.connId, 
#endif
        "%s", buf);
    }

    s->update_num = 0;
  }

  /* Score */
  if (s->update & TETRIS_UPDATE_SCORE)
  {
    s->db.gotoxy(s->db.connId, s->db.size_x+10, s->db.size_y/2-2);
    s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Score: %4d", s->score);
  }
  
  /* Level */
  if (s->update & TETRIS_UPDATE_LEVEL)
  {
    s->db.gotoxy(s->db.connId, s->db.size_x+10, s->db.size_y/2);
    s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Level: %4d", s->level);
  }
  
  /* Level */
  if (s->update & TETRIS_UPDATE_LINES)
  {
    s->db.gotoxy(s->db.connId, s->db.size_x+10, s->db.size_y/2+2);
    s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Lines: %4d", s->lines);
  }
  
  if (s->update & TETRIS_UPDATE_SHAPE)
  {
    /* Display current tetrimon */
    TETRIS_DisplayTetrimon(&s->curr, (s->update == TETRIS_UPDATE_ALL) ? NULL : &s->prev, 1, s);
  }
  
  if (s->update == TETRIS_UPDATE_ALL)
  {
    /* Next piece */
    s->db.gotoxy(s->db.connId, s->db.size_x+10, s->db.size_y/2+4);
    s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Next:");
  }

  if (s->update & TETRIS_UPDATE_NEXT_SHAPE)
  {
    TETRIS_DisplayTetrimon(&s->next, NULL, 0, s);
  }

  /* Put the cursur in the corner */
  s->db.gotoxy(s->db.connId, 0, s->db.size_y);

  /* Reset the update indication (next time want to update only new things) */
  s->update = 0;
}





/*************************************************************************
* Function: TETRIS_Welcome
* Abstract: Display welcome messgae.
**************************************************************************/
TETRIS_ERROR_T TETRIS_Welcome(TETRIS_DB_T *s)
{
  TETRIS_ERROR_T ret = TETRIS_OK_E;
  char ch=0;
  
  s->db.cls(s->db.connId);
  s->db.gotoxy(s->db.connId, 30, 10);
  s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Welcome to TETRIS!");

  s->db.gotoxy(s->db.connId, 20, 12);
  s->db.print(
#if TETRIS_MULTIPLE_CONN
      s->db.connId, 
#endif
      "Press ENTER to start the game or ESC to exit");

  do {
    ch = s->db.getch(s->db.connId, TETRIS_DELAY_FOREVER);
  } while((ch != TETRIS_KEY_ENTER) && (ch != TETRIS_KEY_ESC));

  if (ch == TETRIS_KEY_ESC)
    ret = TETRIS_END_GAME_E;

  return ret;
}


/*************************************************************************
* Function: TETRIS_Byebye
* Abstract: Display welcome messgae.
**************************************************************************/
TETRIS_ERROR_T TETRIS_Byebye(TETRIS_DB_T *s)
{
  TETRIS_ERROR_T ret = TETRIS_OK_E;
  char ch=0;

  if (s->status == TETRIS_STATUS_FINISH)
  {
    s->db.gotoxy(s->db.connId, 10, 10);
    s->db.print(
  #if TETRIS_MULTIPLE_CONN
        s->db.connId, 
  #endif
        "     Game Over!     ");

    s->db.gotoxy(s->db.connId, 4, 12);
    s->db.print(
  #if TETRIS_MULTIPLE_CONN
        s->db.connId, 
  #endif
        " Press any key to continue...");
  }

  ch = s->db.getch(s->db.connId, TETRIS_DELAY_FOREVER);

  return ret;
}



/*************************************************************************
* Function: TETRIS_validate
* Abstract: Check the input from the user.
**************************************************************************/
TETRIS_ERROR_T TETRIS_validate(TETRIS_INIT_DB_T *db)
{
  long ret = TETRIS_OK_E;
  
  if (!db->print || !db->cls || !db->gotoxy || !db->getch)
  {
    ret = TETRIS_ERROR_MISSING_FUNC_E;
  }
  else if ((db->size_x < TETRIS_MIN_X) || (TETRIS_MAX_X < db->size_x))
  {
    ret = TETRIS_ERROR_INVALID_X_E;
  }
  else if ((db->size_y < TETRIS_MIN_Y) || (TETRIS_MAX_Y < db->size_y))
  {
    ret = TETRIS_ERROR_INVALID_Y_E;
  }

  return ret;
}



/*************************************************************************
* Function: TETRIS_init
* Abstract: Allocate memory for the board.
**************************************************************************/
TETRIS_ERROR_T TETRIS_init_db(TETRIS_DB_T **ps, TETRIS_INIT_DB_T *db)
{
  int i;
  TETRIS_ERROR_T ret;
  int size;
  TETRIS_DB_T *s;

  ret = TETRIS_validate(db);
  if (ret != TETRIS_OK_E)
    return ret;

  /* Take the save game */
  s = (TETRIS_DB_T*)db->self;

  /* If there is no saved game - create new */
  if (s == NULL)
  {
    s = (TETRIS_DB_T*)TETRIS_MALLOC(sizeof(*s));
    if (s == NULL)
      return TETRIS_ERROR_MALLOC_E;
  }

  /* Copy parametes */
  memcpy(&s->db, db, sizeof(s->db));


  /* Check if the game already init */
  if (!s->db.self)
  {
    /* Seed random */
    //srand (time(NULL));

    /* Init the score */
    s->score = 0;
    s->level = TETRIS_LEVEL_START;

    /* Init the tetris screen */
    size = (s->db.size_x + sizeof(char*)) * s->db.size_y;
    s->screen = (char**)TETRIS_MALLOC(size);
    if (!s->screen)
    {
      TETRIS_FREE(s);
      return TETRIS_ERROR_MALLOC_E;
    }

    memset(s->screen, TETRIS_SIGN_EMPTY, size);

    for (i=0; i<s->db.size_y; i++)
    {
      s->screen[i] = (char*)((int)(s->screen + s->db.size_y) + (s->db.size_x * i));
    }

    s->update_lines = (int*)TETRIS_MALLOC(sizeof(int*) * s->db.size_y);
    if (!s->update_lines)
    {
      TETRIS_FREE(s->screen);
      TETRIS_FREE(s);
      return TETRIS_ERROR_MALLOC_E;
    }

    s->update_num = 0;
    memset(s->update_lines, 0, (sizeof(int*) * s->db.size_y));

    /* Choose a piece */
    memset(&s->prev, 0, sizeof(s->prev));
    TETRIS_ChooseTetrimon(&s->curr, s);
    s->curr.y = 1;
    s->curr.x = s->db.size_x / 2 - 2;
    TETRIS_ChooseTetrimon(&s->next, s);

    db->self = s;
    s->db.self = (void*)&s;
    s->delay = TETRIS_DEFAULT_DELAY;
    s->speed = TETRIS_SPEED_START;
    s->lines = 0;
    s->margin = s->db.size_y;
  }

  *ps = s;

  return TETRIS_OK_E;
}




/*****************************************************************************
*  Function: TETRIS_CheckMove
*  Absract:  Check if a movement is valid.
*****************************************************************************/
int TETRIS_CheckMove(TETRIS_TETRIMON_T *shape, TETRIS_DB_T *s)
{
  int ret = TETRIS_OK_E;
  int i, j;
  unsigned short mask = 0x8000;
  int x, y;

  for (i=0; i<4; i++)
  {
    y = shape->y + i;

    for (j=0; j<4; j++)
    {
      x = shape->x + j;

      if (mask & Pieces[shape->piece].display[shape->direction])
      {
        if ((x<0 || s->db.size_x <= x) ||
          (y<0 || s->db.size_y <= y))
          ret = TETRIS_ERROR_INVALID_MOVE_E;
        else if (s->screen[y][x] != TETRIS_SIGN_EMPTY)
          ret = TETRIS_ERROR_INVALID_MOVE_E;
      }

      mask >>= 1;
    }
  }

  return ret;
}



/*****************************************************************************
*  Function: TETRIS_AddShapeToBoard
*  Absract:  .
*****************************************************************************/
int TETRIS_AddShapeToBoard(TETRIS_DB_T *s)
{
  int ret = TETRIS_OK_E;
  int i, j;
  unsigned short mask = 0x8000;
  int x, y;
  int lines = 0;
  int complete[4] = {0};
  char *temp;
  
  /* Check if user droped the shape - need to remove the prev shape and draw it in the new place */
  if (s->curr.y != s->prev.y)
  {
    TETRIS_DisplayTetrimon(&s->curr, &s->prev, 1, s);
  }

  /* Add the shape to the board */
  for (i=0; i<4; i++)
  {
    y = s->curr.y + i;

    for (j=0; j<4; j++)
    {
      x = s->curr.x + j;

      if (mask & Pieces[s->curr.piece].display[s->curr.direction])
      {
        s->screen[y][x] = Pieces[s->curr.piece].color;

        if (y < s->margin)
          s->margin = y;
      }

      mask >>= 1;
    }
  }


  mask = 0xF000;
  /* Check if need to remove full lines */
  for (i=0; i<4; i++, mask >>= 4)
  {
    if (Pieces[s->curr.piece].display[s->curr.direction] & mask)
    {
      y = s->curr.y + i;

      /* Check if all the line is no empty */
      for (x=0; (x<s->db.size_x) && (s->screen[y][x] != TETRIS_SIGN_EMPTY); x++);

      /* Full line! */
      if (x == s->db.size_x)
      {
        complete[lines++] = y;
        s->update_lines[s->update_num++] = y;
        
        s->update |= TETRIS_UPDATE_SCREEN;
        memset(s->screen[y], TETRIS_SIGN_FULL_LINE, s->db.size_x);
      }
    }
  }
  
  if (lines)
  {
    /* Display the full lines */
    TETRIS_Display(s);
    TASK_DELAY(80);

    for (i=0; i<lines; i++)
    {
      y = complete[i];
      memset(s->screen[y], TETRIS_SIGN_EMPTY, s->db.size_x);
      
      temp = s->screen[y];
      for (; s->margin<y; y--)
      {
          s->screen[y] = s->screen[y-1];
      }
      s->screen[y] = temp;
      s->margin++;
    }

    for (y=s->margin-lines; y<=complete[lines-1]; y++)
    {
      s->update_lines[s->update_num++] = y;
    }

    s->update |= TETRIS_UPDATE_SCREEN;
    s->update |= TETRIS_UPDATE_LINES;
  }  
  

  /* Set the score */
  s->score += lines * 100 + 50;
  s->update |= TETRIS_UPDATE_SCORE;


  /* Update level */
  s->lines += lines;
  if (s->level < TETRIS_LEVEL_MAX)
  {
    if ((s->level * TETRIS_LEVEL_LINES_FOR_NEXT) < s->lines)
    {
      s->level++;
      s->speed = (s->speed * (100-TETRIS_SPEED_FACTOR)) / 100;
      s->update |= TETRIS_UPDATE_LEVEL;
    }
  }
  
  /* Select new tetrimon */
  s->next.y = 1;
  s->next.x = s->db.size_x / 2 - 2;
  s->prev = s->curr = s->next;
  s->update |= TETRIS_UPDATE_SHAPE;
  
  TETRIS_ChooseTetrimon(&s->next, s);
  s->update |= TETRIS_UPDATE_NEXT_SHAPE;


  /* Check if there is space to the new tetrimon */
  if (TETRIS_CheckMove(&s->curr, s) != TETRIS_OK_E)
  {
    s->status = TETRIS_STATUS_FINISH;
    ret = TETRIS_END_GAME_E;
  }

  return ret;
}



/*****************************************************************************
*  Function: TETRIS_Move
*  Absract:  Check if it is possible to move/rotate the shape. If yes, update 
*            the shape position.
*****************************************************************************/
int TETRIS_Move(char ch, TETRIS_DB_T *s)
{
  int ret = TETRIS_OK_E;
  TETRIS_TETRIMON_T shape;

  /* Copy the original params */
  shape = s->prev = s->curr;

  /* Set the new value */
  switch (ch)
  {
    case TETRIS_KEY_ARROW_LEFT:
      shape.x--;
      break;
    case TETRIS_KEY_ARROW_RIGHT:
      shape.x++;
      break;
    case TETRIS_KEY_ROTATE:
      shape.direction = (shape.direction+1) & 0x03;
      break;
    case TETRIS_KEY_ARROW_DOWN:
      shape.y++;
      break;
    case TETRIS_KEY_DROP_DOWN:
      while (TETRIS_CheckMove(&shape, s) == TETRIS_OK_E)
        shape.y++;
      break;
  }

  if ((ret = TETRIS_CheckMove(&shape, s)) == TETRIS_OK_E)
  {
    s->curr = shape;
    s->update |= TETRIS_UPDATE_SHAPE;
  }
  else if ((ch == TETRIS_KEY_ARROW_DOWN) || (ch == TETRIS_KEY_DROP_DOWN))
  {
    shape.y--;
    s->curr = shape;
    ret = TETRIS_AddShapeToBoard(s);
  }

  return ret;
}



/*****************************************************************************
*  Function: TETRIS_Start
*  Absract:  The main function of the game.
*****************************************************************************/
int TETRIS_Start(TETRIS_DB_T *s)
{
  int ret = TETRIS_OK_E;
  char ch;

  /* present the board */
  s->update = TETRIS_UPDATE_ALL;
  TETRIS_Display(s);
  s->status = TETRIS_STATUS_RUN;

  /* Main loop */
  while (s->status == TETRIS_STATUS_RUN)
  {
    /* Get a key from user */
    ch = s->db.getch(s->db.connId, s->speed);
    
    switch (ch)
    {
      case TETRIS_KEY_TIMEOUT:
        ch = TETRIS_KEY_ARROW_DOWN;
        /* Do no break -want to do the same path as arrow down! */
        
      case TETRIS_KEY_ARROW_UP:
      case TETRIS_KEY_ARROW_RIGHT:
      case TETRIS_KEY_ARROW_LEFT:
      case TETRIS_KEY_ARROW_DOWN:
      case TETRIS_KEY_DROP_DOWN:
        ret = TETRIS_Move(ch, s);
        break;

      case TETRIS_KEY_ESC:
        s->status = TETRIS_STATUS_STOP;
        ret = TETRIS_PAUSE_GAME_E;
        break;

      default:
        ret = TETRIS_ERROR_INVALID_KEY_E;
        break;
    }

    if (ret == TETRIS_OK_E)
    {
      /* present the board */
      TETRIS_Display(s);
    }
  }

  return ret;
}




/*************************************************************************
* Function: TETRIS_end
* Abstract: Free allocated memory of the board.
**************************************************************************/
TETRIS_ERROR_T TETRIS_end(TETRIS_DB_T *s)
{
  if (s->status == TETRIS_STATUS_FINISH)
  {
    TETRIS_Byebye(s);
    
    /* Free memory in case the game was finished */
    TETRIS_FREE(s->screen);
  }

  s->db.cls(s->db.connId);

  return TETRIS_OK_E;
}






/*************************************************************************
* Function: TETRIS_start
* Abstract: Main function.
**************************************************************************/
long TETRIS_start(TETRIS_INIT_DB_T *db)
{
  long ret = TETRIS_OK_E;
  TETRIS_DB_T *s = NULL;
  
  if (TETRIS_init_db(&s, db) == TETRIS_OK_E)
  {
    /* Say wellcome */
    if (TETRIS_Welcome(s) != TETRIS_END_GAME_E)
    {
      ret = TETRIS_Start(s);
    }

    TETRIS_end(s);
  }

  return ret;
}


