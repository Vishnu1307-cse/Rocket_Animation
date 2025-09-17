#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#define PI 3.1415926535

//initial position
float rocket_x = 100.0f;
float rocket_y = 370.0f;
float rocket_angle_deg = 0.0f;
float t_param = 0.0f;
const float animation_speed = 0.001f;

//Clipping window size
float cam_left = 50, cam_right = 150;
float cam_bottom = 250, cam_top = 450;

const float COLOR_BLACK[]   = {0.0f, 0.0f, 0.0f};
const float COLOR_WHITE[]   = {1.0f, 1.0f, 1.0f};
const float COLOR_RED[]     = {1.0f, 0.0f, 0.0f};
const float COLOR_SILVER[]  = {0.8f, 0.8f, 0.9f};
const float COLOR_DARKGREY[]= {0.3f, 0.3f, 0.3f};
const float COLOR_BLUE[]    = {0.2f, 0.4f, 1.0f};
const float COLOR_ORANGE[]  = {0.9f, 0.3f, 0.1f};

struct Point { int x, y; };

// Bresenham Line Drawing Algorithm
void drawLine(Point p0, Point p1, const float color[3]) {
    glColor3fv(color);
    int x0 = p0.x, y0 = p0.y;
    int x1 = p1.x, y1 = p1.y;
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = x0<x1 ? 1 : -1, sy = y0<y1 ? 1 : -1;
    int err = dx - dy;

    while(true) {
        glBegin(GL_POINTS);
        glVertex2i(x0, y0);
        glEnd();
        if(x0 == x1 && y0 == y1) break;
        int e2 = 2*err;
        if(e2 > -dy) { err -= dy; x0 += sx; }
        if(e2 < dx) { err += dx; y0 += sy; }
    }
}

// Scanline Fill Algorithm
void scanlineFillPolygon(const std::vector<Point>& polygon, const float color[3]) {
    if(polygon.empty()) return;
    int minY = polygon[0].y, maxY = polygon[0].y;
    for(auto& p: polygon) { if(p.y < minY) minY = p.y; if(p.y > maxY) maxY = p.y; }

    for(int y=minY; y<=maxY; y++){
        std::vector<int> intersections;
        for(size_t i=0;i<polygon.size();i++){
            Point p1 = polygon[i], p2 = polygon[(i+1)%polygon.size()];
            if(p1.y == p2.y) continue;
            if((y >= std::min(p1.y,p2.y)) && (y <= std::max(p1.y,p2.y))){
                int x = int(p1.x + (float)(y - p1.y)*(p2.x - p1.x)/(p2.y - p1.y));
                intersections.push_back(x);
            }
        }
        std::sort(intersections.begin(), intersections.end());
        glColor3fv(color);
        for(size_t i=0;i+1<intersections.size();i+=2){
            glBegin(GL_POINTS);
            for(int x=intersections[i]; x<=intersections[i+1]; x++)
                glVertex2i(x,y);
            glEnd();
        }
    }
}

// Midpoint Circle Drawing Algorithm
void drawCircleOutline(int cx, int cy, int r, const float color[3]) {
    glColor3fv(color);
    int x = 0, y = r;
    int d = 1 - r;
    while(y >= x) {
        glBegin(GL_POINTS);
        glVertex2i(cx + x, cy + y); glVertex2i(cx - x, cy + y);
        glVertex2i(cx + x, cy - y); glVertex2i(cx - x, cy - y);
        glVertex2i(cx + y, cy + x); glVertex2i(cx - y, cy + x);
        glVertex2i(cx + y, cy - x); glVertex2i(cx - y, cy - x);
        glEnd();
        x++;
        if(d < 0) d += 2*x + 1;
        else { y--; d += 2*(x - y) + 1; }
    }
}

// Scanline Fill Algorithm
void drawFilledCircleScanline(int cx, int cy, int r, const float color[3]) {
    glColor3fv(color);
    for(int y=-r; y<=r; y++){
        int dx = int(sqrt(r*r - y*y));
        int x_start = cx - dx;
        int x_end   = cx + dx;
        glBegin(GL_POINTS);
        for(int x=x_start; x<=x_end; x++) glVertex2i(x, cy+y);
        glEnd();
    }
}

// Stars (Random pixel glow)
void drawStars() {
    glColor3fv(COLOR_WHITE);
    glBegin(GL_POINTS);
    for(int i=0;i<200;i++) glVertex2i(rand()%800, rand()%600);
    glEnd();
}

