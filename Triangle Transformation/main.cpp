#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h>
/*  初始化显示窗口大小 */
GLfloat winWidth = 800.0, winHeight = 800.0;
/* 设置世界坐标系的显示范围 */
GLfloat xwcMin = 0.0, xwcMax = 225.0;
GLfloat ywcMin = 0.0, ywcMax = 225.0;
/* 定义二维点数据结构 */
class wcPt2D
{
public:
    GLfloat x, y;
};
typedef GLfloat Matrix3x3[3][3];
Matrix3x3 matComposite; // 定义复合矩阵
const GLdouble pi = 3.14159;

void init(void)
{
    /* 设置显示窗口的背景颜色为白色 */
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

/*  将三角形顶点坐标初始化  */
void initTriangle(wcPt2D *verts)
{
    verts[0] = {15.0f, 20.0f};
    verts[1] = {30.0f, 20.0f};
    verts[2] = {25.0f, 50.0f};
}

/* 构建3*3的单位矩阵 */
void matrix3x3SetIdentity(Matrix3x3 matIdent3x3)
{
    GLint row, col;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            matIdent3x3[row][col] = (row == col);
}

/* 变换矩阵m1前将m1与矩阵m2相乘,并将结果存放到m2中 */
void matrix3x3PreMultiply(Matrix3x3 m1, Matrix3x3 m2)
{
    GLint row, col;
    Matrix3x3 matTemp;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            matTemp[row][col] = m1[row][0] * m2[0][col] + m1[row][1] * m2[1][col] + m1[row][2] * m2[2][col];
    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            m2[row][col] = matTemp[row][col];
}

/* 平移变换函数，参数为x轴平移量tx，y轴平移量ty */
void translate2D(GLfloat tx, GLfloat ty)
{
    Matrix3x3 matTransl;
    /* 初始化平移矩阵为单位矩阵 */
    matrix3x3SetIdentity(matTransl);

    matTransl[0][2] = tx;
    matTransl[1][2] = ty;
    /*  将平移矩阵前乘到复合矩阵matComposite中 */
    matrix3x3PreMultiply(matTransl, matComposite);
}

/* 旋转变换函数，参数为中心点pivotPt和旋转角度theta */
void rotate2D(wcPt2D pivotPt, GLfloat theta)
{
    Matrix3x3 matRot;

    /*  初始化旋转矩阵为单位矩阵 */
    matrix3x3SetIdentity(matRot);
    matRot[0][0] = cos(theta);
    matRot[0][1] = -sin(theta);
    matRot[0][2] = pivotPt.x * (1 - cos(theta)) + pivotPt.y * sin(theta);
    matRot[1][0] = sin(theta);
    matRot[1][1] = cos(theta);
    matRot[1][2] = pivotPt.y * (1 - cos(theta)) - pivotPt.x * sin(theta);

    /*  将旋转矩阵前乘到复合矩阵matComposite中 */
    matrix3x3PreMultiply(matRot, matComposite);
}

/* 比例变换函数，参数为基准点fixedPt和缩放比例sx、sy */
void scale2D(GLfloat sx, GLfloat sy, wcPt2D fixedPt)
{
    Matrix3x3 matScale;

    /* 初始化缩放矩阵为单位矩阵  */
    matrix3x3SetIdentity(matScale);

    matScale[0][0] = sx;
    matScale[0][2] = (1 - sx) * fixedPt.x;
    matScale[1][1] = sy;
    matScale[1][2] = (1 - sy) * fixedPt.y;

    /*  将缩放矩阵前乘到复合矩阵matComposite中 */
    matrix3x3PreMultiply(matScale, matComposite);
}

/*中心对称函数，参数为中心点centerPt*/
void centrosymmetry2D(wcPt2D centerPt)
{
    Matrix3x3 matCentrosymmetry;
    matrix3x3SetIdentity(matCentrosymmetry);
    /*将中心点移动回原点的矩阵*/
    translate2D(-centerPt.x, -centerPt.y);
    matCentrosymmetry[0][0] = -1.0;
    matCentrosymmetry[1][1] = -1.0;
    /*  将中心对成矩阵前乘到复合矩阵matComposite中 */
    matrix3x3PreMultiply(matCentrosymmetry, matComposite);
    /* 将中心重新移动回去 */
    translate2D(centerPt.x, centerPt.y);
}

/*错切函数，参数为沿着x轴的错切量xShear，沿着y轴的错切量yShear，错切变换的参考点fixedPt*/
void shearcut2D(GLfloat xShear, GLfloat yShear, wcPt2D fixedPt)
{
    Matrix3x3 matShearcut;
    matrix3x3SetIdentity(matShearcut);
    /*将中心点移动回原点的矩阵*/
    translate2D(-fixedPt.x, -fixedPt.y);
    matShearcut[0][1] = xShear;
    matShearcut[1][0] = yShear;
    matrix3x3PreMultiply(matShearcut, matComposite);
    /* 将中心重新移动回去 */
    translate2D(fixedPt.x, fixedPt.y);
}

/*  利用复合矩阵计算变换后坐标 */
void transformVerts2D(GLint nVerts, wcPt2D *verts)
{
    GLint k;
    GLfloat temp;
    for (k = 0; k < nVerts; k++)
    {
        temp = matComposite[0][0] * verts[k].x + matComposite[0][1] * verts[k].y + matComposite[0][2];
        verts[k].y = matComposite[1][0] * verts[k].x + matComposite[1][1] * verts[k].y + matComposite[1][2];
        verts[k].x = temp;
    }
}

/*  三角形绘制函数 */
void triangle(wcPt2D *verts)
{
    GLint k;

    glBegin(GL_TRIANGLES);
    for (k = 0; k < 3; k++)
        glVertex2f(verts[k].x, verts[k].y);
    glEnd();
}

/* 坐标轴绘制函数 */
void Axis()
{
    // 绘制坐标轴
    glColor3f(0.0, 0.0, 0.0); // 设置前景色为黑色
    glBegin(GL_LINES);
    // x轴
    glVertex2f(-winWidth, 0.0);
    glVertex2f(winWidth, 0.0);
    // y轴
    glVertex2f(0.0, -winHeight);
    glVertex2f(0.0, winHeight);
    glEnd();
}

/* 显示初始三角形函数 */
void displayOrigin(wcPt2D *verts)
{
    glColor3f(0.0, 0.0, 1.0); // 设置前景色为蓝色
    triangle(verts);          // 显示蓝色三角形（初始）
}

void myDisplay()
{
    /* 定义三角形的初始位置 */
    GLint nVerts = 3;
    wcPt2D verts[3] = {{15.0f, 20.0f}, {30.0f, 20.0f}, {25.0f, 50.0f}};

    /*  计算三角形中心位置 */
    wcPt2D centroidPt;

    GLint k, xSum = 0, ySum = 0;
    for (k = 0; k < nVerts; k++)
    {
        xSum += verts[k].x;
        ySum += verts[k].y;
    }
    centroidPt.x = GLfloat(xSum) / GLfloat(nVerts);
    centroidPt.y = GLfloat(ySum) / GLfloat(nVerts);

    /*  设置几何变换参数*/
    wcPt2D pivPt, fixedPt;
    pivPt = centroidPt;
    fixedPt = centroidPt;

    GLfloat tx = 0.0, ty = 100.0;
    GLfloat sx = 0.5, sy = 0.5;
    GLdouble theta = pi / 2.0;
    wcPt2D center = {0.0f, 0.0f};
    GLfloat xShear = 1.0, yShear = 0.0;

    wcPt2D referencePt = {22.0f, 32.0f};

    wcPt2D relativelyCenterPt = {2.0f, 3.0f};

    GLdouble theta_2 = pi / 6.0;

    glClear(GL_COLOR_BUFFER_BIT); // 清空显示窗口

    // 绘制坐标轴

    Axis();

    // 绘制初始三角形

    displayOrigin(verts);

    // // 1.平移

    // matrix3x3SetIdentity(matComposite);
    // translate2D(tx, ty); // 向上平移100
    // transformVerts2D(nVerts, verts);
    // glColor3f(1.0, 0.0, 0.0); // 设置前景色为红色
    // triangle(verts);          // 显示红色三角形（平移后）

    // // 2.缩放

    // matrix3x3SetIdentity(matComposite);
    // initTriangle(verts);      // 恢复最开始的三角形
    // scale2D(sx, sy, fixedPt); // 缩小1倍
    // transformVerts2D(nVerts, verts);
    // glColor3f(0.0, 1.0, 0.0); // 设置前景色为绿色
    // triangle(verts);          // 显示绿色三角形（缩放后）

    // // 3.旋转

    // matrix3x3SetIdentity(matComposite);
    // initTriangle(verts);         // 恢复最开始的三角形
    // rotate2D(centroidPt, theta); // 旋转90
    // transformVerts2D(nVerts, verts);
    // glColor3f(1.0, 1.0, 0.0); // 设置前景色为黄色
    // triangle(verts);          // 显示黄色三角形（旋转后）

    // // 4.中心对称

    // matrix3x3SetIdentity(matComposite);
    // initTriangle(verts);      // 恢复最开始的三角形
    // centrosymmetry2D(center); // 以坐标原点为中心点，中心对称
    // transformVerts2D(nVerts, verts);
    // glColor3f(0.0, 1.0, 1.0); // 设置前景色为青色
    // triangle(verts);          // 显示青色三角形（中心对称后）

    // // 5.错切

    // matrix3x3SetIdentity(matComposite);
    // initTriangle(verts);                  // 恢复最开始的三角形
    // shearcut2D(xShear, yShear, verts[0]); // 沿着x轴错切
    // transformVerts2D(nVerts, verts);
    // glColor3f(1.0, 0.0, 1.0); // 设置前景色为粉色
    // triangle(verts);          // 显示粉色三角形（错切后）

    // 6.对原始的三角形，相对（22，32）放大1.5

    matrix3x3SetIdentity(matComposite);
    initTriangle(verts);              // 恢复最开始的三角形
    scale2D(1.5f, 1.5f, referencePt); // 相对参考点放大1.5倍
    transformVerts2D(nVerts, verts);
    glColor3f(0.0f, 0.5f, 0.5f); // 设置前景色为苍绿色
    triangle(verts);             // 显示苍绿色三角形，相对（22，32）放大后

    // 7.对6中的三角形，相对（2，3）中心对称

    matrix3x3SetIdentity(matComposite);
    centrosymmetry2D(relativelyCenterPt);
    transformVerts2D(nVerts, verts);
    glColor3f(0.0f, 0.0f, 0.5f); // 设置前景色为深蓝色
    triangle(verts);             // 显示深蓝色三角形，相对（2，3）中心对称后

    // 8.对7中三角形，以坐标原点为中心逆时针旋转30度

    matrix3x3SetIdentity(matComposite);
    rotate2D(center, theta_2);
    transformVerts2D(nVerts, verts);
    glColor3f(0.5f, 0.0f, 0.0f); // 设置前景色为深红色
    triangle(verts);             // 显示深红色三角形，相对（2，3）中心对称后

    // 初始三角形被后续放大的三角形所覆盖，因此重新绘制一遍
    initTriangle(verts);
    displayOrigin(verts);

    glFlush();
}

void reshape(GLsizei newWidth, GLsizei newHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-winWidth / 5.0, winWidth / 5.0, -winHeight / 5.0, winHeight / 5.0);

    glViewport(0, 0, newWidth, newHeight);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("TriangleTransformation");

    init();
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}