#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QDebug>
#include <QTime>
#include <QCoreApplication>
#include "opencv2/opencv.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <QDir>

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

void newDir(QString dirPath)
{
    QDir dir(dirPath);
    QDir dir2;
    if(!dir.exists())
    {
        qDebug() << "création du répertoire " << dirPath;
        dir2.mkpath(dirPath);
    }
}

/*
 *
 * Fonction qui permet d'uploader une image image sur le serveur 1&1 ionos
 *
 * */
void uploadImage(QString id_photo)
{
    CURL *curl;
    CURLcode res;
    struct stat file_info;
    curl_off_t speed_upload, total_time;
    FILE *fd;

    QString s_chemin_save_image = qApp->applicationDirPath() + "/images/images_boites/tmp.jpg";
    fd = fopen(s_chemin_save_image.toStdString().c_str(), "rb"); /* open file to upload */
    if(!fd){
        qDebug() << "erreur open file"; /* can't continue */
    }

    /* to get the file size */
    if(fstat(fileno(fd), &file_info) != 0) {
        qDebug() << "erreur file size"; /* can't continue */
    }

    curl = curl_easy_init();
    if(curl) {
        /* upload to this place */
        QString s_url = "sftp://access870637260.webspace-data.io/img/images_boites/" + id_photo + ".jpg";
        curl_easy_setopt(curl, CURLOPT_URL, s_url.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, "u104675865");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "qZJchZ7T");
        //curl_easy_setopt(curl, CURLOPT_PORT, 22);

        /* tell it to "upload" to the URL */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* set where to read from (on Windows you need to use READFUNCTION too) */
        curl_easy_setopt(curl, CURLOPT_READDATA, fd);

        /* and give the size of the upload (optional) */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t)file_info.st_size);

        /* enable verbose for easier tracing */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            /* now extract transfer info */
            curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

            fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec during %"
                    CURL_FORMAT_CURL_OFF_T ".%06ld seconds\n",
                    speed_upload,
                    (total_time / 1000000), (long)(total_time % 1000000));

        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    fclose(fd);
}

/*
 *
 * Fonction qui permet de télécharger une image à partir d'une url donnée en premier paramètre. L'image est sauvegardée à l'emplacement donné en second paramètre.
 *
 * */
void downloadImage(QString url_image, QString chemin_image)
{
    string url = url_image.toUtf8().constData();
    string fileName = chemin_image.toStdString();
    char* char_fileName;
    char_fileName = &fileName[0];
    char* char_url;
    char_url = &url[0];
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();

    if (curl)
    {
        fp = fopen(char_fileName,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, char_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        const char *str = curl_easy_strerror(res);
        qDebug() << "libcurl said" << str << endl;
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

/*
 *
 * Fonction qui permet de trouver une correspondance eventuelle entre l'image donnée en premier paramètre et les images données en second paramètre.
 * Renvoi la position dans le vector si une correspondance est trouvée, sinon renvoi -1.
 *
 * */
int findImage(Mat img, vector<Mat> images)
{
    Mat img1 = img.clone();
    int minHessian = 400;
    Ptr<SURF> detector = SURF::create(minHessian);
    vector<KeyPoint> keypoints1;
    Mat descriptors1;

    detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);

    for(unsigned int k = 0; k < images.size(); k++)
    {
        Mat img2 = images[k].clone();
        int goodM = 0;
        vector<KeyPoint> keypoints2;
        Mat descriptors2;
        detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);
        Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
        vector<vector<DMatch>> knn_matches;
        matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );
        const float ratio_thresh = 0.7f;
        vector<DMatch> good_matches;
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
                goodM++;
            }
        }

        // TODO: paramètre a ajuster eventuellement
        //if(goodM >= 300)
        if(goodM >= 200) // TODO: ceci est un test
        {
            return k;
        }
    }

    return -1;
}

/*
 *
 * Fonction qui permet de mettre une demie seconde de delais lors de son l'appel.
 * Sa principale utilité est de laisser le temps à une image de s'enregistrer sur l'ordinateur quand une photo est prise par la webcam.
 * Sans ça, il y a des problèmes pour le display immédiat de l'image dans la gui.
 *
 * */
void delay()
{
    QTime dieTime = QTime::currentTime().addMSecs(500);

    while(QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

QString guillemets(QString s)
{
    return "\"" + s + "\"";
}
