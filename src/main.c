#include <genesis.h>
#include <resources.h>

#define ENEMIES 55
#define MAX_PLAYER_BULLETS 1
#define MAX_ENEMY_BULLETS	2
#define SHIELDS 4
#define LEFT_EDGE 70
#define RIGHT_EDGE 250
#define TOP_EDGE 24
#define BOTTOM_EDGE 200

//Variables needed throughout the program
int enemiesLeft = 0; //Number of enemies remaining
int bulletsOnScreen = 0; //Number of bullets currently on screen
int enemyBulletsOnScreen = 0; //Number of enemy bullets currently on screen
int i; //Used for loops
int score = 0; //Initial score when starting the game
int hi_score = 0; //Hi-Score, initially 0, but then increased when score > hi-score
int movespeed = 1; //Initial movement speed of the enemies
int tick = 0; //Used to handle game speed - ticks counted in the gameCycle loop.
int enemyAnimateTick = 0;
int spritetick = 0; //Used to hide the bullet_top_screen sprite.
int mothershiptick = 0; //Used to trigger a saucer event;
int enemy_explode_tick = 0; //Used to count steps to show the enemy exploded sprite;
int level = 0; //The starting level of the game - used to track enemy vertical position at level start
int bulletType; //The type of bullet the enemy will shoot
int shootColumn; //The column of enemies that will shoot

//Head up display - populated fully in updateScoreDisplay
char hud_string_line_1[40] = "SCORE<1>  HI-SCORE";
char hud_score[40] = "";
char hud_hi_score[40] = "";

//Set the game direction
typedef enum{
    LEFT,
    RIGHT
}GAME_DIRECTION;
int dir=RIGHT;

//Variable to track if the player has shot a bullet
bool player_bullet_shot = FALSE;

//Variable to track if the game is started
bool game_running = FALSE;

//Variable to track if the mothership is on screen
bool mothership_shown = FALSE;

//Variable to track if the top bullet 'miss' is on screen
bool bullet_top_shown = FALSE;

//Variable to track if the exploded enemy is on screen
bool exploded_enemy_shown = FALSE;

//Variable to track shoot rate of enemies
bool time_to_shoot = FALSE;

//Function to draw text in the center of the screen, creates a variable for the vertical position of the text
int v_pos;
void centeredText(char s[], int v_pos){
	VDP_drawText(s, 20 - strlen(s)/2 ,v_pos);
}

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
    centeredText(hud_string_line_1, 0);
		VDP_drawText(hud_score,11,1);
		VDP_drawText(hud_hi_score,21,1);
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
  int row;
  int column;
	int health;
  int points;
	Sprite* sprite;
	char name[8];
} Entity;

//Define the sprite properties
Entity player = {0, 0, 16, 8, 0, 0, 0, 0, 0, 0, 0, "PLAYER"};
Entity player_bullets[MAX_PLAYER_BULLETS];
Entity enemies[ENEMIES];
Entity shields[SHIELDS];
Entity bullet_top_screen =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "BULLET_T"};
Entity exploded_enemy =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "EXP_E"};
Entity mothership_sprite =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "MOTHER"};
Entity exploded_mothership =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "EXP_M"};
Entity rolling_bullet_sprite = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "ROLL_B"};
Entity plunger_bullet_sprite = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "PLUNGE_B"};
Entity squiggly_bullet_sprite = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "SQUIGG_B"};

//Sprites for title screen - handled differently to the entities above.
Sprite* mo; //Mothership
Sprite* se; //Small enemy
Sprite* me; //Medium enemy
Sprite* le; //Large enemy

