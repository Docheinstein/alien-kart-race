#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include "game.h"
#include "level.h"
#include "kart.h"
#include "map.h"
#include "fileutil.h"
#include "matrixutil.h"
#include "resourceutil.h"
#include "geometryutil.h"
#include "log.h"
/*
sf:: Texture outOfMaptexture;

sf:: Texture greenSolidTexture;
sf:: Texture redSolidTexture;

sf:: Texture streetSolidTexture;

sf:: Texture earthHorizonBackgroundTexture;
const std::string Map::EARTH_AUTOGENERATED_PREFIX = "earth_generated_";
*/
const double Map::HORIZON_LINE_Y = Game::WINDOW_HEIGHT / 2;



Map::Map() {
	mMatrix = NULL;
	/*
	earthHorizonBackgroundTexture.loadFromFile(ResourceUtil::image("earth_horizon_background_no_ground.png"));
	greenSolidTexture.loadFromFile(ResourceUtil::image("green_solid.png"));
	streetSolidTexture.loadFromFile(ResourceUtil::image("street_solid.png"));
	redSolidTexture.loadFromFile(ResourceUtil::image("red_solid.png"));
	outOfMaptexture.loadFromFile(ResourceUtil::image("invalid.png"));

	mEarthStreet1Texture.loadFromFile(ResourceUtil::image("earth_street_1.png"));
	mEarthStreet2Texture.loadFromFile(ResourceUtil::image("earth_street_2.png"));
	mEarthStreet3Texture.loadFromFile(ResourceUtil::image("earth_street_3.png"));
	mEarthStreet4Texture.loadFromFile(ResourceUtil::image("earth_street_4.png"));
	mEarthStreet5Texture.loadFromFile(ResourceUtil::image("earth_street_5.png"));
	mEarthStreet6Texture.loadFromFile(ResourceUtil::image("earth_street_6.png"));

	mEarthGrass1Texture.loadFromFile(ResourceUtil::image("earth_grass_1.png"));
	mEarthGrass2Texture.loadFromFile(ResourceUtil::image("earth_grass_2.png"));
	mEarthGrass3Texture.loadFromFile(ResourceUtil::image("earth_grass_3.png"));

	mEarthDelimiterBelowTexture.loadFromFile(ResourceUtil::image("earth_delimiter_below.png"));
	mEarthDelimiterAboveTexture.loadFromFile(ResourceUtil::image("earth_delimiter_above.png"));
	mEarthDelimiterLeftTexture.loadFromFile(ResourceUtil::image("earth_delimiter_left.png"));
	mEarthDelimiterRightTexture.loadFromFile(ResourceUtil::image("earth_delimiter_right.png"));
	*/
/*
	for (int i = 0; i < AUTOGENERATED_TEXTURE_COUNT; i++) {
		mAutoGeneratedTextures[i].loadFromFile(
			ResourceUtil::image(
				EARTH_AUTOGENERATED_PREFIX +
				std::to_string(i + EARTH_AUTOGENERATED_STARTING_INDEX) +
				".png"
			)
		);
	}
*/

	/*
	for (int i = 0; i < PERSPECTIVE_TILE_ARRAY_LENGTH; i++) {
		PerspectiveMatrixTile perspectiveTile;
		perspectiveTile.tile = Invalid;
		mPerspectiveTileArray[i] = perspectiveTile;
	}
	*/

	mMapRenderedTiles.setPrimitiveType(sf::Quads);
    mMapRenderedTiles.resize(65 * 65 * 4);
}
/*
sf::Texture * Map::textureForTileType(MapTileType type) {

	if (type == OutOfMap)
		return &outOfMaptexture;
	if (type == Empty)
		return &greenSolidTexture;
	if (type == Fulfilled)
		return &streetSolidTexture;
	if (type == Event)
		return &redSolidTexture;

	if (type == EarthStreet1)
		return &mEarthStreet1Texture;
	if (type == EarthStreet2)
		return &mEarthStreet2Texture;
	if (type == EarthStreet3)
		return &mEarthStreet3Texture;
	if (type == EarthStreet4)
		return &mEarthStreet4Texture;
	if (type == EarthStreet5)
		return &mEarthStreet5Texture;
	if (type == EarthStreet6)
		return &mEarthStreet6Texture;

	if (type == EarthGrass1)
		return &mEarthGrass1Texture;
	if (type == EarthGrass2)
		return &mEarthGrass2Texture;
	if (type == EarthGrass3)
		return &mEarthGrass3Texture;

	if (type == EarthDelimiterBelow)
		return &mEarthDelimiterBelowTexture;
	if (type == EarthDelimiterAbove)
		return &mEarthDelimiterAboveTexture;
	if (type == EarthDelimiterLeft)
		return &mEarthDelimiterLeftTexture;
	if (type == EarthDelimiterRight)
		return &mEarthDelimiterRightTexture;

	if (type >= EARTH_AUTOGENERATED_STARTING_INDEX && type < (EARTH_AUTOGENERATED_STARTING_INDEX + AUTOGENERATED_TEXTURE_COUNT)) {
		return &mAutoGeneratedTextures[type - EARTH_AUTOGENERATED_STARTING_INDEX];
	}

	return &greenSolidTexture;
}
*/
Map::~Map() {
	MatrixUtil::deleteMatrix<MapMatrixTile>(mMatrix, mRowCount);
}

