#include "consol.h"
#include <QKeyEvent>
#include <QTextBlock>
#include <mainwindow.h>
#include <cassert>
#include <QColorDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QPalette>
Console*Console::console = nullptr;
MapPars Parser::funcs
{
    PairPars("set", forParser::set),
    PairPars("read", forParser::read),
    PairPars("save", forParser::save),
    PairPars("add", forParser::add),
    PairPars("erase", forParser::erase),
    PairPars("update", forParser::update),
    PairPars("help", forParser::help),
    PairPars("get", forParser::get)
};
MapPars Function_set::variables
{
    PairPars("color", Function_set::color),
    PairPars("mode", Function_set::mode),
    PairPars("grid", Function_set::grid),
    PairPars("width", Function_set::width)
};
MapPars2 Function_get::variables
{
    PairPars2("color", Function_get::color),
    PairPars2("mode", Function_get::mode),
    PairPars2("grid", Function_get::grid),
    PairPars2("width", Function_get::width)
};
QString Parser::midSpace(const QString&str, int pos)
{
    int size = str.size();
    int i = pos;
    for(; i< size; i++)
        if(!str[i].isSpace())
            break;
    if(i >= size)return "";

    return str.mid(i);
}
QString Parser::selectSpace(const QString&str, int pos,int*endS)
{
    if(pos < 0)return "";
    QString rtrn = "";
    int size = str.size(), st =pos, n=0 ;
    bool was = 0;
    int i = pos;
    for(; i < size; i++)
        if(!str[i].isSpace())
        {
            if(!was)
                st = i;
            n++;
            was = true;
        }
        else
            if(was)
                goto jump;
    if(was)
    {
jump:
        rtrn = str.mid(st, n);
        if(endS != nullptr)
            *endS = i;
    }
    return rtrn;
}
QColor Parser::strToColor(const QString&str)
{
    int colors[3]{0,0,0};
    int size = str.size();
    for(int i = 0, indC = 0; i< size;i++)
    {
        QChar ch = str[i].toLatin1();
        if(ch >= '0' && ch <= '9')
        {
            QString s;
            for(;i < size&& str[i] >= '0' && str[i]<= '9';i++)
                s+=str[i];
            colors[indC++] = s.toInt();
            if(indC >= 3)
                break;
        }
    }
    return QColor(colors[0], colors[1], colors[2]);
}
void Parser::getRL(const QString&str, QString&r, QString&l)
{
    int i = str.indexOf(' ');
    if(i< 0){ //there is no space
        l = str;
        r = "";
    }
    else
    {
        l = str.left(i);
        r = Parser::midSpace(str, i);
    }
}
void Parser::getParametres(const QString &str, std::string &params, int *ind)
{
    int ind2 = 0;
    QString st =Parser::selectSpace(str,0,&ind2);
    while(st != "")
    {
        if(st[0] == '-')
        {
            for(int i = 1; i< st.size(); i++)
                params.push_back(st[i].toLatin1());
            if(ind!=nullptr)
                *ind = ind2;
        }
        st =Parser::selectSpace(str,ind2,&ind2);
    }
}
void Parser::getParsAndArgs(const QString&str, parsAndArgs&paa)
{
    int ind = 0;
    QString st = Parser::selectSpace(str, 0, &ind);
    while(st.size()!=0)
    {
        if(st[0] == '-')
            for(int i = 1; i< st.size(); i++)
                paa.pars.push_back(st[i].toLatin1());
        else
            paa.arg = st;
        st =Parser::selectSpace(str,ind,&ind);
    }
}
QString Parser::colorToStr(const QColor&c)
{
    return QString("(%1;%2;%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}
Console::Console(const QString&promptA):prompt(promptA),
    maxHistSize(100)
{
    histPos = history.end();
    assert(console == nullptr);
    QPalette pal;
    pal.setColor(QPalette::Base, QColor(74, 0, 69));
    pal.setColor(QPalette::Text, QColor(250,250,250));
    this->setPalette(pal);
    console = this;
    insertPrompt(false);
}
void Console::insertPrompt(bool newBlock)
{
    if(newBlock)
        textCursor().insertBlock();
    textCursor().insertText(prompt);
}
void Console::keyPressEvent(QKeyEvent *ev){
    int key = ev->key();
    if(key == Qt::Key_Backspace && ev->modifiers()==Qt::NoModifier){
        if(textCursor().positionInBlock()> prompt.size())
            textCursor().deletePreviousChar();
    }
    else if(key == Qt::Key_Return && ev->modifiers()==Qt::NoModifier)
        onEnter();
    else if(key == Qt::Key_Up)
        histBack();
    else if(key == Qt::Key_Down)
        histForw();
    else
       this->QPlainTextEdit::keyPressEvent(ev);
}
void Console::onEnter()
{
    if(textCursor().positionInBlock() == prompt.length())
    {
        insertPrompt(true);
        return;
    }
    QString entered = textCursor().block().text().mid(prompt.length());
    histAdd(entered);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);
    this->setTextCursor(cursor);
    bool succes = commandEntered(entered);
    if(!succes)
        print(entered, Qt::red);
    else
        print(entered);
}
bool Console::commandEntered(QString&str)
{
    int pos = str.indexOf(' ');//we take first word
    //for example, there is "set color ...". We take word "set"
    if(pos < 0)//there is no spaces. We take all str.
        pos = str.size();//For instance, command "help"
    auto it = Parser::funcs.find(str.left(pos));
    if(it != Parser::funcs.end())//there is command
    {
        str = Parser::midSpace(str,pos);
        bool s =  it.value()(str);
        this->setFocus();
        return s;
    }
    //for example, there is "set   color -a opt". We send "color -a opt"
    else
        str = "Unknown command";
    this->setFocus();
    return false;
}
void Console::print(const QString&str)
{
    textCursor().insertBlock();
    textCursor().insertText(str);
    insertPrompt(str != "");
}
void Console::print(const QString&str, QColor c)
{
    textCursor().insertBlock();
    QTextCharFormat format;
    format.setForeground(c);
    this->setCurrentCharFormat(format);
    textCursor().insertText(str);
    format.setForeground(palette().color(QPalette::Text));
    this->setCurrentCharFormat(format);
    insertPrompt(str != "");
}
void Console::printWithCodes(QString str)
{
    //codes: c - color
    QColor start;
    QTextCharFormat format;
    format.setForeground(palette().color(QPalette::Text));
    textCursor().insertBlock();
    int lastind = 0;
    int ind = str.indexOf('\\');
    while(ind != -1)
    {
        QChar ch = str[ind+1];
        if(ch == 'c')
        {
            start = format.foreground().color();
            QString temp = Parser::selectSpace(str, ind);
            textCursor().insertText(str.mid(lastind,ind - lastind));
            format.setForeground(Parser::strToColor(temp));
            this->setCurrentCharFormat(format);
            lastind = ind + temp.size() + 1;//temp.size() already include 'c',
            //so here + 1
        }
        else if(ch == 'd')
        {

            textCursor().insertText(str.mid(lastind,ind - lastind));
            format.setForeground(start);
            this->setCurrentCharFormat(format);
            lastind = ind + 2;
        }
        ind = str.indexOf('\\', ind+1);
    }
    ind = str.size();
    textCursor().insertText(str.mid(lastind,ind - lastind));
}
void Console::setColor(QPalette::ColorRole role,QColor c)
{
    QPalette pal = this->palette();
    pal.setColor(role, c);
    this->setPalette(pal);
}
void Console::histAdd(const QString&str)
{
    history.push_back(str);
    if(history.size() > maxHistSize)
        history.pop_front();
    histPos = history.end();
}
void Console::histForw()
{
    if(histPos != history.end())
        histPos++;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    insertPrompt(false);
    if(histPos != history.end())
        textCursor().insertText(*histPos);
}
void Console::histBack()
{
    if(histPos != history.begin())
        histPos--;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    insertPrompt(false);
    if(history.size() != 0)
        cursor.insertText(*histPos);
}
bool Function_set::color(QString&str)
{
    parsAndArgs ps;
    Parser::getParsAndArgs(str, ps);
    QColor color;
    if(ps.arg.size()==0 || ps.pars.empty())
    {
        str = "set color: there is no parametre or argument";
        return false;
    }
    else if(ps.arg == "opt")
        color = QColorDialog::getColor();
    else color = Parser::strToColor(ps.arg);
    for(QChar&x:ps.pars)
    {
        if(!PrimeWindow::mainW->setColor(color, x.toLatin1()))
        {
            if(x == 'c')
                Console::console->setColor(QPalette::Base, color);
            else if(x == 't')
                Console::console->setColor(QPalette::Text, color);
            else
            {
                str = "set color: unknown parametre";
                return false;
            }
        }
    }
    str = "";
    return true;
}
bool Function_set::mode(QString&str)
{
    int ind = 0;
    QString parOrArg = Parser::selectSpace(str, 0,&ind).toUpper();
    bool par = (parOrArg[0] == '-');//is there parametre, such as '-d'
    if(par)
        parOrArg = Parser::selectSpace(str.toUpper(), ind);
    if(parOrArg == "2D")
        PrimeWindow::mainW->setMode2(true, par);
    else if(parOrArg == "3D")
        PrimeWindow::mainW->setMode2(false, par);
    else
    {
        str= "set mode: invalid argument";
        return false;
    }
    str = "";
    return true;
}
bool Function_set::grid(QString&str)
{
    parsAndArgs ps;
    Parser::getParsAndArgs(str, ps);
    if(ps.pars.empty() || ps.arg.size()==0)
    {
        str = "set grid: there is no parametre or argument";
        return false;
    }
    for(QChar&x:ps.pars)
        if(x == 'c') //color
        {
            str = "-g " + ps.arg;
            color(str);
        }
        else if(x == 'q'){
            bool ok;
            int qty = ps.arg.toInt(&ok);
            if(ok && qty > 0)
                PrimeWindow::mainW->getGrapth2d()->gridResize(qty);
            else{
                str = "set grid -q: invalid argument(must be number > 0)";
                return false;
            }
            str = "";
        }
        else
        {
            str ="set grid: invalid parametre(must be -c or -q)";
            return false;
        }
    return true;
}
bool Function_set::width( QString&str)
{
    bool ok;
    int par = Parser::selectSpace(str).toInt(&ok);
    if(ok && par > 0)
    {
        PrimeWindow::mainW->getGrapth2d()->setWidthLine(par);
        PrimeWindow::mainW->perform();
    }
    else
    {
        str=  "set width: invalid argument";
        return false;
    }
    str = "";
    return true;
}
bool Function_get::color(QString&str)
{
    std::string pars;
    Parser::getParametres(str, pars, nullptr);
    if(pars.empty())
    {
        str = "get color: there is no parametres";
        return false;
    }
    str="";
    int sizpars = pars.size();
    for(int i = 0; i< sizpars; i++)
    {
        if(i> 0)
            str+='\n';
        char ch = pars[i];
        switch (ch) {
        case 'b':
            str+= Parser::colorToStr(CommonData::backround);
            break;
        case 'a':
            str+= Parser::colorToStr(PrimeWindow::mainW->palette().color(QPalette::Background));
            break;
        case 'p':
            str+= Parser::colorToStr(PrimeWindow::mainW->palette().color(QPalette::Button));
            break;
        case 'g':
            str+= Parser::colorToStr(CommonData::gr.colorGrid);
            break;
        case 'l':
            str+= Parser::colorToStr(CommonData::labelsColor);
            break;
        case 'c':
            str += Parser::colorToStr(Console::console->palette().color(QPalette::Base));
            break;
        case 't':
            str += Parser::colorToStr(Console::console->palette().color(QPalette::Text));
            break;
        default:
            str= "get color: wrong parametre";
            return false;
            break;
        }
    }
    return true;
}
bool Function_get::mode(QString&str)
{
    str = (PrimeWindow::mainW->isMode2d()?"2D": "3D");
    return true;
}
bool Function_get::grid(QString&str)
{
    std::string pars;
    Parser::getParametres(str, pars, nullptr);
    for(char ch:pars)
    {
        switch(ch)
        {
        case 'c':
            str = Parser::colorToStr(CommonData::gr.colorGrid);
            break;
        case 'q':
            str = QString::number(CommonData::gr.gridProp);
            break;
        default:
            str= "get grid: there is no parametres";
            return false;
            break;
        }
    }
    return true;
}
bool Function_get::width(QString&str)
{
    str = QString::number(PrimeWindow::mainW->getGrapth2d()->getWidthLine());
    return true;
}
namespace forParser {
bool get( QString&str)
{
    //there is no word "get" in str.
    QString l;

    Parser::getRL(str, str, l);
    auto it = Function_get::variables.find(l);
    if(it != Function_get::variables.end())
        return it.value()(str);
    l = l.toUpper();
    if(l == "SIZE" )
        str = QString::number(PrimeWindow::mainW->SizeD->value());
    else if(l == "CENTERX" )
        str = QString::number(PrimeWindow::mainW->XD->value());
    else if(l == "CENTERY" )
        str = QString::number(PrimeWindow::mainW->YD->value());
    else if(l == "CENTERZ")
        str = QString::number(PrimeWindow::mainW->getGrapth3d()->getCenterZ());
    else if(l == "OX")
        str = QString::number(PrimeWindow::mainW->AccuracyD->value());
    else if(l == "OY")
        str = QString::number(PrimeWindow::mainW->getGrapth3d()->getCountSteps().second);
    else{
        str =  "get: unknown argument or paramtre";
        return false;
    }
    return true;
}
bool set( QString&str)
{
    //there is no word "set".
    QString l;
    Parser::getRL(str, str, l);
    auto it = Function_set::variables.find(l);
    if(it!= Function_set::variables.end())
        return it.value()(str);
    bool ok;
    double n = str.toDouble(&ok);
    l = l.toUpper();
    if(ok)
    {
        Manage3d*d3 = PrimeWindow::mainW->getManage3d();
        if(d3 != nullptr)
        {
            if(l == "OY")
                d3->qtyStepsYD->setValue(n);
            else goto other;
            return true;
        }
        else
        {}
        other:
        if(l == "SIZE" )
            PrimeWindow::mainW->SizeD->setValue(n);
        else if(l == "CENTERX" )
            PrimeWindow::mainW->XD->setValue(n);
        else if(l == "CENTERY" )
            PrimeWindow::mainW->YD->setValue(n);
        else if(l == "CENTERZ")
            d3->ZD->setValue(n);
        else if(l == "OX")
            PrimeWindow::mainW->AccuracyD->setValue(n);
        else if(l == "OY")
            PrimeWindow::mainW->getGrapth3d()->setCountSteps(-1,n);
        else{
            str= "set: unknown argument or paramtre";
            return false;
        }
    }
    else{
        str= "set: unknown argument or paramtre";
        return false;
    }
    str="";
    return true;
}
bool read( QString&str)
{
    QString arg = Parser::selectSpace(str, 0);
    if(arg == "opt")
        arg = QFileDialog::getOpenFileName(Console::console, "Choose file for reading", "",
                                      "Plotter Function File (*.fpf);;All files(*)");
    QFile file(arg);
    if(!file.open(QIODevice::ReadOnly))
        return "read commands: cannot open the file. Perhaps it doesn't exist";
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        if(line.size() == 0)
            break;
        arg = Console::console->commandEntered(line);
        if(arg.size()!=0)
        {
            file.close();
            str= "read commands: occured some error("+arg+")";
            return false;
        }
    }
    file.close();
    str="";
    return true;
}
bool save( QString&str)
{
    using namespace CommonData;
#define mw PrimeWindow::mainW
    int ind = 0;
    bool commands = 1, appl = 1;
    QString arg = Parser::selectSpace(str,0,&ind);
    if(arg == "functions")//save only functions
        appl = 0; //we set setting to zero
    else if(arg == "settings")//save only settings
        commands = 0;//we set function to zero
    else //that's name of file or "opt"
        goto jump;
    arg = Parser::selectSpace(str,ind);//file name or "opt"
    jump:
    if(arg == "opt")//"opt" means, that we want to choose file in a new window
        arg = QFileDialog::getSaveFileName(Console::console, "Choose file for reading", "",
                                           "Plotter Function File (*.fpf);;All files(*)");
    QFile file(arg);
    if(!file.open(QIODevice::WriteOnly))
    {
        str= "read commands: cannot open the file. Perhaps it doesn't exist";
        return false;
    }
    QTextStream out(&file);//"save: unknown argument";
    if(appl)//save settings
    {
        out<<"set color -a "+Parser::colorToStr(mw->getColor());
        out<<"\nset color -b "+Parser::colorToStr(backround);
        out<<"\nset color -g "+Parser::colorToStr(gr.colorGrid);
        out<<"\nset color -p "+
             Parser::colorToStr(mw->palette().color(QPalette::Button));
        out<<"\nset color -l "+ Parser::colorToStr(labelsColor);
        out<<"\nset color -c "+ Parser::colorToStr(Console::console->palette().color(QPalette::Base));
        out<<"\nset color -t "+ Parser::colorToStr(Console::console->palette().color(QPalette::Text));
        out<<"\nset mode " + (mw->isMode2d() == true ? QString("2d") : QString("3d"));
        out<<"\nset grid -q " + QString::number(CommonData::gr.gridProp);
        out<<"\nset width " + QString::number(mw->getGrapth2d()->getWidthLine())<<"\n";
    }
    if(commands)//save functions
    {
        QVector<LineButt*>&lbs = mw->getManage()->getItems();
        if(lbs.empty())//nothing to save
             goto goOut;
        auto it = lbs.begin();
        out<<"add "+ (*it)->edit->text();
        ++it;
        for(;it!= lbs.end();++it)
        {
            out<<"\nadd "+(*it)->edit->text();
        }
    }
    goOut:
    file.close();
    str="";
    return true;
}

bool add( QString&str)
{
    PrimeWindow::mainW->getManage()->on_AddF(Parser::midSpace(str));
    str = "";
    return true;
}
bool erase( QString&str)
{
    QVector<LineButt*>&lbs = PrimeWindow::mainW->getManage()->getItems();
    if(lbs.empty()){
        str= "erase: there is no to erase";
        return false;
    }
    QString arg = Parser::selectSpace(str);
    int index = 0;
    if(str == "last")
        index = lbs.size() - 1;
    else if(str != "first") //all other
    {
        bool ok;
        index = arg.toInt(&ok);
        if(!ok){
            str= "erase: invalid index(it must be a number)";
            return false;
        }
        if(index < 0 || index >= lbs.size()){
            str = "erase: index must be larger 0 and lower count functions";
            return false;
        }
    }
    lbs[index]->butt->click();
    str= "";
    return true;
}
bool update(QString&str)
{
    PrimeWindow::mainW->on_Update();
    str = "";
    return true;
}
bool help(QString&str)
{
    QFile file("helpRus");
    if(!file.open(QIODevice::ReadOnly))
    {
        str= "help: cannot open \"helpRus\". Perhaps it doesn't exist. Download it";
        return false;
    }
    QTextStream qts(&file);
    QString st = qts.readAll();
    if(str.size()!=0)
    {
        int i = st.indexOf(str);
        Console::console->printWithCodes(st.mid(i));
    }
    else Console::console->printWithCodes(st);
    str= "";
    return true;
}
}
