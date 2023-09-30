/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reviewed By: Alex Elbel and Andrew Bergentahl
 */

#include "frameeditor.h"
#include "qgraphicssceneevent.h"

//!
//! \brief FrameEditor::FrameEditor Sets up the UI and the default instance variables
//! \param parent The parent widget
//!
FrameEditor::FrameEditor(QWidget *parent) : QWidget(parent), ui(new Ui::frameEditor) {
    ui->setupUi(this);
    QCoreApplication::instance()->installEventFilter(this);

    // Set up the default color, frame size and selected tool.
    currentColor = Qt::black;
    sizeValue = 32;
    selectedTool = "brush";
    mirror = false;
}

//!
//! \brief FrameEditor::setupNewFrame Handles resetting all the curent objects, updating the UI, and updating the object vectors
//! \param size The size of the new frame
//! \param model The model object so the objects can be added to the models vectors
//!
void FrameEditor::setupNewFrame(int size, Model* model) {
    // Set up how the frame looks on screen for the user
    scene = new QGraphicsScene;
    ui->graphicsView->resize(512, 512);
    ui->graphicsView->setSceneRect(0, 0, 512, 512);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setScene(scene);

    updateGridTile(size);

    scene->setSceneRect(ui->graphicsView->rect());

    // Initializes the current map and adds it to the map vector
    currentMap = new QPixmap(sizeValue, sizeValue);
    currentMap->fill(Qt::transparent);
    model->maps.push_back(currentMap);

    // Initializes the current painter and adds it to the painter vector
    currentPainter = new QPainter(currentMap);
    model->painters.push_back(currentPainter);

    // Initializes the current item and adds it to the item vector
    currentItem = new QGraphicsPixmapItem(scaledMap());
    scene->addItem(currentItem);
    model->items.push_back(currentItem);
}

//!
//! \brief FrameEditor::newFrame Creates a new frame and tells the view to change the displayed frame number
//! \param model The model object to get the number of frames from
//!
void FrameEditor::newFrame(Model* model) {
    setupNewFrame(sizeValue, model);
    emit changeFrameNumber(model->maps.size());
}

//!
//! \brief FrameEditor::deleteCurrentFrame Deletes the current frame from the model and all objects associated with it
//! \param model The model to delete the objects from
//!
void FrameEditor::deleteCurrentFrame(Model* model) {
    int frameIndex = std::find(model->maps.begin(), model->maps.end(), currentMap) - model->maps.begin();

    // Delete the frame from the backing vector, painter, and items
    model->maps.erase(model->maps.begin() + frameIndex);
    model->items.erase(model->items.begin() + frameIndex);
    model->painters.erase(model->painters.begin() + frameIndex);

    // If there are still existing frames change to the next one else add a new default frame
    if(model->maps.size() > 0) changeCurrentFrame(frameIndex + 1, model);
    else newFrame(model);
}

//!
//! \brief FrameEditor::changeCurrentFrame Changes the currently selected frame to the specified frame number
//! \param frameNumber The number of frame to change to
//! \param model The model object to update the frame objects in
//!
void FrameEditor::changeCurrentFrame(int frameNumber, Model* model) {
    scene->removeItem(currentItem);

    // Checks that the chosen frame is an existing frame.
    if((int) model->maps.size() > frameNumber - 1) {
        sizeValue = model->size;
        currentMap = model->maps[frameNumber - 1];

        // Make sure the grid tile background is scaled
        updateGridTile(sizeValue);

        // Make sure the pixmap is scaled
        model->items[frameNumber - 1]->setPixmap(scaledMap());
        currentItem = model->items[frameNumber - 1];
        currentPainter = model->painters[frameNumber - 1];
        scene->addItem(currentItem);

        emit changeFrameNumber(frameNumber);
    }
    // The user tried to change the frame to a non-existent frame so display the last frame
    else {
        currentItem = model->items[model->items.size() - 1];
        currentMap = model->maps[model->maps.size() - 1];
        currentPainter = model->painters[frameNumber - 1];

        emit changeFrameNumber(model->maps.size());
    }
}

