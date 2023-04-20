#include "grapth2d.h"
#include "mainwindow.h"
#include <QResizeEvent>
#define diffFont 6
#define space 5
#define COLUMN_SIZE 20
namespace CommonData {
grid gr(19);
grid::grid(uint grPr) :colorGrid(100,50,100),
    gridProp(grPr),lines(gridProp*2){}
void grid::setPos(int w, int h)
{
    double plus =  w / static_cast<double>(gr.gridProp + 1),
            sum = plus;
    uint i = 0;
    for(;i < gridProp; i++, sum += plus)
        lines[i].setLine(sum, 0, sum, h);
    plus = h /static_cast<double>(gr.gridProp + 1);
    sum = plus;
    for(;i < gridProp * 2; i++, sum+=plus)
        lines[i].setLine(0, sum, w, sum);
}
void grid::resize(uint grPr){
    gridProp = grPr;
    lines.resize(grPr*2);
}
QColor backround = Qt::white;
QColor labelsColor = Qt::black;
}
void setFunc(const char*t, SolverEq*sq)
{
    if(strcmp(t, "") == 0)return;
    char*eq = new char[strlen(t)+ 1];
    fast_ptr<char> ifExcept(eq);//it will delete eq
    strcpy(eq,t);
    sq->clear();
    sq->read(eq);//it can throw exception
}

