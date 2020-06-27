#include <genesis.h>
#include <resources.h>

#define ENEMIES 55
#define MAX_BULLETS	3
#define LEFT_EDGE 0
#define RIGHT_EDGE 320

//Variables needed throughout the program
int enemiesLeft = 0; //Number of enemies remaining
int bulletsOnScreen = 0; //Number of bullets currently on screen
int i; //Used for loops
int score = 0; //Initial score when starting the game
int hi_score = 0; //Hi-Score, initially 0, but then increased when score > hi-score
int movespeed = 1; //Initial movement speed of the enemies

//Head up display - populated fully in updateScoreDisplay
char hud_string_line_1[40] = "            SCORE<1>  HI-SCORE";
char hud_score[40] = "";
char hud_hi_score[40] = "";

//Set the game direction
typedef enum{
    LEFT,
    RIGHT
}GAME_DIRECTION;
int dir=RIGHT;

//Variable to track if the enemy sprites should drop or not
bool drop = FALSE;

//Variable to track if the player has shot a bullet
bool player_bullet_shot = FALSE;

//Variable to track if the game is started
bool game_running  = FALSE;

//Variables to track the different music files
bool music_1_playing = FALSE;
bool music_2_playing = FALSE;
bool music_3_playing = FALSE;
bool music_4_playing = FALSE;
bool music_5_playing = FALSE;
bool music_6_playing = FALSE;
bool music_7_playing = FALSE;

//Handle the scoreboard
void updateScoreDisplay(){
		if (hi_score < score)
		{
			hi_score = score;
		}
		sprintf(hud_score,"%d",score);
		sprintf(hud_hi_score,"%d",hi_score);
    VDP_clearText(0,0,40);
		VDP_clearText(0,1,40);
    VDP_drawText(hud_string_line_1,0,0);
		VDP_drawText(hud_score,12,1);
		VDP_drawText(hud_hi_score,22,1);
}

//Array for sprites
typedef struct
{
	int x;
	int y;
	int w;
	int h;
	int velx;
	int vely;
	int health;
  int points;
	Sprite* sprite;
	char name[8];
} Entity;

//Define the sprite properties
Entity player = {0, 0, 16, 8, 0, 0, 0, 0, 0, "PLAYER"};
Entity bullets[MAX_BULLETS];
Entity enemies[ENEMIES];

//Define sound effects
#define PLAYER_BULLET_SFX       65
#define ALIEN_DESTROYED_SFX     66

//Function to kill a sprite
void killEntity(Entity* e)
{
	e->health = 0;
	SPR_setVisibility(e->sprite,HIDDEN);
}

//Function to revive a sprite
void reviveEntity(Entity* e)
{
	e->health = 1;
	SPR_setVisibility(e->sprite,VISIBLE);
}

//Function to handle player bullets
void shootBullet()
{
	if(player_bullet_shot == FALSE)
	{
  	Entity* b;
    u16 i = 0;
    for(i=0; i<1; i++)
		{
    	b = &bullets[i];
      if(b->health == 0)
			{
				b->x = player.x+4;
      	b->y = player.y;

      	reviveEntity(b);
      	b->vely = -3;

      	SPR_setPosition(b->sprite,b->x,b->y);
      	bulletsOnScreen++;
				player_bullet_shot = TRUE;

        //Play the shot sound effect
        XGM_startPlayPCM(PLAYER_BULLET_SFX, 1, SOUND_PCM_CH2);
      	break;
    	}
  	}
	}
}

//Function to move player bullets on screen
void positionBullets()
{
	u16 i = 0;
	Entity *b;
	for(i = 0; i < MAX_BULLETS; i++)
	{
    b = &bullets[i];
    if(b->health > 0)
		{
			b->y += b->vely;
			if(b->y + b->h < 0)
			{
    		killEntity(b);
    		bulletsOnScreen--;
				player_bullet_shot = FALSE;
			} else {
				SPR_setPosition(b->sprite,b->x,b->y);
			}
    }
	}
}

//Function to handle joypad input in game
void doActionJoy(u8 numjoy, u16 value)
{
  if (numjoy == JOY_1)
  {
    if (value & BUTTON_START && game_running == FALSE)
    {
      game_running = TRUE;
    }

    if (value & BUTTON_LEFT)
    {
			if (player.x > 0)
			{
				player.x -= player.velx;
				SPR_setPosition(player.sprite,player.x,player.y);
			}
    }

    if (value & BUTTON_RIGHT)
    {
			if (player.x < 304)
			{
				player.x += player.velx;
				SPR_setPosition(player.sprite,player.x,player.y);
			}
    }

		if (value & BUTTON_A)
    {
      shootBullet();
    }
  }
}

