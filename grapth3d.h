#ifndef PAINTGRAPTH3D_H
#define PAINTGRAPTH3D_H
#include <grapth2d.h>
#include <QtOpenGL>
#include <QFile>
typedef std::pair<int,int> vec2i;
class Text
{
    QFont* font;
public:
    Text();
    void initfont(const QFont & f); // set up a font
    void print(QOpenGLShaderProgram& shader, QOpenGLFunctions*f ,int buffer,const QString&text);
    void getBuffers(const QString&text, std::vector<QOpenGLBuffer>&vec,QColor color = Qt::black,QVector3D location = QVector3D(0, 0, 0),
                    float mult = 0.1f);
};
struct Point
{
    QVector3D pos;
    QVector3D color;
    Point():pos(), color(){}
    Point(const QVector3D&p, const QVector3D&c);
};
typedef std::list<std::vector<GLuint>> mylist;

class PaintGrapth3d;
class Function3d: public Function2d
{
public:
    //QOpenGLBuffer*vertexes;
    std::vector<Point> vertexes;
    mylist along,against;
    int count;
    Function3d(QColor c, PaintGrapth3d*);
    Function3d():Function2d(),
       count(0){}//for manual
    ~Function3d();
    void clear();
};
class PaintGrapth3d: public QOpenGLWidget, public ABC_Grapth, protected QOpenGLFunctions
{
    struct struct_grid
    {
        void init();
        QOpenGLBuffer*along;//for x and y
        QOpenGLBuffer*against;// for x and y
        int count;
    };
    struct struct_axis
    {
        std::vector<QOpenGLBuffer>labels[3];
        void init();
        QOpenGLBuffer*OxOyOz;
        struct_axis():OxOyOz(nullptr){}
        ~struct_axis(){delete OxOyOz; }
    };
    void setGrid();
    void setAxis();
    void drawGrid(int buff, int color);
    struct_grid grid;
    struct_axis axis;
    QOpenGLShaderProgram*sh_program;
    QMatrix4x4*projection;
    QMatrix4x4*view;
    float angleX, angleY, radius;
    QPoint lastPos;
    void draw(std::vector<Function3d*>&funcs, int buff, int color);
    int countStepsX, countStepsY;
    float centerZ;
    float red, green, blue;
public:
    PaintGrapth3d();
    void setCountSteps(int cx, int cy=-1);
    void setCenter(const QVector3D&vec);
    void setSizeXY(FlDb size);
    QVector3D getCenter()const{return QVector3D(centerX, centerY, centerZ); }
    float getCenterZ()const{return centerZ; }
    const std::pair<int,int>& getCountSteps(){return std::pair<int,int>(countStepsX, countStepsY); }
    void perform(std::vector<Function3d*>&funcs);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void setColorBackround(QColor);
};
#endif // PAINTGRAPTH3D_H
