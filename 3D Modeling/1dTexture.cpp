#include <GL/freeglut.h>
#include <stdio.h>
#include <cmath>

float elevationData[206][143];
static GLfloat triangleVertices[206][143][12] = {};
int numCols;
int numRows;
float cellSize;
float noDataValue;

// 立方体索引
GLubyte triangleIndices[] = {
    0, 1, 3,
    0, 2, 3,
    0, 1, 2,
    1, 2, 3};

// 窗口大小
int windowWidth = 800;
int windowHeight = 600;

// 旋转角度
GLfloat rotateX = 0.0f;
GLfloat rotateY = 0.0f;

GLfloat cameraX = 0.5f;  // 摄像机X轴位置
GLfloat cameraY = 0.5f;  // 摄像机Y轴位置
GLfloat cameraZ = -0.6f; // 摄像机Z轴位置

bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float centerX = windowWidth / 2.0f;
float centerY = windowHeight / 2.0f;

GLfloat Colors[20][3] = {
    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.25f, 0.85f},
    {0.0f, 0.5f, 0.7f},
    {0.0f, 0.55f, 0.55f},
    {0.0f, 0.6f, 0.4f},
    {0.0f, 0.75f, 0.25f},
    {0.0f, 0.9f, 0.1f},
    {0.2f, 0.75f, 0.15f},
    {0.4f, 0.6f, 0.2f},
    {0.5f, 0.5f, 0.3f},
    {0.6f, 0.4f, 0.4f},
    {0.7f, 0.4f, 0.4f},
    {0.8f, 0.4f, 0.4f},
    {0.9f, 0.55f, 0.55f},
    {0.95f, 0.7f, 0.7f},
    {1.0f, 0.85f, 0.85f},
    {1.0f, 1.0f, 1.0f}};

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
GLuint textureID;
int textureWidth = 16;             // 纹理宽度
unsigned char textureData[16 * 3]; // 纹理数据

float max = elevationData[0][0];

int mode = 1;

void init()
{
  glShadeModel(GL_FLAT);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  for (int i = 0; i < textureWidth; i++)
  {
    textureData[i * 3] = (unsigned char)(_tex_1d[i] & 0xFF);
    textureData[i * 3 + 1] = (unsigned char)(_tex_1d[i] >> 8 & 0xFF);
    textureData[i * 3 + 2] = (unsigned char)(_tex_1d[i] >> 16 & 0xFF);
  }
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_1D, textureID);
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

      triangleVertices[i][j][0] = (float)i / (float)maxRow;
      triangleVertices[i][j][1] = (float)j / (float)maxCol;
      triangleVertices[i][j][2] = -elevationData[i][j] / max / 10;

      triangleVertices[i][j][3] = (float)(i + 1) / (float)maxRow;
      triangleVertices[i][j][4] = (float)j / (float)maxCol;
      triangleVertices[i][j][5] = -elevationData[i + 1][j] / max / 10;

      triangleVertices[i][j][6] = (float)i / (float)maxRow;
      triangleVertices[i][j][7] = (float)(j + 1) / (float)maxCol;
      triangleVertices[i][j][8] = -elevationData[i][j + 1] / max / 10;

      triangleVertices[i][j][9] = (float)(i + 1) / (float)maxRow;
      triangleVertices[i][j][10] = (float)(j + 1) / (float)maxCol;
      triangleVertices[i][j][11] = -elevationData[i + 1][j + 1] / max / 10;
    }
  }

  // 关闭文件
  fclose(demFile);
}

void setupLighting()
{
  glEnable(GL_DEPTH_TEST);
  // 设置点光源的参数
  GLfloat lightColor[] = {0.2f, 0.8f, 0.0f, 1.0f};          // 光源颜色
  GLfloat lightPosition[] = {0.0f, 1000.0f, 2000.0f, 1.0f}; // 光源位置

  // 设置材质属性
  GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};  // 环境反射光颜色
  GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  // 漫反射光颜色
  GLfloat materialSpecular[] = {0.0f, 0.5f, 0.0f, 1.0f}; // 镜面反射光颜色
  GLfloat shininess = 50.0f;                             // 镜面反射光的高光指数

  // 设置点光源属性
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  // 设置材质属性
  // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
  // glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  // glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  // 启用光照
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

// 绘制三角形
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

  // 设置视角位置
  glTranslatef(0.0, 0.0, cameraZ);
  glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
  glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

  // setupLighting();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);

  // printf("%f %f %f\n", cameraX, cameraY, cameraZ);
  gluLookAt(cameraX, cameraY, cameraZ, cameraX, cameraY, 0.0, 0.0, 1.0, 0.0);

  // 设置纹理参数
  if (mode == 1)
  {
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  else if (mode == 2)
  {
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  // 为纹理对象提供数据
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, textureWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, textureID);
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < numRows - 1; ++i)
  {
    for (int j = 0; j < numCols - 1; ++j)
    {

      // 0
      glTexCoord1d(triangleVertices[i][j][2] * (-10));
      glVertex3f(triangleVertices[i][j][0], triangleVertices[i][j][1], triangleVertices[i][j][2]);
      // 1
      glTexCoord1d(triangleVertices[i][j][5] * (-10));
      glVertex3f(triangleVertices[i][j][3], triangleVertices[i][j][4], triangleVertices[i][j][5]);
      // 2
      glTexCoord1d(triangleVertices[i][j][8] * (-10));
      glVertex3f(triangleVertices[i][j][6], triangleVertices[i][j][7], triangleVertices[i][j][8]);

      // 1
      glTexCoord1d(triangleVertices[i][j][5] * (-10));
      glVertex3f(triangleVertices[i][j][3], triangleVertices[i][j][4], triangleVertices[i][j][5]);
      // 2
      glTexCoord1d(triangleVertices[i][j][8] * (-10));
      glVertex3f(triangleVertices[i][j][6], triangleVertices[i][j][7], triangleVertices[i][j][8]);
      // 3
      glTexCoord1d(triangleVertices[i][j][11] * (-10));
      glVertex3f(triangleVertices[i][j][9], triangleVertices[i][j][10], triangleVertices[i][j][11]);
      // glVertexPointer(3, GL_FLOAT, 0, triangleVertices[i][j]);
      // glDrawElements(GL_TRIANGLES, 4 * 3, GL_UNSIGNED_BYTE, triangleIndices);
    }
  }
  glEnd();
  glDisable(GL_TEXTURE_1D);
  glDisableClientState(GL_VERTEX_ARRAY);
  glutSwapBuffers();
}

// 鼠标按键回调函数
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
    rotateY += ((x - mouseX) / 10.0);
    rotateX += ((y - mouseY) / 10.0);

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
  case 'a':
    cameraX += 0.01f;
    break;
  case 'd':
    cameraX -= 0.01f;
    break;
  case 's':
    cameraZ -= 0.01f; // 摄像机向前移动
    break;
  case 'w':
    cameraZ += 0.01f; // 摄像机向后移动
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
  }

  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  loadData();
  glutInit(&argc, argv);
  // glShadeModel(GL_SMOOTH);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("3D MODELING");
  init();

  // 注册回调函数
  glutDisplayFunc(renderScene);
  glutMouseFunc(mouseButton);
  glutMotionFunc(mouseMoveEvent);
  glutKeyboardFunc(keyboard);

  glEnable(GL_DEPTH_TEST);

  // 设置窗口背景颜色为白色
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glutMainLoop();

  return 0;
}