void Map::loadMap(MapType mapType) {
	mTileset.loadFromFile(ResourceUtil::image("earth_tileset.png"));
	int rowCount, colCount;

	MatrixUtil::deleteMatrix<MapMatrixTile>(mMatrix, mRowCount);

	std::string mapPath = ResourceUtil::map("earth.txt");
	FileUtil::getMatrixSize<int>(mapPath.c_str(), rowCount, colCount);

	mRowCount = rowCount;
	mColCount = colCount / 2;

	d("Map file path: ", mapPath);
	d("Map size: ", mRowCount, "x", mColCount);

	MatrixUtil::initMatrix<MapMatrixTile>(mMatrix, mRowCount, mColCount);

	loadMapMatrix(mapPath.c_str());

	std::string minimapPath = ResourceUtil::map("earth_minimap.txt");
	loadMiniMapMatrix(minimapPath.c_str());

	createMiniMap();
}

int Map::getTile(int row, int col) {
	return mMatrix[row][col].type;
}

void Map::update() {
	// createMapGrid();
}

void Map::drawMapGrid() {
	sf::Texture mapGridTexture;
	mapGridTexture.loadFromImage(*mMapGrid);

	sf::Sprite mapGridSprite;
	mapGridSprite.setTexture(mapGridTexture);

	Game::instance().window()->draw(mapGridSprite);
}

void Map::drawMap() {
	createMapGrid();

	//const sf::Texture * textureRef = textureForTileType(EarthStreet4);
	Game::instance().window()->draw(mMapRenderedTiles, &mTileset);

	// drawMapGrid();

/*
	for (int i = 0; i < PERSPECTIVE_TILE_ARRAY_LENGTH; i++) {
		if (mPerspectiveTileArray[i].tile != Invalid) {
			const sf::Texture * textureRef = textureForTileType(mPerspectiveTileArray[i].tile);
			Game::instance().window()->draw(mPerspectiveTileArray[i].vertices, textureRef);
		}
	}
*/

/*
	for (std::vector<PerspectiveMatrixTile>::const_iterator iter = mPerspectiveTiles.begin();
		iter != mPerspectiveTiles.end();
		iter++
	) {
		const sf::Texture * textureRef = textureForTileType((*iter).tile);
		Game::instance().window()->draw((*iter).vertices, textureRef);
	}
*/
	// drawHorizonBackground();
}
/*
void Map::drawHorizonBackground() {
	Game::instance().window()->draw(mHorizonBackgroundSprite);
}
*/

void Map::drawMiniMap() {
	sf::Texture miniMapTexture;
	miniMapTexture.loadFromImage(*mMiniMap);

	sf::Sprite miniMapSprite;
	miniMapSprite.setTexture(miniMapTexture);

	miniMapSprite.scale(
		(double) MINI_MAP_WIDTH / mColCount,
		(double) MINI_MAP_HEIGHT / mRowCount);
	miniMapSprite.setPosition(MINI_MAP_X, MINI_MAP_Y);

	Game::instance().window()->draw(miniMapSprite);
}

int Map::colCount() {
	return mColCount;
}

int Map::rowCount() {
	return mRowCount;
}

std::string Map::getMapFilePath(MapType mapType) {
	switch (mapType) {
	case FirstMap:
		return ResourceUtil::map("earth.txt");
	}
	return "";
}