//Function to draw text in the center of the screen, creates a variable for the vertical position of the text
int v_pos;
void centeredText(char s[], int v_pos){
	VDP_drawText(s, 20 - strlen(s)/2 ,v_pos);
}

//Function to handle the game over event
void gameOver()
{
  char msg_game_over[10] = "GAME OVER\0";
  centeredText(msg_game_over, 8);
  waitMs(5000);
  game_running = FALSE;
  music_1_playing = FALSE;
  enemiesLeft = 55;
  score = 0;
  movespeed = 1;
  SYS_reset();
}

//Function to display a start screen
void startScreen()
{
  //Setup the text strings
  char msg_start_1[5] = "PLAY\0";
  char msg_start_2[15] = "SPACE INVADERS\0";
  char msg_start_3[22] = "*SCORE ADVANCE TABLE*\0";
  char msg_start_4[11] = "=? MYSTERY\0";
  char msg_start_5[11] = "=30 POINTS\0";
  char msg_start_6[11] = "=20 POINTS\0";
  char msg_start_7[11] = "=10 POINTS\0";
  char msg_start_8[11] = "PUSH START\0";

  //Write title screen text
  centeredText(msg_start_1, 6);
  centeredText(msg_start_2, 8);
  centeredText(msg_start_3, 12);
  centeredText(msg_start_4, 14);
  centeredText(msg_start_5, 16);
  centeredText(msg_start_6, 18);
  centeredText(msg_start_7, 20);

  //Loop the "Press Start" flashing message, while waiting for the player to press it
  i = 0;
  while(game_running == FALSE)
  {
    doActionJoy(JOY_1, JOY_readJoypad(JOY_1));
    if (i == 0){
      centeredText(msg_start_8, 24);
    }
    if (i == 5000){
      VDP_clearText(0,24,40);
    }
    i++;
    if (i == 10000)
    {
      i = 0;
    }
  }

  //Once the game has been started, clear the text from the screen
  VDP_clearTextArea(0,6,40,20);
}

//Function to check if the edge has been hit
void edgeHit()
{
  for (i = 0; i < ENEMIES; i++)
	{
		Entity* e = &enemies[i];
		if( (e->x+e->w) > RIGHT_EDGE || e->x < LEFT_EDGE)
		{
			if(e->health > 0)
			{
				dir=!dir;
				drop = TRUE;
        break; //We don't want the drop to repeat for all of the enemies that hit the edge - only the first
			}
		}
	}
}

//Function to animate the enemies
void animateEnemies()
{
    for (i = 0; i < ENEMIES; i++)
    {
		    Entity* e = &enemies[i];
		    SPR_nextFrame(e->sprite);
    }
}

//Function to move enemies around the screen - we work large to small, because of the screen edge
void positionEnemies()
{
  for(i = 0; i < ENEMIES; i++)
	{
		Entity* e = &enemies[i];
		if(e->health > 0)
		{
      if (drop == TRUE)
      {
				e->y = e->y +4;
			}
			if (dir == LEFT)
			{
        e->velx = -1 * movespeed;
			} else {
				e->velx = 1 * movespeed;
			}
			e->x += e->velx;
      if (e->y > 185)
      {
        gameOver();
      } else {
        SPR_setPosition(e->sprite,e->x,e->y);
      }
		}
	}
	drop = FALSE;
}

//Function to determine if an object has collided - full credit to ohsat for this :)
int collideEntities(Entity* a, Entity* b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}

//Function to handle collisions
void handleCollisions(){
	Entity* b;
	Entity* e;
	int i = 0;
	int j = 0;
	for(i = 0; i < MAX_BULLETS; i++)
	{
    b = &bullets[i];
    if(b->health > 0)
		{
			for(j = 0; j < ENEMIES; j++)
			{
      	e = &enemies[j];
        if(e->health > 0)
				{
					if(collideEntities( b, e ))
					{
						killEntity(e);
    				killEntity(b);

    				enemiesLeft--;
    				bulletsOnScreen--;

						score += e->points;
						updateScoreDisplay();

						player_bullet_shot = FALSE;

            //Play the alien destroyed sound effect
            XGM_startPlayPCM(ALIEN_DESTROYED_SFX, 1, SOUND_PCM_CH3);
    				break;
    			}
				}
      }
		}
	}
}

