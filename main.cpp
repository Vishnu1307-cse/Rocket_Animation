#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <time.h> // For srand

// Define a constant for the value of PI
#define PI 3.1415926535

// ####################################################################
// ## ANIMATION STATE
// ####################################################################
// Start the rocket at the top of the left planet, with a small offset
float rocket_x = 100.0f;
float rocket_y = 350.0f + 20.0f; // Planet radius + offset
float rocket_angle_deg = 70.0f; // Initial angle for takeoff

// Parabola control variables
float t_param = 0.0f; // Parameter from 0 (start) to 1 (end)
const float animation_speed = 0.008f; // Controls how fast the rocket travels along the arc

// ####################################################################
// ## DATA STRUCTURES & HELPERS
// ####################################################################

struct Point { int x, y; };

// Global color definitions
const float COLOR_BLACK[] = {0.0f, 0.0f, 0.0f};
const float COLOR_WHITE[] = {1.0f, 1.0f, 1.0f};
const float COLOR_RED[] = {1.0f, 0.0f, 0.0f};
const float COLOR_SILVER[] = {0.8f, 0.8f, 0.9f};
const float COLOR_DARK_GREY[] = {0.3f, 0.3f, 0.3f};
const float COLOR_BLUE[] = {0.2f, 0.4f, 1.0f};
const float COLOR_ORANGE[] = {0.9f, 0.3f, 0.1f};
// A unique color for the boundary that won't be in the scene otherwise
const float COLOR_BOUNDARY[] = {1.0f, 0.0f, 1.0f}; // Magenta

// The basic drawing unit for all other algorithms
void drawPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// ####################################################################
// ## ALGORITHM IMPLEMENTATIONS
// ####################################################################

// ## 1. Bresenham's Line Drawing Algorithm ##
void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        drawPixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

// ## 2. Midpoint Circle Drawing Algorithm ##
void midpointCircle(int xc, int yc, int r) {
    int x = 0, y = r;
    int p = 1 - r;
    auto plotPoints = [&](int curX, int curY) {
        drawPixel(xc + curX, yc + curY); drawPixel(xc - curX, yc + curY);
        drawPixel(xc + curX, yc - curY); drawPixel(xc - curX, yc - curY);
        drawPixel(xc + curY, yc + curX); drawPixel(xc - curY, yc + curX);
        drawPixel(xc + curY, yc - curX); drawPixel(xc - curY, yc - curX);
    };
    plotPoints(x, y);
    while (x < y) {
        x++;
        if (p < 0) { p += 2 * x + 1; }
        else { y--; p += 2 * (x - y) + 1; }
        plotPoints(x, y);
    }
}

// ## 3. Boundary Fill Algorithm (4-Connected, Recursive) ##
void boundaryFill4(int x, int y, const float fillColor[3], const float borderColor[3]) {
    float currentPixel[3];
    glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, currentPixel);

    const float EPSILON = 0.01f; // Tolerance for float comparison
    bool isBorder = (fabs(currentPixel[0] - borderColor[0]) < EPSILON &&
                     fabs(currentPixel[1] - borderColor[1]) < EPSILON &&
                     fabs(currentPixel[2] - borderColor[2]) < EPSILON);
    bool isFill = (fabs(currentPixel[0] - fillColor[0]) < EPSILON &&
                   fabs(currentPixel[1] - fillColor[1]) < EPSILON &&
                   fabs(currentPixel[2] - fillColor[2]) < EPSILON);

    if (!isBorder && !isFill) {
        glColor3fv(fillColor);
        drawPixel(x, y);
        // Note: No glFlush() here for performance. It will be flushed later.
        if (x > 0 && x < 800 && y > 0 && y < 600) { // Stay within window bounds
            boundaryFill4(x + 1, y, fillColor, borderColor);
            boundaryFill4(x - 1, y, fillColor, borderColor);
            boundaryFill4(x, y + 1, fillColor, borderColor);
            boundaryFill4(x, y - 1, fillColor, borderColor);
        }
    }
}

// ####################################################################
// ## DRAWING FUNCTIONS
// ####################################################################

void drawPolygonOutline(const std::vector<Point>& vertices) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        bresenhamLine(vertices[i].x, vertices[i].y, vertices[(i + 1) % vertices.size()].x, vertices[(i + 1) % vertices.size()].y);
    }
}

Point getCentroid(const std::vector<Point>& vertices) {
    long long sumX = 0, sumY = 0;
    for (const auto& v : vertices) { sumX += v.x; sumY += v.y; }
    return {(int)(sumX / vertices.size()), (int)(sumY / vertices.size())};
}

void drawStars() {
    glColor3fv(COLOR_WHITE);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        glVertex2i(rand() % 800, rand() % 600);
    }
    glEnd();
}

void drawPlanets() {
    // Planet 1 (Earth-like)
    glColor3fv(COLOR_BOUNDARY); // 1. Draw temporary magenta boundary
    midpointCircle(100, 300, 50);
    boundaryFill4(100, 300, COLOR_BLUE, COLOR_BOUNDARY); // 2. Fill with blue
    glColor3fv(COLOR_BLUE); // 3. Redraw border with final color
    midpointCircle(100, 300, 50);

    // Planet 2 (Mars-like)
    glColor3fv(COLOR_BOUNDARY); // 1. Draw temporary magenta boundary
    midpointCircle(700, 300, 80);
    boundaryFill4(700, 300, COLOR_ORANGE, COLOR_BOUNDARY); // 2. Fill with orange
    glColor3fv(COLOR_ORANGE); // 3. Redraw border with final color
    midpointCircle(700, 300, 80);
}

