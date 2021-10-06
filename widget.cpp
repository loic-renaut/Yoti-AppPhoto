#include "widget.h"
#include "utils.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    // lors du lancement de l'application, on vérifie que le dossier images/images_boites existe bien dans le repertoire actif et on le créé si necessaire
    newDir("images/images_boites");

    //TODO: dl l'image logo yoti ?

    /*
     *
     * structure du programme
     *
     * */
    mainLayout = new QHBoxLayout;

    stackedWidget = new QStackedWidget;

    page_login = new QWidget;
    page_takePicture = new QWidget;
    page_recherche = new QWidget;

    stackedWidget->addWidget(page_login);
    stackedWidget->addWidget(page_takePicture);
    stackedWidget->addWidget(page_recherche);

    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    stackedWidget->setCurrentWidget(page_login);

    // page_login
    page_loginLayout = new QVBoxLayout;

    page_loginGroupBox = new QGroupBox;
    page_loginGroupBox->setTitle("Connexion au compte :");

    page_loginGroupBoxLayout = new QFormLayout;

    page_loginLineEditPseudo = new QLineEdit;
    page_loginLineEditPassword = new QLineEdit;

    page_loginGroupBoxLayout->addRow("Pseudo :", page_loginLineEditPseudo);
    page_loginGroupBoxLayout->addRow("Mot de passe :", page_loginLineEditPassword);

    page_loginLineEditPseudo->setText("root");
    page_loginLineEditPassword->setText("1234");

    page_loginGroupBox->setLayout(page_loginGroupBoxLayout);
    page_loginLayout->addWidget(page_loginGroupBox);

    page_loginDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    page_loginLayout->addWidget(page_loginDialogButtonBox);

    page_login->setLayout(page_loginLayout);

    connect(page_loginDialogButtonBox, &QDialogButtonBox::accepted, this, [&] () {
        /*
         *
         * paramètres de connexion à la bdd
         * quand le programme sera opérationel, changer ces infos par celle du serveur bdd ovh
         *
         * */

        /*
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        //db.setUserName("root");
        db.setUserName(page_loginLineEditPseudo->text());
        //db.setPassword("1234");
        db.setPassword(page_loginLineEditPassword->text());
        db.setDatabaseName("yoti_test");
        */


        // connexion bdd ovh
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("rl239142-001.dbaas.ovh.net");
        db.setPort(35191);
        //db.setUserName("loic");
        db.setUserName(page_loginLineEditPseudo->text());
        //db.setPassword("qZJchZ7T");
        db.setPassword(page_loginLineEditPassword->text());
        db.setDatabaseName("yoti_test");


        if(db.open())
        {
            // si la connexion est reussi on créé les autres pages du stackedwidget et on affiche la page suivante
            stackedWidget->setCurrentWidget(page_takePicture);

            // on recupere l'id du trieur avec le pseudo
            QSqlQuery requeteIdTrieur;

            requeteIdTrieur.exec("SELECT * FROM operateur WHERE username='" + page_loginLineEditPseudo->text() + "';");
            while(requeteIdTrieur.next()) {
                id_operateur = requeteIdTrieur.value("id_operateur").toInt();
            }
            qDebug() << id_operateur;

            /*
             *
             * page_takePicture
             *
             * */
            timer.start();
            timer1s = new QTimer;
            QObject::connect(timer1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
            timer1s->start(1000);

            page_takePictureLayout = new QVBoxLayout;

            page_takePicturePushButtonTakePicture = new QPushButton;
            page_takePicturePushButtonTakePicture->setText("Prendre en photo");
            page_takePictureLayout->addWidget(page_takePicturePushButtonTakePicture);

            connect(page_takePicturePushButtonTakePicture, &QPushButton::clicked, this, [&]() {
                page_takePictureCamera->searchAndLock();
                page_takePictureCameraImageCapture->capture(qApp->applicationDirPath() + "/images/images_boites/tmp.jpg");
                page_takePictureCamera->unlock();
                delay(); // laisse 1 seconde de delais pour l'enregistrement de l'image
                QPixmap pixPicture(qApp->applicationDirPath() + "/images/images_boites/tmp.jpg");
                page_rechercheLabelPicture->setPixmap(pixPicture.scaled(800, 500, Qt::KeepAspectRatio));
                stackedWidget->setCurrentWidget(page_recherche);

                // auto completion du nom des jouets
                QStringList stringListNom;
                if(db.open())
                {
                    QSqlQuery requeteNom;

                    if(requeteNom.exec("SELECT * FROM nom_jouet;"))
                    {
                        while(requeteNom.next())
                        {
                            stringListNom << requeteNom.value("pk_nom").toString();
                        }
                    }

                    db.close();
                }
                else
                {
                    qDebug() << "Erreur connection bdd !";
                }

                QCompleter *completer = new QCompleter(stringListNom, this);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                page_rechercheGroupBoxInformationsComplementairesLineEditNom->setCompleter(completer);
            });

            page_takePictureCameraViewfinder = new QCameraViewfinder;

            if (QCameraInfo::availableCameras().count() > 0)
            {
                page_takePictureCamera = new QCamera;
                //page_takePictureCamera = new QCamera("/dev/video0", this);

                // Par exemple :
                QCameraViewfinderSettings viewfinderSettings;
                viewfinderSettings.setResolution(1280, 960);
                page_takePictureCamera->setViewfinderSettings(viewfinderSettings);

                page_takePictureCamera->setViewfinder(page_takePictureCameraViewfinder);

                page_takePictureCameraImageCapture = new QCameraImageCapture(page_takePictureCamera, this);
                page_takePictureCamera->start();
            }
            else
            {
                qDebug() << "Aucune caméra disponible !";
                QMessageBox::information(this, "Erreur caméra", "Aucune caméra disponible !");
            }

            page_takePictureLayout->addWidget(page_takePictureCameraViewfinder);

            page_takePicture->setLayout(page_takePictureLayout);

            /*
     *
     * page_recherche
     *
     * */
            page_rechercheLayout = new QVBoxLayout;


            page_rechercheLabelPicture = new QLabel;
            page_rechercheLayout->addWidget(page_rechercheLabelPicture);

            page_rechercheGroupBoxInformationsComplementaires = new QGroupBox;
            page_rechercheGroupBoxInformationsComplementaires->setTitle("Informations complémentaires :");

            page_rechercheGroupBoxInformationsComplementairesLayout = new QFormLayout;

            page_rechercheGroupBoxInformationsComplementairesLineEditNom = new QLineEdit;
            page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres = new QLineEdit;
            page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification = new QLineEdit;
            page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresCollecte = new QLineEdit;

            page_rechercheGroupBoxInformationsComplementairesLayout->addRow("Code-barres du jeu :", page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres);
            page_rechercheGroupBoxInformationsComplementairesLayout->addRow("Code-barres identification :", page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification);
            page_rechercheGroupBoxInformationsComplementairesLayout->addRow("Code-barres collecte :", page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresCollecte);
            page_rechercheGroupBoxInformationsComplementairesLayout->addRow("Nom du jeu :", page_rechercheGroupBoxInformationsComplementairesLineEditNom);

            page_rechercheGroupBoxInformationsComplementaires->setLayout(page_rechercheGroupBoxInformationsComplementairesLayout);

            page_rechercheLayout->addWidget(page_rechercheGroupBoxInformationsComplementaires);

            page_rechercheDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

            connect(page_rechercheDialogButtonBox, &QDialogButtonBox::accepted, this, &Widget::page_rechercheAccepted);
            connect(page_rechercheDialogButtonBox, &QDialogButtonBox::rejected, this, [&]() {
                stackedWidget->setCurrentWidget(page_takePicture);
                page_takePictureCamera->start();
            });

            page_rechercheLayout->addWidget(page_rechercheDialogButtonBox);

            page_recherche->setLayout(page_rechercheLayout);
        }
        else
        {
            QMessageBox::information(this, "Erreur login", "Informations de connexion erronées !");
        }
    });
}

