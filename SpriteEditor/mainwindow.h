/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reviewed By: Marcus Dao and Slade Lim
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qspinbox.h"
#include <QMainWindow>
#include <model.h>
#include <QColorDialog>
#include <QButtonGroup>
#include <QFileDialog>
#include "ui_mainwindow.h"
#include "QScreen"
#include "QMessageBox"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Model* model, QWidget *parent = nullptr);
    ~MainWindow();

    QCursor eraserCursor;
    QCursor fillCursor;
    QCursor fillAllCursor;
    QCursor eyedropCursor;
    QCursor brushCursor;
    QCursor circleCursor;
    QCursor squareCursor;
    QCursor rectangleCursor;
    QString fileName;
    QColorDialog colorDialog;
signals:
    void saveFile(QString filename);
    void loadFile(QString filename);
    void startNewProject(QString name, Model* model);
    void activeTool(QString toolName);
    void previewPlayed(QSpinBox* frameCount);
    void frameAdded(Model* model);
    void frameChanged(int frameNumber, Model* model);
    void frameDeleted(Model* model);
    void activeMirror(bool active);

private:
    Model* model;
    Ui::MainWindow *ui;
    QPixmap getPixmapFromIcon(QIcon icon);
    void displayCurrentColor(QColor color);
    void setFrameNumber(int frameNumber);
    void untoggleActive(QAction* currToggle);
    void toggleCursor(QCursor cursor, bool toggled);
    void addFrame();
    void changeFrame(int num);
    void deleteFrame();
    QString previousTool = "brush";

private slots:
    void actionEraserToggled(bool toggled);
    void actionBrushToggled(bool toggled);
    void actionFillAllToggled(bool toggled);
    void actionFillToggled(bool toggled);
    void actionEyedropToolToggled(bool toggled);
    void actionColorPickerToggled(bool toggled);
    void actionReadMeTriggered();
    void actionNewTriggered();
    void actionSaveTriggered();
    void actionOpenTriggered();
    void onFpsSpinBoxValueChanged(int value);
    void onFpsSliderValueChanged(int value);
    void actionCircleTriggered(bool toggled);
    void actionSquareTriggered(bool toggled);
    void actionRectangleTriggered(bool toggled);
    void actionShapesTriggered(bool triggered);
    void onHelpButtonPressed();
    void onButtonNewOk();
    void onButtonNewCancel();
    void onPreviewStart();
    void setPreviewFrame(QPixmap frame);
    void displayOpenImageSizeError();
};
#endif // MAINWINDOW_H
