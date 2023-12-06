#include <GL/freeglut.h>

// 三角形在三维坐标系的坐标
GLfloat triangleVertices[] = {
    15.0f, 20.0f, 0.0f,
    30.0f, 20.0f, 0.0f,
    25.0f, 50.0f, 0.0f};

// 立方体索引
GLubyte triangleIndices[] = {
    0, 1, 2};

// 窗口大小
int windowWidth = 800;
int windowHeight = 600;

// 旋转角度
GLfloat rotateX = 0.0f;
GLfloat rotateY = 0.0f;

GLfloat cameraX = -10.0f;  // 摄像机X轴位置
GLfloat cameraY = -10.0f;  // 摄像机Y轴位置
GLfloat cameraZ = -500.0f; // 摄像机Z轴位置

// 绘制三角形
void renderScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 颜色缓冲全部设置为黑色，深度缓冲全部设置为最大值
  glLoadIdentity();

  // 设置视口和投影矩阵
  glViewport(0, 0, windowWidth, windowHeight);
  glMatrixMode(GL_PROJECTION); // 投影坐标系，实现近大远小的效果
  glLoadIdentity();
  gluPerspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f);
  glMatrixMode(GL_MODELVIEW);

  // 设置视角位置
  glTranslatef(cameraX, cameraY, cameraZ);
  glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
  glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

  // gluLookAt(cameraX, cameraY, cameraZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

  // 绘制三角形
  glColor3f(1.0f, 0.5f, 0.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, triangleVertices);
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, triangleIndices);
  glDisableClientState(GL_VERTEX_ARRAY);

  // 绘制坐标轴
  glBegin(GL_LINES);
  // x轴
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(-300.0f, 0.0f, 0.0f);
  glVertex3f(300.0f, 0.0f, 0.0f);
  // y轴
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, -300.0f, 0.0f);
  glVertex3f(0.0f, 300.0f, 0.0f);
  // z轴
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, -300.0f);
  glVertex3f(0.0f, 0.0f, 300.0f);
  glEnd();

  glutSwapBuffers();
}

// // 鼠标左键点击，回到初始视角
// void mouseEvent(int button, int state, int x, int y)
// {
//   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
//   {
//     rotateX = 0.0f;
//     rotateY = 0.0f;
//   }
// }

// 鼠标移动，调整视口角度
void mouseMoveEvent(int x, int y)
{
  static int lastX = -1;
  static int lastY = -1;

  if (lastX == -1)
  {
    lastX = x;
    lastY = y;
    return;
  }

  rotateX += y - lastY;
  rotateY += x - lastX;

  lastX = x;
  lastY = y;

  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 's':
    cameraZ -= 1.0f; // 摄像机向前移动
    break;
  case 'w':
    cameraZ += 1.0f; // 摄像机向后移动
    break;
  case 'a':
    cameraX -= 1.0f; // 摄像机向左移动
    break;
  case 'd':
    cameraX += 1.0f; // 摄像机向右移动
    break;
  }

  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("3D Triangle");

  // 注册回调函数
  glutDisplayFunc(renderScene);
  // glutMouseFunc(mouseEvent);
  glutMotionFunc(mouseMoveEvent);
  glutKeyboardFunc(keyboard);

  glEnable(GL_DEPTH_TEST);

  // 设置窗口背景颜色为白色
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glutMainLoop();

  return 0;
}