//Define sound effects
#define PLAYER_BULLET_SFX         65
#define ALIEN_DESTROYED_SFX       66
#define MUSIC_NOTE_1_SFX          67
#define MUSIC_NOTE_2_SFX          68
#define MUSIC_NOTE_3_SFX          69
#define MUSIC_NOTE_4_SFX          70
#define MOTHERSHIP_SFX            71
#define PLAYER_DIED_SFX           72
#define MOTHERSHIP_DESTROYED_SFX  73

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
void shootPlayerBullet()
{
	if(player_bullet_shot == FALSE)
	{
  	Entity* b;
    for(i=0; i<1; i++)
		{
    	b = &player_bullets[i];
      if(b->health == 0)
			{
				b->x = player.x+6;
      	b->y = player.y;

      	reviveEntity(b);
      	b->vely = -4;

      	SPR_setPosition(b->sprite,b->x,b->y);
      	bulletsOnScreen++;
				player_bullet_shot = TRUE;

        //Play the shot sound effect
        XGM_startPlayPCM(PLAYER_BULLET_SFX, 1, SOUND_PCM_CH3);
      	break;
    	}
  	}
	}
}

/*Generate a random number, with the options minimum, maximum*/
int RandomNumberGenerator(const int nMin, const int nMax)
{
  int nRandomNumber = 0;
  nRandomNumber = random()%((nMax+1)-nMin) + nMin;
  return nRandomNumber;
}

//Function to handle enemy bullets
void shootEnemyBullet()
{
  if (enemyBulletsOnScreen < MAX_ENEMY_BULLETS)
  {
    shootColumn = RandomNumberGenerator(1,11);
    //We want to loop through the enemies backwards, soas to 'hit' the bottom row first
    for (i = ENEMIES; i > 0; i--)
    {
      Entity* e = &enemies[i];
      Entity* eb;
      if (e->column == shootColumn && e->health == 1)
      {
        bulletType = RandomNumberGenerator(1,3);
        switch (bulletType)
        {
          case 1:
            eb = &rolling_bullet_sprite;
            if (eb->health < 1)
            {
              reviveEntity(eb);
              eb->x = e->x + (e->w/2) - 1;
              eb->y = e->y + 4;
              SPR_setPosition(eb->sprite,eb->x,eb->y);
              enemyBulletsOnScreen++;
              return;
            }
          case 2:
            eb = &plunger_bullet_sprite;
            if (eb->health < 1)
            {
              reviveEntity(eb);
              eb->x = e->x + (e->w/2) - 1;
              eb->y = e->y + 4;
              SPR_setPosition(eb->sprite,eb->x,eb->y);
              enemyBulletsOnScreen++;
              return;
            }
          case 3:
            eb = &squiggly_bullet_sprite;
            if (eb->health < 1)
            {
              reviveEntity(eb);
              eb->x = e->x + (e->w/2) - 1;
              eb->y = e->y + 4;
              SPR_setPosition(eb->sprite,eb->x,eb->y);
              enemyBulletsOnScreen++;
              return;
            }
        }
      }
    }
  }
}

//Function to move bullets on screen
void positionBullets()
{
  //Move the player bullets
	Entity *b;
	for(i = 0; i < MAX_PLAYER_BULLETS; i++)
	{
    b = &player_bullets[i];
    if(b->health > 0)
		{
			b->y += b->vely;
      //Check to see if the bullet hit the top of the screen. If so, change the sprite
			if(b->y + b->h < TOP_EDGE)
			{
    		killEntity(b);
    		bulletsOnScreen--;
				player_bullet_shot = FALSE;
        if (bullet_top_shown == FALSE)
        {
          spritetick = 0;
          bullet_top_shown = TRUE;
          SPR_setVisibility(bullet_top_screen.sprite,VISIBLE);
          SPR_setPosition(bullet_top_screen.sprite,b->x - 4,16);
        }
			} else {
				SPR_setPosition(b->sprite,b->x,b->y);
			}
    }
	}

  //Move the enemy bullets
  Entity* eb;
  eb = &rolling_bullet_sprite;
  eb->y += 2;
  SPR_setPosition(eb->sprite,eb->x,eb->y);
  if (eb->health > 0 && eb->y > BOTTOM_EDGE)
  {
    killEntity(eb);
    eb->y = -16;
    enemyBulletsOnScreen--;
  }
  //Move the enemy bullets
  eb = &plunger_bullet_sprite;
  eb->y += 2;
  SPR_setPosition(eb->sprite,eb->x,eb->y);
  if (eb->health > 0 && eb->y > BOTTOM_EDGE)
  {
    killEntity(eb);
    eb->y = -16;
    enemyBulletsOnScreen--;
  }
  //Move the enemy bullets
  eb = &squiggly_bullet_sprite;
  eb->y += 2;
  SPR_setPosition(eb->sprite,eb->x,eb->y);
  if (eb->health > 0 && eb->y > BOTTOM_EDGE)
  {
    killEntity(eb);
    eb->y = -16;
    enemyBulletsOnScreen--;
  }
}

