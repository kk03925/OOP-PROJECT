/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

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
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL );

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

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Walking animation
const int STANDING_ANIMATION_FRAMES = 2;
const int WALKING_RIGHT_ANIMATION_FRAMES = 12;
const int WALKING_LEFT_ANIMATION_FRAMES = 12;
const int THROWING_RIGHT_ANIMATION_FRAMES = 4;
const int THROWING_LEFT_ANIMATION_FRAMES = 4;
const int DUCKING_ANIMATION_FRAMES = 1;
SDL_Rect gWalkingRightSpriteClips[ WALKING_RIGHT_ANIMATION_FRAMES ];
SDL_Rect gWalkingLeftSpriteClips[ WALKING_LEFT_ANIMATION_FRAMES ];
SDL_Rect gThrowingLeftSpriteClips[ THROWING_LEFT_ANIMATION_FRAMES ];
SDL_Rect gThrowingRightSpriteClips[ THROWING_RIGHT_ANIMATION_FRAMES ];
SDL_Rect gDuckingSpriteClips[ DUCKING_ANIMATION_FRAMES ];
SDL_Rect gStandingSpriteClips [ STANDING_ANIMATION_FRAMES];

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

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy( gRenderer, mTexture, clip, &renderQuad );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
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
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
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

	//Load sprite sheet texture
	if( !gSpriteSheetTexture.loadFromFile( "transparent_aladdin_sprite_sheet.png" ) )
	{
		printf( "Failed to load walking animation texture!\n" );
		success = false;
	}
	else
	{
		//Set sprite clips
		gStandingSpriteClips [ 0 ].x = 48;
		gStandingSpriteClips [ 0 ].y = 65;
		gStandingSpriteClips [ 0 ].w = 128;
		gStandingSpriteClips [ 0 ].h = 327;

		gStandingSpriteClips [ 1 ].x = 2974;
		gStandingSpriteClips [ 1 ].y = 62;
		gStandingSpriteClips [ 1 ].w = 125;
		gStandingSpriteClips [ 1 ].h = 328;

		gWalkingRightSpriteClips[ 0 ].x =   62;
		gWalkingRightSpriteClips[ 0 ].y =   439;
		gWalkingRightSpriteClips[ 0 ].w =  176;
		gWalkingRightSpriteClips[ 0 ].h = 310;

		gWalkingRightSpriteClips[ 1 ].x =   268;
		gWalkingRightSpriteClips[ 1 ].y =   441;
		gWalkingRightSpriteClips[ 1 ].w =  144;
		gWalkingRightSpriteClips[ 1 ].h = 308;

		gWalkingRightSpriteClips[ 2 ].x =   453;
		gWalkingRightSpriteClips[ 2 ].y =   436;
		gWalkingRightSpriteClips[ 2 ].w =  142;
		gWalkingRightSpriteClips[ 2 ].h = 316;

		gWalkingRightSpriteClips[ 3 ].x =   620;
		gWalkingRightSpriteClips[ 3 ].y =   440;
		gWalkingRightSpriteClips[ 3 ].w =  174;
		gWalkingRightSpriteClips[ 3 ].h = 321;

		gWalkingRightSpriteClips[ 4 ].x =   826;
		gWalkingRightSpriteClips[ 4 ].y =   449;
		gWalkingRightSpriteClips[ 4 ].w =  218;
		gWalkingRightSpriteClips[ 4 ].h = 307;

		gWalkingRightSpriteClips[ 5 ].x =   1072;
		gWalkingRightSpriteClips[ 5 ].y =   437;
		gWalkingRightSpriteClips[ 5 ].w =  192;
		gWalkingRightSpriteClips[ 5 ].h =  315;

		gWalkingRightSpriteClips[ 6 ].x =   1313;
		gWalkingRightSpriteClips[ 6 ].y =   439;
		gWalkingRightSpriteClips[ 6 ].w =  164;
		gWalkingRightSpriteClips[ 6 ].h = 308;

		gWalkingRightSpriteClips[ 7 ].x =   1507;
		gWalkingRightSpriteClips[ 7 ].y =   440;
		gWalkingRightSpriteClips[ 7 ].w =  140;
		gWalkingRightSpriteClips[ 7 ].h = 308;

		gWalkingRightSpriteClips[ 8 ].x =   1671;
		gWalkingRightSpriteClips[ 8 ].y =   425;
		gWalkingRightSpriteClips[ 8 ].w =  152;
		gWalkingRightSpriteClips[ 8 ].h = 320;

		gWalkingRightSpriteClips[ 9 ].x =   1852;
		gWalkingRightSpriteClips[ 9 ].y =   417;
		gWalkingRightSpriteClips[ 9 ].w =  185;
		gWalkingRightSpriteClips[ 9 ].h =   327;

		gWalkingRightSpriteClips[ 10 ].x =   2060;
		gWalkingRightSpriteClips[ 10 ].y =   442;
		gWalkingRightSpriteClips[ 10 ].w =  225;
		gWalkingRightSpriteClips[ 10 ].h = 310;

		gWalkingRightSpriteClips[ 11 ].x =   2328;
		gWalkingRightSpriteClips[ 11 ].y =   428;
		gWalkingRightSpriteClips[ 11 ].w =  184;
		gWalkingRightSpriteClips[ 11 ].h = 325;

        gDuckingSpriteClips [ 0 ].x = 1950;
		gDuckingSpriteClips [ 0 ].y = 208;
		gDuckingSpriteClips [ 0 ].w = 139;
		gDuckingSpriteClips [ 0 ].h = 178;

		gDuckingSpriteClips [ 1 ].x = 3140;
		gDuckingSpriteClips [ 1 ].y = 210;
		gDuckingSpriteClips [ 1 ].w = 136;
		gDuckingSpriteClips [ 1 ].h = 178;



		gWalkingLeftSpriteClips[ 0 ].x =   4990;
		gWalkingLeftSpriteClips[ 0 ].y =   435;
		gWalkingLeftSpriteClips[ 0 ].w =  178;
		gWalkingLeftSpriteClips[ 0 ].h = 316;

		gWalkingLeftSpriteClips[ 1 ].x =   4816;
		gWalkingLeftSpriteClips[ 1 ].y =   444;
		gWalkingLeftSpriteClips[ 1 ].w =  149;
		gWalkingLeftSpriteClips[ 1 ].h = 302;

		gWalkingLeftSpriteClips[ 2 ].x =   4636;
		gWalkingLeftSpriteClips[ 2 ].y =   436;
		gWalkingLeftSpriteClips[ 2 ].w =  141;
		gWalkingLeftSpriteClips[ 2 ].h = 318;

		gWalkingLeftSpriteClips[ 3 ].x =   4430;
		gWalkingLeftSpriteClips[ 3 ].y =   440;
		gWalkingLeftSpriteClips[ 3 ].w =  178;
		gWalkingLeftSpriteClips[ 3 ].h = 324;


		gWalkingLeftSpriteClips[ 4 ].x =   4185;
		gWalkingLeftSpriteClips[ 4 ].y =   446;
		gWalkingLeftSpriteClips[ 4 ].w =  216;
		gWalkingLeftSpriteClips[ 4 ].h = 311;

		gWalkingLeftSpriteClips[ 5 ].x =   3962;
		gWalkingLeftSpriteClips[ 5 ].y =   436;
		gWalkingLeftSpriteClips[ 5 ].w =  196;
		gWalkingLeftSpriteClips[ 5 ].h = 319;

		gWalkingLeftSpriteClips[ 6 ].x =   3752;
		gWalkingLeftSpriteClips[ 6 ].y =   443;
		gWalkingLeftSpriteClips[ 6 ].w =  167;
		gWalkingLeftSpriteClips[ 6 ].h = 306;

		gWalkingLeftSpriteClips[ 7 ].x =   3586;
		gWalkingLeftSpriteClips[ 7 ].y =   442;
		gWalkingLeftSpriteClips[ 7 ].w =  137;
		gWalkingLeftSpriteClips[ 7 ].h = 306;

		gWalkingLeftSpriteClips[ 8 ].x =   3406;
		gWalkingLeftSpriteClips[ 8 ].y =   423;
		gWalkingLeftSpriteClips[ 8 ].w =  153;
		gWalkingLeftSpriteClips[ 8 ].h = 324;

		gWalkingLeftSpriteClips[ 9 ].x =   3187;
		gWalkingLeftSpriteClips[ 9 ].y =   415;
		gWalkingLeftSpriteClips[ 9 ].w =  189;
		gWalkingLeftSpriteClips[ 9 ].h = 328;

		gWalkingLeftSpriteClips[ 10 ].x =   2945;
		gWalkingLeftSpriteClips[ 10 ].y =   442;
		gWalkingLeftSpriteClips[ 10 ].w =  226;
		gWalkingLeftSpriteClips[ 10 ].h = 311;

		gWalkingLeftSpriteClips[ 11 ].x =   2720;
		gWalkingLeftSpriteClips[ 11 ].y =   432;
		gWalkingLeftSpriteClips[ 11 ].w =  181;
		gWalkingLeftSpriteClips[ 11 ].h =   320;



	}

	return success;
}