//!
//! \brief FrameEditor::startNewProject Removes the frame objects from the model and creates a new default frame with the specified size
//! \param size The new project size
//! \param model The model that holds the frames objects
//!
void FrameEditor::startNewProject(QString size, Model* model) {
    // Parses the QString into an int size value
    sizeValue = size.mid(0, size.indexOf(" ")).toInt();

    // Starts a new project by clearing backing items, maps, painter, and resetting size
    model->items.clear();
    model->maps.clear();
    model->painters.clear();
    model->size = sizeValue;
    setupNewFrame(sizeValue, model);

    emit changeFrameNumber(1);
}

//!
//! \brief FrameEditor::setColor Updates the current color
//! \param color The new current color
//!
void FrameEditor::setColor(const QColor &color) {
    currentColor = qRgb(color.red(), color.green(), color.blue());
}

//!
//! \brief FrameEditor::activeTool Updates the active tool
//! \param toolName The name of the currently selected tool
//!
void FrameEditor::activeTool(QString toolName) {
    selectedTool = toolName;
}

//!
//! \brief FrameEditor::activeMirror Sets the mirror tool to true
//! \param active Whether the mirror is active or not
//!
void FrameEditor::activeMirror(bool active) {
    mirror = active;
}

//!
//! \brief FrameEditor::eventFilter Handles all mouseevents on the frame editor
//! \param obj Object that activated event
//! \param event Type of mouse event
//! \return boolean If mouse is moving return true
//!
bool FrameEditor::eventFilter(QObject *obj, QEvent *event) {
    QPointF point = this->mapFromGlobal(QCursor::pos());

    // Convert from 512 by 512 to user-sepecified/default QPixmap size
    QPointF scaledPoint(point.x() * sizeValue / 512, point.y() * sizeValue / 512);

    // If the point is not in the frame editor, ignore the mouse event
    if (scaledPoint.x() < 0 || scaledPoint.y() < 0 || scaledPoint.x() > sizeValue || scaledPoint.y() > sizeValue) {
        return false;
    }

    // Mouse is pressed and is not being held
    if(event->type() == QGraphicsSceneMouseEvent::MouseButtonPress && !mouseHeld){
        handlePaintAction(scaledPoint);
    }

    // When mouse is moving, draw over all
    if(event->type() == QGraphicsSceneMouseEvent::MouseMove){
        if(mouseHeld) {
            lastPoint = &point;
            handlePaintAction(scaledPoint);
        }
        return true;

    // Mouse is first pressed and held event becomes true
    } else if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
        if(mouseEvent->button() == Qt::LeftButton)
            mouseHeld = true;

    // If mouse is released, held is set to false
    } else if(event->type() == QEvent::MouseButtonRelease){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
        if(mouseEvent->button() == Qt::LeftButton)
            mouseHeld = false;
    }

    return false;
}

