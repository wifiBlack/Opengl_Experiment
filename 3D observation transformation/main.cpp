#define STB_IMAGE_IMPLEMENTATION
#include <GL/freeglut.h>
#include "stb_image.h"

float mouseX, mouseY;
bool mouseLeftDown;
bool mouseRightDown;

float elevationData[206][143];
static GLfloat triangleVertices[206][143][12] = {};
int numCols;
int numRows;
float cellSize;
float noDataValue;

float max = elevationData[0][0];

int mode = 0;

// 立方体顶点坐标
GLfloat cubeVertices[] = {
    // 前面
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    // 后面
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f};

// 立方体索引
GLubyte cubeIndices[] = {
    // 前面
    0, 1, 2, 3,
    // 后面
    4, 5, 6, 7,
    // 左面
    0, 3, 7, 4,
    // 右面
    1, 2, 6, 5,
    // 上面
    3, 2, 6, 7,
    // 下面
    0, 1, 5, 4};

// 窗口大小
int windowWidth = 800;
int windowHeight = 600;

// 立方体旋转角度
GLfloat rotateX = -42.0f;
GLfloat rotateY = 0.0f;

// 设置观察点
GLfloat cameraX = -0.0f; // 摄像机X轴位置
GLfloat cameraY = -0.0f; // 摄像机Y轴位置
GLfloat cameraZ = -0.4f; // 摄像机Z轴位置

GLuint texture = 0;
GLuint loadTexture(const char *filename)
{
  // glEnable(GL_TEXTURE_GEN_S);
  // glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE);
  glGenTextures(1, &texture);
  // printf("%d\n", texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // glBindTexture(GL_TEXTURE_2D, texture);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, STBI_rgb_alpha);
  //
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
  }
  else
  {
    printf("%s", "Fail!");
    glDeleteTextures(1, &texture); // Delete the texture
    texture = 0;                   // Set texture to 0 to indicate failure
  }

  return texture;
}

void loadData()
{
  // 打开DEM.asc文件
  FILE *demFile = fopen("dem.asc", "r");
  if (demFile == NULL)
  {
    printf("无法打开DEM.asc文件\n");
  }

  // 读取DEM.asc文件的头部信息

  fscanf(demFile, "ncols %d\n", &numCols);
  fscanf(demFile, "nrows %d\n", &numRows);
  fscanf(demFile, "xllcorner %*f\n"); // 忽略不需要的字段
  fscanf(demFile, "yllcorner %*f\n"); // 忽略不需要的字段
  fscanf(demFile, "cellsize %f\n", &cellSize);
  fscanf(demFile, "NODATA_value %f\n", &noDataValue);

  // 读取DEM.asc文件中的高程信息
  for (int i = 0; i < numRows; i++)
  {
    for (int j = 0; j < numCols; j++)
    {
      fscanf(demFile, "%f", &elevationData[i][j]);
      if (elevationData[i][j] == noDataValue)
      {
        elevationData[i][j] = 0.0;
      }
      else
      {
        elevationData[i][j] = elevationData[i][j] / 1000;
      }
    }
  }

  for (int i = 0; i < numRows; ++i)
  {
    for (int j = 0; j < numCols; ++j)
    {
      if (elevationData[i][j] > max)
      {
        max = elevationData[i][j];
      }
    }
  }
  // printf("%f", max);
  int maxRow = numRows;
  int maxCol = numCols;
  // 归一化并创建缓冲数据
  for (int i = 0; i < numRows - 1; ++i)
  {
    for (int j = 0; j < numCols - 1; ++j)
    {

      triangleVertices[i][j][0] = (float)i / (float)maxRow - 0.5;
      triangleVertices[i][j][1] = (float)j / (float)maxCol - 0.5;
      triangleVertices[i][j][2] = elevationData[i][j] / max / 10 - 0.3;

      triangleVertices[i][j][3] = (float)(i + 1) / (float)maxRow - 0.5;
      triangleVertices[i][j][4] = (float)j / (float)maxCol - 0.5;
      triangleVertices[i][j][5] = elevationData[i + 1][j] / max / 10 - 0.3;

      triangleVertices[i][j][6] = (float)i / (float)maxRow - 0.5;
      triangleVertices[i][j][7] = (float)(j + 1) / (float)maxCol - 0.5;
      triangleVertices[i][j][8] = elevationData[i][j + 1] / max / 10 - 0.3;

      triangleVertices[i][j][9] = (float)(i + 1) / (float)maxRow - 0.5;
      triangleVertices[i][j][10] = (float)(j + 1) / (float)maxCol - 0.5;
      triangleVertices[i][j][11] = elevationData[i + 1][j + 1] / max / 10 - 0.3;
    }
  }

  // 关闭文件
  fclose(demFile);
}