void Map::createMapGrid() {
	const int WIDTH = Game::WINDOW_WIDTH;
	const int HEIGHT = Game::WINDOW_HEIGHT;
	// Number of tiles that will be rendered in each direction.
	const int MATRIX_RENDERED_TILES_RADIUS = 32;

	// Amount of pixel a single tile will occupy.
	const int TILE_SIZE = 72;

	// Y coordinate of the horizon line.
	// const double HORIZON_LINE_Y = Game::WINDOW_HEIGHT / 2;

	// Total amount of tiles rendered per axis.
	const int MATRIX_RENDERED_TILES_DIAMETER = MATRIX_RENDERED_TILES_RADIUS * 2 + 1;

 	Kart *k = Game::instance().level()->kart();

	// Angle between 0 and 6.28 representing the direction of the kart.
	const double ANGLE = k->direction();

	// mMapGrid = new sf::Image();
	// mMapGrid->create(WIDTH, HEIGHT, sf::Color(255, 255, 255));
	// mPerspectiveTiles.clear();

	const int INT_COL = k->col();
	const int INT_ROW = k->row();

	// Basepoint
	double BASE_POINT_Y = HEIGHT /* - Kart::MARGIN_FROM_BOTTOM */;
	double BASE_POINT_X = WIDTH / 2;

	// Vanish points
 	double VANISH_POINT_1_X = BASE_POINT_X - tan(M_PI / 2 - ANGLE) * (BASE_POINT_Y - HORIZON_LINE_Y);
	double VANISH_POINT_2_X = BASE_POINT_X + tan(ANGLE) * (BASE_POINT_Y - HORIZON_LINE_Y);

	Point bp = Point { BASE_POINT_X, BASE_POINT_Y };
	Point vp1 = Point { VANISH_POINT_1_X, HORIZON_LINE_Y };
	Point vp2 = Point { VANISH_POINT_2_X, HORIZON_LINE_Y };

	// Calculate the perspective points since tiles shares contact points.
	Point perspectivePoints[MATRIX_RENDERED_TILES_DIAMETER + 1][MATRIX_RENDERED_TILES_DIAMETER + 1];

	// d("\n");

	for (int r = 0; r < MATRIX_RENDERED_TILES_DIAMETER + 1; r++) {
		for (int c = 0; c < MATRIX_RENDERED_TILES_DIAMETER + 1; c++) {
			// Indexes of the considered tile (absolute).
			const int absoluteMatrixIndexCol = INT_COL - MATRIX_RENDERED_TILES_RADIUS + c;
			const int absoluteMatrixIndexRow = INT_ROW - MATRIX_RENDERED_TILES_RADIUS + r;

			// Offset that the considered tile has respect to the kart tile (relative).
			const double relativeToCenterCol = k->col() - absoluteMatrixIndexCol;
			const double relativeToCenterRow = k->row() - absoluteMatrixIndexRow;

			// Offsets of the projected points on the baseline relative to the center.
			const double colOffset = (relativeToCenterCol) / cos (ANGLE);
			const double rowOffset = (relativeToCenterRow) / cos(M_PI / 2 - ANGLE);

			Point pc = { bp.x - colOffset * TILE_SIZE, bp.y };
			Point pr = { bp.x + rowOffset * TILE_SIZE, bp.y };

			// Line that join projected points on the baseline to the vanish points.
			Line pc_vp2 = GeometryUtil::lineForTwoPoints(pc, vp2);
			Line pr_vp1 = GeometryUtil::lineForTwoPoints(pr, vp1);

			// Definitive perspective points
			Point pp = GeometryUtil::intersectionForTwoLines(pc_vp2, pr_vp1);

			perspectivePoints[r][c] = pp;
			// d("Perspective point at (", r, ", ", c, "): ", pp);
		}
	}


	int i = 0;
	for (int r = 0; r < MATRIX_RENDERED_TILES_DIAMETER; r++) {
		for (int c = 0; c < MATRIX_RENDERED_TILES_DIAMETER; c++) {

			const int tileIndex = i * 4;

			// Indexes of the considered tile (absolute).
			const int absoluteMatrixIndexCol = INT_COL - MATRIX_RENDERED_TILES_RADIUS + c;
			const int absoluteMatrixIndexRow = INT_ROW - MATRIX_RENDERED_TILES_RADIUS + r;

			int tilesetRow;
			int tilesetCol;
			// sf::VertexArray vertices;
			// PerspectiveMatrixTile & perspectiveTile = mPerspectiveTileArray[i];

			if (absoluteMatrixIndexCol >= 0 && absoluteMatrixIndexCol < mColCount &&
				absoluteMatrixIndexRow >= 0 && absoluteMatrixIndexRow < mRowCount) {
					tilesetRow = mMatrix[absoluteMatrixIndexRow][absoluteMatrixIndexCol].row;
					tilesetCol = mMatrix[absoluteMatrixIndexRow][absoluteMatrixIndexCol].col;
			}
			else {
				// w("Trying to access out of the matrix");
				// w("MATRIX_RENDERED_TILES_RADIUS: ", MATRIX_RENDERED_TILES_RADIUS);
				// w("absoluteMatrixIndexRow: ", absoluteMatrixIndexRow);
				// w("absoluteMatrixIndexCol: ", absoluteMatrixIndexCol);
				// w("k-row(): ", k->row());
				// w("k->col(): ", k->col());
				// w("INT_ROW: ", INT_ROW);
				// w("INT_COL: ", INT_COL);
				// w("r: ", r);
				// w("c: ", c);

				// tileType = OutOfMap;
			}

			Point ppUL = perspectivePoints[r][c];
			Point ppUR = perspectivePoints[r][c + 1];
			Point ppDL = perspectivePoints[r + 1][c];
			Point ppDR = perspectivePoints[r + 1][c + 1];

			const bool insideScreenHeight =
				(ppUL.y < HEIGHT ||
				ppUR.y < HEIGHT ||
				ppDL.y < HEIGHT ||
				ppDR.y < HEIGHT);

			const bool insideScreenWidth =
				// Ensure the points are right to left border
				(ppUL.x > 0 ||
				ppUR.x > 0 ||
				ppDL.x > 0 ||
				ppDR.x > 0)
				 	&&
				// Ensure the points are left to right border
				(ppUL.x < WIDTH ||
				ppUR.x < WIDTH ||
				ppDL.x < WIDTH ||
				ppDR.x < WIDTH);

			const bool underHorizonLine =
				(ppUL.y > HORIZON_LINE_Y &&
				ppUR.y > HORIZON_LINE_Y &&
				ppDR.y > HORIZON_LINE_Y &&
				ppDL.y > HORIZON_LINE_Y);

			if  (insideScreenHeight && insideScreenWidth && underHorizonLine) {

				// sf::Vertex* quad = &mMapRenderedTiles[(i) * 4];

				mMapRenderedTiles[tileIndex].position = sf::Vector2f(ppUL.x, ppUL.y);
				//mMapRenderedTiles[tileIndex].texCoords = sf::Vector2f(60, 60);
				mMapRenderedTiles[tileIndex].texCoords = sf::Vector2f(tilesetCol, tilesetRow);

				mMapRenderedTiles[tileIndex + 1].position = sf::Vector2f(ppUR.x, ppUR.y);
				// mMapRenderedTiles[tileIndex + 1].texCoords = sf::Vector2f(60 + 15, 60);
				mMapRenderedTiles[tileIndex + 1].texCoords = sf::Vector2f(tilesetCol + 15, tilesetRow);

				mMapRenderedTiles[tileIndex + 2].position = sf::Vector2f(ppDR.x, ppDR.y);
				//mMapRenderedTiles[tileIndex + 2].texCoords = sf::Vector2f(60 + 15, 60 + 15);
				mMapRenderedTiles[tileIndex + 2].texCoords = sf::Vector2f(tilesetCol + 15, tilesetRow + 15);

				mMapRenderedTiles[tileIndex + 3].position = sf::Vector2f(ppDL.x, ppDL.y);
				// mMapRenderedTiles[tileIndex + 3].texCoords = sf::Vector2f(60, 60 + 15);
				mMapRenderedTiles[tileIndex + 3].texCoords = sf::Vector2f(tilesetCol, tilesetRow + 15);


				/*
				vertices.setPrimitiveType(sf::Quads);
				// vertices.clear();
				vertices.append(sf::Vertex(sf::Vector2f(ppUL.x, ppUL.y)));
				vertices.append(sf::Vertex(sf::Vector2f(ppUR.x, ppUR.y)));
				vertices.append(sf::Vertex(sf::Vector2f(ppDR.x, ppDR.y)));
				vertices.append(sf::Vertex(sf::Vector2f(ppDL.x, ppDL.y)));

				vertices[0].texCoords = sf::Vector2f(0, 0);
				vertices[1].texCoords = sf::Vector2f(15, 0);
				vertices[2].texCoords = sf::Vector2f(15, 15);
				vertices[3].texCoords = sf::Vector2f(0, 15);
*/

				// drawPoint(mMapGrid, ppUL, sf::Color::Red, 3);
				// drawPoint(mMapGrid, ppUR, sf::Color::Red, 3);
				// drawPoint(mMapGrid, ppDR, sf::Color::Red, 3);
				// drawPoint(mMapGrid, ppDL, sf::Color::Red, 3);

				// mPerspectiveTiles.push_back(perspectiveTile);
/*
				const sf::Texture * textureRef = textureForTileType(tileType);
				Game::instance().window()->draw(vertices, textureRef);
*/
			}
			else {
				mMapRenderedTiles[tileIndex].position = sf::Vector2f(-10, -10);
				//mMapRenderedTiles[tileIndex].texCoords = sf::Vector2f(60, 60);
				mMapRenderedTiles[tileIndex].texCoords = sf::Vector2f(0, 0);

				mMapRenderedTiles[tileIndex + 1].position = sf::Vector2f(-10, -10);
				// mMapRenderedTiles[tileIndex + 1].texCoords = sf::Vector2f(60 + 15, 60);
				mMapRenderedTiles[tileIndex + 1].texCoords = sf::Vector2f(0, 0);

				mMapRenderedTiles[tileIndex + 2].position = sf::Vector2f(-10, -10);
				//mMapRenderedTiles[tileIndex + 2].texCoords = sf::Vector2f(60 + 15, 60 + 15);
				mMapRenderedTiles[tileIndex + 2].texCoords = sf::Vector2f(0, 0);

				mMapRenderedTiles[tileIndex + 3].position = sf::Vector2f(-10, -10);
				// mMapRenderedTiles[tileIndex + 3].texCoords = sf::Vector2f(60, 60 + 15);
				mMapRenderedTiles[tileIndex + 3].texCoords = sf::Vector2f(0, 0);
			}

			i++;
		}
	}
}

