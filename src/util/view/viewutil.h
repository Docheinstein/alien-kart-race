#ifndef VIEW_UTIL_H
#define VIEW_UTIL_H

#include "game.h"
#include "geometryutil.h"

class ViewUtil {
public:
    static const Point BASE_POINT;
	static const double HORIZON_LINE_Y;
	static const double RENDERED_TILE_SIZE;
	static const double TILE_SIZE;
    static const double CAMERA_TILE_BEHIND_PLAYER_COUNT;

    static Vector cameraVector(const Vector &v);
    static Point cameraPoint(const Vector &v);

    static bool isOnScreen(const Point &p);
    static bool isUnderHorizon(const Point &p);
    static bool isOnScreenUnderHorizon(const Point &p);
};

#endif // VIEW_UTIL_H