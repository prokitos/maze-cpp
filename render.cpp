#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <Windows.h>
#include "render.h"

using namespace std;

// console size
int screenWidth = 120;			
int screenHeight = 40;	

// map size
int mapWidth = 16;				
int mapHeight = 16;

double playerPosX = 14.7;			
double playerPosY = 5.09;
double playerVisDirection = 0.0;			
double playerVisAngle = 3.0/ 4.0;
double maxRender = 12.0;			
double playerMoveSpeed = 5.0;			

wstring map;
bool isInfinity = true;

void mainEngine()
{
    // Screen Buffer
	char   *screen = new char  [screenWidth*screenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Create Map
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	while (isInfinity)
	{

        playerControl();

		for (int x = 0; x < screenWidth; x++)
		{
            // угол каждого луча, и скорость движения лучей
			double rayAngle = (playerVisDirection - playerVisAngle/2.0) + ((double)x / (double)screenWidth) * playerVisAngle;
			double rayStep = 0.02;		  									
			double distanceToWall = 0.0; 

            // попадание луча в стену, или попадание между двух стен
			bool isHitWall = false;		
			bool rayOnBoundary = false;		

			double eyeX = sinf(rayAngle);
			double eyeY = cosf(rayAngle);

			while (!isHitWall && distanceToWall < maxRender)
			{
				distanceToWall += rayStep;

                // интовое представление текущей позиции луча, для сравнения с картой
				int rayPosX = (int)(playerPosX + eyeX * distanceToWall);
				int rayPosY = (int)(playerPosY + eyeY * distanceToWall);
				
				// если луч вышел за границы карты
				if (rayPosX < 0 || rayPosX >= mapWidth || rayPosY < 0 || rayPosY >= mapHeight)
				{
					isHitWall = true;		
					distanceToWall = maxRender;
				}
				else
				{
					// иначе смотрим если корды луча на кордах стены
					if (map.c_str()[rayPosX * mapWidth + rayPosY] == '#')
						rayOnWall(&rayPosX, &rayPosY, &eyeX, &eyeY, &isHitWall, &rayOnBoundary);
				}
			}
		
			// расстояние от потолка до пола
			int roof = (double)(screenHeight/2.0) - screenHeight / ((double)distanceToWall);
			int floor = screenHeight - roof;

			// текстура стен
			short nShade = ' ';
			if (distanceToWall <= maxRender / 4.0)			nShade = '0';
			else if (distanceToWall < maxRender / 3.0)		nShade = 'H';
			else if (distanceToWall < maxRender / 2.0)		nShade = 'I';
			else if (distanceToWall < maxRender)			nShade = ':';
			else											nShade = ' ';

            // очень важно !!!
			if (rayOnBoundary)		nShade = ' ';
			
            // перебор по высоте внутри перебора по ширине
			for (int y = 0; y < screenHeight; y++)
			{
				addTextureToMapCache(y, x, &roof, screen, &nShade, &floor);
			}
		}

		// отображение
		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}
    
}; 

// луч попал в стену
void rayOnWall(int* rayPosX, int* rayPosY, double* eyeX, double* eyeY, bool* isHitWall, bool* rayOnBoundary)
{
	*isHitWall = true;

	vector<pair<float, float>> p;

	// ребра
	for (int tx = 0; tx < 2; tx++)
		for (int ty = 0; ty < 2; ty++)
		{
			float vy = (float)*rayPosY + ty - playerPosY;
			float vx = (float)*rayPosX + tx - playerPosX;
			float d = sqrt(vx*vx + vy*vy); 
			float dot = (*eyeX * vx / d) + (*eyeY * vy / d);
			p.push_back(make_pair(d, dot));
		}

	// видим только 2 ребра одновременно, поэтому сортируем
	sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });
	
	// угол различия ребра, и попадание на границе между стен
	double boundAngle = 0.001;
	if (acos(p.at(0).second) < boundAngle) *rayOnBoundary = true;
	if (acos(p.at(1).second) < boundAngle) *rayOnBoundary = true;
	if (acos(p.at(2).second) < boundAngle) *rayOnBoundary = true;
	
};


// добавление в буфер экрана всех элементов, но не отрисовка
void addTextureToMapCache(int y, int x, int* roof, char* screen, short* nShade, int* floor)
{
	// Либо пустота, либо стена, либо пол
	if(y <= *roof)
		screen[y*screenWidth + x] = ' ';
	else if(y > *roof && y <= *floor)
		screen[y*screenWidth + x] = *nShade;
	else
	{				
		// текстура пола
		double b = 1.0 - (((double)y -screenHeight/2.0) / ((double)screenHeight / 2.0));
		if (b < 0.25)		*nShade = '#';
		else if (b < 0.5)	*nShade = 'x';
		else if (b < 0.75)	*nShade = '.';
		else if (b < 0.9)	*nShade = '-';
		else				*nShade = ' ';
		screen[y*screenWidth + x] = *nShade;
	}
}

// управление персом
void playerControl()
{

    if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
        playerVisDirection -= (playerMoveSpeed * 0.0002) ;

    if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        playerVisDirection += (playerMoveSpeed * 0.0002) ;
    
    if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
    {
        playerPosX += sinf(playerVisDirection) * playerMoveSpeed * 0.0002;
        playerPosY += cosf(playerVisDirection) * playerMoveSpeed * 0.0002;
        if (map.c_str()[(int)playerPosX * mapWidth + (int)playerPosY] == '#')
        {
            playerPosX -= sinf(playerVisDirection) * playerMoveSpeed * 0.0002;
            playerPosY -= cosf(playerVisDirection) * playerMoveSpeed * 0.0002 ;
        }			
    }

    if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
    {
        playerPosX -= sinf(playerVisDirection) * playerMoveSpeed * 0.0002 ;
        playerPosY -= cosf(playerVisDirection) * playerMoveSpeed * 0.0002 ;
        if (map.c_str()[(int)playerPosX * mapWidth + (int)playerPosY] == '#')
        {
            playerPosX += sinf(playerVisDirection) * playerMoveSpeed * 0.0002 ;
            playerPosY += cosf(playerVisDirection) * playerMoveSpeed * 0.0002 ;
        }
    }

	if (GetAsyncKeyState((unsigned short)VK_ESCAPE) & 0x8000)
	{
		isInfinity = false;
	}

};