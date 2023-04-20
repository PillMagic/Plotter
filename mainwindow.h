#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QColor>
#include <QPalette>
#include "ui_mainwindow.h"
#include "grapth3d.h"
class PrimeWindow;
class LineButt;
class Console;
class Manage3d;
/*Нормальное вращение, картинки на кнопках в LineButt, */
template<class F1, class F2>
F1*moving(F2*fs);

class ABS_Manage: public QObject
{
protected:
    PrimeWindow&wind;
    QVector<LineButt*> itemsArea;
    virtual void delFunc(int ind) = 0;
    void on_AddFCommon(const QString&s = "");
    virtual void setColorFunc(int ind, QColor c)=0;
public:
    ABS_Manage(PrimeWindow&wind);
    void deleteData();
    QVector<LineButt*>&getItems(){return itemsArea;}
    virtual void on_Update() = 0;
    virtual void on_AddF(const QString&s = "") = 0;
    virtual void perform() = 0;
    virtual void repaint() = 0;
    virtual void perfRep() = 0;//perform and repaint
    void delIArea(LineButt*lb);// it deletes Item in Area
    void setColorLine(LineButt*lb, QColor qc);
    void delIArea(int ind);
    virtual ~ABS_Manage(){}
    template<class M1, class M2>
    friend void fromM1toM2(M1*from, M2*to);
};
class LineButt: public QObject
{
    Q_OBJECT
    static ABS_Manage*commonManage;
public: 
    QLineEdit*edit;
    QPushButton*color;
    QPushButton*butt;
    LineButt(const char* buttonStr, QWidget*parent = 0);
    ~LineButt();
    friend ABS_Manage;
public slots:
    void deleteMe();
    void setColor();
};
class Manage2d: public ABS_Manage
{
    Q_OBJECT
    PaintGrapth2d*grapth2d;
protected:
    void delFunc(int ind)override;
    void setColorFunc(int ind, QColor c)override;
public:
    Manage2d(PrimeWindow&wind, PaintGrapth2d*);
    std::vector<Function2d*> functions;
    void perform()override;
    void repaint()override;
    void perfRep()override;
    ~Manage2d();
    template<class M1, class M2>
    friend void fromM1toM2(M1*from, M2*to);
    void on_Update()override;
    void on_AddF(const QString&s = "")override;
public slots:
    void XChanged(double);
    void YChanged(double);
    void AccurChanged(int);
    void SizeXYChanged(double n);
    void SliderChanged(int);
    void ModeChang(int);//mode of auto or normal
};
#define qtyStepsXD AccuracyD
class Manage3d: public ABS_Manage
{
    Q_OBJECT
    PaintGrapth3d*grapth3d;
    QLabel*ZL;
    std::vector<Function3d*> functions;
protected:
    void delFunc(int ind)override;
    void setColorFunc(int ind, QColor c)override;
public:
    QDoubleSpinBox* ZD;
    QSpinBox*qtyStepsYD;
    Manage3d(PrimeWindow&wind, PaintGrapth3d*);
    std::vector<Function3d*>&getFunctions();
    void perform()override;
    void repaint()override;
    void perfRep()override;
    void on_Update()override;
    void on_AddF(const QString&s = "")override;
    ~Manage3d();
    template<class M1, class M2>
    friend void fromM1toM2(M1*from, M2*to);
};

class PrimeWindow: public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
    Manage2d*w2d;
    Manage3d*w3d;
    PaintGrapth2d*grapth2d;
    PaintGrapth3d*grapth3d;
    ABS_Manage*  curManage;//active grapth at this time
    ABC_Grapth*  curGrapth;
    QPalette palet;
    QVBoxLayout* LayInFuncArea;
    Console*console;
    QAction*actionMode;
public:
    PrimeWindow(Console*console, PaintGrapth2d*gr2d,PaintGrapth3d*gr3d,QWidget*parent = 0);
    static PrimeWindow*mainW;
    PaintGrapth2d*getGrapth2d()const{return grapth2d;}
    PaintGrapth3d*getGrapth3d()const{return grapth3d;}
    ABC_Grapth*getGrapth()const{return curGrapth; }
    ABS_Manage*getManage(){return curManage; }
    Manage2d*getManage2d(){return w2d; }
    Manage3d*getManage3d(){return w3d; }
    bool isMode2d(){return actionMode->isChecked(); }
    void closeEvent(QCloseEvent *event)override;
    void perform();
    void updateGrapth();
    ~PrimeWindow();
    bool setColor(QColor, char type);
    QColor getColor()const{
        return palet.color(QPalette::Background);
    }
    QPalette getPalette()const {return palet; }
    friend ABS_Manage;
    friend Manage2d;
    friend Manage3d;
    friend Console;
    void setMode2(bool mode2d, bool d);
    void printLocation(const QString&txt);
public slots:
    void setMode(bool mode2d);
    void on_Update();
    void on_AddF();
};
#endif //MAINWINDOW_H
