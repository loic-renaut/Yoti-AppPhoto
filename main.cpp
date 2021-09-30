#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    // permet de mettre l'application en plein écran
    w.showMaximized();
    // permet de modifier le titre de l'application
    w.setWindowTitle("Yoti App Jouets");
    // permet de changer l'icone de l'application, l'image se trouve dans le fichier build
    w.setWindowIcon(QIcon(qApp->applicationDirPath() + "/images/yoti_icon.ico"));

    w.show();

    return a.exec();
}
