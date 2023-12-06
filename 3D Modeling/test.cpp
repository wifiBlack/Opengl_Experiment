#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cctype>
#include <iostream>
#include <cmath>
#include <cstring>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <fstream>
#include <sstream>
#include <GL/glext.h>

// 视角相关参数
double theta = 0.0f;
double phi = 0.0f;
double cameraDistance = 1000.0f;
double cameraHeight = 0.0f;
int lastMouseX = 0;
int lastMouseY = 0;
int mouseDown = 0;

// 鼠标按键状态
bool leftMousePressed = false;
bool rightMousePressed = false;
bool wheelMousePressed = false;
double rotationSpeed = 0.01f;
double translationSpeed = 10.0f;
double rotationSpeedZ = 0.01f;

// 窗口大小
const int windowWidth = 800;
const int windowHeight = 600;
double dx = 0;
double dy = 0;
// 坐标数据相关参数
const int nrows = 206;
const int ncols = 144;
int nrow;
int ncol;
double xllcorner;
double yllcorner;
double cellsize;
int NODATA_value;
double minn = 999999;
double maxx = 0;
GLdouble **arr = new GLdouble *[nrows];
void loadCoordinatesFromFile(const char *filename)
{
    std::ifstream inFile(filename);
    if (!inFile)
    {
        std::cerr << "读取文件时出错！" << std::endl;
        return;
    }
    std::string line;
    std::getline(inFile, line);
    std::istringstream(line.substr(6)) >> ncol;
    std::getline(inFile, line);
    std::istringstream(line.substr(6)) >> nrow;
    std::getline(inFile, line);
    std::istringstream(line.substr(10)) >> xllcorner;
    std::getline(inFile, line);
    std::istringstream(line.substr(10)) >> yllcorner;
    std::getline(inFile, line);
    std::istringstream(line.substr(9)) >> cellsize;
    std::getline(inFile, line);
    std::istringstream(line.substr(13)) >> NODATA_value;
    for (int i = 0; i < nrows; ++i)
    {
        for (int j = 0; j < ncols; ++j)
        {
            inFile >> arr[i][j];
            if (arr[i][j] == NODATA_value)
            {
                continue;
            }
            else
            {
                if (arr[i][j] > maxx)
                    maxx = arr[i][j];
                if (arr[i][j] < minn)
                    minn = arr[i][j];
            }
            // std::cout<<arr[i][j]<<std::endl;
        }
    }
    for (int i = 0; i < nrows; ++i)
    {
        for (int j = 0; j < ncols; ++j)
        {
            if (arr[i][j] == NODATA_value)
            {
                arr[i][j] = minn / 10000;
            }
            else
            {
                arr[i][j] /= 10000;
            }
        }
    }
    maxx /= 10000;
    minn /= 10000;
}

class Point3D
{
public:
    double x, y, z, w;

    Point3D(double x, double y, double z, double w = 1.0) : x(x), y(y), z(z), w(w) {}
};

class Triangle3D
{
public:
    Point3D p1, p2, p3;

    Triangle3D(Point3D p1, Point3D p2, Point3D p3) : p1(p1), p2(p2), p3(p3) {}
};
GLuint texture;
GLuint loadTexture(const char *filename)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << filename << std::endl;
        glDeleteTextures(1, &texture); // Delete the texture if loading fails
        texture = 0;                   // Set texture to 0 to indicate failure
    }

    return texture;
}

