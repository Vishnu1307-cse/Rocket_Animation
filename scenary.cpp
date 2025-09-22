#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

#define WIDTH 800
#define HEIGHT 600

void putPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}


//init (x1,y1) p=2dx-dy; x++; p=p+2dy; p=p+2dy-2dx y++;
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        putPixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

//x=x1+t1*dx y=y1+t2*dy
bool liangBarsky(float x0, float y0, float x1, float y1,
                 float xmin, float ymin, float xmax, float ymax,
                 float &cx0, float &cy0, float &cx1, float &cy1) {
    float dx = x1 - x0, dy = y1 - y0;
    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {x0 - xmin, xmax - x0, y0 - ymin, ymax - y0};
    float u1 = 0.0f, u2 = 1.0f;

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            if (q[i] < 0) return false;
        } else {
            float u = q[i] / p[i];
            if (p[i] < 0) {
                if (u > u1) u1 = u;
            } else {
                if (u < u2) u2 = u;
            }
        }
    }

    if (u1 > u2) return false;

    cx0 = x0 + u1 * dx;
    cy0 = y0 + u1 * dy;
    cx1 = x0 + u2 * dx;
    cy1 = y0 + u2 * dy;
    return true;
}

//init (0,r) p =1-r; (x+1,y) p=p+2x+3; (x+1,y-1) p=p+2(x-y)+5
void midpointCircle(int xc, int yc, int r) {
    int x = 0, y = r;
    int d = 1 - r;
    while (y >= x) {
        putPixel(xc + x, yc + y);
        putPixel(xc - x, yc + y);
        putPixel(xc + x, yc - y);
        putPixel(xc - x, yc - y);
        putPixel(xc + y, yc + x);
        putPixel(xc - y, yc + x);
        putPixel(xc + y, yc - x);
        putPixel(xc - y, yc - x);
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

//ymax x(ymin) 1/m
void scanlineFill(vector<pair<int,int>> polygon) {
    int n = polygon.size();
    int ymin = 9999, ymax = -9999;
    for (int i = 0; i < n; i++) {
        ymin = min(ymin, polygon[i].second);
        ymax = max(ymax, polygon[i].second);
    }
    for (int y = ymin; y <= ymax; y++) {
        vector<float> nodes;
        int j = n - 1;
        for (int i = 0; i < n; i++) {
            int xi = polygon[i].first, yi = polygon[i].second;
            int xj = polygon[j].first, yj = polygon[j].second;
            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                float x = xi + (float)(y - yi) * (xj - xi) / (yj - yi);
                nodes.push_back(x);
            }
            j = i;
        }
        sort(nodes.begin(), nodes.end());
        for (int i = 0; i < (int)nodes.size(); i += 2) {
            if (i + 1 < (int)nodes.size()) {
                for (int x = (int)nodes[i]; x < (int)nodes[i+1]; x++) {
                    putPixel(x, y);
                }
            }
        }
    }
}

pair<int,int> applyScaling(int x, int y, float sx, float sy) {
    return { (int)(x * sx), (int)(y * sy) };
}

void drawSun(int xc, int yc, int r) {
    vector<pair<int,int>> circle;
    int num_segments = 360;
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        int x = xc + (int)(r * cos(theta));
        int y = yc + (int)(r * sin(theta));
        circle.push_back({x, y});
    }
    glColor3f(1.0, 1.0, 0.0);
    scanlineFill(circle);
    glColor3f(0.0, 0.0, 0.0);
    midpointCircle(xc, yc, r);
}

float zoomFactor = 5.0f;

void drawMountains() {
    glColor3f(0.2, 0.2, 0.2);
    vector<pair<int,int>> mountain1 = {{100,100}, {250,300}, {400,100}};
    scanlineFill(mountain1);
    vector<pair<int,int>> mountain2 = {{300,100}, {450,350}, {600,100}};
    scanlineFill(mountain2);
}

void drawLakeBase() {
    glColor3f(0.0, 0.0, 0.5);
    vector<pair<int,int>> rect = {{0,0}, {WIDTH,0}, {WIDTH,100}, {0,100}};
    scanlineFill(rect);
}

void drawLakeReflection() {
    glColor3f(0.15, 0.15, 0.15);
    vector<pair<int,int>> mountain1 = {{100,100}, {250,300}, {400,100}};
    for (auto &p : mountain1) {
        auto ref = applyScaling(p.first, p.second-100, 1, -1);
        p.second = ref.second + 100;
    }
    scanlineFill(mountain1);
    vector<pair<int,int>> mountain2 = {{300,100}, {450,350}, {600,100}};
    for (auto &p : mountain2) {
        auto ref = applyScaling(p.first, p.second-100, 1, -1);
        p.second = ref.second + 100;
    }
    scanlineFill(mountain2);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (zoomFactor > 1.2f) {
        drawSun(500, 450, (int)(40 * zoomFactor));
    } else {
        glColor3f(0.0, 0.0, 0.0);


        float cx0, cy0, cx1, cy1;
        if (liangBarsky(0, 100, WIDTH, 100, 0, 0, WIDTH, HEIGHT, cx0, cy0, cx1, cy1)) {
            drawLineBresenham((int)cx0, (int)cy0, (int)cx1, (int)cy1);
        }

        drawMountains();
        drawLakeBase();
        drawLakeReflection();
        drawSun(500, 450, (int)(40 * zoomFactor));
    }
    glFlush();
}

void update(int value) {
    if (zoomFactor > 1.0f) zoomFactor -= 0.05f;
    glutPostRedisplay();
    glutTimerFunc(30, update, 0);
}

void init() {
    glClearColor(0.8, 0.9, 1.0, 1.0);
    glColor3f(0.2, 0.2, 0.2);
    glPointSize(1);
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Scenery");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(30, update, 0);
    glutMainLoop();
    return 0;
}
