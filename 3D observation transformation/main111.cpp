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
GLfloat cameraZ = -2.0f; // 摄像机Z轴位置

bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float centerX = windowWidth / 2.0f;
float centerY = windowHeight / 2.0f;

float max = elevationData[0][0];

int mode = 1;

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

// 绘制三角形
void renderScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 颜色缓冲全部设置为黑色，深度缓冲全部设置为最大值
  glLoadIdentity();

  // 设置视口和投影矩阵
  glViewport(0, 0, windowWidth, windowHeight);
  glMatrixMode(GL_PROJECTION); // 投影坐标系，实现近大远小的效果
  glLoadIdentity();
  gluPerspective(45.0f, (float)windowWidth / (float)windowHeight, 0.0001f, 100.0f);
  glMatrixMode(GL_MODELVIEW);

  // 设置视角位置
  glTranslatef(0.0, 0.0, cameraZ);
  glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
  glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

  // setupLighting();
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnableClientState(GL_VERTEX_ARRAY);

  // printf("%f %f %f\n", cameraX, cameraY, cameraZ);

  gluLookAt(cameraX, cameraY, cameraZ, cameraX, cameraY, 0.0, 0.0, 1.0, 0.0);

  glBegin(GL_TRIANGLES);
  glColor3f(0.5, 0.5, 0.5);
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

  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();
  glutSwapBuffers();
}

// // 鼠标按键回调函数
// void mouseButton(int button, int state, int x, int y)
// {
//   mouseX = x;
//   mouseY = y;
//   if (button == GLUT_LEFT_BUTTON)
//   {
//     if (state == GLUT_DOWN)
//     {
//       mouseLeftDown = true;
//     }
//     else if (state == GLUT_UP)
//     {
//       mouseLeftDown = false;
//     }
//   }
//   if (button == GLUT_RIGHT_BUTTON)
//   {
//     if (state == GLUT_DOWN)
//     {
//       mouseRightDown = true;
//     }
//     else if (state == GLUT_UP)
//     {
//       mouseRightDown = false;
//     }
//   }
// }

// void mouseMoveEvent(int x, int y)
// {
//   if (mouseRightDown)
//   {
//     rotateY += ((x - mouseX) / 10.0);
//     rotateX += ((y - mouseY) / 10.0);

//     mouseX = x;
//     mouseY = y;
//   }
//   if (mouseLeftDown)
//   {
//     cameraX += (mouseX - x) / 1000;
//     cameraY += (mouseY - y) / 1000;
//     mouseX = x;
//     mouseY = y;
//   }
//   glutPostRedisplay();
// }

// void keyboard(unsigned char key, int x, int y)
// {
//   switch (key)
//   {
//   case 'a':
//     cameraX += 0.01f;
//     break;
//   case 'd':
//     cameraX -= 0.01f;
//     break;
//   case 's':
//     cameraZ -= 0.01f; // 摄像机向前移动
//     break;
//   case 'w':
//     cameraZ += 0.01f; // 摄像机向后移动
//     break;
//   }

//   glutPostRedisplay();
// }

int main(int argc, char **argv)
{

  glutInit(&argc, argv);
  // glShadeModel(GL_SMOOTH);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("3D MODELING");
  loadData();

  // 注册回调函数
  glutDisplayFunc(renderScene);
  // glutMouseFunc(mouseButton);
  // glutMotionFunc(mouseMoveEvent);
  // glutKeyboardFunc(keyboard);

  glEnable(GL_DEPTH_TEST);

  // 设置窗口背景颜色为白色
  // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glutMainLoop();

  return 0;
}