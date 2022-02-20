//
// Created by steven on 2/20/2022.
//

#include <cstdlib>
#include <assert.h>
#include "TileMap.h"
#include "raylib.h"

TileMap::TileMap(const char* filePath, float worldTileSize) {
    this->filePath = filePath;
    this->fileSize = 0;
    this->fileData = LoadFileData(filePath, &fileSize);
    assert(fileSize > 0);

    // assume square tiles
    this->tileWidth = (int)worldTileSize;
    this->tileHeight = (int)worldTileSize;
    this->tileWidthInverse = 1.0f / worldTileSize;
    this->tileHeightInverse = 1.0f / worldTileSize;

    // assume square map & power of 2 size
    double s = sqrt(fileSize);
    assert((s - (int)s) > -0.001 && (s - (int)s) < 0.001);
    this->width = (int)s;
    this->height = width;

    this->offsetX = width * -(worldTileSize / 2);
    this->offsetY = height * -(worldTileSize / 2);
}

TileInfo TileMap::getTileAtWorldCoords(float x, float y) {
    int tx = (int)((x - offsetX) * tileWidthInverse);
    int ty = (int)((y - offsetY) * tileHeightInverse);
    TileInfo tileInfo = {};
    if (tx >= 0 && ty >= 0 && tx < width && ty < height) {
        tileInfo.tileType = fileData[(ty * width) + tx];
    }
    tileInfo.x = (tx * tileWidth + offsetX);
    tileInfo.y = (ty * tileHeight + offsetY);
    return tileInfo;
}