#ifndef CONSOL_H
#define CONSOL_H
#include <QPlainTextEdit>
#include <map>
#include <deque>
#include <vector>

struct parsAndArgs
{
    std::vector<QChar>pars;
    QString arg;
    parsAndArgs(){}
    parsAndArgs(const std::vector<QChar>&ps, const QString&as):
        pars(ps), arg(as){}
};

typedef QMap<QString, bool(*)( QString&)> MapPars;
typedef QMap<QString, bool(*)( QString&)> MapPars2;
typedef std::pair<QString, bool(*)( QString&)>PairPars2;
typedef std::pair<QString, bool(*)( QString&)> PairPars;
typedef std::vector<QString> vecstr;
class Function_set
{
public:
    static bool color(QString&str);
    static bool mode(QString&str);
    static bool grid(QString&str);
    static bool width(QString&str);
    static MapPars variables;
};
class Function_get
{
public:
    static bool color(QString&str);
    static bool mode(QString&str);
    static bool grid(QString&str);
    static bool width(QString&str);
    static MapPars2 variables;
};

namespace forParser {
bool set(QString&str);
bool get(QString&str);
bool read(QString&str);
bool save(QString&str);
bool add(QString&str);
bool erase(QString&str);
bool update(QString&str);
bool help(QString&str);
}
class Parser
{
public:
    static QColor strToColor(const QString&str);
    static QString colorToStr(const QColor&c);
    static MapPars funcs;
    //pass spaces from pos and do QString mid. for ex: "  mode - 3d"
    //from 0 we pass two spaces and cut right str, i.e. we have after it "mode - 3d"
    //from 3 we have "ode - 3d"
    //from 6 we have "- 3d"
    static QString midSpace(const QString&, int pos = 0);
    //we select str, that's between spaces or space and '\n'. for ex: "  mode - 3d"
    //we have from 0 "mode", but from 6 we have "-"
    static QString selectSpace(const QString&, int pos = 0, int*endS = nullptr);
    static void getParametres(const QString&,
                              std::string&params, int*ind); //parametres
    static void getParsAndArgs(const QString&, parsAndArgs&paa);
    static void getRL(const QString&str,QString&r, QString&l);
};
class PrimeWindow;
class Console: public QPlainTextEdit
{
protected:
    void insertPrompt(bool newBlock);
    QString prompt;
    void onEnter();
    //history
    std::deque<QString>history;
    uint maxHistSize;
    std::deque<QString>::iterator histPos;
    void histAdd(const QString&str);
    void histBack();
    void histForw();
public:
    static Console*console;
    Console(const QString&prompt = "[user]:~$ ");
    void print(const QString&str, QColor c);
    void print(const QString&str);
    void printWithCodes(QString str);//
    //print with different codes
    //that set color text and etc
    void keyPressEvent(QKeyEvent *)override;
    void mousePressEvent(QMouseEvent *)override{setFocus();}
    void setColor(QPalette::ColorRole role,QColor c);
    bool commandEntered(QString&str);
};

#endif // CONSOL_H