// Flames
void drawFlames(Point baseL, Point baseR, float angle_deg, float scale) {
    float flame_angle_rad = (angle_deg - 90.0f) * PI / 180.0f;
    float dir_x = cos(flame_angle_rad);
    float dir_y = sin(flame_angle_rad);

    for(int i=0;i<15;i++){
        float t = (float)rand()/RAND_MAX;
        Point base = { int(baseL.x + t*(baseR.x-baseL.x)), int(baseL.y + t*(baseR.y-baseL.y)) };
        int flame_length = int((10 + rand()%20) * scale);
        Point tip = { int(base.x + flame_length*dir_x), int(base.y + flame_length*dir_y) };
        drawLine({base.x-2, base.y}, tip, COLOR_ORANGE);
        drawLine({base.x+2, base.y}, tip, COLOR_ORANGE);
    }
}

// Rocket
void drawRocket(Point translate, float angle_deg, float scale){
    float rad = angle_deg * PI / 180.0f;
    float cosA = cos(rad), sinA = sin(rad);

    auto transform = [&](Point p){
        return Point{ int(scale*(p.x*cosA - p.y*sinA)) + translate.x,
                      int(scale*(p.x*sinA + p.y*cosA)) + translate.y };
    };

    std::vector<Point> nose = {transform({0,40}), transform({-15,10}), transform({15,10})};
    std::vector<Point> body = {transform({-15,10}), transform({15,10}), transform({15,-30}), transform({-15,-30})};
    std::vector<Point> leftFin = {transform({-15,0}), transform({-15,-25}), transform({-25,-35})};
    std::vector<Point> rightFin = {transform({15,0}), transform({15,-25}), transform({25,-35})};
    std::vector<Point> exhaust = {transform({-10,-30}), transform({10,-30}), transform({15,-40}), transform({-15,-40})};
    Point exhaustBaseL = transform({-15,-40});
    Point exhaustBaseR = transform({15,-40});

    drawFlames(exhaustBaseL, exhaustBaseR, angle_deg, scale);
    scanlineFillPolygon(body, COLOR_WHITE);
    scanlineFillPolygon(nose, COLOR_RED);
    scanlineFillPolygon(leftFin, COLOR_RED);
    scanlineFillPolygon(rightFin, COLOR_RED);

    // Draw edges for outlines
    for(int i=0;i<nose.size();i++) drawLine(nose[i], nose[(i+1)%nose.size()], COLOR_RED);
    for(int i=0;i<body.size();i++) drawLine(body[i], body[(i+1)%body.size()], COLOR_WHITE);
    for(int i=0;i<leftFin.size();i++) drawLine(leftFin[i], leftFin[(i+1)%leftFin.size()], COLOR_RED);
    for(int i=0;i<rightFin.size();i++) drawLine(rightFin[i], rightFin[(i+1)%rightFin.size()], COLOR_RED);
    for(int i=0;i<exhaust.size();i++) drawLine(exhaust[i], exhaust[(i+1)%exhaust.size()], COLOR_DARKGREY);
}

//Planets
void drawPlanets(){
    drawFilledCircleScanline(100, 300, 50, COLOR_BLUE);
    drawCircleOutline(100, 300, 50, COLOR_BLUE);

    drawFilledCircleScanline(700, 300, 80, COLOR_RED);
    drawCircleOutline(700, 300, 80, COLOR_RED);
}

//Display
void display(){
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(cam_left, cam_right, cam_bottom, cam_top);

    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    drawStars();
    drawPlanets();

    float scale_factor = 1.0f - fabs(0.5f - t_param);
    drawRocket({int(rocket_x), int(rocket_y)}, rocket_angle_deg, scale_factor);

    glutSwapBuffers();
}

//Update rocket position (Scaling, rotation and transformation application)
void update(int value){
    Point start={100,370}, end={700,400}, peak={400,550};
    if(t_param<1.0f){
        t_param += animation_speed;
        float t=t_param, u=1.0f-t;

        rocket_x = u*u*start.x + 2*u*t*peak.x + t*t*end.x;
        rocket_y = u*u*start.y + 2*u*t*peak.y + t*t*end.y;

        rocket_angle_deg = -360.0f * t_param * (1.0f - t_param);

        cam_left   = 50 - 50*t;
        cam_right  = 150 + 650*t;
        cam_bottom = 250 - 250*t;
        cam_top    = 450 + 150*t;
    } else{
        static float zoom_t=0.0f;
        if(zoom_t<1.0f) zoom_t+=0.005f;
        cam_left   = 600 - 50*(1-zoom_t);
        cam_right  = 800 + 50*(1-zoom_t);
        cam_bottom = 200 - 50*(1-zoom_t);
        cam_top    = 400 + 50*(1-zoom_t);
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void myInit(){ glClearColor(0,0,0,1); }

int main(int argc, char** argv){
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800,600);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Interstellar 2.0");
    myInit();
    glutDisplayFunc(display);
    glutTimerFunc(25, update, 0);
    glutMainLoop();
    return 0;
}