//!
//! \brief FrameEditor::handlePaintAction Handles all tool painting
//! \param scaledPoint Point at which to paint on the frame editor
//!
void FrameEditor::handlePaintAction(QPointF scaledPoint) {
    int sizeScalar = sizeValue / 16;
    QPointF inversePoint(sizeValue - scaledPoint.x(), scaledPoint.y());

    if(selectedTool == "brush") {
        // Draws on the scaledPoint pixel, if mirrored, draws on the inverse pixel as well
        if(mirror){
            fillPixel(currentColor, scaledPoint);
            fillPixel(currentColor, inversePoint);
        } else {
            fillPixel(currentColor, scaledPoint);
        }
    } else if(selectedTool == "erasor") {
        // Erases on the scaledPoint pixel, if mirrored, erases on the inverse pixel as well
        if(mirror) {
            fillPixel(Qt::transparent, scaledPoint);
            fillPixel(Qt::transparent, inversePoint);
        } else {
            fillPixel(Qt::transparent, scaledPoint);
        }
    } else if(selectedTool == "fill" && !mouseHeld) {
        // Gets the color to fill, and calls recursive fill method
        QColor fillColor = currentMap->toImage().pixelColor(scaledPoint.x(), scaledPoint.y());
        if(!(fillColor == currentColor)) {
            fillDriver(fillColor, scaledPoint);
        }
    } else if(selectedTool == "fillAll" && !mouseHeld) {
        // Gets the color to fill, and calls recursive fill all method
        QColor fillColor = currentMap->toImage().pixelColor(scaledPoint.x(), scaledPoint.y());
        fillAllDriver(fillColor);
    } else if(selectedTool == "eyedrop" && !mouseHeld) {
        // Gets the color of the pixel, and sets the current color to that color
        currentColor = QColor(currentMap->toImage().pixel(scaledPoint.x(), scaledPoint.y()));
        emit changeCurrentColor(currentColor);
    } else if(selectedTool == "rectangle") {
        // Draws a rectangle using fillShapeSize method, and if mirrored, and inverse rectangle
        if(mirror){
            fillShapeSize(scaledPoint, QSize(2 * sizeScalar, sizeScalar));
            fillShapeSize(inversePoint, QSize(2 * sizeScalar, sizeScalar));
        } else {
            fillShapeSize(scaledPoint, QSize(2 * sizeScalar, sizeScalar));
        }
    } else if(selectedTool == "circle") {
        int sizeOffset = sizeScalar == 1 ? 0 : sizeScalar / 2;
        int edgesOffset = sizeScalar == 1 ? 1 : sizeScalar;

        if(mirror){
            // Fills the initial square
            fillShapeSize(scaledPoint, QSize(2 * edgesOffset, 2 * edgesOffset));

            // Fills the edges of the square to give the circle a "rounded" look
            fillShapeSize(QPointF(scaledPoint.x() - 3 * edgesOffset + sizeOffset, scaledPoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(scaledPoint.x() + 3 * edgesOffset - sizeOffset, scaledPoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(scaledPoint.x(), scaledPoint.y() - 3 * edgesOffset + sizeOffset), QSize(edgesOffset, sizeOffset));
            fillShapeSize(QPointF(scaledPoint.x(), scaledPoint.y() + 3 * edgesOffset - sizeOffset), QSize(edgesOffset, sizeOffset));

            // Fills the initial square
            fillShapeSize(inversePoint, QSize(2 * edgesOffset, 2 * edgesOffset));

            // Fills the edges of the square to give the circle a "rounded" look
            fillShapeSize(QPointF(inversePoint.x() - 3 * edgesOffset + sizeOffset, inversePoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(inversePoint.x() + 3 * edgesOffset - sizeOffset, inversePoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(inversePoint.x(), inversePoint.y() - 3 * edgesOffset + sizeOffset), QSize(edgesOffset, sizeOffset));
            fillShapeSize(QPointF(inversePoint.x(), inversePoint.y() + 3 * edgesOffset - sizeOffset), QSize(edgesOffset, sizeOffset));
        } else {
            // Fills the initial square
            fillShapeSize(scaledPoint, QSize(2 * edgesOffset, 2 * edgesOffset));

            // Fills the edges of the square to give the circle a "rounded" look
            fillShapeSize(QPointF(scaledPoint.x() - 3 * edgesOffset + sizeOffset, scaledPoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(scaledPoint.x() + 3 * edgesOffset - sizeOffset, scaledPoint.y()), QSize(sizeOffset, edgesOffset));
            fillShapeSize(QPointF(scaledPoint.x(), scaledPoint.y() - 3 * edgesOffset + sizeOffset), QSize(edgesOffset, sizeOffset));
            fillShapeSize(QPointF(scaledPoint.x(), scaledPoint.y() + 3 * edgesOffset - sizeOffset), QSize(edgesOffset, sizeOffset));
        }
    } else if(selectedTool == "square") {
        // Draws a square using fillShapeSize, and if mirrored, and inverse square
        if(mirror){
            fillShapeSize(scaledPoint, QSize(sizeScalar, sizeScalar));
            fillShapeSize(inversePoint, QSize(sizeScalar, sizeScalar));
        } else {
            fillShapeSize(scaledPoint, QSize(sizeScalar, sizeScalar));
        }
    }
}

//!
//! \brief FrameEditor::fillDriver Recursive driver method to fill
//! \param fillColor Color to replace or fill
//! \param point Point at which to fill
//!
void FrameEditor::fillDriver(QColor fillColor, QPointF point) {
    // The color at the point you want to fill
    QColor pixelColor = currentMap->toImage().pixelColor(point.x(), point.y());

    // If the color is the color you want to fill, fills recursively up, right, down, and left
    if (pixelColor == fillColor) {
        fillPixel(currentColor, point);
        QPointF right(point.x() + 1, point.y());
        QPointF down(point.x(), point.y() - 1);
        QPointF left(point.x() - 1, point.y());
        QPointF up(point.x(), point.y() + 1);
        fillDriver(fillColor, right);
        fillDriver(fillColor, down);
        fillDriver(fillColor, left);
        fillDriver(fillColor, up);
    } else {
        return;
    }
}

//!
//! \brief FrameEditor::fillShapeSize Fills a shape with a specified size
//! \param centerPoint The center point to start drawing from
//! \param size The size of the shape
//!
void FrameEditor::fillShapeSize(QPointF centerPoint, QSize size) {
    // Fills a shape starting from a center point outwards
    for(int w = -size.width(); w <= size.width(); w++) {
        for(int h = -size.height(); h <= size.height(); h++) {
            fillPixel(currentColor, QPointF(centerPoint.x() + w, centerPoint.y() + h));
        }
    }
}

//!
//! \brief FrameEditor::fillAllDriver Fills all instances of a color in a grid to a new color
//! \param fillColor Color to replace old color with
//!
void FrameEditor::fillAllDriver(QColor fillColor) {
    // Loops through the entire grid and fills the color you want to fill
    for(int row = 0; row < currentMap->height(); ++row) {
        for(int col = 0; col < currentMap->width(); ++col) {
            QColor pixelColor = currentMap->toImage().pixelColor(col, row);
            if (pixelColor == fillColor) {
                QPoint point(col, row);
                fillPixel(currentColor, point);
            }
        }
    }
}

//!
//! \brief FrameEditor::fillPixel Fills one pixel in the frame
//! \param color The color to fill the pixel
//! \param point The position of the pixel
//!
void FrameEditor::fillPixel(QColor color, QPointF point) {
    QRect rect(point.x(), point.y(), 1, 1);

    // If erasor is active, erases the pixel color, else draws the pixel with a color
    if(color == Qt::transparent && selectedTool == "erasor"){
        currentPainter->setCompositionMode(QPainter::CompositionMode_Clear);
        currentPainter->eraseRect(rect);
        currentPainter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    } else {
        currentPainter->fillRect(rect, color);
    }
    currentItem->setPixmap(scaledMap());
}

//!
//! \brief FrameEditor::scaledMap Calculates the scaled map of the current pixmap and then displays it
//! \return return the resized pixmap
//!
QPixmap FrameEditor::scaledMap() {
    // Get the graphicsView size
    QRectF viewRect = ui->graphicsView->sceneRect();

    // Get the inputted pixmap size
    QRectF pixmapRect(0, 0, currentMap->width(), currentMap->height());
    pixmapRect = pixmapRect.translated(viewRect.center() - pixmapRect.center());

    // Perform the scaling, centers the pixmap, and then scales it to graphicsView size
    QTransform transform;
    transform.translate(pixmapRect.center().x(), pixmapRect.center().y());
    transform.scale(viewRect.width() / pixmapRect.width(), viewRect.height() / pixmapRect.height());
    transform.translate(-pixmapRect.center().x(), -pixmapRect.center().y());

    // Apply the transformation and return the resulting pixmap
    return currentMap->transformed(transform);
}

//!
//! \brief FrameEditor::updateGridTile Sets the size of the background grid depending on the image size
//! \param size The image size
//!
void FrameEditor::updateGridTile(int size) {
    switch (size){
        case 16:
            ui->graphicsView->setBackgroundBrush(QImage(":/images/Images/gridTile.jpg").scaled(128,128));
            break;
        case 32:
            ui->graphicsView->setBackgroundBrush(QImage(":/images/Images/gridTile.jpg").scaled(64,64));
            break;
        case 64:
            ui->graphicsView->setBackgroundBrush(QImage(":/images/Images/gridTile.jpg").scaledToHeight(32).scaledToWidth(32));
            break;
        case 128:
            ui->graphicsView->setBackgroundBrush(QImage(":/images/Images/gridTile.jpg").scaledToHeight(16).scaledToWidth(16));
            break;
        case 256:
            ui->graphicsView->setBackgroundBrush(QImage(":/images/Images/gridTile.jpg").scaledToHeight(8).scaledToWidth(8));
            break;
    }
}

//!
//! \brief FrameEditor::~FrameEditor Deconstructs FrameEditor
//!
FrameEditor::~FrameEditor() {
    delete ui;
}
