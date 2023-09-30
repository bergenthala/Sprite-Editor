/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reviewed By: Gunnar Hovik and Alex Elbel
 */

#include "model.h"

//!
//! \brief Model::Model Constructor
//! \param parent The parent object
//!
Model::Model(QObject *parent) : QObject{parent} {
    size = 32;
}

//!
//! \brief Model::~Model Destructor
//!
Model::~Model(){
}

//!
//! \brief Model::LoadFile Opens a JSON sprite file and parses information into the Model class and its members
//! \param filename The file to be opened (includes path)
//!
void Model::loadFile(QString filename) {
    // Open file
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        // Get all contents
        QString contents = file.readAll();
        file.close();

        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(contents.toUtf8(), &error);

        QJsonObject jsonObject = document.object();

        // Check for any errors
        if (document["height"].toInt() != document["width"].toInt() || !jsonObject.contains("height") || !jsonObject.contains("width") || !jsonObject.contains("numberOfFrames") || !jsonObject.contains("frames") || error.error != QJsonParseError::NoError) {
            emit loadImageError();
            return;
        }

        // Prepare for new sprite
        maps.clear();
        items.clear();
        painters.clear();

        // Extract information
        size = document["height"].toInt();

        // Get frame data
        QJsonObject frames = document["frames"].toObject();
        for (int i = 0; i < document["numberOfFrames"].toInt(); i++) {
            // Obtain the frame array
            QJsonArray currFrame = frames["frame" + QString::number(i)].toArray();

            QImage image(size, size, QImage::Format_RGBA8888);

            // Loop through columns
            for (int y = 0; y < size; y++) {
                QJsonArray row = currFrame[y].toArray();

                // Loop through rows
                for (int x = 0; x < size; x++) {
                    QJsonArray pixels = row[x].toArray();
                    int r = pixels[0].toInt();
                    int g = pixels[1].toInt();
                    int b = pixels[2].toInt();
                    int a = pixels[3].toInt();
                    image.setPixelColor(x, y, QColor(r, g, b, a));
                }
            }

            // Add parsed information
            QPixmap *frame = new QPixmap(size, size);
            *frame = QPixmap::fromImage(image);
            maps.push_back(frame);
            QPainter *painter = new QPainter(frame);
            painters.push_back(painter);
            QGraphicsPixmapItem *item = new QGraphicsPixmapItem(*frame);
            items.push_back(item);
        }
    }

    // Signal to display the first frame of the sprite
    emit loadFrame(1);
}

//!
//! \brief Model::saveFile Saves the sprite using JSON format
//! \param filename The file to be opened (includes path)
//!
void Model::saveFile(QString filename) {
    // Create json object to store data
    QJsonObject json;

    // Add parameters
    json["height"] = size;
    json["width"] = size;
    json["numberOfFrames"] = (int)maps.size();

    // Add frames
    QJsonObject frames;
    // Loop through each frame in maps
    for (int i = 0; i < (int)maps.size(); i++) {
        QJsonArray currframe;

        // Loop through each column in the image
        for (int y = 0; y < size; y++) {
            QJsonArray row;
            // Loop through each row
            for (int x = 0; x < size; x ++) {
                // Convert QPixmap to a QImage to get QColor
                QColor color = maps[i]->toImage().pixelColor(x, y);
                int r = color.red();
                int g = color.green();
                int b = color.blue();
                int a = color.alpha();

                // Store QColor of current pixel in row
                QJsonArray rgba = { r, g, b, a };
                row.append(rgba);
            }
            // Add row to the frame
            currframe.append(row);
        }
        // Add frame to the array
        frames["frame" + QString::number(i)] = currframe;
    }
    // Add all frames to the json object
    json["frames"] = frames;

    // Write to file
    QJsonDocument document(json);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(document.toJson());
        file.close();
    }
}

//!
//! \brief Model::toggleLoop Toggles the loop feature on or off
//! \param toggle Boolean to either toggle loop on or off
//!
void Model::toggleLoop(bool toggle){
    previewLooping = toggle;
}

//!
//! \brief Model::playPreview A recursive that plays each frame the user created in an animation format
//! \param fpsStorage The object that stores the fps
//!
void Model::playPreview(QSpinBox* fpsStorage){
    // Obtain fps value from QSpinBox
    int fps = fpsStorage->value();

    // Loop through each frame in maps and display it
    for(int i = 0; i < (int)maps.size(); i++){
        QTimer::singleShot((i * (1000/fps)) , this, [=](){emit setPreviewFrame(*maps[i]);});
    }
    QPixmap empty;

    if(previewLooping)
        QTimer::singleShot((maps.size() * (1000/fps)) , this, [=](){playPreview(fpsStorage);});
    else
        QTimer::singleShot((maps.size() * (1000/fps)) , this, [=](){emit setPreviewFrame(empty);});
}