void close()
{
	//Free loaded images
	gSpriteSheetTexture.free();

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

			//Event handler
			SDL_Event e;

			//Current animation frame
			int WalkingFrame = 0;

			int orientation = 0;

			SDL_Rect* currentClip = &gStandingSpriteClips[orientation];

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }


                    if (e.type==SDL_KEYDOWN)
                    {
                        if (e.key.keysym.sym==SDLK_RIGHT)
                        {
                            orientation = 0;
                            cout<<"right key was pressed"<<endl;
                            currentClip = &gWalkingRightSpriteClips[ WalkingFrame / 4 ];
                            //Go to next frame
                            ++WalkingFrame;
                        }
                        else if (e.key.keysym.sym == SDLK_DOWN)
                        {
                            cout<<"down key was pressed"<<endl;
                            cout<<orientation<<endl;
                            currentClip = &gDuckingSpriteClips[ orientation ];
                        }
                        else if (e.key.keysym.sym == SDLK_LEFT)
                        {
                            cout<<"left was pressed"<<endl;
                            orientation = 1;
                            currentClip = &gWalkingLeftSpriteClips[WalkingFrame / 4];
                            ++WalkingFrame;
                        }
                    }

				 if (e.type == SDL_KEYUP)
                 {
                    currentClip = &gStandingSpriteClips[ orientation ];
                    WalkingFrame=0;
                 }



				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Render current frame
				gSpriteSheetTexture.render( ( SCREEN_WIDTH - currentClip->w ) / 2, ( SCREEN_HEIGHT - currentClip->h ) / 2, currentClip );

				//Update screen
				SDL_RenderPresent( gRenderer );


				//Cycle animation
				if( WalkingFrame / 4 >= WALKING_RIGHT_ANIMATION_FRAMES)
				{
					WalkingFrame = 0;
				}

			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
}
