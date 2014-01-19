#ifndef __WIZ_H__
#define __WIZ_H__

/**
 * Simple class for handling both 2D position and size.
 */
struct Point {
	int16 x;	///< The horizontal part of the point
	int16 y;	///< The vertical part of the point

	Point() : x(0), y(0) {}
	Point(int16 x1, int16 y1) : x(x1), y(y1) {}
	bool operator==(const Point &p) const { return x == p.x && y == p.y; }
	bool operator!=(const Point &p) const { return x != p.x || y != p.y; }
	Point operator+(const Point &delta) const {	return Point(x + delta.x, y + delta.y);	}
	Point operator-(const Point &delta) const {	return Point(x - delta.x, y - delta.y);	}

	void operator+=(const Point &delta) {
		x += delta.x;
		y += delta.y;
	}

	void operator-=(const Point &delta) {
		x -= delta.x;
		y -= delta.y;
	}

	/**
	 * Return the square of the distance between this point and the point p.
	 *
	 * @param p		the other point
	 * @return the distance between this and p
	 */
	/*uint sqrDist(const Point &p) const {
		int diffx = ABS(p.x - x);
		if (diffx >= 0x1000)
			return 0xFFFFFF;

		int diffy = ABS(p.y - y);
		if (diffy >= 0x1000)
			return 0xFFFFFF;

		return uint(diffx * diffx + diffy * diffy);
	}*/
};

template<typename T> inline T MIN(T a, T b)	{ return (a < b) ? a : b; }
template<typename T> inline T MAX(T a, T b)	{ return (a > b) ? a : b; } 

/**
 * Simple class for handling a rectangular zone.
 *
 * Note: This implementation is built around the assumption that (top,left) is
 * part of the rectangle, but (bottom,right) is not. This is reflected in
 * various methods, including contains(), intersects() and others.
 *
 * Another very wide spread approach to rectangle classes treats (bottom,right)
 * also as a part of the rectangle.
 *
 * Conceptually, both are sound, but the approach we use saves many intermediate
 * computations (like computing the height in our case is done by doing this:
 *   height = bottom - top;
 * while in the alternate system, it would be
 *   height = bottom - top + 1;
 *
 * When writing code using our Rect class, always keep this principle in mind!
*/
struct Rect {
	int16 top, left;		///< The point at the top left of the rectangle (part of the rect).
	int16 bottom, right;	///< The point at the bottom right of the rectangle (not part of the rect).

	Rect() : top(0), left(0), bottom(0), right(0) {}
	Rect(int16 w, int16 h) : top(0), left(0), bottom(h), right(w) {}
	Rect(int16 x1, int16 y1, int16 x2, int16 y2) : top(y1), left(x1), bottom(y2), right(x2) {
		//assert(isValidRect());
	}
	//bool operator==(const Rect &rhs) const { return equals(rhs); }
	//bool operator!=(const Rect &rhs) const { return !equals(rhs); }

	int16 width() const { return right - left; }
	int16 height() const { return bottom - top; }

	//void setWidth(int16 aWidth) {
	//	right = left + aWidth;
	//}

	//void setHeight(int16 aHeight) {
	//	bottom = top + aHeight;
	//}

	/**
	 * Check if given position is inside this rectangle.
	 *
	 * @param x the horizontal position to check
	 * @param y the vertical position to check
	 *
	 * @return true if the given position is inside this rectangle, false otherwise
	 */
	bool contains(int16 x, int16 y) const {
		return (left <= x) && (x < right) && (top <= y) && (y < bottom);
	}

	/**
	 * Check if given point is inside this rectangle.
	 *
	 * @param p the point to check
	 *
	 * @return true if the given point is inside this rectangle, false otherwise
	 */
	bool contains(const Point &p) const {
		return contains(p.x, p.y);
	}

	/**
	 * Check if the given rect is contained inside this rectangle.
	 *
	 * @param r The rectangle to check
	 *
	 * @return true if the given rect is inside, false otherwise
	 */
	bool contains(const Rect &r) const {
		return (left <= r.left) && (r.right <= right) && (top <= r.top) && (r.bottom <= bottom);
	}

	/**
	 * Check if the given rect is equal to this one.
	 *
	 * @param r The rectangle to check
	 *
	 * @return true if the given rect is equal, false otherwise
	 */
	//bool equals(const Rect &r) const {
	//	return (left == r.left) && (right == r.right) && (top == r.top) && (bottom == r.bottom);
	//}

	/**
	 * Check if given rectangle intersects with this rectangle
	 *
	 * @param r the rectangle to check
	 *
	 * @return true if the given rectangle is inside the rectangle, false otherwise
	 */
	//bool intersects(const Rect &r) const {
	//	return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
	//}

	/**
	 * Find the intersecting rectangle between this rectangle and the given rectangle
	 *
	 * @param r the intersecting rectangle
	 *
	 * @return the intersection of the rectangles or an empty rectangle if not intersecting
	 */
	//Rect findIntersectingRect(const Rect &r) const {
	//	if (!intersects(r))
	//		return Rect();
	//
	//	return Rect(MAX(r.left, left), MAX(r.top, top), MIN(r.right, right), MIN(r.bottom, bottom));
	//}

	/**
	 * Extend this rectangle so that it contains r
	 *
	 * @param r the rectangle to extend by
	 */
	void extend(const Rect &r)
	{
		left = MIN(left, r.left);
		right = MAX(right, r.right);
		top = MIN(top, r.top);
		bottom = MAX(bottom, r.bottom);
	}