//Function to handle joypad input in game
void doActionJoy(u8 numjoy, u16 value)
{
  if (numjoy == JOY_1)
  {
    if (value & BUTTON_START && game_running == FALSE)
    {
      //Game is about to start - release the title screen sprites
      SPR_releaseSprite (se);
      SPR_releaseSprite (me);
      SPR_releaseSprite (le);
      SPR_releaseSprite (mo);
      game_running = TRUE;
    }

    if (value & BUTTON_LEFT)
    {
			if (player.x > LEFT_EDGE)
			{
				player.x -= player.velx;
				SPR_setPosition(player.sprite,player.x,player.y);
			}
    }

    if (value & BUTTON_RIGHT)
    {
			if (player.x+player.w < RIGHT_EDGE)
			{
				player.x += player.velx;
				SPR_setPosition(player.sprite,player.x,player.y);
			}
    }

		if (value & BUTTON_A)
    {
      shootPlayerBullet();
    }
  }
}

//Function to handle the game over event
void gameOver()
{
  char msg_game_over[10] = "GAME OVER\0";
  centeredText(msg_game_over, 8);
  waitMs(5000);
  game_running = FALSE;
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
  char msg_start_4[12] = " =? MYSTERY\0";
  char msg_start_5[12] = " =30 POINTS\0";
  char msg_start_6[12] = " =20 POINTS\0";
  char msg_start_7[12] = " =10 POINTS\0";
  char msg_start_8[11] = "PUSH START\0";

  //Write title screen text
  centeredText(msg_start_1, 6);
  centeredText(msg_start_2, 8);
  centeredText(msg_start_3, 12);
  centeredText(msg_start_4, 14);
  centeredText(msg_start_5, 16);
  centeredText(msg_start_6, 18);
  centeredText(msg_start_7, 20);

  //Draw the invaders
  mo = SPR_addSprite(&mothership,106,112,TILE_ATTR(PAL1,0,FALSE,FALSE));
  se = SPR_addSprite(&small_enemy,112,128,TILE_ATTR(PAL2,0,FALSE,FALSE));
  me = SPR_addSprite(&medium_enemy,111,144,TILE_ATTR(PAL2,0,FALSE,FALSE));
  le = SPR_addSprite(&large_enemy,110,160,TILE_ATTR(PAL2,0,FALSE,FALSE));
  SPR_update();

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

//Function to check if the edge has been hit -if so, move the enemies appropriately
void edgeHit()
{
  //Loop through all the enemies
  for (i = 0; i < ENEMIES; i++)
	{
		Entity* e = &enemies[i];
    //If the enemy is alive then loop
    if(e->health > 0)
    {
      //If the enemy has hit the edge then loop
      if( (e->x+e->w) > RIGHT_EDGE || e->x < LEFT_EDGE)
      {
        //Set the new position for all of the enemies at once
        int j;
        for (j = 0; j < ENEMIES; j++)
        {
          Entity* e = &enemies[j];

          //Move the enemy down screen
          e->y = e->y +4;

          //If we are going left, go left, otherwise, go right
          if (dir == LEFT)
    			{
            e->velx = 1;
    			} else {
            e->x -= 1;
            e->velx = -1;
    			}
        }
        //Reverse the direction
        dir=!dir;

        //Exit the function
        return;
			}
		}
  }
}

//Function to animate enemies
void animateEnemies()
{
  Entity* e = &enemies[enemyAnimateTick];
  if(e->health > 0)
  {
    SPR_nextFrame(e->sprite);
  }
}

//Function to move enemies horizontally
void positionEnemies()
{
  for(i = 0; i < ENEMIES; i++)
	{
		Entity* e = &enemies[i];
		if(e->health > 0)
		{
      e->x += e->velx;
      SPR_setPosition(e->sprite,e->x,e->y);
		}
	}
}

//Function to determine if an object has collided - full credit to ohsat for this :)
int collideEntities(Entity* a, Entity* b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}

//Function to handle collisions
void handleCollisions(){
  Entity* p; //Player
  Entity* b; //Player Bullets
	Entity* e; //Enemies
	int i = 0;
	int j = 0;
	for(i = 0; i < MAX_PLAYER_BULLETS; i++)
	{
    b = &player_bullets[i];
    if(b->health > 0)
		{
      //Loop through the enemies - did we hit one?
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

            //Show an exploded_enemy sprite in it's place
            SPR_setVisibility(exploded_enemy.sprite,VISIBLE);
            SPR_setPosition(exploded_enemy.sprite,e->x,e->y);
            exploded_enemy_shown = TRUE;
            enemy_explode_tick = 0;
    			}
				}
      }
      e = &mothership_sprite;
      if(collideEntities( b, e ))
      {
        killEntity(e);
        killEntity(b);
        bulletsOnScreen--;
        //Play the mothership destroyed sound effect
        SND_stopPlayPCM_XGM(SOUND_PCM_CH1);
        XGM_startPlayPCM(MOTHERSHIP_DESTROYED_SFX, 1, SOUND_PCM_CH4);

        //Show an exploded_mothership sprite in it's place
        SPR_setVisibility(exploded_mothership.sprite,VISIBLE);
        SPR_setPosition(exploded_mothership.sprite,e->x,e->y);
        exploded_enemy_shown = TRUE;
        player_bullet_shot = FALSE;
        enemy_explode_tick = 0;
      }
		}
    p = &player;
    e = &plunger_bullet_sprite;
    if(collideEntities( e, p ))
    {
      killEntity(e);
      enemyBulletsOnScreen--;

      //Play the player destroyed sound effect
      XGM_startPlayPCM(PLAYER_DIED_SFX, 1, SOUND_PCM_CH4);
    }
    e = &rolling_bullet_sprite;
    if(collideEntities( e, p ))
    {
      killEntity(e);
      enemyBulletsOnScreen--;

      //Play the player destroyed sound effect
      XGM_startPlayPCM(PLAYER_DIED_SFX, 1, SOUND_PCM_CH4);
    }
    e = &squiggly_bullet_sprite;
    if(collideEntities( e, p ))
    {
      killEntity(e);
      enemyBulletsOnScreen--;

      //Play the player destroyed sound effect
      XGM_startPlayPCM(PLAYER_DIED_SFX, 1, SOUND_PCM_CH4);
    }
	}
}

