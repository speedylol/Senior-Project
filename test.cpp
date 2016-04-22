//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//Dimension of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue);

		//Set the blending
		void setBlendMode( SDL_BlendMode blending );

		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};


//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Box collision detector
bool checkCollision( SDL_Rect a, SDL_Rect b );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gFooTexture;
LTexture gBackgroundTexture;
LTexture gModulatedTexture;

//Scene sprites
const int WALKING_ANIMATION_FRAMES = 4;
SDL_Rect gSpriteClips[ WALKING_ANIMATION_FRAMES ];

LTexture gPlayerTexture;
LTexture gSpriteSheetTexture;


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

/*
	Class to represent the player
*/

class Player
{
public:
	static const int PLAYER_WIDTH = 20;
	static const int PLAYER_HEIGHT = 20;

	static const int PLAYER_VEL = 2;

	Player();

	void handleEvent( SDL_Event& e);

	void move();

	void render();

private:
	//the X and Y offsets of the player
	int mPosX, mPosY;

	//The velocity of the player
	int mVelX, mVelY;
};

Player::Player() {

	mPosX = 0;
	mPosY = 0;

	mVelX = 0;
	mVelY = 0;
}



void Player::handleEvent( SDL_Event& e ) 
{
	//if a key is pressed
	if(e.type == SDL_KEYDOWN && e.key.repeat == 0 )
	{
		switch(e.key.keysym.sym)
		{

		case SDLK_w: mVelY -= PLAYER_VEL; break;
		case SDLK_s: mVelY += PLAYER_VEL; break;
		case SDLK_a: mVelX -= PLAYER_VEL; break;
		case SDLK_d: mVelX += PLAYER_VEL; break;

		}
	} else if( e.type == SDL_KEYUP && e.key.repeat == 0) {

		switch( e.key.keysym.sym )
		{

		case SDLK_w: mVelY -= PLAYER_VEL; break;
		case SDLK_s: mVelY += PLAYER_VEL; break;
		case SDLK_a: mVelX -= PLAYER_VEL; break;
		case SDLK_d: mVelX += PLAYER_VEL; break;

		}
	}
}

void Player::move() 
{

	mPosX += mVelX;

	if( (mPosX < 0) || (mPosX + PLAYER_WIDTH > SCREEN_WIDTH)) {
		mPosX -= mVelX;
	}

	mPosY += mVelY;
	if( (mPosY < 0) || (mPosY + PLAYER_HEIGHT > SCREEN_HEIGHT)) {
		mPosY -= mVelY;
	}
}

void Player::render() 
{
	gPlayerTexture.render( mPosX, mPosY );
}

/*
	Funtions of the LTexture class
*/

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) 
{
	SDL_SetTextureColorMod( mTexture, red, green, blue);
}

void LTexture::setBlendMode( SDL_BlendMode blending ) 
{
	SDL_SetTextureBlendMode( mTexture, blending);
}

void LTexture::setAlpha( Uint8 alpha ) 
{
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL ) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//render to screen
	SDL_RenderCopy( gRenderer, mTexture, clip, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}


bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load Foo' texture
	if( !gSpriteSheetTexture.loadFromFile("foo2.png")) {
		success = false;
	} else {

		//Set sprite clips
		gSpriteClips[0].x = 0;
		gSpriteClips[0].y = 0;
		gSpriteClips[0].w = 90;
		gSpriteClips[0].h = 104;

		gSpriteClips[1].x = 90;
		gSpriteClips[1].y = 0;
		gSpriteClips[1].w = 105;
		gSpriteClips[1].h = 104;

		gSpriteClips[2].x = 195;
		gSpriteClips[2].y = 0;
		gSpriteClips[2].w = 75;
		gSpriteClips[2].h = 104;

		gSpriteClips[3].x = 270;
		gSpriteClips[3].y = 0;
		gSpriteClips[3].w = 90;
		gSpriteClips[3].h = 104;
	}

	if( !gBackgroundTexture.loadFromFile("bg.png")) {
		success = false;
	}

	return success;
}

bool checkCollision( SDL_Rect a, SDL_Rect b) 
{
	//sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//calculating sides of Rectangle A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//calculating the sides of Rectangle B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	if( bottomA <= topB )
		return false;

	if( topA >= bottomB)
		return false;

	if( rightA <= leftB )
		return false;

	if( leftA >= rightB )
		return false;

	return true;
}

void close()
{
	//Free loaded images
	gFooTexture.free();
	gBackgroundTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main( int argc, char* args[] )
{

	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;
			bool walking = false;

			//Event handler
			SDL_Event e;

			Player player;

			SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

			const int velX = 10;

			int frame = 0;
			int x = 0;
			int y = 0;

			SDL_Rect collider = gSpriteClips[frame];

			SDL_Rect wall;
			wall.x = 300;
			wall.y = 40;
			wall.w = 40;
			wall.h = 400; 

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					} else {
						
						if(e.type == SDL_KEYDOWN) {
							
							switch(e.key.keysym.sym) {
							case SDLK_w: y -= 30; break;
							case SDLK_d: walking = true; x += velX; break;
							case SDLK_a: walking = true; x -= velX; break;

							}
						} else if( e.type == SDL_KEYUP) {
							walking = false;
						}
					}

				}

				collider.x = x;

				if( x < 0 || ( x + 90 > LEVEL_WIDTH ) || checkCollision(collider, wall)) {
					x -= velX;
					collider.x = x;
				}
				

				camera.x = (x + 90 / 2 ) - SCREEN_WIDTH / 2;
				camera.y = (y + 104 / 2 ) - SCREEN_HEIGHT / 2;

				if(camera.x < 0)
					camera.x = 0;

				if(camera.y < 0)
					camera.y = 0;

				if( camera.x > LEVEL_WIDTH - camera.w )
					camera.x = LEVEL_WIDTH - camera.w;

				if( camera.y > LEVEL_HEIGHT - camera.h)
					camera.y = LEVEL_HEIGHT - camera.h;

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );	

				gBackgroundTexture.render(0, 0, &camera);
				
				//Render current frame
				SDL_Rect* currentClip = &gSpriteClips[ frame / 4 ];
				gSpriteSheetTexture.render(  (x - camera.x),  (y - camera.y) + 366, currentClip );


				// gSpriteSheetTexture.render( (10) + x, ( (SCREEN_HEIGHT - currentClip->h ) / 2) + y, currentClip );

				SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderDrawRect( gRenderer, &wall); 

				//Update screen
				SDL_RenderPresent( gRenderer );

				

				if(walking) {
					//Go to next frame
					++frame;

					//Cycle animation
					if( frame / 4 >= WALKING_ANIMATION_FRAMES )
					{
						frame = 0;
					}
				} else {
					frame = 0;
				}
				
				
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}