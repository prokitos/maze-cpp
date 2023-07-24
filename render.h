void playerControl();
void mainEngine();
void addTextureToMapCache(int y, int x, int* roof, char* screen, short* nShade, int* floor);
void rayOnWall(int* rayPosX, int* rayPosY, double* eyeX, double* eyeY, bool* isHitWall, bool* rayOnBoundary);