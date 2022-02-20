
#ifndef MONSTERS_TILEMAP_H
#define MONSTERS_TILEMAP_H

struct TileInfo {
        float x;
        float y;
        char tileType;
};

class TileMap {

private:

    const char* filePath;
    unsigned char * fileData;
    unsigned int fileSize;
    int width;
    int height;
    float offsetX;
    float offsetY;
    float tileWidthInverse;
    float tileHeightInverse;

public:
    int tileWidth;
    int tileHeight;
    TileMap(const char* filePath, float tileSize);
    TileInfo getTileAtWorldCoords(float x, float y);

};


#endif //MONSTERS_TILEMAP_H
