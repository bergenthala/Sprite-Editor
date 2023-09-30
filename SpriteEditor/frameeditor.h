/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reviewed By: Alex Elbel and Andrew Bergentahl
 */

#ifndef FRAMEEDITOR_H
#define FRAMEEDITOR_H

#include <QWidget>
#include <QMouseEvent>
#include <vector>
#include <QtGui>
#include <QLabel>
#include <model.h>
#include "qgraphicsitem.h"
#include "qgraphicsitem.h"
#include "ui_frameeditor.h"

namespace Ui {
class frameEditor;
}

class FrameEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FrameEditor(QWidget *parent = nullptr);
    ~FrameEditor();

    void newFrame(Model* model);
    void changeCurrentFrame(int frameNumber, Model* model);
    void setColor(const QColor &color);
    void startNewProject(QString size, Model* model);
    void activeTool(QString activeTool);
    void activeMirror(bool active);
    void deleteCurrentFrame(Model* model);
    void setupNewFrame(int size, Model* model);
    QString selectedTool;

private:
    int sizeValue;
    bool mirror;
    QPixmap *currentMap;
    QGraphicsPixmapItem *currentItem;
    QPainter *currentPainter;
    QGraphicsScene *scene;
    QColor currentColor;
    Ui::frameEditor *ui;
    void fillPixel(QColor color, QPointF point);
    void fillShapeSize(QPointF centerPoint, QSize size);
    void fillDriver(QColor color, QPointF point);
    void fillAllDriver(QColor color);
    void handlePaintAction(QPointF point);
    void updateGridTile(int size);
    QPixmap scaledMap();

signals:
    void changeFrameNumber(int frameNumber);
    void changeCurrentColor(QColor color);

protected:
    bool mouseHeld = false;
    QPointF* lastPoint = nullptr;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // FRAMEEDITOR_H
