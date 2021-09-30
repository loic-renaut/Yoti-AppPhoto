#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QtMultimediaWidgets>
#include <QtSql>
#include <QSqlDatabase>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void page_rechercheAccepted();

    void timer_initialisation();

private:
    // variables globales
    QTimer* timer1s;
    QElapsedTimer timer;
    int id_operateur = -1;

    // bdd
    QSqlDatabase db;

    // structure generale du programme
    QHBoxLayout* mainLayout;
    QStackedWidget* stackedWidget;

    // page_login
    QWidget* page_login;
    QVBoxLayout* page_loginLayout;

    QGroupBox* page_loginGroupBox;
    QFormLayout* page_loginGroupBoxLayout;

    QLineEdit* page_loginLineEditPseudo;
    QLineEdit* page_loginLineEditPassword;

    QDialogButtonBox* page_loginDialogButtonBox;

    // page_takePicture
    QWidget* page_takePicture;
    QVBoxLayout* page_takePictureLayout;
    QPushButton* page_takePicturePushButtonTakePicture;
    QCamera* page_takePictureCamera;
    QCameraViewfinder* page_takePictureCameraViewfinder;
    QCameraImageCapture* page_takePictureCameraImageCapture;

    // page_recherche
    QWidget* page_recherche;
    QVBoxLayout* page_rechercheLayout;
    QLabel* page_rechercheLabelPicture;
    QGroupBox* page_rechercheGroupBoxInformationsComplementaires;
    QFormLayout* page_rechercheGroupBoxInformationsComplementairesLayout;
    QLineEdit* page_rechercheGroupBoxInformationsComplementairesLineEditNom;
    QLineEdit* page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres;
    QLineEdit* page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification;
    QLineEdit* page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresCollecte;
    QDialogButtonBox* page_rechercheDialogButtonBox;
};

#endif // WIDGET_H