	/**
	 * Extend this rectangle in all four directions by the given number of pixels
	 *
	 * @param offset the size to grow by
	 */
	//void grow(int16 offset) {
	//	top -= offset;
	//	left -= offset;
	//	bottom += offset;
	//	right += offset;
	//}

	/*void clip(const Rect &r) {
		assert(isValidRect());
		assert(r.isValidRect());

		if (top < r.top) top = r.top;
		else if (top > r.bottom) top = r.bottom;

		if (left < r.left) left = r.left;
		else if (left > r.right) left = r.right;

		if (bottom > r.bottom) bottom = r.bottom;
		else if (bottom < r.top) bottom = r.top;

		if (right > r.right) right = r.right;
		else if (right < r.left) right = r.left;
	}*/

	//void clip(int16 maxw, int16 maxh) {
	//	clip(Rect(0, 0, maxw, maxh));
	//}

	//bool isEmpty() const {
	//	return (left >= right || top >= bottom);
	//}

	//bool isValidRect() const {
	//	return (left <= right && top <= bottom);
	//}

	//void moveTo(int16 x, int16 y) {
	//	bottom += y - top;
	//	right += x - left;
	//	top = y;
	//	left = x;
	//}

	//void translate(int16 dx, int16 dy) {
	//	left += dx; right += dx;
	//	top += dy; bottom += dy;
	//}

	//void moveTo(const Point &p) {
	//	moveTo(p.x, p.y);
	//}

	//void debugPrint(int debuglevel = 0, const char *caption = "Rect:") const {
	//	debug(debuglevel, "%s %d, %d, %d, %d", caption, left, top, right, bottom);
	//}

	/**
	 * Create a rectangle around the given center.
	 * @note the center point is rounded up and left when given an odd width and height
	 */
	//static Rect center(int16 cx, int16 cy, int16 w, int16 h) {
	//	int x = cx - w / 2, y = cy - h / 2;
	//	return Rect(x, y, x + w, y + h);
	//}
};


struct WizPolygon {
	Point vert[5];
	Rect bound;
	int id;
	int numVerts;
	bool flag;
};

struct WizImage {
	int resNum;
	int x1;
	int y1;
	int zorder;
	int state;
	int flags;
	int shadow;
	int field_390;
	int palette;
};

struct WizParameters {
	int field_0;
	byte filename[260];
	//Common::Rect box;
	int processFlags;
	int processMode;
	int field_11C;
	int field_120;
	int field_124;
	int field_128;
	int field_12C;
	int field_130;
	int field_134;
	int field_138;
	int compType;
	int fileWriteMode;
	int angle;
	int scale;
	int polygonId1;
	int polygonId2;
	int resDefImgW;
	int resDefImgH;
	int sourceImage;
	int params1;
	int params2;
	uint8 remapColor[256];
	uint8 remapIndex[256];
	int remapNum;
	int dstResNum;
	uint16 fillColor;
	byte string1[4096];
	byte string2[4096];
	int field_2399;
	int field_239D;
	int field_23A1;
	int field_23A5;
	int field_23A9;
	int field_23AD;
	int field_23B1;
	int field_23B5;
	int field_23B9;
	int field_23BD;
	int field_23C1;
	int field_23C5;
	int field_23C9;
	int field_23CD;
	//Common::Rect box2;
	int field_23DE;
	int spriteId;
	int spriteGroup;
	int field_23EA;
	WizImage img;
};

enum WizImageFlags {
	kWIFHasPalette = 0x1,
	kWIFRemapPalette = 0x2,
	kWIFPrint = 0x4,
	kWIFBlitToFrontVideoBuffer = 0x8,
	kWIFMarkBufferDirty = 0x10,
	kWIFBlitToMemBuffer = 0x20,
	kWIFIsPolygon = 0x40,
	kWIFFlipX = 0x400,
	kWIFFlipY = 0x800
};

enum WizProcessFlags {
	kWPFSetPos = 0x1,
	kWPFShadow = 0x4,
	kWPFScaled = 0x8,
	kWPFRotate = 0x10,
	kWPFNewFlags = 0x20,
	kWPFRemapPalette = 0x40,
	kWPFClipBox = 0x200,
	kWPFNewState = 0x400,
	kWPFUseFile = 0x800,
	kWPFUseDefImgWidth = 0x2000,
	kWPFUseDefImgHeight = 0x4000,
	kWPFPaletteNum = 0x8000,
	kWPFDstResNum = 0x10000,
	kWPFFillColor = 0x20000,
	kWPFClipBox2 = 0x40000,
	kWPFMaskImg = 0x80000,
	kWPFParams = 0x100000
}; 

struct AWIZ_t
{
	void* RGBS;
	uint32_t Compression;
	uint32_t Width;
	uint32_t Height;
	fpos_t WIZD;
};

enum {
		NUM_POLYGONS = 200,
		NUM_IMAGES   = 255
	};

extern WizPolygon _polygons[NUM_POLYGONS]; 

extern WizParameters _wizParams;

void displayWizImage(WizImage *pwi);
void processWizImage(const WizParameters *params);
void loadWizCursor(int resId, int palette);

void polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y);
void polygonCalcBoundBox(Point *vert, int numVerts, Rect &bound);
void polygonErase(int fromId, int toId);
int polygonHit(int id, int x, int y);
bool polygonDefined(int id);
bool polygonContains(const WizPolygon &pol, int x, int y);

#endif