void Map::createMiniMap() {
	mMiniMap = new sf::Image();
	mMiniMap->create(mColCount, mRowCount, sf::Color(255, 255, 255));

	for (int y = 0; y < mRowCount; y++) {
		for (int x = 0; x < mColCount; x++) {
			if (mMatrix[y][x].full)
				mMiniMap->setPixel(x, y, sf::Color(128, 128, 128));
			//else
			//	mMiniMap->setPixel(x, y, sf::Color(0, 0, 0));
	  	}
	}
}

void Map::loadMapMatrix(const char *fileName) {
	std::ifstream inFile(fileName);
	if (!inFile)
		return;
	int r = -1;
	int c = mColCount;

	do {
		// End of line reached
		if (c == mColCount) {
			c = 0;
			r++;
		}
		if (r < mRowCount) {
			inFile >> mMatrix[r][c].row;
			inFile >> mMatrix[r][c].col;
			c++;
		}
	} while (inFile && r < mRowCount);
}

void Map::loadMiniMapMatrix(const char *fileName) {
	std::ifstream inFile(fileName);
	if (!inFile)
		return;
	int r = -1;
	int c = mColCount;

	int anInt;

	do {
		// End of line reached
		if (c == mColCount) {
			c = 0;
			r++;
		}
		if (r < mRowCount) {
			inFile >> anInt;
			mMatrix[r][c].full = (anInt == 1 ? true : false);
			c += 1;
		}
	} while (inFile && r < mRowCount);
}