//PaintGrapth2d
PaintGrapth2d::PaintGrapth2d(QWidget*p,FlDb centX, FlDb centY, FlDb sizeXY)
    :ABC_Grapth(sizeXY,centX,centY),accur(1),mode(0),widthLine(2)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    drawMe = new QPixmap;
    drawMe->fill(CommonData::backround);
}
void PaintGrapth2d::resizeEvent(QResizeEvent*ev)
{
    int w = width(),h = size().height();
    QPixmap*nw = new QPixmap(w, h);
    delete drawMe;
    drawMe = nw;
    CommonData::gr.setPos(w - COLUMN_SIZE,h- COLUMN_SIZE);
    drawMe->fill(CommonData::backround);
    PrimeWindow::mainW->perform();
    forSolverEq::exactSqrt = sizeXY / height();
}
void PaintGrapth2d::gridResize(uint qty)
{
    CommonData::gr.resize(qty);
    CommonData::gr.setPos(width() - COLUMN_SIZE,height()- COLUMN_SIZE);
    PrimeWindow::mainW->perform();
}
void PaintGrapth2d::perform(std::vector<Function2d*>&funcs)
{
    //if there is no function to perform
    if(funcs.empty())return;
    QPainter painter(drawMe);
    QPainterPath path;
    QPen pen;
    //size, when is drawing grapth without coordinats(nums)
    FlDb realW = width()- COLUMN_SIZE,
            realH = height()-COLUMN_SIZE,x1,x2,x,
            y,scalex,scaley,
            halfW = realW / 2, halfH = realH / 2,
            bound_y1,bound_y2,t;
    {
        if(mode == Mode::AUTO)
            plus = 0.01 / accur;
        else
            plus = sizeXY / (realW*accur);
        FlDb halfSizeXY = sizeXY / 2;
        //left bound of grapth and right bound of grapth
        x1 = -halfSizeXY +centerX;
        x2 = halfSizeXY + centerX;
        //(x*scalex; y*scaley) is point on window
        scalex = realW / sizeXY;
        scaley = realH / sizeXY;
        bound_y1=centerY-halfSizeXY;
        bound_y2=centerY+halfSizeXY;
        //fill window
        painter.fillRect(QRect(0,0,realW, realH), CommonData::backround);
        QColor colorl = PrimeWindow::mainW->getColor();
        painter.fillRect(QRect(realW,0,width(), height()),
                         colorl);
        painter.fillRect(QRect(0,realH,realW, height()),
                         colorl);
        pen.setColor(CommonData::gr.colorGrid);
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        //draw grid
        painter.drawLines
                (CommonData::gr.lines.data(), CommonData::gr.lines.size());
        //draw Ox
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        FlDb s = halfSizeXY - centerX;
        if(!(s < 0 || s > sizeXY))
        {
            s = s / sizeXY * realW;
            painter.drawLine(s, 0, s, realH);
        }
        //draw Oy
        s = halfSizeXY + centerY;
        if(!(s < 0 || s > sizeXY))
        {
            s = s / sizeXY * realH;
            painter.drawLine(0, s, realW, s);
        }
    }
    //draw coordinates
    pen.setWidth(1);
    pen.setColor(CommonData::labelsColor);
    painter.setPen(pen);
    drawNums(&painter);
    pen.setWidth(widthLine);
    mapCD mp({ pairCD('x',x1) });
    for(uint i = 0; i < funcs.size(); i++){
        Function2d&f = *(funcs[i]);
        SolverEq& sq = *(f.func);
        if(sq.empty())
            continue;
        FlDb x = x1 - plus;
        y = findStart(plus,x2,bound_y1,bound_y2,sq,x,mp);
        path.moveTo((x-centerX)*scalex + halfW, halfH - (y-centerY)*scaley);
        x+=plus;
        for(;x < x2; x+=plus)
        {
            mp['x'] = x;
            try {
                y = sq.calcul(mp);
                if(std::isnan(y) || std::isinf(y))
                    goto rasriv;
            }
            catch (...) { goto rasriv; }
            path.lineTo((x-centerX)*scalex + halfW, halfH - (y-centerY)*scaley);
            if(y< bound_y1 || y> bound_y2)
            {
rasriv:
                y = findStart(plus,x2,bound_y1,bound_y2,sq,x,mp);
                path.moveTo((x-centerX)*scalex + halfW, halfH - (y-centerY)*scaley);
            }
        }
        pen.setColor(f.color);
        painter.setPen(pen);
        painter.drawPath(path);
        path = QPainterPath();
    }
}
void PaintGrapth2d::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(QRect(0,0,drawMe->width(),drawMe->height()),*(drawMe));
}
#define diffFont 3
#define space 2
void PaintGrapth2d::drawNums(QPainter*painter)
{
    using std::max;
    using CommonData::gr;
    double plus =  sizeXY / static_cast<double>(gr.gridProp + 1),
            sum = -sizeXY / 2 + centerX;
    sum+=plus;
    int w = width(), h = height();
    int qtyFloat = 3.0 - log10(sizeXY) - 0.01;
    if(qtyFloat < 0)qtyFloat = 0;
    double maxN =max({fabs(-sizeXY + centerX),fabs(sizeXY + centerX),
                      fabs(-sizeXY + centerY),fabs(sizeXY + centerY)});
    int f = QString::number(-maxN, 'f', qtyFloat).size();
    f = (w - space * (gr.gridProp - 1)) /
            static_cast<double>(gr.gridProp * f) - diffFont;
    char modeNum = 'f';
    if(f >= sizeFontTimes)
        f = sizeFontTimes-1;
    else if(f < 0){
        f = 0;
        modeNum = 'e';
    }
    painter->setFont(QFont("Times", fontTimes[f]));
    f+=diffFont;
    QString n;
    uint i = 0;
    for(; i < gr.gridProp; i++, sum+=plus){
        n = QString::number(sum, modeNum, qtyFloat);
        painter->drawText(gr.lines[i].x2() - n.size() / 2 * f, h, n);
    }
    sum = sizeXY / 2 + centerY - plus;
    for(; i < gr.gridProp * 2; i++, sum-=plus)
    {
        n = QString::number(sum, modeNum, qtyFloat);
        painter->drawText(w - n.size()*f, gr.lines[i].y2(), n);
    }
}
void PaintGrapth2d::setWidthLine(uint s)
{
    widthLine = s;
}
void PaintGrapth2d::settings(int Accuracy, bool mode )
{
    this->mode = mode;
    if(Accuracy != 0)
        accur = Accuracy;
}
double PaintGrapth2d::findStart(FlDb plus, FlDb x2, FlDb bound_y1, FlDb bound_y2,SolverEq&sq,FlDb&x, mapCD&mp)
{
    double y = x2; //for avoiding unitialising
    do
    {
        x+=plus;
        if(x >= x2)
            return bound_y2 + 1;
        mp['x'] = x;
        try {
            y = sq.calcul(mp);
        }catch(...){continue;}
    }while(y < bound_y1 || y > bound_y2 || std::isnan(y) || std::isinf(y));
    x-=plus;
    try{
        mp['x'] = x;
        double y2 = sq.calcul(mp);
        if(!(std::isnan(y2) || std::isinf(y2)))
            y = y2;
        else x+=plus;
    }catch(...){x+=plus;}
    return y;
}
void PaintGrapth2d::mousePressEvent(QMouseEvent *event)
{
    /*  if(event->button() == Qt::MouseButton::RightButton)
    {
        QString txt = QString("%1; %2");
        txt = txt.arg(event->pos().x()).arg(event->pos().y());
        PrimeWindow::mainW->printLocation(txt);
    }
*/
}
void PaintGrapth2d::mouseMoveEvent(QMouseEvent *event)
{
    if(/*event->button() == Qt::MouseButton::RightButton*/1)
    {
        QSize siz = size();
        float o1 = (event->pos().x() / (float)siz.width() - 0.5 ) * sizeXY;
        float o2 = (event->pos().y() / (float)siz.height()- 0.5) * sizeXY;
        PrimeWindow::mainW->printLocation(QString("%1; %2").arg(o1).arg(o2));
    }
}
