#include <genesis.h>
#include <resources.h>

#define SMALL_ENEMIES 11
#define MEDIUM_ENEMIES 22
#define LARGE_ENEMIES 22
#define MAX_BULLETS	3
#define PLAYER_MAX_BULLETS	1
#define LEFT_EDGE 0
#define RIGHT_EDGE 320
int enemiesLeft = 0;
int bulletsOnScreen = 0;
int i;
int score = 0;
int hi_score = 0;
char hud_string_line_1[40] = "";
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

//Variable to track if the player has shot a bullets
bool player_bullet_shot = FALSE;

//Handle the scoreboard
void updateScoreDisplay(){
		if (hi_score < score)
		{
			hi_score = score;
		}
		sprintf(hud_string_line_1,"            SCORE<1>  HI-SCORE");
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
	Sprite* sprite;
	char name[8];
} Entity;

//Define the sprite properties
Entity player = {0, 0, 16, 8, 0, 0, 0, 0, "PLAYER"};
Entity bullets[MAX_BULLETS];
Entity small_enemies[SMALL_ENEMIES];
Entity medium_enemies[MEDIUM_ENEMIES];
Entity large_enemies[LARGE_ENEMIES];

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

//Function to check if the edge has been hit
void edgeHit()
{

	for (i = 0; i < SMALL_ENEMIES; i++)
	{
		Entity* se = &small_enemies[i];
		if( (se->x+se->w) > RIGHT_EDGE || se->x < LEFT_EDGE)
		{
			if(se->health > 0)
			{
				dir=!dir;
				drop = TRUE;
			}
		}
	}

	for (i = 0; i < MEDIUM_ENEMIES; i++)
	{
		Entity* me = &medium_enemies[i];
		if( (me->x+me->w) > RIGHT_EDGE || me->x < LEFT_EDGE)
		{
			if(me->health > 0)
			{
				dir=!dir;
				drop = TRUE;
			}
		}
	}

	for (i = 0; i < LARGE_ENEMIES; i++)
	{
		Entity* le = &large_enemies[i];
		if( (le->x+le->w) > RIGHT_EDGE || le->x < LEFT_EDGE)
		{
			if(le->health > 0)
			{
				dir=!dir;
				drop = TRUE;
			}
		}
	}

}

//Function to move enemies around the screen
void positionEnemies()
{
  for(i = 0; i < SMALL_ENEMIES; i++)
	{
		Entity* se = &small_enemies[i];
		if(se->health > 0)
		{
			if (dir == LEFT)
			{
				se->velx = -1;
			} else {
				se->velx = 1;
			}
			se->x += se->velx;
			if (drop == TRUE){
				se->y = se->y +1;
			}
			SPR_setPosition(se->sprite,se->x,se->y);
		}
	}
  for(i = 0; i < MEDIUM_ENEMIES; i++)
	{
		Entity* me = &medium_enemies[i];
		if(me->health > 0)
		{
			if (dir == LEFT)
			{
				me->velx = -1;
			} else {
				me->velx = 1;
			}
			me->x += me->velx;
			if (drop == TRUE){
				me->y = me->y +1;
			}
			SPR_setPosition(me->sprite,me->x,me->y);
		}
	}
  for(i = 0; i < LARGE_ENEMIES; i++)
	{
		Entity* le = &large_enemies[i];
		if(le->health > 0)
		{
			if (dir == LEFT)
			{
				le->velx = -1;
			} else {
				le->velx = 1;
			}
			le->x += le->velx;
			if (drop == TRUE){
				le->y = le->y +1;
			}
			SPR_setPosition(le->sprite,le->x,le->y);
		}
	}
	drop = FALSE;
}