void bulletTopHide()
{
  if (bullet_top_shown == TRUE && spritetick == 20) {
    bullet_top_screen.health = 0;
    SPR_setVisibility(bullet_top_screen.sprite,HIDDEN);
    spritetick = 0;
    bullet_top_shown = FALSE;
  }
  spritetick++;
}

void explodeEnemyHide()
{
  if (exploded_enemy_shown == TRUE && enemy_explode_tick == 20) {
    exploded_enemy.health = 0;
    SPR_setVisibility(exploded_enemy.sprite,HIDDEN);
    SPR_setVisibility(exploded_mothership.sprite,HIDDEN);
    enemy_explode_tick = 0;
    exploded_enemy_shown = FALSE;
  }
  enemy_explode_tick++;
}

bool mothership_sfx_playing = FALSE;
void showMothership()
{
  Entity* e;
  if (mothershiptick==600 && mothership_shown == FALSE)
  {
    e = &mothership_sprite;
    mothership_sprite.vely = 1;
    mothership_sprite.health = 1;
    reviveEntity(e);
    mothership_shown = TRUE;
  }
  if (mothership_shown == TRUE)
  {

    if (mothership_sfx_playing == FALSE){
        XGM_startPlayPCM(MOTHERSHIP_SFX, 1, SOUND_PCM_CH4);
        mothership_sfx_playing = TRUE;
    }
    mothership_sprite.x += mothership_sprite.vely;
    SPR_setPosition(mothership_sprite.sprite,mothership_sprite.x,mothership_sprite.y);
  }
  if (mothership_shown == TRUE && mothership_sprite.x + mothership_sprite.w > RIGHT_EDGE + 100)
  {
    mothership_sprite.vely = 0;
    mothership_sprite.x = -32;
    mothership_sprite.health = 0;
    SPR_setPosition(mothership_sprite.sprite,mothership_sprite.x,mothership_sprite.y);
    mothership_shown = FALSE;
    mothership_sfx_playing = FALSE;
    mothershiptick = 0;
  }
  mothershiptick++;
}