void Map::drawPoint(sf::Image * map, const Point &p, sf::Color color, int size = 7) {
	if (p.x < 0 || p.x >= Game::WINDOW_WIDTH || p.y < 0 || p.y >= Game::WINDOW_HEIGHT)
		return;
	for (int x = - size / 2; x <= size / 2; x++) {
		for (int y = - size / 2; y <= size / 2; y++) {
			map->setPixel(p.x + x, p.y + y, color);
		}
	}
}

void Map::drawPoint(sf::Image * map, const IPoint &p, sf::Color color, int size = 7) {
	if (p.x < 0 || p.x >= Game::WINDOW_WIDTH || p.y < 0 || p.y >= Game::WINDOW_HEIGHT)
		return;
	for (int x = - size / 2; x <= size / 2; x++) {
		for (int y = - size / 2; y <= size / 2; y++) {
			map->setPixel(p.x + x, p.y + y, color);
		}
	}
}

void Map::drawLine(double x1, double y1, double x2, double y2) {
	sf::VertexArray lineThick(sf::Quads, 4);

	lineThick[0].position = sf::Vector2f(10, 10);
	lineThick[1].position = sf::Vector2f(20, 10);
	lineThick[2].position = sf::Vector2f(100, 100);
	lineThick[3].position = sf::Vector2f(80, 100);

	lineThick[0].color = sf::Color::Red;
	lineThick[1].color = sf::Color::Red;
	lineThick[2].color = sf::Color::Red;
	lineThick[3].color = sf::Color::Red;
	// d("Drawing line at", x1, y1, x2, y2);
	Game::instance().window()->draw(lineThick);
}
