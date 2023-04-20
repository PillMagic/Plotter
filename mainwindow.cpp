#include "mainwindow.h"
#include <cassert>
#include <limits>
#include <QMessageBox>
#include <QLineEdit>
#include "consol.h"
ABS_Manage*LineButt::commonManage = nullptr;
PrimeWindow*PrimeWindow::mainW = nullptr;
//PrimeWindowout.close();

PrimeWindow::PrimeWindow(Console*cs,PaintGrapth2d*gr2d,PaintGrapth3d*gr3d,QWidget*parent):
    w3d(nullptr),QMainWindow(parent),grapth2d(gr2d),grapth3d(gr3d),console(cs)
{
    assert(mainW == nullptr);
    mainW = this;
    this->setupUi(this);
    horizontalLayout_3->addWidget(grapth2d);
    horizontalLayout_3->addWidget(grapth3d);
    grapth3d->setHidden(1);
    w2d = new Manage2d(*this,grapth2d);
    curGrapth = grapth2d;
    curManage = w2d;
    palet = palette();
    //Well, we should add menu >
    QMenu*Mmenu = this->menuBar()->addMenu("&File");
    QAction*t1 = new QAction("&Save", this);
    t1->setShortcut(tr("CTRL+S"));
    QAction* t2 = new QAction("&Save as...", this);
    t2->setShortcut(tr("CTRL+SHIFT+S"));
    QAction* t3 = new QAction("&Open", this);
    t3->setShortcut(tr("CTRL+O"));
    Mmenu->addAction(t1);
    Mmenu->addAction(t2);
    Mmenu->addAction(t3);
    Mmenu = this->menuBar()->addMenu("&Additionally");
    actionMode = new QAction("&2D grapth", this);
    t2 = new QAction("&Console", this);
    actionMode->setCheckable(true);
    actionMode->setChecked(true);
    connect(actionMode, &QAction::triggered, this, &PrimeWindow::setMode);
    connect(t2, &QAction::triggered, console, &Console::show);
    Mmenu->addAction(actionMode);
    Mmenu->addAction(t2);
    //Well, we should set Max and Min of Write-Widgets
    std::numeric_limits<double> Dlim;
    SizeD->setRange(0.01,Dlim.max());
    XD->setRange(-Dlim.max() + 10,Dlim.max() - 10);
    YD->setRange(-Dlim.max() + 10,Dlim.max() - 10);
    AccuracyD->setRange(1,std::numeric_limits<int>::max() - 10);
    SizeSlider->setRange(1, 1000);
    QWidget*container = new QWidget(FuncArea);
    LayInFuncArea = new QVBoxLayout(container);
    FuncArea->setWidget(container);
    connect(UpdateB, &QPushButton::clicked, this, &PrimeWindow::on_Update);
    connect(AddFB, &QPushButton::clicked, this, &PrimeWindow::on_AddF);
}
PrimeWindow::~PrimeWindow()
{
    curManage = nullptr;
    delete console;
    delete w2d;
    delete w3d;
    delete grapth2d;
    delete grapth3d;
}
void PrimeWindow::setMode(bool mode2d)
{
    setMode2(mode2d,false);
}
void PrimeWindow::setMode2(bool mode2d, bool d)
{
    static bool dontWorryBeHappy = 0; //for avoiding endless loop
    if(dontWorryBeHappy)return;
    if(mode2d)
    {
        if(w3d == nullptr)return;
        if(!d)
           curManage->deleteData();
        w2d = new Manage2d(*this, grapth2d);
        if(d)fromM1toM2(w3d, w2d);
        delete w3d;
        w3d = nullptr;
        curGrapth = grapth2d;
        curManage = w2d;
    }
    else
    {
        if(w2d==nullptr)return;
        if(!d)
            curManage->deleteData();
        w3d = new Manage3d(*this,grapth3d);
        if(d)fromM1toM2(w2d, w3d);
        delete w2d;
        w2d = nullptr;
        curManage = w3d;
        curGrapth = grapth3d;
    }
    grapth2d->setVisible(mode2d);
    grapth3d->setVisible(!mode2d);
    dontWorryBeHappy = 1;
    actionMode->setChecked(mode2d); //it call PrimeWindow::setMode
    //so, we need to return, when dontWorryBeHappy == true
    //otherwise endless loop will be here
    dontWorryBeHappy = 0;
}
void PrimeWindow::perform()
{
    if(curManage == nullptr)
        return;
    curManage->perform();
    if(w3d==nullptr)
        grapth2d->repaint();
}
void PrimeWindow::updateGrapth()
{
    if(curManage == nullptr)
        return;
    if(w3d==nullptr)
        curManage->perfRep();
    else
    {
        grapth3d->update();
    }
}
bool PrimeWindow::setColor(QColor c, char type)
{
    switch (type) {
    case 'b':
        CommonData::backround = c;
        grapth3d->setColorBackround(c);
        break;
    case 'a':
        palet.setColor(QPalette::Background,c);
        this->setPalette(palet);
        break;
    case 'p':
        palet.setColor(QPalette::Button,c);
        this->setPalette(palet);
        break;
    case 'g':
        CommonData::gr.colorGrid = c;
        //grapth3d->activeTheme()->setGridLineColor(c);
        break;
    case 'l':
        CommonData::labelsColor = c;
        //grapth3d->activeTheme()->setLabelTextColor(c);
        break;
    default:
        return false;
    }
    perform();
    return true;
}
void PrimeWindow::printLocation(const QString&txt)
{
    locationLabel->setText(txt);
}
void PrimeWindow::closeEvent(QCloseEvent *event)
{
    curManage = nullptr;
    delete console;
    delete w2d;
    delete w3d;
    delete grapth2d;
    delete grapth3d;
}
void PrimeWindow::on_Update()
{
    curManage->on_Update();
}
void PrimeWindow::on_AddF()
{
    curManage->on_AddF();
}
//LineButt
LineButt::LineButt(const char* buttonStr, QWidget*parent): QObject (parent)
{
    butt = new QPushButton(buttonStr, parent);
    edit = new QLineEdit(parent);
    edit->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    butt->setMaximumSize(40, 40);
    color = new QPushButton("C", parent);
    color->setMaximumSize(QSize(40,40));
    color->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    connect(butt, SIGNAL(clicked()), this, SLOT(deleteMe()));
    connect(color, SIGNAL(clicked()), this, SLOT(setColor()));
}
void LineButt::setColor()
{
    QColor cl = QColorDialog::getColor(Qt::white, PrimeWindow::mainW);
    LineButt::commonManage->setColorLine(this, cl);
}
void LineButt::deleteMe()
{
    LineButt::commonManage->delIArea(this);
    delete this;
}
LineButt::~LineButt()
{
    delete this->butt;
    delete this->edit;
    delete this->color;
}
//ABS_Manage
ABS_Manage::ABS_Manage(PrimeWindow&w):wind(w){
    LineButt::commonManage = this;
}
void ABS_Manage::deleteData()
{
    for(LineButt*x:itemsArea)
        delete x;
    QLayoutItem*item;
    while((item = wind.LayInFuncArea->takeAt(0)) != 0)
    {
        wind.LayInFuncArea->removeItem(item);
        delete item;
    }
}
void ABS_Manage::delIArea(LineButt*lb){
    int ind = itemsArea.indexOf(lb);
    delIArea(ind);
}
void ABS_Manage::delIArea(int ind)
{
    if(ind < 0)return;
    itemsArea.remove(ind);
    delete  wind.LayInFuncArea->takeAt(ind);// delete LayoutItem. FIXME
    wind.LayInFuncArea->invalidate();
    delFunc(ind);
}
void ABS_Manage::setColorLine(LineButt*lb, QColor qc)
{
    int ind = itemsArea.indexOf(lb);
    setColorFunc(ind, qc);
    wind.updateGrapth();
}
void ABS_Manage::on_AddFCommon(const QString&s){
    QHBoxLayout*added = new QHBoxLayout();
    LineButt*l = new LineButt("X");
    added->addWidget(l->butt);
    added->addWidget(l->edit);
    added->addWidget(l->color);
    l->edit->setText(s);
    itemsArea.append(l);
    wind.LayInFuncArea->addLayout(added);
}
//Manage2d
Manage2d::Manage2d(PrimeWindow&wind, PaintGrapth2d*gr2d):
    ABS_Manage(wind), grapth2d(gr2d){
    //connect(wind.UpdateB, &QPushButton::clicked, this, &Manage2d::on_Update);
    //connect(wind.AddFB, &QPushButton::clicked, this, &Manage2d::on_AddF);
    wind.XD->setValue(gr2d->getCenterX());
    wind.YD->setValue(gr2d->getCenterY());
    wind.SizeD->setValue(gr2d->getSizeXY());
    connect(wind.SizeD, SIGNAL(valueChanged(double)), this, SLOT(SizeXYChanged(double)));
    connect(wind.XD, SIGNAL(valueChanged(double)), this, SLOT(XChanged(double)));
    connect(wind.YD, SIGNAL(valueChanged(double)), this, SLOT(YChanged(double)));
    connect(wind.AccuracyD, SIGNAL(valueChanged(int)), this, SLOT(AccurChanged(int)));
    connect(wind.ModeC, SIGNAL(stateChanged(int)), this, SLOT(ModeChang(int)));
    connect(wind.SizeSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderChanged(int)));
}
void Manage2d::setColorFunc(int ind, QColor c)
{
    disconnect(wind.SizeD, SIGNAL(valueChanged(double)), this, SLOT(SizeXYChanged(double)));
    functions[ind]->color = c;
}
Manage2d::~Manage2d()
{
    disconnect(wind.SizeD, SIGNAL(valueChanged(double)), this, SLOT(SizeXYChanged(double)));
    disconnect(wind.XD, SIGNAL(valueChanged(double)), this, SLOT(XChanged(double)));
    disconnect(wind.YD, SIGNAL(valueChanged(double)), this, SLOT(YChanged(double)));
    disconnect(wind.AccuracyD, SIGNAL(valueChanged(int)), this, SLOT(AccurChanged(int)));
    disconnect(wind.ModeC, SIGNAL(stateChanged(int)), this, SLOT(ModeChang(int)));
    disconnect(wind.SizeSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderChanged(int)));
    for(Function2d*x: functions)
        delete x;
}
void Manage2d::on_AddF(const QString&s){
    ABS_Manage::on_AddFCommon(s);
    functions.push_back(new Function2d(QColor(rand()%200, rand()%200, rand()%200)));
}
void Manage2d::delFunc(int ind){
    delete functions[ind];
    functions.erase(functions.begin() + ind);
}
void Manage2d::XChanged(double n){
    grapth2d->setCenterX(n);
    perfRep();
}
void Manage2d::YChanged(double n){
    grapth2d->setCenterY(n);
    perfRep();
}
void Manage2d::AccurChanged(int n){
    grapth2d->accur = n;
    perfRep();
}
void Manage2d::SizeXYChanged(double n){
    grapth2d->setSizeXY(n);
    perfRep();
}
void Manage2d::SliderChanged(int n){
    wind.SizeD->setValue(n);
}
void Manage2d::ModeChang(int m){
    if(m == 2)grapth2d->settings(grapth2d->accur, true);
    else if(m ==0)grapth2d->settings(grapth2d->accur, false);
    perfRep();
}
void Manage2d::on_Update()
{
    std::setlocale(LC_ALL, "C");
    grapth2d->set(wind.SizeD->value(), wind.XD->value(), wind.YD->value());
    grapth2d->settings(wind.AccuracyD->value(), wind.ModeC->isChecked());
    int sizeIts = itemsArea.size();
    for(int i = 0; i< sizeIts; i++)
    {
        try {
            setFunc(itemsArea[i]->edit->text().toUtf8(), functions[i]->func);
        } catch (const char*er) {
            QMessageBox::warning(&wind, "Warning", QString("<html><head/><body><p align=\"center\"><span style=\" font-weight:600; color:#aa0000;\">%1</span></p></body></html>")
                                 .arg(er));
            return;
        }
    }
    grapth2d->perform(functions);
    grapth2d->repaint();
}
void Manage2d::repaint(){grapth2d->repaint();}
void Manage2d::perform(){grapth2d->perform(functions);}
void Manage2d::perfRep(){grapth2d->perform(functions); repaint();}
//Manage3d
Manage3d::Manage3d(PrimeWindow&wind, PaintGrapth3d*gr3d):
    ABS_Manage(wind), grapth3d(gr3d)
{
    //change window for 3d mode.
    ZD = new QDoubleSpinBox;
    qtyStepsYD = new QSpinBox;
    qtyStepsYD->setRange(1,INT_MAX);
    double mxDouble = std::numeric_limits<double>::max();
    ZD->setRange(-mxDouble, mxDouble);
    QString str = wind.XL->text().replace("X", "Z");
    ZL = new QLabel(str, &wind);
    wind.grid->removeWidget(wind.SizeD);
    delete wind.SizeD;
    wind.SizeD = new QDoubleSpinBox(&wind);
    wind.SizeD->setRange(0.01, std::numeric_limits<double>::max());
    wind.grid->addWidget(wind.SizeD, 0, 1, 1, 1);
    wind.grid->addWidget(ZL, 0, 2, 1, 1);
    wind.grid->addWidget(ZD, 0, 3, 1, 1);
    wind.grid->addWidget(qtyStepsYD, 3,3,1,1);
    wind.AccuracyL->setText(wind.AccuracyL->text().replace("Accuracy", "Qty steps Ox"));
    wind.ModeL->setText(wind.ModeL->text().replace("Auto", "Qty steps Oy"));
    wind.ModeC->hide();
    //wind.qtyStepsXD is equal to wind.AccuracyD
    //I defined AccuracyD as qtyStepsXD
    wind.SizeSlider->hide();
    wind.horizontalLayout_3->update();
    //connect(wind.UpdateB, &QPushButton::clicked, this, &Manage3d::on_Update);
    //connect(wind.AddFB, &QPushButton::clicked, this, &Manage3d::on_AddF);
    wind.AccuracyD->setValue(50);
    qtyStepsYD->setValue(50);
    wind.XD->setValue(gr3d->getCenterX());
    wind.YD->setValue(gr3d->getCenterY());
    wind.SizeD->setValue(gr3d->getSizeXY());
    ZD->setValue(gr3d->getCenterZ());
}
std::vector<Function3d*>&Manage3d::getFunctions(){
    return functions;
}
Manage3d::~Manage3d()
{
    wind.grid->removeWidget(ZD);
    wind.grid->removeWidget(qtyStepsYD);
    wind.grid->removeWidget(ZL);
    delete ZD;
    delete qtyStepsYD;
    delete ZL;
    wind.grid->removeWidget(wind.SizeD);
    wind.grid->addWidget(wind.SizeD, 0, 3, 1, 1);
    wind.AccuracyL->setText(wind.AccuracyL->text().replace("Qty steps Ox", "Accuracy"));
    wind.ModeL->setText(wind.ModeL->text().replace("Qty steps Oy", "Auto"));
    wind.ModeC->setHidden(0);
    wind.SizeSlider->setHidden(0);
    //wind.grapth3d->hide();
    wind.horizontalLayout_3->update();
    for(Function3d*x: functions)
        delete x;
    wind.AccuracyD->setValue(1);
}
void Manage3d::setColorFunc(int ind, QColor c)
{
    Function3d&fnc = *(functions[ind]);
    std::vector<Point>&verts = fnc.vertexes;
    fnc.color = c;
    QVector3D color = QVector3D(c.red() / 256.0f, c.green() / 256.0f, c.blue() / 256.0f);
    for(auto&x: verts)
    {
        x.color = color;
    }
}
void Manage3d::on_AddF(const QString&s){
    ABS_Manage::on_AddFCommon(s);
    functions.push_back(new Function3d(QColor(rand()%200, rand()%200, rand()%200),grapth3d));
}
void Manage3d::on_Update()
{
    std::setlocale(LC_ALL, "C");
    grapth3d->setSizeXY(wind.SizeD->value());
    grapth3d->setCenter(QVector3D(wind.XD->value(),  ZD->value(), wind.YD->value()));
    grapth3d->setCountSteps(wind.qtyStepsXD->value(), qtyStepsYD->value());
    int sizeIts = itemsArea.size();
    for(int i = 0; i< sizeIts; i++)
    {
        try {
            setFunc(itemsArea[i]->edit->text().toUtf8(), functions[i]->func);
        } catch (const char*er) {
            QMessageBox::warning(&wind, "Warning", QString("<html><head/><body><p align=\"center\"><span style=\" font-weight:600; color:#aa0000;\">%1</span></p></body></html>")
                                 .arg(er));
            return;
        }
    }
    grapth3d->perform(functions);
    grapth3d->update();
}
void Manage3d::repaint(){grapth3d->repaint();}
void Manage3d::perform(){grapth3d->perform(functions);}
void Manage3d::perfRep(){grapth3d->perform(functions); repaint();}
void Manage3d::delFunc(int ind){
    delete functions[ind];
    functions.erase(functions.begin() + ind);
}

Function2d*moving(Function3d*f3d)
{
    Function2d*f2d = new Function2d;
    f2d->func = f3d->Function2d::func;
    f3d->Function2d::func = nullptr;
    f2d->color = f3d->Function2d::color;
    return f2d;
}
Function3d*moving(Function2d*f2d)
{
    Function3d*f3d = new Function3d;
    f3d->color = f2d->color;
    f3d->Function2d::func = f2d->func;
    f2d->func = nullptr;
    return f3d;
}
template<class M1, class M2>
void fromM1toM2(M1*from, M2*to)
{
    to->itemsArea = std::move(from->itemsArea);
    int siz = from->functions.size();
    to->functions.reserve(siz);
    for(int i = 0; i < siz; i++)
        to->functions.push_back(moving(from->functions[i]));
}