void displaytable(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)windowWidth / windowHeight, 0.1, 2000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double cameraX = cameraDistance * cos(theta) * cos(phi) - 1000 + dx;
    double cameraY = cameraDistance * sin(phi) + 30 + dy;
    double cameraZ = cameraDistance * sin(theta) * cos(phi) + 30;
    gluLookAt(cameraX, cameraY, cameraZ, 0, 0, 0, 0, 0, 1);
    glScalef(1.0f / cellsize, 1.0f / cellsize, 1.0f / cellsize);
    glBindTexture(GL_TEXTURE_2D, texture);
    // glColor3f(0.0, 0.0, 0.0);
    for (int i = 0; i < nrows - 1; ++i)
    {
        for (int j = 0; j < ncols - 1; ++j)
        {
            double z1 = arr[i][j];
            double z2 = arr[i + 1][j];
            double z3 = arr[i][j + 1];
            double z4 = arr[i + 1][j + 1];
            double x1 = (i)*1.0 / nrows;
            double x2 = (i + 1) * 1.0 / nrows;
            double x3 = (i)*1.0 / nrows;
            double x4 = (i + 1) * 1.0 / nrows;
            double y1 = j * 1.0 / ncols;
            double y2 = (j)*1.0 / ncols;
            double y3 = (j + 1) * 1.0 / ncols;
            double y4 = (j + 1) * 1.0 / ncols;
            // printf("%f", x1);
            glBegin(GL_LINES);
            glTexCoord2d(x1, y1);
            // printf("%f", x1);
            glVertex3d(j * cellsize, arr[i][j], i * cellsize);
            glTexCoord2d(x3, y3);
            glVertex3d((j + 1) * cellsize, arr[i][j + 1], i * cellsize);

            glTexCoord2d(x3, y3);
            glVertex3d((j + 1) * cellsize, arr[i][j + 1], i * cellsize);
            glTexCoord2d(x2, y2);
            glVertex3d(j * cellsize, arr[i + 1][j], (i + 1) * cellsize);

            glTexCoord2d(x2, y2);
            glVertex3d(j * cellsize, arr[i + 1][j], (i + 1) * cellsize);
            glTexCoord2d(x1, y1);
            glVertex3d(j * cellsize, arr[i][j], i * cellsize);
            glEnd();
            //  glBegin(GL_TRIANGLES);
            // glTexCoord2d(x1,y1); glVertex3d(i * cellsize,arr[i][j],j * cellsize);
            // glTexCoord2d(x4,y4); glVertex3d((i+1) * cellsize,arr[i+1][j + 1],(j + 1) * cellsize);
            // glTexCoord2d(x2,y2); glVertex3d((i + 1) * cellsize,arr[i + 1][j] ,j * cellsize);
            // glEnd();

            //  glBegin(GL_TRIANGLES);
            // glTexCoord2d(x1,y1); glVertex3d(i * cellsize,arr[i][j],j * cellsize);
            // glTexCoord2d(x3,y3); glVertex3d(i * cellsize,arr[i][j + 1],(j + 1) * cellsize);
            // glTexCoord2d(x4,y4); glVertex3d((i + 1) * cellsize,arr[i + 1][j+1] ,(j+1) * cellsize);
            // glEnd();
        }
    }

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glFlush();
    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            leftMousePressed = true;
            // 更新鼠标最后的坐标
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
            leftMousePressed = false;
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            rightMousePressed = true;
            // 更新鼠标最后的坐标
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
            rightMousePressed = false;
    }
}

void motion(int x, int y)
{
    if (leftMousePressed)
    {
        // 平移操作
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        // 根据deltaX和deltaY修改相机位置
        dx += deltaX;
        dy -= deltaY; // 注意y坐标的反向变化

        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
    else if (rightMousePressed)
    {
        // 旋转操作
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        // 根据deltaX和deltaY修改theta和phi
        theta += deltaX * rotationSpeed;
        phi += deltaY * rotationSpeed;

        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
    for (int i = 0; i < nrows; ++i)
    {
        arr[i] = new GLdouble[ncols];
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("3D Transformations");
    loadCoordinatesFromFile("dem.asc");
    GLuint texture = loadTexture("dom.jpg");
    glutDisplayFunc(displaytable);

    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();

    for (int i = 0; i < nrows; ++i)
    {
        delete[] arr[i];
    }
    delete[] arr;
    return 0;
}