//Play the different music files, depending on how many enemies are alive
void musicCycles()
{
      if (enemiesLeft == 55 && music_1_playing == FALSE)
      {
        XGM_startPlay(music_1);
        music_1_playing = TRUE;
      }
      if (enemiesLeft == 40 && music_2_playing == FALSE)
      {
        XGM_startPlay(music_2);
        music_2_playing = TRUE;
      }
      if (enemiesLeft == 32 && music_3_playing == FALSE)
      {
        movespeed ++;
        XGM_startPlay(music_3);
        music_3_playing = TRUE;
      }
      if (enemiesLeft == 24 && music_4_playing == FALSE)
      {
        XGM_startPlay(music_4);
        music_4_playing = TRUE;
      }
      if (enemiesLeft == 16 && music_5_playing == FALSE)
      {
        movespeed ++;
        XGM_startPlay(music_5);
        music_5_playing = TRUE;
      }
      if (enemiesLeft == 12 && music_6_playing == FALSE)
      {
        XGM_startPlay(music_6);
        music_6_playing = TRUE;
      }
      if (enemiesLeft == 8 && music_7_playing == FALSE)
      {
        movespeed ++;
        XGM_startPlay(music_7);
        music_7_playing = TRUE;
      }
}

int main()
{

  //Draw the initial scoreboard
	updateScoreDisplay();

  //Check to see if the game is running - if not, show the start screen
  if (game_running == FALSE)
  {
    startScreen();
  }

	//Initialise the sprite engine
  SPR_init();

  //Add the player
  player.x = 152;
  player.y = 192;
  player.health = 1;
	player.velx = 2;
  player.sprite = SPR_addSprite(&player_ship,player.x,player.y,TILE_ATTR(PAL1,0,FALSE,FALSE));
  SPR_update();

	//Create all bullet sprites
	Entity* b = bullets;
	for(i = 0; i < MAX_BULLETS; i++){
    b->x = 0;
    b->y = -10;
    b->w = 8;
    b->h = 8;
    b->sprite = SPR_addSprite(&player_bullet,bullets[0].x,bullets[0].y,TILE_ATTR(PAL1,0,FALSE,FALSE));
    sprintf(b->name, "Bu%d",i);
    b++;
	}

  //Create all enemy sprites
  Entity* e = enemies;
  for(i = 0; i < ENEMIES; i++)
  {
    if(i <=10){
      e->x = 72 + i*16;
      e->y = 32;
      e->w = 8;
      e->h = 8;
      e->velx = 1;
      e->health = 1;
      e->points = 30;
      e->sprite = SPR_addSprite(&small_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
      sprintf(e->name, "En%d",i);
  		enemiesLeft++;
      e++;
		} else if (i<=21){
      e->x = 70 + (i-11)*16;
	    e->y = 48;
	    e->w = 11;
	    e->h = 8;
	    e->velx = 1;
	    e->health = 1;
      e->points = 20;
	    e->sprite = SPR_addSprite(&medium_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
		} else if (i<=32){
      e->x = 70 + (i-22)*16;
	    e->y = 64;
	    e->w = 11;
	    e->h = 8;
	    e->velx = 1;
	    e->health = 1;
      e->points = 20;
	    e->sprite = SPR_addSprite(&medium_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    } else if (i<=43){
      e->x = 70 + (i-33)*16;
	    e->y = 80;
	    e->w = 12;
	    e->h = 8;
	    e->velx = 1;
	    e->health = 1;
      e->points = 10;
	    e->sprite = SPR_addSprite(&large_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    } else if (i<=55){
      e->x = 70 + (i-44)*16;
	    e->y = 96;
	    e->w = 12;
	    e->h = 8;
	    e->velx = 1;
	    e->health = 1;
      e->points = 10;
	    e->sprite = SPR_addSprite(&large_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    }
  }

  //Set the palette 1 colours from the player sprite
  VDP_setPalette(PAL1, player_ship.palette->data);
  VDP_setPalette(PAL2, small_enemy.palette->data);

  //Setup the sound effects and music
  XGM_setPCM(PLAYER_BULLET_SFX, player_bullet_sfx, sizeof(player_bullet_sfx));
  XGM_setPCM(ALIEN_DESTROYED_SFX, alien_destroyed_sfx, sizeof(alien_destroyed_sfx));

  while(game_running == TRUE)
  {
		//Watch for joypad inputs (direction controls)
    doActionJoy(JOY_1, JOY_readJoypad(JOY_1));

    //Play the soundtrack
    musicCycles();

		//Have the enemies reached the edge?
		edgeHit();

		//Update enemy positions
		positionEnemies();

		//Update the bullet positions
		positionBullets();

		//Detect if items have collided
		handleCollisions();

		//Update all the sprites on screen
		SPR_update();

		//Wait for vsync to stop flickering
    VDP_waitVSync();

		//Animate the enemies
		//animateEnemies();

		//Debugging - show FPS on screen
		VDP_showFPS(TRUE);
  }
  return 0;
}
