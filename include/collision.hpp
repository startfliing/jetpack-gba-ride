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

    virtual void setPos(int dx, int dy) = 0;
    virtual void rotate(u16 alpha) = 0;
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
    void setPos(int dx, int dy) override;
    void rotate(u16 alpha) override;
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

    void setPos(int dx, int dy) override;
    void rotate(u16 alpha) override;
};

// ============================================================================
// PILL (CAPSULE) COLLISION SHAPE
// ============================================================================

class Pill : public CollisionShape {
private:
    int x1, y1;         // start point (world coordinates)
    int x2, y2;         // end point (world coordinates)
    int mx, my;
    int length;
    int radius;         // radius of the pill
    
public:
    Pill() : x1(0), y1(0), x2(0), y2(0), mx(0), my(0), length(0), radius(0) {}
    Pill(int x1, int y1, int x2, int y2, int r) : x1(x1), y1(y1), x2(x2), y2(y2), mx((x2+x1)/2), my((y2+y1)/2), length(x1 - mx > 0 ? x1-mx : mx-x1), radius(r) {}
    
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

    void setPos(int dx, int dy) override;

    // Rotation - rotates both endpoints around the pill's center point
    void rotate(u16 alpha) override;
};

// ============================================================================
// HELPER FUNCTION DECLARATIONS
// ============================================================================

Rectangle getPlayerRect(int playerX, int playerY);
Circle getLaserCircle(int laserScreenX, int laserScreenY);
Circle getHazardCircle(int hazardX, int hazardY, int radius);
Pill getLaserPill(int startX, int startY, int endX, int endY, int radius);
Pill getMovingPill(int centerX, int centerY, int dirX, int dirY, int length, int radius);

#endif // COLLISION_HPP