void Draw_1()
{

  if (mode == 0)
  {
    glEnable(GL_DEPTH_TEST);
  }

  else if (mode == 1)
  {
    glDisable(GL_DEPTH_TEST);
  }
  else if (mode == 2)
  {
    glDepthFunc(GL_ALWAYS);
  }
  else if (mode == 3)
  {
    glDepthFunc(GL_LESS);
  }
  else if (mode == 4)
  {
    glDepthFunc(GL_GREATER);
  }
  glColor3f(1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  glPushMatrix();
  // 设置视角
  glTranslatef(cameraX, cameraY, cameraZ);
  glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
  glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
  // printf("%f\n", rotateX);
  // printf("%f\n", rotateY);
  // printf("%f\n", cameraZ);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glPolygonMode(GL_FRONT, GL_FILL);
  glBegin(GL_QUADS);

  // Front face
  glTexCoord2f(1.0, 1.0);
  glVertex3f(0.25 / 4, -0.25 / 4, 0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, 0.25 / 4);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, 0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.25 / 4, 0.25 / 4, 0.25 / 4);

  // Back face
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, -0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.25 / 4, -0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(0.25 / 4, 0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, -0.25 / 4);

  // Left face
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, -0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, 0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, 0.25 / 4);

  // Right face
  glTexCoord2f(1.0, 1.0);
  glVertex3f(0.25 / 4, -0.25 / 4, 0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.25 / 4, 0.25 / 4, 0.25 / 4);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(0.25 / 4, 0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(0.25 / 4, -0.25 / 4, -0.25 / 4);

  // Top face
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(0.25 / 4, 0.25 / 4, -0.25 / 4);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(0.25 / 4, 0.25 / 4, 0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-0.25 / 4, 0.25 / 4, 0.25 / 4);

  // Bottom face
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, -0.25 / 4);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.25 / 4, -0.25 / 4, 0.25 / 4);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(0.25 / 4, -0.25 / 4, 0.25 / 4);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.25 / 4, -0.25 / 4, -0.25 / 4);
  glEnd();

  glPopMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE);
}

void Draw_dem()
{
  glPushMatrix();
  glTranslatef(cameraX, cameraY, cameraZ);
  glRotatef(-42.0f, 1.0f, 0.0f, 0.0f);
  // glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < numRows - 1; ++i)
  {
    for (int j = 0; j < numCols - 1; ++j)
    {

      // 0

      glVertex3f(triangleVertices[i][j][0], triangleVertices[i][j][1], triangleVertices[i][j][2]);
      // 1

      glVertex3f(triangleVertices[i][j][3], triangleVertices[i][j][4], triangleVertices[i][j][5]);
      // 2

      glVertex3f(triangleVertices[i][j][6], triangleVertices[i][j][7], triangleVertices[i][j][8]);

      // 1

      glVertex3f(triangleVertices[i][j][3], triangleVertices[i][j][4], triangleVertices[i][j][5]);
      // 2

      glVertex3f(triangleVertices[i][j][6], triangleVertices[i][j][7], triangleVertices[i][j][8]);
      // 3

      glVertex3f(triangleVertices[i][j][9], triangleVertices[i][j][10], triangleVertices[i][j][11]);
      // glVertexPointer(3, GL_FLOAT, 0, triangleVertices[i][j]);
      // glDrawElements(GL_TRIANGLES, 4 * 3, GL_UNSIGNED_BYTE, triangleIndices);
    }
  }
  glEnd();
  glPopMatrix();
}

// 绘制立方体和立方体的轮廓线
void renderScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 颜色缓冲全部设置为黑色，深度缓冲全部设置为最大值

  glLoadIdentity();

  // 设置视口和投影矩阵
  glViewport(0, 0, windowWidth, windowHeight);
  glMatrixMode(GL_PROJECTION); // 投影坐标系，实现近大远小的效果
  glLoadIdentity();
  gluPerspective(45.0f, (float)windowWidth / (float)windowHeight, 0.01f, 100.0f);
  glMatrixMode(GL_MODELVIEW);

  Draw_1();
  Draw_dem();

  glutSwapBuffers();
}

void mouseButton(int button, int state, int x, int y)
{
  mouseX = x;
  mouseY = y;
  if (button == GLUT_LEFT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouseLeftDown = true;
    }
    else if (state == GLUT_UP)
    {
      mouseLeftDown = false;
    }
  }
  if (button == GLUT_RIGHT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouseRightDown = true;
    }
    else if (state == GLUT_UP)
    {
      mouseRightDown = false;
    }
  }
}

void mouseMoveEvent(int x, int y)
{
  if (mouseRightDown)
  {
    rotateY += ((x - mouseX) / 2.0);
    rotateX += ((y - mouseY) / 2.0);

    mouseX = x;
    mouseY = y;
  }
  if (mouseLeftDown)
  {
    cameraX += (mouseX - x) / 1000;
    cameraY += (mouseY - y) / 1000;
    mouseX = x;
    mouseY = y;
  }
  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 's':
    cameraZ -= 0.2f; // 摄像机向前移动
    break;
  case 'w':
    cameraZ += 0.2f; // 摄像机向后移动
    break;
  case 'a':
    cameraX -= 0.2f; // 摄像机向左移动
    break;
  case 'd':
    cameraX += 0.2f; // 摄像机向右移动
    break;
  case '0':
    mode = 0;
    break;
  case '1':
    mode = 1;
    break;
  case '2':
    mode = 2;
    break;
  case '3':
    mode = 3;
    break;
  case '4':
    mode = 4;
    break;
  }

  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("3D Cube");
  loadData();
  loadTexture("whu.png");

  // 注册回调函数
  glutDisplayFunc(renderScene);
  glutMouseFunc(mouseButton);
  glutMotionFunc(mouseMoveEvent);
  glutKeyboardFunc(keyboard);
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_FRONT);

  // 设置窗口背景颜色为白色
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glutMainLoop();

  return 0;
}