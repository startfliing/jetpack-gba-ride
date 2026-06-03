#include "collision.hpp"

// ============================================================================
// RECTANGLE COLLISION IMPLEMENTATIONS
// ============================================================================

bool Rectangle::collidesWith(const CollisionShape& other) const {
    return other.collidesWithRectangle(*this);
}

bool Rectangle::collidesWithRectangle(const Rectangle& rect) const {
    return !(getRight() < rect.getLeft() ||
             getLeft() > rect.getRight() ||
             getBottom() < rect.getTop() ||
             getTop() > rect.getBottom());
}

bool Rectangle::collidesWithCircle(const Circle& circle) const {
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

bool Rectangle::collidesWithPill(const Pill& pill) const {
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

void Rectangle::setPos(int nx, int ny) { 
    x = nx;
    y = ny;
}

void Rectangle::rotate(u16 alpha){
    return;
}

// ============================================================================
// CIRCLE COLLISION IMPLEMENTATIONS
// ============================================================================

bool Circle::collidesWith(const CollisionShape& other) const {
    return other.collidesWithCircle(*this);
}

bool Circle::collidesWithCircle(const Circle& circle) const {
    int dx = getX() - circle.getX();
    int dy = getY() - circle.getY();
    int distSquared = (dx * dx) + (dy * dy);
    int radiusSum = getRadius() + circle.getRadius();
    return distSquared < (radiusSum * radiusSum);
}

bool Circle::collidesWithRectangle(const Rectangle& rect) const {
    // Reverse the call to Rectangle's method
    return rect.collidesWithCircle(*this);
}

bool Circle::collidesWithPill(const Pill& pill) const {
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

void Circle::setPos(int nx, int ny) { 
    x = nx;
    y = ny;
}

void Circle::rotate(u16 alpha){
    return;
}

// ============================================================================
// PILL COLLISION IMPLEMENTATIONS
// ============================================================================

bool Pill::collidesWith(const CollisionShape& other) const {
    return other.collidesWithPill(*this);
}

bool Pill::collidesWithRectangle(const Rectangle& rect) const {
    return rect.collidesWithPill(*this);
}

bool Pill::collidesWithCircle(const Circle& circle) const {
    return circle.collidesWithPill(*this);
}

bool Pill::collidesWithPill(const Pill& pill) const {
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

void Pill::setPos(int nx, int ny) {
    int dx = x2-x1;
    int dy = y2-y1;
    x1 = nx;
    y1 = ny;
    x2 = nx + dx;
    y2 = ny + dy;
}

void Pill::rotate(u16 alpha) {
    // Calculate the center point of the pill
 
    // Get sin and cos from TONC lookup tables for the delta angle
    // This applies the minimal rotation needed to set the pill to angle alpha
    int sine = lu_sin(alpha);
    int cosine = lu_cos(alpha);
    
    int dx = (cosine * length)>>12;
    int dy = (sine * length)>>12;
    
    // Translate back to world coordinates
    x1 = mx - dx;
    y1 = my + dy;
    x2 = mx + dx;
    y2 = my - dy;
}

// ============================================================================
// HELPER FUNCTIONS FOR CREATING GAME OBJECT COLLISION SHAPES
// ============================================================================

// Get player bounding rectangle
// Player sprite is 32x32 pixels in world coordinates
Rectangle getPlayerRect(int playerX, int playerY) {
    // Player sprite is 32x32 in game units
    return Rectangle(playerX - 16, playerY - 16, 32, 32);
}

// Get laser circle from screen position and size
// Lasers are 64x64 sprites, using them as 32-pixel radius circles
Circle getLaserCircle(int laserScreenX, int laserScreenY) {
    // Convert screen position to world position (assuming camera follows player)
    // For now, use screen coordinates directly
    // Radius of 16 pixels for the laser
    return Circle(laserScreenX + 32, laserScreenY + 32, 16);
}

// Get hazard circle (missiles, etc.)
// Generic hazard collision circle
Circle getHazardCircle(int hazardX, int hazardY, int radius) {
    return Circle(hazardX + radius, hazardY + radius, radius);
}

// Get laser pill (beam or elongated laser)
// Represents a directed laser beam as a capsule shape
Pill getLaserPill(int startX, int startY, int endX, int endY, int radius) {
    return Pill(startX, startY, endX, endY, radius);
}

// Create a pill from current position and direction
// Useful for moving hazards like continuous beams
Pill getMovingPill(int centerX, int centerY, int dirX, int dirY, int length, int radius) {
    // Normalize direction and extend along it
    int x1 = centerX - (dirX * length) / 2;
    int y1 = centerY - (dirY * length) / 2;
    int x2 = centerX + (dirX * length) / 2;
    int y2 = centerY + (dirY * length) / 2;
    return Pill(x1, y1, x2, y2, radius);
}
