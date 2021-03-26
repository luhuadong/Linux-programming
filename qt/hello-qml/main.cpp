#include <QGuiApplication>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQuickView view;
    view.setSource(QUrl::fromLocalFile("application.qml"));
    view.show();
    
    return app.exec();
}
