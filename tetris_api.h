
#ifndef _tetris_api_h
#define _tetris_api_h

#include <stdio.h>
#include <string.h>

#if WIN32
#define TETRIS_MALLOC(x)   malloc(x)
#define TETRIS_FREE(x)     free(x)
#else
/* include for RAD_MALLOC and RAD_FREE*/
#include "radoscfg.h"
#include "rtosdef.h"

#define TETRIS_MALLOC(x)   RAD_MALLOC(x, 0)
#define TETRIS_FREE(x)     RAD_FREE(x, 0)
#endif

#define TETRIS_MULTIPLE_CONN       1
#define TETRIS_MUX_MAX_CONN_NUM    16


#define TETRIS_KEY_ENTER       13
#define TETRIS_KEY_ESC         27
#define TETRIS_KEY_SPACE       32
#define TETRIS_KEY_PLUS       '+'
#define TETRIS_KEY_MINUS     '-'

#define TETRIS_KEY_ARROW_UP    72
#define TETRIS_KEY_ARROW_DOWN  80
#define TETRIS_KEY_ARROW_LEFT  75
#define TETRIS_KEY_ARROW_RIGHT 77

#define TETRIS_KEY_TIMEOUT 255



typedef enum {
  TETRIS_OK_E,
  TETRIS_END_GAME_E,
  TETRIS_PAUSE_GAME_E,
  TETRIS_ERROR_E,
  TETRIS_ERROR_MISSING_FUNC_E,
  TETRIS_ERROR_MALLOC_E,
  TETRIS_ERROR_INVALID_X_E,
  TETRIS_ERROR_INVALID_Y_E,
  TETRIS_ERROR_INVALID_KEY_E,
  TETRIS_ERROR_INVALID_MOVE_E,
  TETRIS_ERROR_LAST_E,
} TETRIS_ERROR_T;

typedef enum {
	TETRIS_STATUS_INIT = 0, 
	TETRIS_STATUS_RUN, 
	TETRIS_STATUS_STOP, 
	TETRIS_STATUS_FINISH
}TETRIS_STATUS_E;

typedef void(*TETRIS_GotoXY)(int ConnID, const int x, const int y);
typedef void(*TETRIS_print)(
#if TETRIS_MULTIPLE_CONN
      int ConnID, 
#endif
      const char *format, ...);
typedef void(*TETRIS_cls)(int ConnID);
typedef char(*TETRIS_getch)(int ConnID, const int timeout); /*in miliseconds*/


/* Internal DB for Tetris */
typedef struct {
  short size_x;
  short size_y;
  int max_tetris_length;
  TETRIS_GotoXY gotoxy;
  TETRIS_print print;
  TETRIS_cls cls;
  TETRIS_getch getch;
  int connId;
  void *self;
  unsigned long queue_id;
} TETRIS_INIT_DB_T;



long TETRIS_start(TETRIS_INIT_DB_T *db);


#endif /*!_tetris_api_h*/

