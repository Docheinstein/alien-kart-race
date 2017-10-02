#include <SFML/Graphics.hpp>

#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include "map.h"
#include "const.h"
#include "level.h"
#include "playerkart.h"
#include "fileutil.h"
#include "matrixutil.h"
#include "resourceutil.h"
#include "geometryutil.h"
#include "perspectiveutil.h"
#include "viewutil.h"

#define LOG_TAG "{Map} ";
#define CAN_LOG 1

enum SectorsFileFieldIndex {
	ULy,
	ULx,
	URy,
	URx,
	DRy,
	DRx,
	DLy,
	DLx,
	Type
};

// Number of tiles that will be rendered in each direction.
const int MATRIX_RENDERED_TILES_RADIUS = 32;

// Total amount of tiles rendered per axis.
const int MATRIX_RENDERED_TILES_DIAMETER = MATRIX_RENDERED_TILES_RADIUS * 2 + 1;

Map::Map(sf::RenderWindow *window, Level *level) {
	mWindow = window;
	mLevel = level;

	mMatrix = NULL;

	mDebugGridImage = new sf::Image();
	mDebugGridTexture = new sf::Texture();
	mDebugGridSprite = new sf::Sprite();
	mRenderedTiles = new sf::VertexArray();
	mTileset = new sf::Texture();

	const double OUT_OF_SCREEN_TILES_ESTIMATED_PERCENTAGE = 0.5;
	mRenderedTiles->setPrimitiveType(sf::Quads);
    mRenderedTiles->resize(static_cast<int>(
		MATRIX_RENDERED_TILES_DIAMETER * MATRIX_RENDERED_TILES_DIAMETER * 4 *
		OUT_OF_SCREEN_TILES_ESTIMATED_PERCENTAGE));
}

Map::~Map() {
	MatrixUtil::deleteMatrix<Tile>(mMatrix, mRowCount);
	delete mDebugGridSprite;
	delete mDebugGridTexture;
	delete mDebugGridImage;
	delete mRenderedTiles;
	delete mTileset;
}

Map::TileEvent Map::tileEvent(const Point &p) {
	int row = p.y;
	int col = p.x;
	// d("Asking row", row, " col", col);
	if (row >= 0 && row < mRowCount && col >= 0 && col < mColCount)
		return mMatrix[row][col].event;
	return Unpassable;
}

Map::Sector Map::sector(const Point &p) {
	int index = 0;
    for (std::vector<Sector>::iterator sectIter = mSectors.begin();
        sectIter != mSectors.end();
        sectIter++, index++) {
			if ((*sectIter).quad.contains(p))
				return (*sectIter);
	}
	Sector sectNotFound;
	sectNotFound.index = -1;
	return sectNotFound;
}

int Map::sectorCount() {
	return mSectors.size();
}

int Map::aiPathCount() {
	return mAIPathsFilename.size();
}

const char *Map::aiPath(unsigned int index) {
	if (index < mAIPathsFilename.size())
		return mAIPathsFilename[index];
	return randomAIPath();
}

const char *Map::randomAIPath() {
	int randomIndex = rand() % mAIPathsFilename.size();
	return mAIPathsFilename[randomIndex];
}

void Map::mapMatrixFillFunction(int readValue, int row, int col) {
	int halfCol = col / 2;
	bool firstOfPair = (col % 2 == 0);
	firstOfPair ?
		mMatrix[row][halfCol].tilesetY = readValue :
		mMatrix[row][halfCol].tilesetX = readValue;
}

void Map::eventsMatrixFillFunction(int readValue, int row, int col) {
	mMatrix[row][col].event = static_cast<TileEvent>(readValue);
}

void Map::sectorsFillFunction(int readValue, int row, int col) {
	if (col == 0) {
		Sector s;
		s.index = sectorCount();
		mSectors.push_back(s);
	}

	SectorsFileFieldIndex readField = static_cast<SectorsFileFieldIndex>(col);
	Quad &q = mSectors[row].quad;

	switch(readField) {
	case ULy:
		q.ul.y = readValue;
		break;
	case ULx:
		q.ul.x = readValue;
		break;
	case URy:
		q.ur.y = readValue;
		break;
	case URx:
		q.ur.x = readValue + 1;
		break;
	case DRy:
		q.dr.y = readValue + 1;
		break;
	case DRx:
		q.dr.x = readValue + 1;
		break;
	case DLy:
		q.dl.y = readValue + 1;
		break;
	case DLx:
		q.dl.x = readValue;
		break;
	case Type:
		mSectors[row].type = static_cast<SectorDirectionType>(readValue);
		break;
	}
}