//Function to animate the enemies
void animateEnemies()
{
	for(i = 0; i < SMALL_ENEMIES; i++)
	{
		Entity* se = &small_enemies[i];
		SPR_nextFrame(se->sprite);
	}
	for(i = 0; i < MEDIUM_ENEMIES; i++)
	{
		Entity* me = &medium_enemies[i];
		SPR_nextFrame(me->sprite);
	}
	for(i = 0; i < LARGE_ENEMIES; i++)
	{
		Entity* le = &large_enemies[i];
		SPR_nextFrame(le->sprite);
	}
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

//Credit to ohsat for this :)
int collideEntities(Entity* a, Entity* b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}

//Function to handle collisions
void handleCollisions(){
	Entity* b;
	Entity* se;
	Entity* me;
	Entity* le;
	int i = 0;
	int j = 0;
	for(i = 0; i < MAX_BULLETS; i++)
	{
    b = &bullets[i];
    if(b->health > 0)
		{
			for(j = 0; j < SMALL_ENEMIES; j++)
			{
      	se = &small_enemies[j];
        if(se->health > 0)
				{
					if(collideEntities( b, se ))
					{
						killEntity(se);
    				killEntity(b);

    				enemiesLeft--;
    				bulletsOnScreen--;

						score += 30;
						updateScoreDisplay();

						player_bullet_shot = FALSE;
    				break;
    			}
				}
      }

			for(j = 0; j < MEDIUM_ENEMIES; j++)
			{
      	me = &medium_enemies[j];
        if(me->health > 0)
				{
					if(collideEntities( b, me ))
					{
						killEntity(me);
    				killEntity(b);

    				enemiesLeft--;
    				bulletsOnScreen--;

						score += 20;
						updateScoreDisplay();

						player_bullet_shot = FALSE;
    				break;
    			}
				}
      }

			for(j = 0; j < LARGE_ENEMIES; j++)
			{
      	le = &large_enemies[j];
        if(le->health > 0)
				{
					if(collideEntities( b, le ))
					{
						killEntity(le);
    				killEntity(b);

    				enemiesLeft--;
    				bulletsOnScreen--;

						score += 10;
						updateScoreDisplay();

						player_bullet_shot = FALSE;
    				break;
    			}
				}
      }

		}
	}
}

//Function to handle joypad input in game
void doActionJoy(u8 numjoy, u16 value)
{
    if (numjoy == JOY_1)
    {
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

int main()
{

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

  //Create the small enemy sprites
  Entity* se = small_enemies;
  for(i = 0; i < SMALL_ENEMIES; i++)
  {
    se->x = 72 + i*16;
    se->y = 32;
    se->w = 8;
    se->h = 8;
    se->velx = 1;
    se->health = 1;
    se->sprite = SPR_addSprite(&small_enemy,se->x,se->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
    sprintf(se->name, "En%d",i);
		enemiesLeft++;
		se++;
  }

	//Create the medium enemy sprites
  Entity* me = medium_enemies;
  for(i = 0; i < MEDIUM_ENEMIES; i++)
  {
		if(i <=10){
			me->x = 70 + i*16;
	    me->y = 48;
	    me->w = 11;
	    me->h = 8;
	    me->velx = 1;
	    me->health = 1;
	    me->sprite = SPR_addSprite(&medium_enemy,me->x,me->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(me->name, "En%d",i);
			enemiesLeft++;
			me++;
		} else {
			me->x = 70 + (i - 11)*16;
	    me->y = 64;
	    me->w = 11;
	    me->h = 8;
	    me->velx = 1;
	    me->health = 1;
	    me->sprite = SPR_addSprite(&medium_enemy,me->x,me->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(me->name, "En%d",i);
			enemiesLeft++;
			me++;
		}
  }

	//Create the large enemy sprites
  Entity* le = large_enemies;
  for(i = 0; i < LARGE_ENEMIES; i++)
  {
		if(i <=10){
			le->x = 70 + i*16;
	    le->y = 80;
	    le->w = 12;
	    le->h = 8;
	    le->velx = 1;
	    le->health = 1;
	    le->sprite = SPR_addSprite(&large_enemy,le->x,le->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(le->name, "En%d",i);
			enemiesLeft++;
			le++;
		} else {
			le->x = 70 + (i - 11)*16;
	    le->y = 96;
	    le->w = 12;
	    le->h = 8;
	    le->velx = 1;
	    le->health = 1;
	    le->sprite = SPR_addSprite(&large_enemy,le->x,le->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(le->name, "En%d",i);
			enemiesLeft++;
			le++;
		}
  }

  //Set the palette 1 colours from the player sprite
  VDP_setPalette(PAL1, player_ship.palette->data);
  VDP_setPalette(PAL2, small_enemy.palette->data);

	//Draw the initial scoreboard
	updateScoreDisplay();

  while(TRUE)
  {
		//Watch for joypad inputs (direction controls)
    doActionJoy(JOY_1, JOY_readJoypad(JOY_1));

		//Have the enemies reached the enge?
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
    //VDP_waitVSync();

		//Animate the enemies
		animateEnemies();

		//Debugging - show FPS on screen
		//VDP_showFPS(TRUE);
  }
  return 0;
}
