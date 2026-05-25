#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "tonc.h"

// Forward declarations
class Rectangle;
class Circle;
class Pill;

// ============================================================================
// BASE COLLISION SHAPE CLASS
// ============================================================================

class CollisionShape {
public:
    virtual ~CollisionShape() = default;
    
    // Pure virtual collision detection methods
    virtual bool collidesWith(const CollisionShape& other) const = 0;
    virtual bool collidesWithRectangle(const Rectangle& rect) const = 0;
    virtual bool collidesWithCircle(const Circle& circle) const = 0;
    virtual bool collidesWithPill(const Pill& pill) const = 0;
};

// ============================================================================
// RECTANGLE COLLISION SHAPE
// ============================================================================

class Rectangle : public CollisionShape {
private:
    int x, y;           // top-left corner (world coordinates)
    int width, height;
    
public:
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    
    // Accessors
    inline int getX() const { return x; }
    inline int getY() const { return y; }
    inline int getWidth() const { return width; }
    inline int getHeight() const { return height; }
    
    // Getters for center and edges
    inline int getCenterX() const { return x + width / 2; }
    inline int getCenterY() const { return y + height / 2; }
    inline int getLeft() const { return x; }
    inline int getRight() const { return x + width; }
    inline int getTop() const { return y; }
    inline int getBottom() const { return y + height; }
    
    // Setters
    inline void setPosition(int newX, int newY) { x = newX; y = newY; }
    inline void setSize(int w, int h) { width = w; height = h; }
    
    // Collision detection - implements pure virtual methods
    bool collidesWith(const CollisionShape& other) const override;
    bool collidesWithRectangle(const Rectangle& rect) const override;
    bool collidesWithCircle(const Circle& circle) const override;
    bool collidesWithPill(const Pill& pill) const override;
};

// ============================================================================
// CIRCLE COLLISION SHAPE
// ============================================================================

class Circle : public CollisionShape {
private:
    int x, y;           // center (world coordinates)
    int radius;
    
public:
    Circle() : x(0), y(0), radius(0) {}
    Circle(int x, int y, int r) : x(x), y(y), radius(r) {}
    
    // Accessors
    inline int getX() const { return x; }
    inline int getY() const { return y; }
    inline int getRadius() const { return radius; }
    
    // Setters
    inline void setPosition(int newX, int newY) { x = newX; y = newY; }
    inline void setRadius(int r) { radius = r; }
    
    // Collision detection - implements pure virtual methods
    bool collidesWith(const CollisionShape& other) const override;
    bool collidesWithRectangle(const Rectangle& rect) const override;
    bool collidesWithCircle(const Circle& circle) const override;
    bool collidesWithPill(const Pill& pill) const override;
};

// ============================================================================
// PILL (CAPSULE) COLLISION SHAPE
// ============================================================================

class Pill : public CollisionShape {
private:
    int x1, y1;         // start point (world coordinates)
    int x2, y2;         // end point (world coordinates)
    int radius;         // radius of the pill
    
public:
    Pill() : x1(0), y1(0), x2(0), y2(0), radius(0) {}
    Pill(int x1, int y1, int x2, int y2, int r) : x1(x1), y1(y1), x2(x2), y2(y2), radius(r) {}
    
    // Accessors
    inline int getX1() const { return x1; }
    inline int getY1() const { return y1; }
    inline int getX2() const { return x2; }
    inline int getY2() const { return y2; }
    inline int getRadius() const { return radius; }
    
    // Setters
    inline void setStartPoint(int newX1, int newY1) { x1 = newX1; y1 = newY1; }
    inline void setEndPoint(int newX2, int newY2) { x2 = newX2; y2 = newY2; }
    inline void setPoints(int newX1, int newY1, int newX2, int newY2) {
        x1 = newX1; y1 = newY1; x2 = newX2; y2 = newY2;
    }
    inline void setRadius(int r) { radius = r; }
    
    // Collision detection - implements pure virtual methods
    bool collidesWith(const CollisionShape& other) const override;
    bool collidesWithRectangle(const Rectangle& rect) const override;
    bool collidesWithCircle(const Circle& circle) const override;
    bool collidesWithPill(const Pill& pill) const override;
};

// ============================================================================
// COLLISION DETECTION IMPLEMENTATIONS (INLINE)
// ============================================================================

// Rectangle collision implementations
inline bool Rectangle::collidesWithRectangle(const Rectangle& rect) const {
    return !(getRight() < rect.getLeft() ||
             getLeft() > rect.getRight() ||
             getBottom() < rect.getTop() ||
             getTop() > rect.getBottom());
}