Widget::~Widget()
{

}

void Widget::timer_initialisation()
{
    timer.restart();
    timer1s = new QTimer;
    QObject::connect(timer1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer1s->start(1000);
}

void Widget::page_rechercheAccepted()
{
    // recherche dans la bdd par nom de jeu et numero de code-barres
    if(db.open())
    {
        vector<Mat> imagesGrayscale;
        vector<QString> cheminImages;
        vector<int> idVector;

        if(page_rechercheGroupBoxInformationsComplementairesLineEditNom->text()=="")
        {
            // On vérifie si l'image a déjà été enregistré dans la BDD, pour cela requete dans la bdd avec le code-barres du jeu
            QSqlQuery requeteFindCodeBarres;
            QString requeteFindCodeBarresQString = "SELECT * FROM jouet WHERE code_barres_jouet = '" + page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres->text() + "';";

            if(requeteFindCodeBarres.exec(requeteFindCodeBarresQString))
            {
                int k = 0;
                while(requeteFindCodeBarres.next())
                {
                    QSqlQuery requeteFindPhoto;
                    QString requeteFindPhotoQString = "SELECT * FROM photo WHERE fk_id_jouet = " + requeteFindCodeBarres.value("id_jouet").toString() + ";";

                    if(requeteFindPhoto.exec(requeteFindPhotoQString))
                    {
                        while(requeteFindPhoto.next())
                        {
                            QString s_url = requeteFindPhoto.value("url_photo").toString();
                            // download de l'image
                            // TODO: changer le nom d'arrivé de la fonction downloadImage
                            QString s = qApp->applicationDirPath() + "/images/images_boites/tmp_dl_" + QString::number(k) + ".jpg";
                            k++;
                            qDebug() << s;
                            downloadImage(s_url, s);
                            cheminImages.push_back(s_url);
                            idVector.push_back(requeteFindPhoto.value("fk_id_jouet").toInt());
                            Mat imageGrayscale = imread(s.toStdString(), IMREAD_GRAYSCALE);
                            if(imageGrayscale.data != NULL)
                            {
                                imagesGrayscale.push_back(imageGrayscale);
                            }
                            else
                            {
                                qDebug() << "Erreur ouverture image !";
                            }
                        }
                    }
                    else
                    {
                        qDebug() << "Erreur requete bdd !";
                    }
                }
            }
            else
            {
                qDebug() << "Erreur requete bdd !";
            }
        }
        else
        {
            // On vérifie si l'image a déjà été enregistré dans la BDD, pour cela requete dans la bdd avec le nom du jeu
            QSqlQuery requeteFindNom;
            QString requeteFindNomQString = "SELECT * FROM jouet WHERE nom_jouet = '" + page_rechercheGroupBoxInformationsComplementairesLineEditNom->text() + "';";

            if(requeteFindNom.exec(requeteFindNomQString))
            {
                int k = 0;
                while(requeteFindNom.next())
                {
                    QSqlQuery requeteFindPhoto;
                    QString requeteFindPhotoQString = "SELECT * FROM photo WHERE fk_id_jouet = " + requeteFindNom.value("id_jouet").toString() + ";";

                    if(requeteFindPhoto.exec(requeteFindPhotoQString))
                    {
                        while(requeteFindPhoto.next())
                        {
                            QString s_url = requeteFindPhoto.value("url_photo").toString();
                            // download de l'image
                            // TODO: changer le nom d'arrivé de la fonction downloadImage
                            QString s = qApp->applicationDirPath() + "/images/images_boites/tmp_dl_" + QString::number(k) + ".jpg";
                            k++;
                            qDebug() << s;
                            downloadImage(s_url, s);
                            cheminImages.push_back(s_url);
                            idVector.push_back(requeteFindPhoto.value("fk_id_jouet").toInt());
                            Mat imageGrayscale = imread(s.toStdString(), IMREAD_GRAYSCALE);
                            if(imageGrayscale.data != NULL)
                            {
                                imagesGrayscale.push_back(imageGrayscale);
                            }
                            else
                            {
                                qDebug() << "Erreur ouverture image !";
                            }
                        }
                    }
                    else
                    {
                        qDebug() << "Erreur requete bdd !";
                    }
                }
            }
            else
            {
                qDebug() << "Erreur requete bdd !";
            }
        }

        QString s2 = qApp->applicationDirPath() + "/images/images_boites/tmp.jpg";
        Mat imagePhoto = imread(s2.toStdString(), IMREAD_GRAYSCALE);

        int posImageTrouvee = findImage(imagePhoto, imagesGrayscale);

        int id_jouet = 0;

        // si l'image est retrouvée dans la bdd
        if(posImageTrouvee >= 0)
        {
            qDebug() << "image retrouvée !";

            // on récupère l'id jouet
            id_jouet = idVector[posImageTrouvee];


        }
        // si la photo n'est pas retrouvée dans la bdd
        else
        {
            QSqlQuery requete;

            // création dans la table jouet, exemplaire et photo
            if(page_rechercheGroupBoxInformationsComplementairesLineEditNom->text()=="")
            {
                QString requeteInsertJouetQString = "INSERT INTO jouet ( code_barres_jouet ) VALUES ( '" + page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres->text() + "' );";
                qDebug() << requeteInsertJouetQString;
                if(requete.exec(requeteInsertJouetQString))
                {

                }
                else
                {

                }

            }
            else
            {
                QString requeteInsertJouetQString = "INSERT INTO jouet ( nom_jouet ) VALUES ( '" + page_rechercheGroupBoxInformationsComplementairesLineEditNom->text() + "' );";
                qDebug() << requeteInsertJouetQString;
                requete.exec("INSERT INTO nom_jouet ( pk_nom ) VALUES ( '" + page_rechercheGroupBoxInformationsComplementairesLineEditNom->text() + "' );");
                if(requete.exec(requeteInsertJouetQString))
                {

                }
                else
                {

                }
            }

            int lastId = 0;
            QSqlQuery requeteLastId;
            QString requeteLastIdQString = "SELECT LAST_INSERT_ID();";

            requeteLastId.exec(requeteLastIdQString);
            while(requeteLastId.next())
            {
                lastId = requeteLastId.value("LAST_INSERT_ID()").toInt();
            }
            qDebug() << lastId;
            id_jouet = lastId;

            //upload de la photo + requete table photo
            uploadImage(QString::number(lastId));

            QSqlQuery requeteInsertPhoto;
            QString requeteInsertPhotoQString = "INSERT INTO photo ( url_photo, nom_photo, fk_id_jouet ) VALUES ( 'http://yoti-shop.com/img/images_boites/" + QString::number(lastId) + ".jpg', 'image_boite', " + QString::number(lastId) + " );";
            qDebug() << requeteInsertPhotoQString;

            if(requeteInsertPhoto.exec(requeteInsertPhotoQString))
            {

            }
            else
            {

            }

            qDebug() << "image non retrouvée :(";

            // requete table est_categorise
            QSqlQuery requeteInsertEstCategorise;
            QString requeteInsertEstCategoriseQString = "INSERT INTO est_categorise ( fk_id_jouet, fk_id_categorie ) VALUES ( " + QString::number(id_jouet) + ", 1 );";

            if(requeteInsertEstCategorise.exec(requeteInsertEstCategoriseQString))
            {
                while(requeteInsertEstCategorise.next())
                {
                    qDebug() << "ajout réussi";
                }
            }
        }

        // requete table exemplaire
        QSqlQuery requeteInsertElement;
        QString requeteInsertElementQString = "INSERT INTO exemplaire ( code_barres_exemplaire, fk_id_jouet, fk_code_barres_collecte ) VALUES ( '" + page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification->text() + "', " + QString::number(id_jouet) + ", '" + page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresCollecte->text() + "' );";

        if(requeteInsertElement.exec(requeteInsertElementQString))
        {
            while(requeteInsertElement.next())
            {
                qDebug() << "ajout réussi";
            }
        }

        QString sTime = QTime::fromMSecsSinceStartOfDay(timer.elapsed()).toString("hh") +":"+ QTime::fromMSecsSinceStartOfDay(timer.elapsed()).toString("mm") +":"+ QTime::fromMSecsSinceStartOfDay(timer.elapsed()).toString("ss");
        qDebug() << sTime;
        QSqlQuery requeteInsertEstTraite;
        requeteInsertEstTraite.exec("INSERT INTO est_traite ( fk_code_barres_exemplaire, fk_id_operateur, date, temps ) VALUES ( '" + page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification->text() + "', " + QString::number(id_operateur) + ", CURDATE(), '" + sTime + "' );");

        db.close();
    }
    else
    {
        qDebug() << "Erreur connection bdd !";
    }

    // on réinitialise les champs et réitère le programme
    page_rechercheGroupBoxInformationsComplementairesLineEditNom->setText("");
    page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarres->setText("");
    page_rechercheGroupBoxInformationsComplementairesLineEditCodeBarresIdentification->setText("");
    stackedWidget->setCurrentWidget(page_takePicture);
    timer_initialisation();
    //page_takePictureCamera->start();
}
