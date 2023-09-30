/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reveiwed By: Gunnar Hovik and Alex Elbel
 */

#ifndef MODEL_H
#define MODEL_H

#include "qgraphicsitem.h"
#include "qspinbox.h"
#include <QObject>
#include <qpixmap.h>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>
#include <iostream>
#include <vector>

using std::vector;

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = nullptr);
    ~Model();

    vector<QPixmap *> maps;
    vector<QGraphicsPixmapItem *> items;
    vector<QPainter *> painters;
    int size;
    void saveFile(QString filename);
    void loadFile(QString filename);

signals:
    void setPreviewFrame(QPixmap frame);
    void loadImageError();
    void loadFrame(int pos);

public slots:
    void playPreview(QSpinBox* frameCount);
    void toggleLoop(bool toggle);

private:
    bool previewLooping;
};

#endif // MODEL_H