void Map::loadMap(const char *mapFilename) {
	MatrixUtil::deleteMatrix<Tile>(mMatrix, mRowCount);

	// Map
	int fileRowCount, fileColCount;

	std::string fileStr = ResourceUtil::raw(mapFilename);
	const char * mapPath = fileStr.c_str();

	FileUtil::getMatrixSize<int>(mapPath, fileRowCount, fileColCount);

	mRowCount = fileRowCount;
	mColCount = fileColCount / 2;

	MatrixUtil::initMatrix<Tile>(mMatrix, mRowCount, mColCount);

	void (Map::*mapMatrixFillFunctionPtr)(int, int, int) = &Map::mapMatrixFillFunction;

	FileUtil::loadStructureFromFileKnowningSize<int, Map>(
	 mapPath, fileRowCount, fileColCount, this, mapMatrixFillFunctionPtr);

	d("Loaded map at path: ", mapPath, " of size: ", mRowCount, "x", mColCount);
}

void Map::loadEvents(const char *eventsFilename) {
	// Map events
	std::string fileStr = ResourceUtil::raw(eventsFilename);
	const char * eventsMapPath = fileStr.c_str();

	void (Map::*eventsMatrixFillFunctionPtr)(int, int, int) = &Map::eventsMatrixFillFunction;

	FileUtil::loadStructureFromFileKnowningSize<int, Map>(
			eventsMapPath, mRowCount, mColCount, this, eventsMatrixFillFunctionPtr);
	d("Loaded events at path: ", eventsMapPath, " of size: ", mRowCount, "x", mColCount);
}

void Map::loadTileset(const char *tilesetFilename) {
 	mTileset->loadFromFile(ResourceUtil::image(tilesetFilename).c_str());
}

void Map::loadSectors(const char *sectorsFilename) {
 	// Map sectors
	int fileRowCount, fileColCount;
	std::string fileStr = ResourceUtil::raw(sectorsFilename);
	const char * sectorsMapPath = fileStr.c_str();
	FileUtil::getMatrixSize<int>(sectorsMapPath, fileRowCount, fileColCount);

	void (Map::*sectorsFillFunctionPtr)(int, int, int) = &Map::sectorsFillFunction;

	FileUtil::loadStructureFromFileKnowningSize<int, Map>(
			sectorsMapPath, fileRowCount, fileColCount, this, sectorsFillFunctionPtr);
 	d("Loaded sectors at path: ", sectorsMapPath, " of size: ", fileRowCount, "x", fileColCount);

    for (std::vector<Sector>::iterator sectIter = mSectors.begin();
        sectIter != mSectors.end();
        sectIter++)  {
		d("Sector:", (*sectIter).quad);
	}
}

void Map::addAIPath(const char *pathFilename) {
	mAIPathsFilename.push_back(pathFilename);
}

void Map::update() {

}

void Map::draw() {
	mDebugGridImage->create(Const::WINDOW_WIDTH, Const::WINDOW_HEIGHT, sf::Color(50, 156, 206, 255));
	mDebugGridTexture->loadFromImage(*mDebugGridImage);
	mDebugGridSprite->setTexture(*mDebugGridTexture);
	mWindow->draw(*mDebugGridSprite);

	updateRenderedTiles();
	mWindow->draw(*mRenderedTiles, mTileset);
}

int Map::colCount() {
	return mColCount;
}

int Map::rowCount() {
	return mRowCount;
}