//Play the different music files on a loop
int music_note = 1;
void musicCycles()
{
  if (music_note == 1){
    XGM_startPlayPCM(MUSIC_NOTE_1_SFX, 1, SOUND_PCM_CH1);
    music_note++;
  } else if (music_note == 2){
    XGM_startPlayPCM(MUSIC_NOTE_2_SFX, 1, SOUND_PCM_CH2);
    music_note++;
  } else if (music_note == 3){
    XGM_startPlayPCM(MUSIC_NOTE_3_SFX, 1, SOUND_PCM_CH1);
    music_note++;
  } else if (music_note == 4){
    XGM_startPlayPCM(MUSIC_NOTE_4_SFX, 1, SOUND_PCM_CH2);
    music_note = 1;
  }
}

//The main timing loop for enemy movement, soundtrack playing, animation
void timingLoop()
{
  if (enemiesLeft == 0){
    SPR_end();
    SPR_reset	();
    waitMs(1000);
    if (level <= 24){
    level += 4;
    }

    SYS_reset();
  }
  if (enemiesLeft <= 1){
    if (tick == 5){
      musicCycles();
      if (dir == LEFT){
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
      }
      if (dir == RIGHT){
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
        positionEnemies();
      }

      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 2){
    if (tick == 7){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 3){
    if (tick == 9){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 4){
    if (tick == 11){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 5){
    if (tick == 12){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 6){
    if (tick == 13){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 7){
    if (tick == 14){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 8){
    if (tick == 16){
      musicCycles();
      positionEnemies();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 10){
    if (tick == 19){
      musicCycles();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 13){
    if (tick == 21){
      musicCycles();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 17){
    if (tick == 24){
      musicCycles();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 22){
    if (tick == 28){
      musicCycles();
      positionEnemies();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 28){
    if (tick == 34){
      musicCycles();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 36){
    if (tick == 39){
      musicCycles();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 43){
    if (tick == 46){
      musicCycles();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft < 50){
    if (tick == 52){
      musicCycles();
      positionEnemies();
      edgeHit();
      tick = 0;
    }
  }
  if (enemiesLeft <= 55){
    if (tick == 60){
      musicCycles();
      positionEnemies();
      //Shoot an enemy bullet
      shootEnemyBullet();
      time_to_shoot = TRUE;
      edgeHit();
      tick = 0;
    }
  }
  enemyAnimateTick++;
  tick++;
  if (enemyAnimateTick > ENEMIES)
  {
    enemyAnimateTick = 0;
  }
}

void gameInit()
{
  //Bullet "hole" at top of screen when you miss
  bullet_top_screen.x = -16;
  bullet_top_screen.y = -16;
  bullet_top_screen.health = 0;
  bullet_top_screen.sprite = SPR_addSprite(&bullet_top,bullet_top_screen.x,bullet_top_screen.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Explosion sprite, when you hit an enemy
  exploded_enemy.x = -16;
  exploded_enemy.y = -16;
  exploded_enemy.health = 0;
  exploded_enemy.sprite = SPR_addSprite(&alien_exploding,exploded_enemy.x,exploded_enemy.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Mothership
  mothership_sprite.x = -32;
  mothership_sprite.y = 24;
  mothership_sprite.w = 19;
  mothership_sprite.health = 0;
  mothership_sprite.velx = 2;
  mothership_sprite.sprite = SPR_addSprite(&mothership_red,mothership_sprite.x,mothership_sprite.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Explosion sprite, when you hit an enemy
  exploded_mothership.x = -16;
  exploded_mothership.y = -16;
  exploded_mothership.health = 0;
  exploded_mothership.sprite = SPR_addSprite(&mothership_exploding,exploded_mothership.x,exploded_mothership.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Rolling Bullet sprite
  rolling_bullet_sprite.x = -16;
  rolling_bullet_sprite.y = -16;
  rolling_bullet_sprite.w = 1;
  rolling_bullet_sprite.health = 0;
  rolling_bullet_sprite.sprite = SPR_addSprite(&rolling_bullet,rolling_bullet_sprite.x,rolling_bullet_sprite.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Plunger Bullet sprite
  plunger_bullet_sprite.x = -16;
  plunger_bullet_sprite.y = -16;
  plunger_bullet_sprite.w = 1;
  plunger_bullet_sprite.health = 0;
  plunger_bullet_sprite.sprite = SPR_addSprite(&plunger_bullet,plunger_bullet_sprite.x,plunger_bullet_sprite.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Squiggly Bullet sprite
  squiggly_bullet_sprite.x = -16;
  squiggly_bullet_sprite.y = -16;
  squiggly_bullet_sprite.w = 1;
  squiggly_bullet_sprite.health = 0;
  squiggly_bullet_sprite.sprite = SPR_addSprite(&squiggly_bullet,squiggly_bullet_sprite.x,squiggly_bullet_sprite.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Create all shield sprites
  Entity* s = shields;
  for(i = 0; i < SHIELDS; i++){
    s->x = 84 + i*44;
    s->y = 166;
    s->w = 22;
    s->h = 16;
    s->sprite = SPR_addSprite(&shield,s->x,s->y,TILE_ATTR(PAL1,0,FALSE,FALSE));
    sprintf(s->name, "Sh%d",i);
    s++;
  }

  //Add the player
  player.x = LEFT_EDGE;
  player.y = 192;
  player.health = 1;
  player.velx = 2;
  player.w = 16;
  player.sprite = SPR_addSprite(&player_ship,player.x,player.y,TILE_ATTR(PAL1,0,FALSE,FALSE));

  //Create all player bullet sprites
  Entity* b = player_bullets;
  for(i = 0; i < MAX_PLAYER_BULLETS; i++){
    b->x = 0;
    b->y = -10;
    b->w = 1;
    b->h = 4;
    b->sprite = SPR_addSprite(&player_bullet,player_bullets[0].x,player_bullets[0].y,TILE_ATTR(PAL1,0,FALSE,FALSE));
    sprintf(b->name, "Bu%d",i);
    b++;
  }

  //Update all the sprites - ran here, as we are adding *lots* of sprites at once.
  SPR_update();

  //Create all enemy sprites
  Entity* e = enemies;
  int c = 1; //To track the column - we need to reset to 1 if it reaches > 11
  for(i = 0; i < ENEMIES; i++)
  {
    if(i <=10){
      e->x = 72 + i*16;
      e->y = 56 + level;
      e->w = 8;
      e->h = 8;
      e->velx = 1;
      e->row = 1;
      e->column = c;
      e->health = 1;
      e->points = 30;
      e->sprite = SPR_addSprite(&small_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
      sprintf(e->name, "En%d",i);
  		enemiesLeft++;
      e++;
		} else if (i<=21){
      e->x = 70 + (i-11)*16;
	    e->y = 72 + level;
	    e->w = 11;
	    e->h = 8;
	    e->velx = 1;
      e->row = 2;
      e->column = c;
	    e->health = 1;
      e->points = 20;
	    e->sprite = SPR_addSprite(&medium_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
		} else if (i<=32){
      e->x = 70 + (i-22)*16;
	    e->y = 88 + level;
	    e->w = 11;
	    e->h = 8;
	    e->velx = 1;
      e->row = 3;
      e->column = c;
	    e->health = 1;
      e->points = 20;
	    e->sprite = SPR_addSprite(&medium_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    } else if (i<=43){
      e->x = 70 + (i-33)*16;
	    e->y = 104 + level;
	    e->w = 12;
	    e->h = 8;
	    e->velx = 1;
      e->row = 4;
      e->column = c;
	    e->health = 1;
      e->points = 10;
	    e->sprite = SPR_addSprite(&large_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    } else if (i<=55){
      e->x = 70 + (i-44)*16;
	    e->y = 120 + level;
	    e->w = 12;
	    e->h = 8;
	    e->velx = 1;
      e->row = 5;
      e->column = c;
	    e->health = 1;
      e->points = 10;
	    e->sprite = SPR_addSprite(&large_enemy,e->x,e->y,TILE_ATTR(PAL2,0,FALSE,FALSE));
	    sprintf(e->name, "En%d",i);
			enemiesLeft++;
			e++;
    }
    if (c > 11){
      c = 1;
    } else {
      c++;
    }
  }

  //Update all sprites
  SPR_update();

}

//Debugging function
char tick_str[8];
char enemies_str[8];
char movespeed_str[8];
char bulletType_str[8];
void debugDisplay()
{
  sprintf(tick_str,"%d",tick);
  sprintf(enemies_str,"%d",enemiesLeft);
  sprintf(movespeed_str,"%d",movespeed);
  sprintf(bulletType_str,"%d",bulletType);
  VDP_clearText(0,21,8);
  VDP_drawText(bulletType_str,0,21);
  VDP_clearText(0,22,8);
  VDP_drawText(tick_str,0,22);
  VDP_clearText(0,23,8);
  VDP_drawText(enemies_str,0,23);
  VDP_clearText(0,24,8);
  VDP_drawText(movespeed_str,0,24);
  VDP_showFPS(TRUE);
}

int main()
{

  //Draw the initial scoreboard
	updateScoreDisplay();

  //Initialise the sprite engine
  SPR_init();

  //Set the palette 1 colours from the player sprite
  VDP_setPalette(PAL1, player_ship.palette->data);
  VDP_setPalette(PAL2, small_enemy.palette->data);

  //Check to see if the game is running - if not, show the start screen
  if (game_running == FALSE)
  {
    startScreen();
  }

  /*Draw the background image*/
  VDP_drawImageEx(BG_B, &background, TILE_ATTR_FULL(PAL1, 0, 0, 0, 1), 0, 0, 1, DMA);

  //Setup the sound effects and music
  XGM_setPCM(PLAYER_BULLET_SFX, player_bullet_sfx, sizeof(player_bullet_sfx));
  XGM_setPCM(ALIEN_DESTROYED_SFX, alien_destroyed_sfx, sizeof(alien_destroyed_sfx));
  XGM_setPCM(MUSIC_NOTE_1_SFX, music_note_1, sizeof(music_note_1));
  XGM_setPCM(MUSIC_NOTE_2_SFX, music_note_2, sizeof(music_note_2));
  XGM_setPCM(MUSIC_NOTE_3_SFX, music_note_3, sizeof(music_note_3));
  XGM_setPCM(MUSIC_NOTE_4_SFX, music_note_4, sizeof(music_note_4));
  XGM_setPCM(MOTHERSHIP_SFX, mothership_sfx, sizeof(mothership_sfx));
  XGM_setPCM(MOTHERSHIP_DESTROYED_SFX, mothership_destroyed_sfx, sizeof(mothership_destroyed_sfx));
  XGM_setPCM(PLAYER_DIED_SFX, player_died_sfx, sizeof(player_died_sfx));

  gameInit();

  while(game_running == TRUE)
  {
		//Watch for joypad inputs (direction controls)
    doActionJoy(JOY_1, JOY_readJoypad(JOY_1));

    //Debugging - show FPS on screen, ticks etc
		debugDisplay();

    //Animate the enemies
    animateEnemies();

    //Update the bullet positions
    positionBullets();

    //Wait for vsync to stop flickering
    VDP_waitVSync();

    //Update all the sprites on screen
    SPR_update();

    //Detect if items have collided
    handleCollisions();

    //Check to see if we should remove the missed bullet sprite
    bulletTopHide();

    //Check to see if we should remove the exploded enemy sprite
    explodeEnemyHide();

    //Check to see if we should show the mothership
    showMothership();

    //Main game timing loop
    timingLoop();

  }
  return 0;
}
