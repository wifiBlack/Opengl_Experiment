#include <stdio.h>
#include <GL/freeglut.h>
#include <limits>

int numRows, numCols;
float cellSize, noDataValue;
const float xllcorner = 17.0;
const float yllcorner = 85.5;

// 创建一个数组来存储高程数据
float elevationData[206][143];
// 定义窗口的大小
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// 定义平移和旋转参数
float translationX = -100.0f;
float translationY = -100.0f;
float rotationX = 0.0f;
float rotationY = 0.0f;

// GLfloat cameraX = -10.0f;  // 摄像机X轴位置
// GLfloat cameraY = -10.0f;  // 摄像机Y轴位置
// GLfloat cameraZ = -200.0f; // 摄像机Z轴位置
// 初始化OpenGL环境
void initGL()
{
  // glClearColor(255.0f, 255.0f, 255.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
}

// 渲染函数

// 渲染函数
void render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // 设置视口和投影矩阵
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glMatrixMode(GL_PROJECTION); // 投影坐标系，实现近大远小的效果
  glLoadIdentity();
  gluPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);

  // // 设置视角
  // glTranslatef(cameraX, cameraY, cameraZ);
  // glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
  // glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
  gluLookAt(0.0, 0.0, -100.0, 0.0, 0.0, 0.0, 0.0, 0, 1.0);

  cellSize = cellSize / 100;
  // 绘制DEM数据
  for (int i = 0; i < numRows - 1; i++)
  {
    for (int j = 0; j < numCols - 1; j++)
    {
      glColor3f(1.0f, 0.5f, 0.0f);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(xllcorner + i * cellSize, yllcorner + j * cellSize, elevationData[i][j]);
      glVertex3f(xllcorner + (i + 1) * cellSize, yllcorner + j * cellSize, elevationData[i + 1][j]);
      glVertex3f(xllcorner + i * cellSize, yllcorner + (j + 1) * cellSize, elevationData[i][j + 1]);
      glVertex3f(xllcorner + (i + 1) * cellSize, yllcorner + (j + 1) * cellSize, elevationData[i + 1][j + 1]);
      glEnd();
    }
  }

  glutSwapBuffers();
}

// 处理鼠标事件
void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    // 记录左键按下时的鼠标位置
    translationX = (float)x / WINDOW_WIDTH;
    translationY = (float)(WINDOW_HEIGHT - y) / WINDOW_HEIGHT;
  }
  else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
  {
    // 记录右键按下时的鼠标位置
    rotationX = (float)x / WINDOW_WIDTH * 360.0f;
    rotationY = (float)(WINDOW_HEIGHT - y) / WINDOW_HEIGHT * 360.0f;
  }
}

// 处理鼠标移动事件
void motion(int x, int y)
{
  // 计算鼠标移动的距离，进行平移和旋转操作
  float deltaX = (float)x / WINDOW_WIDTH - translationX;
  float deltaY = (float)(WINDOW_HEIGHT - y) / WINDOW_HEIGHT - translationY;

  translationX += deltaX;
  translationY += deltaY;

  rotationX += deltaX * 360.0f;
  rotationY += deltaY * 360.0f;

  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  // 打开DEM.asc文件
  FILE *demFile = fopen("dem.asc", "r");
  if (demFile == NULL)
  {
    printf("无法打开DEM.asc文件\n");
    return 1;
  }

  // 读取DEM.asc文件的头部信息

  fscanf(demFile, "ncols %d\n", &numCols);
  fscanf(demFile, "nrows %d\n", &numRows);
  fscanf(demFile, "xllcorner %*f\n"); // 忽略不需要的字段
  fscanf(demFile, "yllcorner %*f\n"); // 忽略不需要的字段
  fscanf(demFile, "cellsize %f\n", &cellSize);
  fscanf(demFile, "NODATA_value %f\n", &noDataValue);

  // 读取DEM.asc文件中的高程数据
  for (int i = 0; i < numRows; i++)
  {
    for (int j = 0; j < numCols; j++)
    {
      fscanf(demFile, "%f", &elevationData[i][j]);
      if (elevationData[i][j] == noDataValue)
      {
        elevationData[i][j] = std::numeric_limits<float>::min();
      }
      else
      {
        elevationData[i][j] = elevationData[i][j] / 1000;
      }

      // float ele = elevationData[i][j];
    }
  }

  // 关闭文件
  fclose(demFile);

  // 初始化GLUT库
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("DEM Viewer");

  // 注册回调函数
  glutDisplayFunc(render);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  // 初始化OpenGL环境
  initGL();

  // 设置窗口背景颜色为白色
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  // 进入主循环
  glutMainLoop();

  return 0;
}