inline bool Rectangle::collidesWithCircle(const Circle& circle) const {
    // Find the closest point on the rectangle to the circle center
    int closestX = clamp(circle.getX(), getLeft(), getRight());
    int closestY = clamp(circle.getY(), getTop(), getBottom());
    
    // Calculate distance between closest point and circle center
    int dx = circle.getX() - closestX;
    int dy = circle.getY() - closestY;
    int distSquared = (dx * dx) + (dy * dy);
    
    // Check if distance is less than radius
    return distSquared < (circle.getRadius() * circle.getRadius());
}

inline bool Rectangle::collidesWithPill(const Pill& pill) const {
    // Vector from pill start to pill end
    int dx_seg = pill.getX2() - pill.getX1();
    int dy_seg = pill.getY2() - pill.getY1();
    
    // Vector from pill start to rectangle center
    int rectCenterX = getCenterX();
    int rectCenterY = getCenterY();
    int dx_rect = rectCenterX - pill.getX1();
    int dy_rect = rectCenterY - pill.getY1();
    
    // Length squared of the pill segment
    int segLenSquared = (dx_seg * dx_seg) + (dy_seg * dy_seg);
    
    // Project rect center onto the pill segment
    int t;
    if (segLenSquared == 0) {
        t = 0;
    } else {
        int dotProduct = (dx_rect * dx_seg) + (dy_rect * dy_seg);
        t = clamp(dotProduct, 0, segLenSquared);
    }
    
    // Find closest point on the pill line segment
    int closestX, closestY;
    if (segLenSquared == 0) {
        closestX = pill.getX1();
        closestY = pill.getY1();
    } else {
        closestX = pill.getX1() + (t * dx_seg) / segLenSquared;
        closestY = pill.getY1() + (t * dy_seg) / segLenSquared;
    }
    
    // Now treat the pill as a circle at the closest point and check rectangle collision
    Circle pillAsCircle(closestX, closestY, pill.getRadius());
    return collidesWithCircle(pillAsCircle);
}

inline bool Rectangle::collidesWith(const CollisionShape& other) const {
    return other.collidesWithRectangle(*this);
}

// Circle collision implementations
inline bool Circle::collidesWithCircle(const Circle& circle) const {
    int dx = getX() - circle.getX();
    int dy = getY() - circle.getY();
    int distSquared = (dx * dx) + (dy * dy);
    int radiusSum = getRadius() + circle.getRadius();
    return distSquared < (radiusSum * radiusSum);
}

inline bool Circle::collidesWithRectangle(const Rectangle& rect) const {
    // Reverse the call to Rectangle's method
    return rect.collidesWithCircle(*this);
}

inline bool Circle::collidesWithPill(const Pill& pill) const {
    // Vector from pill start to pill end
    int dx_seg = pill.getX2() - pill.getX1();
    int dy_seg = pill.getY2() - pill.getY1();
    
    // Vector from pill start to circle center
    int dx_circ = getX() - pill.getX1();
    int dy_circ = getY() - pill.getY1();
    
    // Length squared of the pill segment
    int segLenSquared = (dx_seg * dx_seg) + (dy_seg * dy_seg);
    
    // Project circle center onto the pill segment (clamped to [0, 1])
    int t;
    if (segLenSquared == 0) {
        // Degenerate case: pill start and end are the same point
        t = 0;
    } else {
        // Calculate projection parameter (scaled to avoid division)
        int dotProduct = (dx_circ * dx_seg) + (dy_circ * dy_seg);
        // Clamp t to [0, segLenSquared] range
        t = clamp(dotProduct, 0, segLenSquared);
    }
    
    // Find closest point on the pill line segment
    int closestX, closestY;
    if (segLenSquared == 0) {
        closestX = pill.getX1();
        closestY = pill.getY1();
    } else {
        // Use fixed point math: closest = start + (t / segLenSquared) * segment
        closestX = pill.getX1() + (t * dx_seg) / segLenSquared;
        closestY = pill.getY1() + (t * dy_seg) / segLenSquared;
    }
    
    // Calculate distance from circle center to closest point on pill
    int dx = getX() - closestX;
    int dy = getY() - closestY;
    int distSquared = (dx * dx) + (dy * dy);
    
    // Check if distance is less than combined radius
    int combinedRadius = getRadius() + pill.getRadius();
    return distSquared < (combinedRadius * combinedRadius);
}

inline bool Circle::collidesWith(const CollisionShape& other) const {
    return other.collidesWithCircle(*this);
}

// Pill collision implementations
inline bool Pill::collidesWithRectangle(const Rectangle& rect) const {
    return rect.collidesWithPill(*this);
}

