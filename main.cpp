#include "mainwindow.h"
#include "consol.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    #ifndef QT_NO_OPENGL
    Console*cs = new Console;
    cs->show();
    PaintGrapth2d*gr2d = new PaintGrapth2d;
    PaintGrapth3d*gr3d = new PaintGrapth3d;
    PrimeWindow*window = new PrimeWindow(cs,gr2d,gr3d);
    window->show();
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    /*cs->commandEntered("set mode 3d");
    cs->commandEntered("add x*y");
    cs->commandEntered("update");*/
    return a.exec();
}