void Map::updateRenderedTiles() {
 	Kart *playerKart = mLevel->playerKart();

	IPoint intKartPosition = GeometryUtil::toIPoint(playerKart->position());

	Point vp1, vp2;
	PerspectiveUtil::calculateVanishPointsForAngle(
		playerKart->direction(), ViewUtil::HORIZON_LINE_Y, ViewUtil::BASE_POINT, vp1, vp2
    );

	// drawPoint(&mDebugGridImage, vp1, sf::Color::Magenta, 7);
	// drawPoint(&mDebugGridImage, vp2, sf::Color::Green, 7);

	// Size of the tile in the tileset.
	const int TILE_SIZE = 15;

	// Calculate the perspective points since tiles shares contact points.
	Point perspectivePoints[MATRIX_RENDERED_TILES_DIAMETER + 1][MATRIX_RENDERED_TILES_DIAMETER + 1];

	for (int r = 0; r < MATRIX_RENDERED_TILES_DIAMETER + 1; r++) {
		for (int c = 0; c < MATRIX_RENDERED_TILES_DIAMETER + 1; c++) {
			// Point of the considered tile (absolute coordinate system).
			IPoint renderedTilePoint = {
				intKartPosition.x - MATRIX_RENDERED_TILES_RADIUS + c,
				intKartPosition.y - MATRIX_RENDERED_TILES_RADIUS + r
			};

			perspectivePoints[r][c] = PerspectiveUtil::perspectivePoint(
		        renderedTilePoint,
		        ViewUtil::cameraPoint(playerKart->directionalPoint()),
		        ViewUtil::BASE_POINT,
		        playerKart->direction(),
		        ViewUtil::HORIZON_LINE_Y,
		        ViewUtil::RENDERED_TILE_SIZE,
		        vp1,
		        vp2
		    );
			// if (r == 0) {
			// 	drawPoint(&mDebugGridImage, pp, sf::Color::Red, 5);
			// }
			// else if (c == 0) {
			// 	drawPoint(&mDebugGridImage, pp, sf::Color::Black, 5);
			// }
			// else {
			// 	drawPoint(&mDebugGridImage, pp, sf::Color::Red, 3);
			// }
		}
	}

	int inScreenTiles = 0;
	mRenderedTiles->clear();

	for (int r = 0; r < MATRIX_RENDERED_TILES_DIAMETER; r++) {
		for (int c = 0; c < MATRIX_RENDERED_TILES_DIAMETER; c++) {

			// Point of the considered tile (absolute coordinate system).
			IPoint renderedTilePoint = {
				intKartPosition.x - MATRIX_RENDERED_TILES_RADIUS + c,
				intKartPosition.y - MATRIX_RENDERED_TILES_RADIUS + r
			};

			const double tilesetX = mMatrix[renderedTilePoint.y][renderedTilePoint.x].tilesetX;
			const double tilesetY = mMatrix[renderedTilePoint.y][renderedTilePoint.x].tilesetY;

			const Point & ppUL = perspectivePoints[r][c];
			const Point & ppUR = perspectivePoints[r][c + 1];
			const Point & ppDL = perspectivePoints[r + 1][c];
			const Point & ppDR = perspectivePoints[r + 1][c + 1];

			if (
				(
					!ViewUtil::isOnScreen(ppUL) &&
					!ViewUtil::isOnScreen(ppUR) &&
					!ViewUtil::isOnScreen(ppDL) &&
					!ViewUtil::isOnScreen(ppDR)
				)
					||
				(
					!ViewUtil::isUnderHorizon(ppUL) ||
					!ViewUtil::isUnderHorizon(ppUR) ||
					!ViewUtil::isUnderHorizon(ppDL) ||
					!ViewUtil::isUnderHorizon(ppDR)
				)
					||
				(
					renderedTilePoint.x < 0 || renderedTilePoint.x >= mColCount ||
					renderedTilePoint.y < 0 || renderedTilePoint.y >= mRowCount
				)
			)
				continue;

			mRenderedTiles->append(sf::Vertex(
				sf::Vector2f(ppUL.x, ppUL.y),
				sf::Vector2f(tilesetX, tilesetY)
			));
			mRenderedTiles->append(sf::Vertex(
				sf::Vector2f(ppUR.x, ppUR.y),
				sf::Vector2f(tilesetX + TILE_SIZE, tilesetY)
			));
			mRenderedTiles->append(sf::Vertex(
				sf::Vector2f(ppDR.x, ppDR.y),
				sf::Vector2f(tilesetX + TILE_SIZE, tilesetY + TILE_SIZE)
			));
			mRenderedTiles->append(sf::Vertex(
				sf::Vector2f(ppDL.x, ppDL.y),
				sf::Vector2f(tilesetX, tilesetY + TILE_SIZE)
			));

			inScreenTiles++;

			// drawPoint(&mDebugGridImage, ppUL, sf::Color::Red, 5);
			// drawPoint(&mDebugGridImage, ppUR, sf::Color::Red, 5);
			// drawPoint(&mDebugGridImage, ppDR, sf::Color::Red, 5);
			// drawPoint(&mDebugGridImage, ppDL, sf::Color::Red, 5);
		}
	}

	// d("In screen tiles:", inScreenTiles);
}
void Map::drawPoint(sf::Image * map, const Point &p, sf::Color color, int size = 7) {
	if (p.x < 0 + size / 2 || p.x >= Const::WINDOW_WIDTH - size / 2 || p.y < 0 + size / 2 || p.y >= Const::WINDOW_HEIGHT - size / 2)
		return;
	for (int x = - size / 2; x <= size / 2; x++) {
		for (int y = - size / 2; y <= size / 2; y++) {
			map->setPixel(p.x + x, p.y + y, color);

		}
	}
}

void Map::drawLine(double x1, double y1, double x2, double y2, double thickness) {
	sf::VertexArray lineThick(sf::Quads, 4);

	lineThick[0].position = sf::Vector2f(x1 - thickness / 2, y1 - thickness / 2);
	lineThick[1].position = sf::Vector2f(x2 + thickness / 2, y2 - thickness / 2);
	lineThick[2].position = sf::Vector2f(x2 + thickness / 2, y2 + thickness / 2);
	lineThick[3].position = sf::Vector2f(x1 - thickness / 2, y1 + thickness / 2);

	lineThick[0].color = sf::Color::Red;
	lineThick[1].color = sf::Color::Red;
	lineThick[2].color = sf::Color::Red;
	lineThick[3].color = sf::Color::Red;
	mWindow->draw(lineThick);
}


const char * Map::logTag() {
	return LOG_TAG;
}

bool Map::canLog() {
	return CAN_LOG;
}