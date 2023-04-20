#include <grapth3d.h>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include "mainwindow.h"
#define colorGrid QVector3D(0.7f,0.7f,0.7f)
const int sizeofPoint = sizeof(Point);
const int sizeofQVector3D = sizeof(QVector3D);
Point::Point(const QVector3D&p, const QVector3D&c): pos(p),
    color(c){}
//Function3d
Function3d::Function3d(QColor c, PaintGrapth3d*gr):
    Function2d(c)
{ }
Function3d::~Function3d()
{
    QTextStream q(stdout);
}
void Function3d::clear()
{
    vertexes.clear();
    count = 0;
}
//Text
Text::Text()
{
    font = new QFont("Arial", 10);
}
void Text::initfont(const QFont & f)
{
    font = new QFont(f);
}
void Text::print(QOpenGLShaderProgram&shader,QOpenGLFunctions*f, int buffer,const QString&text)
{
    QPainterPath path;
    path.addText(QPoint(0,0), *font, text);
    auto polys = path.toSubpathPolygons();
    QOpenGLBuffer*gl = new QOpenGLBuffer;
    gl->create();
    gl->bind();
    int i = 0;
    for(QList<QPolygonF>::iterator it = polys.begin(); it!= polys.end(); ++it, ++i)
    {
        std::vector<QVector3D> curP(it->size());
        int j = 0;
        for(QPolygonF::iterator it2 = it->begin(); it2!=it->end(); ++it2, ++j)
        {
            curP[j] = QVector3D(it2->x()*0.05,- it2->y()*0.05, -5.0f);
        }
        gl->allocate(curP.data(), curP.size()*sizeof(QVector3D));
        shader.setAttributeBuffer(buffer, GL_FLOAT, 0, 3, sizeof(QVector3D));
        f->glDrawArrays(GL_LINE_STRIP, 0, curP.size());
    }
    gl->release();
    delete gl;
}
void Text::getBuffers(const QString&text,std::vector<QOpenGLBuffer>&buffs,QColor color,QVector3D location,
                                            float mult)
{
    QPainterPath path;
    path.addText(QPoint(0,0), *font, text);
    auto polys = path.toSubpathPolygons();
    //std::vector<QOpenGLBuffer> buffs(polys.size());
    buffs.resize(polys.size());
    uint i = 0;
    QVector3D col = QVector3D(color.red() / 256.0f, color.green() / 256.0f,
      color.blue() / 256.0f);
    for(auto it = polys.begin(); it!=polys.end();++it,++i)
    {
        int size = it->size();
        std::vector<Point>gl(size);
        QOpenGLBuffer&curr = buffs[i];
        curr.create();
        curr.bind();
        uint ind = 0;
        for(auto iter = it->begin();iter!=it->end();++iter,++ind)
        {
            gl[ind].pos = QVector3D(iter->x()*mult + location.x(), -iter->y()*mult - location.y(),
                                    location.z());
            //gl[ind].pos = QVector3D(x, 0.0f, zz);
            gl[ind].color = col;
        }
        curr.allocate(gl.data(), size*sizeofPoint);
        curr.release();
    }
}
//PaintGrapth3d
PaintGrapth3d::PaintGrapth3d():countStepsX(50), countStepsY(50),ABC_Grapth(4.0)
  ,centerZ(0.0), radius(3.0f), angleX(0.0f), angleY(0.0f), grid(), red(0.5), green(0.5), blue(0.5)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    projection = new QMatrix4x4;
    view =  new QMatrix4x4;
    grid.count = 19;
}
void PaintGrapth3d::initializeGL()
{
    initializeOpenGLFunctions();
    axis.init();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);//z buffer
    glEnable(GL_COLOR_BUFFER_BIT);
    sh_program = new QOpenGLShaderProgram;
    if(!sh_program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                            ":/fragment.fsh"))
    {
        close();
    }
    if(!sh_program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                            ":/vershina.vsh"))
    {
        close();
    }
    if(!sh_program->link())
    {
        close();
    }
    sh_program->bind();
    view->setToIdentity();
    view->translate(0.0f, 0.0f, -radius);
    setAxis();
}
void PaintGrapth3d::struct_axis::init()
{
    OxOyOz = new QOpenGLBuffer;
    OxOyOz->create();
    Text txt;
    txt.getBuffers("Ox", labels[0],Qt::red, QVector3D(1.0f, 0.0f, 0.0f), 0.01f);
    txt.getBuffers("Oz", labels[1],Qt::green, QVector3D(0.0f, -1.0f, 0.0f), 0.01f);
    txt.getBuffers("Oy", labels[2],Qt::blue, QVector3D(0.0f, 0.0f, 1.0f), 0.01f);
}
void PaintGrapth3d::struct_grid::init()
{
    along = new QOpenGLBuffer;
    against = new QOpenGLBuffer;
    if(!along->create())
        exit(31);
    if(!against->create())
        exit(32);
}
void PaintGrapth3d::resizeGL(int width, int height)
{
    static int qty = 0;
    qty++;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(height == 0)
        height = 1;
    projection->setToIdentity();
    projection->perspective(45, width / (float)height, 0.1, 1000000.0f); 
}
void PaintGrapth3d::paintGL()
{
    Manage3d*m3d = PrimeWindow::mainW->getManage3d();
    glClearColor(red,green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(m3d==nullptr)
    {
        return;
    }
    std::vector<Function3d*>&funcs= m3d->getFunctions();
    if(funcs.empty())
        return;
    QMatrix4x4 temp = (*projection)*(*view);
    sh_program->setUniformValue("qt_ModelViewProjectionMatrix",temp);
    int locaddr = sh_program->attributeLocation("qt_Vertex");
    sh_program->enableAttributeArray(locaddr);
    int coloraddr = sh_program->attributeLocation("color");
    sh_program->enableAttributeArray(coloraddr);
    draw(funcs,locaddr,coloraddr );
    axis.OxOyOz->bind();
    sh_program->setAttributeBuffer(locaddr, GL_FLOAT,0, 3, sizeofPoint);
    sh_program->setAttributeBuffer(coloraddr, GL_FLOAT,sizeofQVector3D,
                                   3, sizeofPoint);
    glDrawArrays(GL_LINES, 0, 6);
    axis.OxOyOz->release();
    QOpenGLShaderProgram*shadprogr = sh_program;
    auto lambda = [shadprogr, locaddr, coloraddr](std::vector<QOpenGLBuffer>&labels,
            QOpenGLFunctions*f){
        for(uint i = 0; i<labels.size();i++)
        {
            QOpenGLBuffer&curr = labels[i];
            curr.bind();
            shadprogr->setAttributeBuffer(locaddr, GL_FLOAT,0, 3, sizeofPoint);
            shadprogr->setAttributeBuffer(coloraddr, GL_FLOAT,sizeofQVector3D,3, sizeofPoint);
            f->glDrawArrays(GL_LINE_STRIP, 0, curr.size() / sizeofQVector3D);
            curr.release();
        }
    };
    lambda(axis.labels[0], this);
    lambda(axis.labels[1], this);
    lambda(axis.labels[2], this);
    //draw grid on Ox.
    //drawGrid(locaddr, coloraddr);
    //draw grid on Oy
    //QMatrix4x4 temp2 = temp;
    //temp2.rotate(90, QVector3D(1.0f, 0.0f, 0.0f));
    //sh_program->setUniformValue("qt_ModelViewProjectionMatrix",temp2);
    //drawGrid(locaddr, coloraddr);
}
void PaintGrapth3d::drawGrid(int buff, int color)
{
    grid.along->bind();
    sh_program->setAttributeBuffer(buff, GL_FLOAT,0, 3, sizeofPoint);
    sh_program->setAttributeBuffer(color, GL_FLOAT, sizeofQVector3D,
                                   3, sizeofPoint);
    glDrawArrays(GL_LINES, 0, grid.count*2+2);
    grid.along->release();

    grid.against->bind();
    sh_program->setAttributeBuffer(buff, GL_FLOAT,0, 3, sizeofPoint);
    sh_program->setAttributeBuffer(color, GL_FLOAT, sizeofQVector3D,
                                   3, sizeofPoint);
    glDrawArrays(GL_LINES, 0, grid.count*2+2);
    grid.against->release();
}
void PaintGrapth3d::draw(std::vector<Function3d*>&funcs, int buff, int color)
{
    for(int i = 0; i< funcs.size();i++)
    {
        Function3d&f3d = *(funcs[i]);
        //f3d.vertexes->bind();
        sh_program->setAttributeArray(buff, GL_FLOAT, 0, 3, sizeofPoint);
        sh_program->setAttributeArray(buff, GL_FLOAT, f3d.vertexes.data(), 3, sizeofPoint);
        sh_program->setAttributeArray(color, GL_FLOAT, &f3d.vertexes.data()->color, 3, sizeofPoint);
        /*h_program->setAttributeBuffer(buff, GL_FLOAT,
                                      0, 3, sizeofPoint );
        sh_program->setAttributeBuffer(color, GL_FLOAT, sizeofQVector3D,
                                       3, sizeofPoint);*/
        if(f3d.vertexes.size() != 0)
        {
            for(auto it = f3d.along.begin(); it != f3d.along.end(); ++it)
                glDrawElements(GL_LINE_STRIP, it->size(),
                               GL_UNSIGNED_INT, it->data());
            for(auto it = f3d.against.begin(); it != f3d.against.end(); ++it)
                glDrawElements(GL_LINE_STRIP, it->size(),
                               GL_UNSIGNED_INT, it->data());
        }
        //f3d.vertexes->release();
    }
}
void PaintGrapth3d::mouseMoveEvent(QMouseEvent *event)
{
    float dx = (event->pos().x() - lastPos.x()) / (float)width();
    float dy = (event->pos().y() - lastPos.y()) / (float)height();
    lastPos = event->pos();
    angleX-= dx*180;
    angleY+= dy*180;
    view->setToIdentity();
    view->translate(QVector3D(0.0f, 0.0f, -radius));
    view->rotate(angleY, 1.0, 0.0, 0.0);
    view->rotate(-angleX, 0.0, 1.0, 0.0);
    update();
}

void PaintGrapth3d::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}
void PaintGrapth3d::wheelEvent(QWheelEvent* event)
{
    float angle = event->angleDelta().y() / 120.0f;
    radius -= angle;
    if(radius <= 0.0f)
        radius = 0.01f;
    view->setToIdentity();
    view->translate(QVector3D(0.0f, 0.0f, -radius));
    view->rotate(angleY, 1.0, 0.0, 0.0);
    view->rotate(-angleX, 0.0, 1.0, 0.0);
    update();
}
void PaintGrapth3d::setCountSteps(int cx, int cy)
{
    if(cx > 0)
        countStepsX = cx;
    if(cy > 0)
        countStepsY = cy;

}
void PaintGrapth3d::setCenter(const QVector3D&vec)
{
    centerX = vec.x();
    centerY = vec.y();
    centerZ = vec.z();
}
void PaintGrapth3d::setColorBackround(QColor c)
{
    red = c.red()/256.0f, green = c.green()/ 256.0f, blue=c.blue()/256.0f;
    update();
}
void PaintGrapth3d::setSizeXY(FlDb size)
{
    ABC_Grapth::setSizeXY(size);
    setAxis();
    Text txt;
    size/=2;
    float sizW = this->size().width();
    sizW = sizW / (sizeXY*9500.0f);
   txt.getBuffers("Ox", axis.labels[0],Qt::red, QVector3D(size, 0.0f, 0.0f), sizW);
   txt.getBuffers("Oz", axis.labels[1],Qt::green, QVector3D(0.0f, -size, 0.0f), sizW );
   txt.getBuffers("Oy", axis.labels[2] ,Qt::blue, QVector3D(0.0f, 0.0f, size), sizW);
}
void PaintGrapth3d::setGrid()
{
    float minX = -sizeXY / 2, maxX = -minX + centerX,
            minZ = minX + centerZ, maxZ = -minX + centerZ;
    minX+=centerX;
    float gridPl = sizeXY / (float)grid.count;
    float cur = minX;
    int countM2 = grid.count * 2 + 2;
    Point*along = new Point[countM2];
    Point*against = new Point[countM2];

    for(int i = 0; i < countM2; i++,cur+=gridPl)//set along grid
    {
        along[i] = Point(QVector3D(cur, 0.0f, minZ), colorGrid);
        along[++i] = Point(QVector3D(cur, 0.0f, maxZ), colorGrid);
    }
    cur = minZ;
    for(int i = 0; i < countM2; i++,cur+=gridPl)//set against grid
    {
        against[i] = Point(QVector3D(minX, 0.0f, cur), colorGrid);
        against[++i] = Point(QVector3D(maxX, 0.0f, cur), colorGrid);
    }
    //allocate for along
    grid.along->bind();
    grid.along->allocate(along, countM2* sizeofPoint);
    grid.along->release();
    //alocate for against
    grid.against->bind();
    grid.against->allocate(against, countM2* sizeofPoint);
    grid.against->release();
    //delete arrays
    delete[]along;
    delete[]against;
}
void PaintGrapth3d::setAxis()
{
    float siz = sizeXY / 2.0f;

    Point*ps = new Point[6]{
            Point(QVector3D(-siz, 0.0f, 0.0f),QVector3D(1.0f, 0.0f, 0.0f)),
            Point(QVector3D(siz, 0.0f, 0.0f),QVector3D(1.0f, 0.0f, 0.0f)),
            Point(QVector3D(0.0f, 0.0f, -siz),QVector3D(0.0f, 0.0f, 1.0f)),
            Point(QVector3D(0.0f, 0.0f, siz),QVector3D(0.0f, 0.0f, 1.0f)),
            Point(QVector3D(0.0f, -siz, 0.0f),QVector3D(0.0f, 1.0f, 0.0f)),
            Point(QVector3D(0.0f, siz, 0.0f),QVector3D(0.0f, 1.0f, 0.0f))
};
    axis.OxOyOz->bind();
    axis.OxOyOz->allocate(ps, 6 * sizeofPoint);
    axis.OxOyOz->release();
    delete[]ps;
}
void PaintGrapth3d::perform(std::vector<Function3d*>&funcs)
{
    projection->setToIdentity();
    projection->perspective(45, width() / (float)height(), 0.1, 1000000.0f);
    if(funcs.empty())return;
    float minY = -sizeXY/2,
            minX = minY + centerX,minZ = minY + centerZ,
            maxY = -minY + centerY, maxX = -minY + centerX,
            maxZ = -minY  +centerZ;
    minY+=centerY;
    float  x,z, y, stepX = (sizeXY) / FlDb(countStepsX + 1),
            stepZ = (sizeXY) / FlDb(countStepsY + 1);
    //std::vector<Point>vertexes(countStepsX * countStepsY);
    mapCD mp;
    //std::vector<GLuint>**curAgainst = new std::vector<GLuint>*[countStepsX];
    std::vector<mylist::iterator> curAgainst(countStepsX);
    mylist::iterator along;
    QVector3D color;
    int multXY = countStepsX * countStepsY;
    for(uint indF = 0; indF < funcs.size(); indF++)
    {
        Function3d&f3d = *(funcs[indF]);
        SolverEq&c = *(f3d.func);
        auto&vertexes = f3d.vertexes;
        vertexes = std::vector<Point>(multXY);
        //if(f3d.vertexes == nullptr)
        //{
        //    f3d.vertexes = new QOpenGLBuffer;
        //    f3d.vertexes->create();
        // }
        f3d.along.clear();
        f3d.against.clear();
        f3d.along.resize(countStepsY);
        color = QVector3D(f3d.color.red() / 256.0f,f3d.color.green() / 256.0f, f3d.color.blue() / 256.0f);
        f3d.against.resize(countStepsX);
        {
            std::vector<mylist::iterator>::iterator itv = curAgainst.begin();
            for(mylist::iterator it = f3d.against.begin(); it!= f3d.against.end(); ++it,++itv)
            {
                *itv = it;
                it->reserve(countStepsY);
            }
        }
        if(c.empty())continue;
        z = minZ;
        int s = 0;
        along  = f3d.along.begin();
        for (int i = 0;i < countStepsY ; i++,z+=stepZ)
        {
            mp['y'] = z;
            x = minX;
            if(!along->empty())
            {
                along->shrink_to_fit();
                ++along;
                along->reserve(countStepsX);
            }
            for(int j = 0; j< countStepsX;x+=stepX, j++)
            {
                mp['x'] = x;
                try {
                    y = c.calcul(mp);
                } catch(...) {}
                if(y < minY || y > maxY || std::isnan(y))
                {
                    if(!along->empty())
                    {
                        along->shrink_to_fit();
                        auto it = along;
                        int cap = along->capacity();
                        f3d.along.insert(++it,std::vector<GLuint>());
                        ++along;
                        cap = along->capacity();
                        along->reserve(countStepsX);
                    }
                    if(!curAgainst[j]->empty())
                    {
                        (curAgainst[j])->shrink_to_fit();
                        f3d.against.push_back(std::vector<GLuint>());
                        curAgainst[j] = (--f3d.against.end());
                        curAgainst[j]->reserve(countStepsY);
                    }
                    continue;
                }
                along->push_back(s);
                curAgainst[j]->push_back(s);
                vertexes[s].pos = QVector3D(x - centerX,y - centerY,z - centerZ);
                vertexes[s].color = color;
                s++;
            }
        }
        for(auto it = f3d.against.begin(); it != f3d.against.end(); ++it)
            it->shrink_to_fit();
        f3d.along.erase(++along, f3d.along.end());
        //f3d.vertexes->bind();
        //f3d.vertexes->allocate(vertexes.data(), s* sizeofPoint);
        //f3d.vertexes->release();
    }
}
/*void PaintGrapth3d::perform(std::vector<Function3d*>&funcs)
{
     if(funcs.empty())return;
     float minY = -sizeXY/2,
        minX = minY + centerX,minZ = minY + centerZ,
             maxY = -minY + centerY, maxX = -minY + centerX,
             maxZ = -minY  +centerZ;
     int ddd;
     minY+=centerY;
     float  x,z, y, stepX = (sizeXY) / FlDb(countStepsX + 1),
             stepZ = (sizeXY) / FlDb(countStepsY + 1);
     std::vector<Point>vertexes(countStepsX * countStepsY);
     mapCD mp;
     //std::vector<GLuint>**curAgainst = new std::vector<GLuint>*[countStepsX];
     std::vector<std::vector<GLuint>*> curAgainst(countStepsX);
     for(uint indF = 0; indF < funcs.size(); indF++)
     {
         Function3d&f3d = *(funcs[indF]);
         SolverEq&c = *(funcs[indF]->func);
         if(f3d.vertexes == nullptr)
         {
             f3d.vertexes = new QOpenGLBuffer;
             f3d.vertexes->create();
         }
         f3d.along.clear();
         f3d.against.clear();
         f3d.along.resize(countStepsY);
         f3d.against.resize(countStepsX);
         for(int i = 0; i< countStepsX; i++)
         {
             std::vector<GLuint>*ag = &(f3d.against[i]);
             curAgainst[i] = ag;
             ag->reserve(countStepsY);
         }
         if(c.empty())continue;
         z = minZ;
         int maxJ = 0, i = 0, s = 0;
         std::vector<GLuint>*along = &(f3d.along[i]);
         for (;i < countStepsY ; i++,z+=stepZ)
         {
             mp['y'] = z;
             x = minX;
             if(!along->empty())
             {
                 along = &(f3d.along[i]);
                 along->reserve(countStepsX);
             }
             int real =0;
             for(int j = 0; j< countStepsX;x+=stepX, j++)
             {
                 //QTextStream qts(stdout);
                     ddd = j;
                 mp['x'] = x;
                 try {
                      y = c.calcul(mp);
                 } catch(...) {}
                 if(y < minY || y > maxY || std::isnan(y))
                 {
                     //qts<<"; "<<j;
                     if(!along->empty())
                     {
                         along->shrink_to_fit();
                         f3d.along.push_back(std::vector<GLuint>());
                         along = &(f3d.along.back());
                         along->reserve(countStepsX);
                     }
                     if(!curAgainst[j]->empty())
                     {
                         std::vector<GLuint>*ag = (curAgainst[j]);
                         int cap = ag->capacity();

                         curAgainst[j]->shrink_to_fit();
                         cap = ag->capacity();
                         f3d.against.push_back(std::vector<GLuint>());
                         curAgainst[j] = &(f3d.against.back());
                         curAgainst[j]->reserve(countStepsY);
                     }
                     continue;
                 }
                 qDebug().nospace()<<"A";
                 along->push_back(s);
                 curAgainst[j]->push_back(s);
                 qDebug().nospace()<<"B";
                 vertexes[s].pos = QVector3D(x,y,z);
                 vertexes[s].color = QVector3D(0.0f,0.0f,0.0f);
                 real++;
                 s++;
                 //qts<<j;
             }
             qDebug()<<'\n'<<i;
             if(i >= 125)
                 qDebug()<<"IamHere";
             if(maxJ < real)
                 maxJ = real;
             //along->shrink_to_fit();
         }
         for(int ind = 0; ind< i;ind++)
             f3d.against[ind].shrink_to_fit();
         f3d.along.shrink_to_fit();
         f3d.vertexes->bind();
         f3d.vertexes->allocate(vertexes.data(), s* sizeofPoint);
         f3d.vertexes->release();
       }
}*/
//mylist
/*mylist::mylist(uint size):std::list<std::vector<GLuint> >(size), cap(0)
{}
void mylist::reserve(uint cap)
{
    int diff = this->cap - cap;
    iterator it = this->end();
    std::advance(it, diff);
    if(diff > 0)
        this->erase(it,this->end());
    else this->insert(this->end(), diff, std::vector<GLuint>());
    this->cap=cap;
}
void mylist::reserve_add(uint cap)
{
    reserve(cap + this->cap);
}
void mylist::shrink_to_fit()
{
    reserve(-this->cap);
}*/