void drawFlames(const std::vector<Point>& vertices, float angle_deg) {
    // The flame should point opposite to the rocket's nose.
    // The rocket's "up" is angle_deg, so the flame's direction is angle_deg - 90.
    float flame_angle_rad = (angle_deg - 90.0f) * PI / 180.0f;
    float dir_x = cos(flame_angle_rad);
    float dir_y = sin(flame_angle_rad);

    for (int i = 0; i < 15; ++i) { // Draw 15 flame triangles
        // Get two random points along the base of the exhaust
        int p1_idx = rand() % vertices.size();
        int p2_idx = rand() % vertices.size();
        Point v1 = vertices[p1_idx];
        Point v2 = vertices[p2_idx];

        // Interpolate between the two points to get a random base point
        float t = (float)(rand()) / (float)(RAND_MAX);
        Point base = { (int)(v1.x + t * (v2.x - v1.x)), (int)(v1.y + t * (v2.y - v1.y)) };

        // Flame tip is a random distance along the calculated direction
        int flame_length = 10 + rand() % 20;
        Point tip = { (int)(base.x + flame_length * dir_x), (int)(base.y + flame_length * dir_y) };

        // Random orange/yellow color
        glColor3f(1.0f, 0.5f + (rand() % 50) / 100.0f, 0.0f);

        // Draw a small triangle
        glBegin(GL_TRIANGLES);
        glVertex2i(base.x - 2, base.y);
        glVertex2i(base.x + 2, base.y);
        glVertex2i(tip.x, tip.y);
        glEnd();
    }
}

void drawRocket(Point translate, float angle_deg) {
    // --- Manual Transformation Setup ---
    float angle_rad = angle_deg * PI / 180.0f;
    float cosA = cos(angle_rad), sinA = sin(angle_rad);

    auto transform = [&](const std::vector<Point>& v_in) {
        std::vector<Point> v_out;
        for (const auto& v : v_in) {
            v_out.push_back({(int)(v.x * cosA - v.y * sinA) + translate.x, (int)(v.x * sinA + v.y * cosA) + translate.y});
        }
        return v_out;
    };

    // --- Define, Transform, and Draw each Part ---
    auto nose_cone = transform({ {0, 40}, {-15, 10}, {15, 10} });
    auto body = transform({ {-15, 10}, {15, 10}, {15, -30}, {-15, -30} });
    auto left_fin = transform({ {-15, 0}, {-15, -25}, {-25, -35} });
    auto right_fin = transform({ {15, 0}, {15, -25}, {25, -35} });
    auto exhaust_funnel = transform({ {-10, -30}, {10, -30}, {15, -40}, {-15, -40} });
    
    // The base of the exhaust funnel, to be used for the flame position
    auto exhaust_base = transform({ {-15, -40}, {15, -40} });
    drawFlames(exhaust_base, angle_deg);

    // 1. Draw all temporary boundaries first
    glColor3fv(COLOR_BOUNDARY);
    drawPolygonOutline(nose_cone);
    drawPolygonOutline(body);
    drawPolygonOutline(left_fin);
    drawPolygonOutline(right_fin);
    drawPolygonOutline(exhaust_funnel);

    // 2. Fill all shapes
    boundaryFill4(getCentroid(nose_cone).x, getCentroid(nose_cone).y, COLOR_RED, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(body).x, getCentroid(body).y, COLOR_SILVER, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(left_fin).x, getCentroid(left_fin).y, COLOR_RED, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(right_fin).x, getCentroid(right_fin).y, COLOR_RED, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(exhaust_funnel).x, getCentroid(exhaust_funnel).y, COLOR_DARK_GREY, COLOR_BOUNDARY);

    // 3. Redraw final borders to hide the magenta
    glColor3fv(COLOR_RED);
    drawPolygonOutline(nose_cone);
    drawPolygonOutline(left_fin);
    drawPolygonOutline(right_fin);
    glColor3fv(COLOR_SILVER);
    drawPolygonOutline(body);
    glColor3fv(COLOR_DARK_GREY);
    drawPolygonOutline(exhaust_funnel);
}

// ####################################################################
// ## GLUT AND OPENGL SETUP
// ####################################################################

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawStars();
    drawPlanets();
    drawRocket({(int)rocket_x, (int)rocket_y}, rocket_angle_deg);
    glutSwapBuffers();
}

void update(int value) {
    // Define the start, end, and peak of the parabolic arc
    Point start = {100, 370}; // Top of left planet
    Point end = {700, 400};   // Top of right planet
    Point peak = {400, 550};  // Peak of the arc, high in the center

    if (t_param < 1.0f) {
        t_param += animation_speed;

        // Quadratic Bezier curve formula for the parabola
        float t = t_param;
        float one_minus_t = 1.0f - t;
        
        // Position
        rocket_x = one_minus_t * one_minus_t * start.x + 2 * one_minus_t * t * peak.x + t * t * end.x;
        rocket_y = one_minus_t * one_minus_t * start.y + 2 * one_minus_t * t * peak.y + t * t * end.y;

        // Derivative of the Bezier curve for the tangent angle
        float dx = 2 * one_minus_t * (peak.x - start.x) + 2 * t * (end.x - peak.x);
        float dy = 2 * one_minus_t * (peak.y - start.y) + 2 * t * (end.y - peak.y);
        
        rocket_angle_deg = atan2(dy, dx) * 180.0f / PI;

    } else {
        // Animation finished, you can add logic here to stop or reset
    }

    glutPostRedisplay();          // Tell GLUT to redraw the scene
    glutTimerFunc(16, update, 0); // Request the next update
}

void myInit(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
}

int main(int argc, char** argv) {
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Space Scene - Animated");
    myInit();
    glutDisplayFunc(display);
    glutTimerFunc(25, update, 0); // Start the animation loop
    glutMainLoop();
    return 0;
}
