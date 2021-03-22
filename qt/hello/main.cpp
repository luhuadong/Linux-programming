/*
 * qmake -project QT+=widgets
 * qmake
 * make
 *
 */

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if 0
    QDialog w;
    w.resize(800, 600);

    QLabel label(&w);
    label.setText(QObject::tr("Hello, World!"));
    
    w.show();
#else
    QDialog w;
    w.resize(800, 600);

    QPushButton btn("Hello, World!", &w);
    btn.resize(100, 40);

    w.show();
#endif

    return a.exec();
}