inline bool Pill::collidesWithCircle(const Circle& circle) const {
    return circle.collidesWithPill(*this);
}

inline bool Pill::collidesWithPill(const Pill& pill) const {
    // Pill-to-pill collision: treat each pill as a capsule/cylinder
    // Check if the distance between the two line segments is less than the sum of radii
    
    // Vector from pill1 start to pill1 end
    int dx_seg1 = getX2() - getX1();
    int dy_seg1 = getY2() - getY1();
    
    // Vector from pill2 start to pill2 end
    int dx_seg2 = pill.getX2() - pill.getX1();
    int dy_seg2 = pill.getY2() - pill.getY1();
    
    // Vector from pill1 start to pill2 start
    int dx = pill.getX1() - getX1();
    int dy = pill.getY1() - getY1();
    
    // Calculate segment lengths
    int len1Squared = (dx_seg1 * dx_seg1) + (dy_seg1 * dy_seg1);
    int len2Squared = (dx_seg2 * dx_seg2) + (dy_seg2 * dy_seg2);
    
    // Handle degenerate cases (points instead of segments)
    if (len1Squared == 0 && len2Squared == 0) {
        // Both are points
        int distSq = (dx * dx) + (dy * dy);
        int radSum = getRadius() + pill.getRadius();
        return distSq < (radSum * radSum);
    }
    
    if (len1Squared == 0) {
        // First pill is a point, use circle-pill collision
        Circle c(getX1(), getY1(), getRadius());
        return c.collidesWithPill(pill);
    }
    
    if (len2Squared == 0) {
        // Second pill is a point
        Circle c(pill.getX1(), pill.getY1(), pill.getRadius());
        return collidesWithCircle(c);
    }
    
    // General case: both are segments
    // Project closest points on each segment
    int t1_num = (dx * dx_seg1) + (dy * dy_seg1);
    int t1 = clamp(t1_num, 0, len1Squared);
    
    int t2_num = (-dx * dx_seg2) + (-dy * dy_seg2);
    int t2 = clamp(t2_num, 0, len2Squared);
    
    // Calculate closest points
    int p1x = getX1() + (t1 * dx_seg1) / len1Squared;
    int p1y = getY1() + (t1 * dy_seg1) / len1Squared;
    
    int p2x = pill.getX1() + (t2 * dx_seg2) / len2Squared;
    int p2y = pill.getY1() + (t2 * dy_seg2) / len2Squared;
    
    // Distance between closest points
    int px = p2x - p1x;
    int py = p2y - p1y;
    int distSq = (px * px) + (py * py);
    
    int radSum = getRadius() + pill.getRadius();
    return distSq < (radSum * radSum);
}

inline bool Pill::collidesWith(const CollisionShape& other) const {
    return other.collidesWithPill(*this);
}

// ============================================================================
// HELPER FUNCTIONS FOR CREATING GAME OBJECT COLLISION SHAPES
// ============================================================================

// Get player bounding rectangle
// Player sprite is 32x32 pixels in world coordinates
inline Rectangle getPlayerRect(int playerX, int playerY) {
    // Player sprite is 32x32 in game units
    return Rectangle(playerX - 16, playerY - 16, 32, 32);
}

// Get laser circle from screen position and size
// Lasers are 64x64 sprites, using them as 32-pixel radius circles
inline Circle getLaserCircle(int laserScreenX, int laserScreenY) {
    // Convert screen position to world position (assuming camera follows player)
    // For now, use screen coordinates directly
    // Radius of 16 pixels for the laser
    return Circle(laserScreenX + 32, laserScreenY + 32, 16);
}

// Get hazard circle (missiles, etc.)
// Generic hazard collision circle
inline Circle getHazardCircle(int hazardX, int hazardY, int radius) {
    return Circle(hazardX + radius, hazardY + radius, radius);
}

// Get laser pill (beam or elongated laser)
// Represents a directed laser beam as a capsule shape
inline Pill getLaserPill(int startX, int startY, int endX, int endY, int radius) {
    return Pill(startX, startY, endX, endY, radius);
}

// Create a pill from current position and direction
// Useful for moving hazards like continuous beams
inline Pill getMovingPill(int centerX, int centerY, int dirX, int dirY, int length, int radius) {
    // Normalize direction and extend along it
    int x1 = centerX - (dirX * length) / 2;
    int y1 = centerY - (dirY * length) / 2;
    int x2 = centerX + (dirX * length) / 2;
    int y2 = centerY + (dirY * length) / 2;
    return Pill(x1, y1, x2, y2, radius);
}

#endif // COLLISION_HPP
