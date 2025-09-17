#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

// Define a constant for the value of PI
#define PI 3.1415926535

// ####################################################################
// ## DATA STRUCTURES & HELPERS
// ####################################################################

struct Point { int x, y; };

// Global color definitions
const float COLOR_BLACK[] = {0.0f, 0.0f, 0.0f};
const float COLOR_WHITE[] = {1.0f, 1.0f, 1.0f};
const float COLOR_RED[] = {1.0f, 0.0f, 0.0f};
const float COLOR_SILVER[] = {0.8f, 0.8f, 0.9f};
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
    glFlush(); // CRITICAL: Ensure boundary is drawn before filling
    boundaryFill4(100, 300, COLOR_BLUE, COLOR_BOUNDARY); // 2. Fill with blue
    glColor3fv(COLOR_BLUE); // 3. Redraw border with final color
    midpointCircle(100, 300, 50);

    // Planet 2 (Mars-like)
    glColor3fv(COLOR_BOUNDARY); // 1. Draw temporary magenta boundary
    midpointCircle(700, 300, 80);
    glFlush(); // CRITICAL: Ensure boundary is drawn before filling
    boundaryFill4(700, 300, COLOR_ORANGE, COLOR_BOUNDARY); // 2. Fill with orange
    glColor3fv(COLOR_ORANGE); // 3. Redraw border with final color
    midpointCircle(700, 300, 80);
}

void drawRocket() {
    // --- Manual Transformation Setup ---
    float angle_rad = 25.0f * PI / 180.0f;
    float cosA = cos(angle_rad), sinA = sin(angle_rad);
    Point translate = {180, 280};

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

    // 1. Draw all temporary boundaries first
    glColor3fv(COLOR_BOUNDARY);
    drawPolygonOutline(nose_cone);
    drawPolygonOutline(body);
    drawPolygonOutline(left_fin);
    drawPolygonOutline(right_fin);
    glFlush(); // CRITICAL: Ensure all boundaries are drawn before any filling

    // 2. Fill all shapes
    boundaryFill4(getCentroid(nose_cone).x, getCentroid(nose_cone).y, COLOR_RED, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(body).x, getCentroid(body).y, COLOR_SILVER, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(left_fin).x, getCentroid(left_fin).y, COLOR_RED, COLOR_BOUNDARY);
    boundaryFill4(getCentroid(right_fin).x, getCentroid(right_fin).y, COLOR_RED, COLOR_BOUNDARY);

    // 3. Redraw final borders to hide the magenta
    glColor3fv(COLOR_RED);
    drawPolygonOutline(nose_cone);
    drawPolygonOutline(left_fin);
    drawPolygonOutline(right_fin);
    glColor3fv(COLOR_SILVER);
    drawPolygonOutline(body);
}

// ####################################################################
// ## GLUT AND OPENGL SETUP
// ####################################################################

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawStars();
    drawPlanets();
    drawRocket();
    glFlush(); // Final flush to render everything to the screen
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
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Space Scene - Boundary Fill Corrected");
    myInit();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
