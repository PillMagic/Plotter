#ifndef PAINTGRAPTH2D_H
#define PAINTGRAPTH2D_H
#include "Classes.h"
#include <QPainter>
#include <QWidget>

namespace CommonData {
struct grid
{
    uint gridProp;
    QColor colorGrid;
    std::vector<QLine>lines;
    grid(uint grPr = 9);
    void setPos(int w, int h);
    void resize(uint grPr);
};
extern QColor backround;
extern QColor labelsColor;
extern grid gr;
}
void setFunc(const char*, SolverEq*);
class Function2d
{
public:
    SolverEq* func;
    QColor  color;
    Function2d(QColor c):color(c){
        func = new SolverEq;
    }
    Function2d():func(nullptr){}//for manual
    virtual ~Function2d(){
        delete func;
    }
};
#define FlDb float
class ABC_Grapth
{
protected:
    FlDb centerX, centerY, sizeXY;
public:
    ABC_Grapth(FlDb size,FlDb centerX = 0, FlDb centerY = 0)
        :centerX(centerX), centerY(centerY),sizeXY(size){}
    void setSizeXY(FlDb size){sizeXY = size; }
    void setCenterX(FlDb center){centerX = center; }
    void setCenterY(FlDb center){centerY = center; }
    FlDb getCenterX()const{return centerX;}
    FlDb getCenterY()const{return centerY;}
    FlDb getSizeXY()const{return sizeXY; }
    void set(FlDb size, FlDb centerX, FlDb centerY){
        sizeXY = size;
        this->centerX = centerX;
        this->centerY = centerY;
    }
};
class QPaintEvent;
#define sizeFontTimes 10
class PaintGrapth2d: public ABC_Grapth,public QWidget
{
    int fontTimes[sizeFontTimes]{5,6,7,8,10,11,13,14,16,17};
    QPixmap* drawMe;
    void drawNums(QPainter*painter);
    double findStart(FlDb plus, FlDb x2, FlDb bound_y1, //searching start, when
                   FlDb bound_y2,SolverEq&sq,FlDb&x, mapCD&mp);//find it, return y
    uint widthLine;
    FlDb plus;
public: //data
    enum Mode{NORMAL, AUTO};
    int accur;
    bool mode;
public://funcs
    PaintGrapth2d(QWidget*p = 0,FlDb centX = 0, FlDb centY = 0, FlDb sizeXY = 4.0);
    void paintEvent(QPaintEvent *event) override;
    void settings(int Accuracy, bool mode = Mode::NORMAL);
    void perform(std::vector<Function2d*>&funcs);
    void setWidthLine(uint s);
    uint getWidthLine()const{return widthLine; }
    void resizeEvent(QResizeEvent*ev)override;
    void gridResize(uint qty);
    void mousePressEvent(QMouseEvent *event)override;
    void mouseMoveEvent(QMouseEvent *event) override;
    //void mouseMoveEvent(QMouseEvent *event)override;
    virtual ~PaintGrapth2d()override{}
    QSize sizeHint() const override
    {return QSize(200,200); }
    QSize minimumSizeHint() const override
    {return QSize(100,100); }
    virtual void repaint(){this->QWidget::repaint(); }
};
#endif // PAINTGRAPTH2D_H
