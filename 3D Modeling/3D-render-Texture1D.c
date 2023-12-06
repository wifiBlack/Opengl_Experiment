#include <GL/gl.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>

// 全局变量用于存储高程数据
float elevationData[300][300];
int numRows, numCols;
float cellSize, noDataValue;
float theta = 0.0f;
float phi = 0.0f;
float cameraDistance = 1000;
int lastMouseX = 0;
int lastMouseY = 0;
int lastMouseX2 = 0;
int lastMouseY2 = 0;
float center[3] = {70, 70, 50};
int zmax = 0, zmin = 20000;
float deltaz;

int isRotating = 0,
    isTranslate = 0;

GLuint textureID;
int textureWidth = 16;             // 纹理宽度
unsigned char textureData[16 * 3]; // 纹理数据

// 设置窗口大小
int windowWidth = 800;
int windowHeight = 600;

void calculateNormals();

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // 创建一维纹理
    int _tex_1d[16] = {0x991100,
                       0xCC1100,
                       0xC84500,
                       0xCFC000,
                       0x54CE96,
                       0x00CFC0,
                       0x00FFC0,
                       0x56D660,
                       0x23CF00,
                       0x00C100,
                       0x26972F,
                       0x218528,
                       0x1B824C,
                       0x3D6283,
                       0x65949f,
                       0xF6F1E9};
    for (int i = 0; i < textureWidth; i++)
    {
        textureData[i * 3] = (unsigned char)(_tex_1d[i] & 0xFF);
        textureData[i * 3 + 1] = (unsigned char)(_tex_1d[i] >> 8 & 0xFF);
        textureData[i * 3 + 2] = (unsigned char)(_tex_1d[i] >> 16 & 0xFF);
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_1D, textureID);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 为纹理对象提供数据
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, textureWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
}

// 加载DEM数据函数
void loadDEMData(const char *filename)
{
    // 打开DEM.asc文件
    FILE *demFile = freopen(filename, "r", stdin);
    if (demFile == NULL)
    {
        printf("无法打开DEM.asc文件\n");
        return;
    }

    // 读取DEM.asc文件的头部信息
    fscanf(demFile, "ncols %d\n", &numCols);
    fscanf(demFile, "nrows %d\n", &numRows);
    fscanf(demFile, "xllcorner %*f\n"); // 忽略不需要的字段
    fscanf(demFile, "yllcorner %*f\n"); // 忽略不需要的字段
    fscanf(demFile, "cellsize %f\n", &cellSize);
    fscanf(demFile, "NODATA_value %f\n", &noDataValue);

    // 读取DEM.asc文件中的高程数据
    int i = 0, j = 0;
    for (i = 0; i < numRows; i++)
    {
        for (j = 0; j < numCols; j++)
        {
            float elevat;
            fscanf(demFile, "%f", &elevat);
            if (elevat < 0)
                elevationData[i][j] = 30;
            else
            {
                elevationData[i][j] = elevat / 1000;
                if (zmax < elevat)
                    zmax = elevat;
                else if (zmin > elevat)
                    zmin = elevat;
            }
        }
    }

    deltaz = (zmax - zmin) / 1000;
    printf("(%d, %d, %4f)\n", zmax, zmin, deltaz);
    // 关闭文件
    fclose(demFile);
    return;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 设置交互视角
    gluPerspective(45.0, (float)windowWidth / windowHeight, 0.1, 3000.0);

    glColor3f(1, 1, 1);

    // 根据球坐标系计算相机的位置
    float cameraX = cameraDistance * cos(theta) * cos(phi) + center[0];
    float cameraY = cameraDistance * sin(phi) + center[1];
    float cameraZ = cameraDistance * sin(theta) * cos(phi) + center[2];

    gluLookAt(cameraX, cameraY, cameraZ, center[0], center[1], center[2], 0, cos(phi), 0);

    // 启用纹理映射
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, textureID);
    // 渲染DEM数据
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numRows - 1; i++)
    {
        for (int j = 0; j < numCols - 1; j++)
        {
            float x0 = j * cellSize;
            float y0 = i * cellSize;
            float z0 = elevationData[i][j];

            float x1 = (j + 1) * cellSize;
            float y1 = i * cellSize;
            float z1 = elevationData[i][j + 1];

            float x2 = j * cellSize;
            float y2 = (i + 1) * cellSize;
            float z2 = elevationData[i + 1][j];

            float x3 = (j + 1) * cellSize;
            float y3 = (i + 1) * cellSize;
            float z3 = elevationData[i + 1][j + 1];
            glTexCoord1f((z0 - zmin / 1000) / deltaz);
            glVertex3f(x0, z0, y0);
            glTexCoord1f((z1 - zmin / 1000) / deltaz);
            glVertex3f(x1, z1, y1);
            glTexCoord1f((z3 - zmin / 1000) / deltaz);
            glVertex3f(x3, z3, y3);

            glTexCoord1f((z0 - zmin / 1000) / deltaz);
            glVertex3f(x0, z0, y0);
            glTexCoord1f((z2 - zmin / 1000) / deltaz);
            glVertex3f(x2, z2, y2);
            glTexCoord1f((z3 - zmin / 1000) / deltaz);
            glVertex3f(x3, z3, y3);

            glTexCoord1f((z0 - zmin / 1000) / deltaz);
            glVertex3f(x0, z0, y0);
            glTexCoord1f((z1 - zmin / 1000) / deltaz);
            glVertex3f(x1, z1, y1);
            glTexCoord1f((z2 - zmin / 1000) / deltaz);
            glVertex3f(x2, z2, y2);

            glTexCoord1f((z1 - zmin / 1000) / deltaz);
            glVertex3f(x1, z1, y1);
            glTexCoord1f((z2 - zmin / 1000) / deltaz);
            glVertex3f(x2, z2, y2);
            glTexCoord1f((z3 - zmin / 1000) / deltaz);
            glVertex3f(x3, z3, y3);
        }
    }
    glEnd();
    glutSwapBuffers();
    glDisable(GL_TEXTURE_1D);
}

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            isRotating = 1;
            isTranslate = 0;
            // 更新鼠标最后的坐标
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
        {
            isRotating = 0;
            isTranslate = 0;
        }
    }
    else if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            isRotating = 0;
            isTranslate = 1;
            lastMouseX2 = x;
            lastMouseY2 = y;
        }
        else if (state == GLUT_UP)
        {
            isRotating = 0;
            isTranslate = 0;
        }
    }
}

void motion(int x, int y)
{
    if (isRotating)
    {
        // theta的取值根据x的变化值进行变化
        int deltaX = x - lastMouseX;
        theta += (float)(deltaX)*0.01f;

        // phi的取值根据y的变化而变化
        int deltaY = y - lastMouseY;
        phi += (float)(deltaY)*0.01f;

        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
    if (isTranslate)
    {
        int deltaX = x - lastMouseX2;
        int deltaY = y - lastMouseY2;
        center[0] += deltaX;
        center[2] += deltaY;
        lastMouseX2 = x;
        lastMouseY2 = y;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{

    // 初始化OpenGL窗口和GLUT

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Terrain 3D Render");

    // 加载DEM数据到全局变量
    init();
    loadDEMData("dem.asc");

    glEnable(GL_NORMALIZE);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
    return 0;
}