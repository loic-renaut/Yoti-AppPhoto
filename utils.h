#ifndef UTILS_H
#define UTILS_H

void newDir(QString dirPath);

void uploadImage(QString id_photo);

void downloadImage(QString url_image, QString chemin_image);

int findImage(Mat img, vector<Mat> images);

void delay();

QString guillemets(QString s);

#endif // UTILS_H
