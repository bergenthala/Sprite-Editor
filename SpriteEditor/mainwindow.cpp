/*
 * A7 Sprite Editor
 * Written By: Andrew Bergenthal, Gunnar Hovik, Slade Lim, Marcus Dao, Alex Elbel
 * Date: April 6, 2023
 * Code Reviewed By: Marcus Dao and Slade Lim
 */

#include "mainwindow.h"

//!
//! \brief MainWindow::MainWindow - Constructor for main window,
//!         setups up all connections to Model
//! \param model - Pointer to the model
//! \param parent - Required by QT, unused
//!
MainWindow::MainWindow(Model* model, QWidget *parent)
    : QMainWindow(parent)
    , model(model)
    , ui(new Ui::MainWindow)
{
    // Set the size of the screen to 70% of the user's Screen
    resize(QGuiApplication::primaryScreen()->availableGeometry().size() * 0.7);
    ui->setupUi(this);

    // Set the style of the toolbar's
    ui->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    ui->secondaryToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon );
    ui->secondaryToolBar->setVisible(false);

    // We want to start with a frame ready to go
    ui->frameEditor->setupNewFrame(32, model);

    colorDialog.setVisible(false);

    // Set the minimum frames so that there cannot be zero frames.
    ui->frameNumber->setMinimum(1);

    // Sets the default color of the current color displayer.
    ui->currentColor->setStyleSheet("QLabel { background-color: black }");

    // Set up the New button pop-up
    ui->comboBox->addItem("16 x 16");
    ui->comboBox->addItem("32 x 32");
    ui->comboBox->addItem("64 x 64");
    ui->comboBox->addItem("128 x 128");
    ui->comboBox->addItem("256 x 256");
    ui->groupBox->setVisible(false);

    // Set up the connections from the tool bar to the functions
    connect(ui->actionEraser, &QAction::toggled, this, &MainWindow::actionEraserToggled);
    connect(ui->actionMirror, &QAction::toggled, ui->frameEditor, &FrameEditor::activeMirror);
    connect(this, &MainWindow::activeMirror, ui->frameEditor, &FrameEditor::activeMirror);
    connect(ui->actionBrush, &QAction::triggered, this, &MainWindow::actionBrushToggled);
    connect(ui->actionFill, &QAction::triggered, this, &MainWindow::actionFillToggled);
    connect(ui->actionFill_All, &QAction::triggered, this, &MainWindow::actionFillAllToggled);
    connect(ui->actionEyedrop_Tool, &QAction::triggered, this, &MainWindow::actionEyedropToolToggled);
    connect(ui->actionColor_Picker, &QAction::triggered, this, &MainWindow::actionColorPickerToggled);
    connect(ui->actionReadMe, &QAction::triggered, this, &MainWindow::actionReadMeTriggered);

    // Set up the connections from the PushButtons to the functions
    connect(ui->newButton, &QPushButton::pressed, this, &MainWindow::actionNewTriggered);
    connect(ui->saveButton, &QPushButton::pressed, this, &MainWindow::actionSaveTriggered);
    connect(ui->openButton, &QPushButton::pressed, this, &MainWindow::actionOpenTriggered);
    connect(ui->helpButton, &QPushButton::pressed, this, &MainWindow::onHelpButtonPressed);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::pressed, this, &MainWindow::onButtonNewOk);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &MainWindow::onButtonNewCancel);

    // Set up the connections from the secondaryToolBar to the functions
    connect(ui->actionCircle, &QAction::triggered, this, &MainWindow::actionCircleTriggered);
    connect(ui->actionSquare, &QAction::triggered, this, &MainWindow::actionSquareTriggered);
    connect(ui->actionRectangle, &QAction::triggered, this, &MainWindow::actionRectangleTriggered);
    connect(ui->actionShapes, &QAction::triggered, this, &MainWindow::actionShapesTriggered);

    // Set up the color connection
    connect(&colorDialog, &QColorDialog::colorSelected, ui->frameEditor, &FrameEditor::setColor);
    connect(&colorDialog, &QColorDialog::colorSelected, this, &MainWindow::displayCurrentColor);

    // Connect the fps slider
    connect(ui->fpsSlider, &QSlider::valueChanged, this, &MainWindow::onFpsSliderValueChanged);
    connect(ui->fpsSpinBox, &QSpinBox::valueChanged, this, &MainWindow::onFpsSpinBoxValueChanged);

    // Setup preview
    connect(ui->playPreviewButton, &QPushButton::clicked, this, &MainWindow::onPreviewStart);
    connect(this, &MainWindow::previewPlayed, model, &Model::playPreview);
    connect(model, &Model::setPreviewFrame, this, &MainWindow::setPreviewFrame);
    connect(ui->loopToggle, &QCheckBox::toggled, model, &Model::toggleLoop);
    ui->previewFrame->setAlignment(Qt::AlignCenter);

    connect(ui->frameEditor, &FrameEditor::changeCurrentColor, this, &MainWindow::displayCurrentColor);

    // Setup save/load
    connect(this, &MainWindow::saveFile, model, &Model::saveFile);
    connect(this, &MainWindow::loadFile, model, &Model::loadFile);
    connect(this, &MainWindow::startNewProject, ui->frameEditor, &FrameEditor::startNewProject);

    connect(ui->frameEditor, &FrameEditor::changeFrameNumber, this, &MainWindow::setFrameNumber);

    // Setup frame adding/deleting
    connect(ui->addFrame, &QPushButton::pressed, this, &MainWindow::addFrame);
    connect(this, &MainWindow::frameAdded, ui->frameEditor, &FrameEditor::newFrame);
    connect(ui->frameNumber, &QSpinBox::valueChanged, this, &MainWindow::changeFrame);
    connect(this, &MainWindow::frameChanged, ui->frameEditor, &FrameEditor::changeCurrentFrame);
    connect(ui->deleteFrame, &QPushButton::pressed, this, &MainWindow::deleteFrame);
    connect(this, &MainWindow::frameDeleted, ui->frameEditor, &FrameEditor::deleteCurrentFrame);

    connect(this, &MainWindow::activeTool, ui->frameEditor, &FrameEditor::activeTool);

    // LoadImage error
    connect(model, &Model::loadImageError, this, &MainWindow::displayOpenImageSizeError);

    // LoadImage load frame
    connect(model, &Model::loadFrame, this, &MainWindow::changeFrame);

    // Set up the cursor
    eraserCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/eraser-icon.png")), 0, -32));
    fillCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/painting-bucket-logo-icon.png")), 0, -32));
    fillAllCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/paint-bucket-icon.png")), 32, -32));
    eyedropCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/dosage-icon.png")), 0, -32));
    QTransform tr; tr.rotate(-90);
    brushCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/art-brush-design-icon.png")).transformed(tr), 0, 32));
    rectangleCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/rectangle.png")), 0, 0));
    circleCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/circle.png")), 0, 0));
    squareCursor = QCursor(QCursor(getPixmapFromIcon(QIcon(":/images/Images/square.png")), 0, 0));

    fileName = "";
}

//!
//! \brief MainWindow::~MainWindow Deconstructor for the main window
//!
MainWindow::~MainWindow() {
    delete ui;
}

//!
//! \brief MainWindow::setFrameNumber Updates the value of the current frame
//! \param frameNumber Number to update to
//!
void MainWindow::setFrameNumber(int frameNumber) {
    ui->frameNumber->setValue(frameNumber);
}

//!
//! \brief MainWindow::getPixmapFromIcon Converts an icon to a pixmap
//! \param icon The icon to be converted
//! \return the resultant pixmap
//!
QPixmap MainWindow::getPixmapFromIcon(QIcon icon) {
    QPixmap map = icon.pixmap(icon.actualSize(QSize(32, 32)));
    return map;
}

//!
//! \brief MainWindow::displayCurrentColor Updates the current color display
//! \param color The color to update to
//!
void MainWindow::displayCurrentColor(QColor color) {
    ui->currentColor->setStyleSheet("QLabel { background-color:" + color.name() + "}");
    untoggleActive(ui->actionBrush);
    toggleCursor(brushCursor, true);
    ui->actionBrush->setChecked(true);
    emit activeTool("brush");
}

//!
//! \brief MainWindow::untoggleActive Unselect the current tool
//! \param currToggle The current tool
//!
void MainWindow::untoggleActive(QAction* currToggle) {
    if(currToggle != ui->actionEraser) ui->actionEraser->setChecked(false);
    if(currToggle != ui->actionBrush) ui->actionBrush->setChecked(false);
    if(currToggle != ui->actionFill_All) ui->actionFill_All->setChecked(false);
    if(currToggle != ui->actionFill) ui->actionFill->setChecked(false);
    if(currToggle != ui->actionEyedrop_Tool) ui->actionEyedrop_Tool->setChecked(false);
}

//!
//! \brief MainWindow::toggleCursor Changes the active cursor
//! \param cursor The cursor to be changed
//! \param toggled Whether or not to activate the cursor
//!
void MainWindow::toggleCursor(QCursor cursor, bool toggled) {
    if(toggled) setCursor(cursor);
    else setCursor(Qt::ArrowCursor);

    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
}

//!
//! \brief MainWindow::actionEraserToggled Toggles the eraser tool
//! \param toggled What state to toggle it to
//!
void MainWindow::actionEraserToggled(bool toggled) {
    if(toggled) untoggleActive(ui->actionEraser);

    toggleCursor(eraserCursor, toggled);

    if(toggled) {
        emit activeTool("erasor");
    } else {
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionBrushToggled Toggles the brush tool
//! \param toggled What state to toggle it to
//!
void MainWindow::actionBrushToggled(bool toggled) {
    if(toggled) untoggleActive(ui->actionBrush);
    toggleCursor(brushCursor, toggled);

    if(toggled) {
        emit activeTool("brush");
    } else {
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionFillAllToggled Toggles the fill all tool
//! \param toggled What state to toggle it to
//!
void MainWindow::actionFillAllToggled(bool toggled) {
    if(toggled) untoggleActive(ui->actionFill_All);
    toggleCursor(fillAllCursor, toggled);
    emit activeMirror(false);
    ui->actionMirror->setChecked(false);
    if(toggled) {
        emit activeTool("fillAll");
    } else {
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionFillToggled Toggles the fill tool
//! \param toggled What state to toggle it to
//!
void MainWindow::actionFillToggled(bool toggled) {
    if(toggled) untoggleActive(ui->actionFill);
    toggleCursor(fillCursor, toggled);
    emit activeMirror(false);
    ui->actionMirror->setChecked(false);
    if(toggled) {
        emit activeTool("fill");
    } else {
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionEyedropToolToggled Toggles the eyedrop tool
//! \param toggled What state to toggle it to
//!
void MainWindow::actionEyedropToolToggled(bool toggled) {
    if(toggled) untoggleActive(ui->actionEyedrop_Tool);
    toggleCursor(eyedropCursor, toggled);
    if(toggled) {
        emit activeTool("eyedrop");
    } else {
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionColorPickerToggled Toggles the color picker
//! \param toggled what state to toggle it to
//!
void MainWindow::actionColorPickerToggled(bool toggled) {
    if(!toggled){
        colorDialog.setVisible(true);
        previousTool = (ui->frameEditor->selectedTool);
        emit activeTool("none");
    }
}

//!
//! \brief MainWindow::actionReadMeTriggered Activates the readme menu
//!
void MainWindow::actionReadMeTriggered() {
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    onHelpButtonPressed();
}

//!
//! \brief MainWindow::actionNewTriggered Triggers the shapes menu
//!
void MainWindow::actionNewTriggered() {
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    emit activeTool("none");
    emit activeMirror(false);
    ui->actionMirror->setChecked(false);

    ui->groupBox->setVisible(true);
    ui->frameEditor->hide();
    setCursor(Qt::ArrowCursor);
}

//!
//! \brief MainWindow::onFpsSpinBoxValueChanged Updates the fps slider to match the spinbox
//! \param value value put into the FpsSpinBox
//!
void MainWindow::onFpsSpinBoxValueChanged(int value) {
    ui->fpsSlider->setValue(value);
}

//!
//! \brief MainWindow::onFpsSliderValueChanged Updates the spinbox to match the slider
//! \param value Current value of FpsSlider
//!
void MainWindow::onFpsSliderValueChanged(int value) {
    ui->fpsSpinBox->setValue(value);
}

//!
//! \brief MainWindow::actionCircleTriggered Selects the circle tool
//!
void MainWindow::actionCircleTriggered(bool toggled)
{
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    untoggleActive(ui->actionCircle);
    toggleCursor(circleCursor, toggled);
    emit activeTool("circle");
}

//!
//! \brief MainWindow::actionSquareTriggered Selects the square tool
//!
void MainWindow::actionSquareTriggered(bool toggled) {
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    untoggleActive(ui->actionSquare);
    toggleCursor(squareCursor, toggled);
    emit activeTool("square");

}

//!
//! \brief MainWindow::actionRectangleTriggered Select the rectangle tool
//!
void MainWindow::actionRectangleTriggered(bool toggled) {
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    untoggleActive(ui->actionRectangle);
    toggleCursor(rectangleCursor, toggled);
    emit activeTool("rectangle");
}

//!
//! \brief MainWindow::actionShapesTriggered Changes visability of the shapes sub-menu
//! \param used What state to toggle it to
//!
void MainWindow::actionShapesTriggered(bool used) {
    if(used){
        ui->secondaryToolBar->setVisible(true);
    } else {
        ui->secondaryToolBar->setVisible(false);
    }
}

//!
//! \brief MainWindow::onHelpButtonPressed Displays the readme menu
//!
void MainWindow::onHelpButtonPressed() {
    QMessageBox msgBox;
    emit activeTool("none");
    msgBox.setWindowTitle("Read Me");
    msgBox.setInformativeText("A7 Sprite Editor by Tigers, an application written in Qt6 for CS3505 A7-Sprite Editor.\n\n"

                              "Requirements\n"
                              "The application must be able to:\n"

                              "- Set the size in pixels of the sprite. This can be bounded by a minimum, maximum, and by jumps in size if desired (historically, sprites rendered as textures on simple geometry were most efficient for graphics cards when sized at powers of 2). Or it can be more continuous. Keep in mind that you are not making a paint program, so if you are drawing on 500x500 images, you are probably not enough sprite editor and are aimed too much at being a paint program.\n"
                              "- Adjust the number of frames for the sprite animation - this can be incremental, like \"add a frame\" and \"delete a frame\".\n"
                              "- Allow the user to modify the pixels of a sprite. The most basic form is clicking on pixels. You should also allow the user to draw while moving the mouse.\n"
                              "- Provide a preview of the sprite animation cycle.\n"
                              "- The user should be able to adjust the frames per second of playback.\n"
                              "- There should be an option of seeing the playback at the actual size of the sprite (even when super tiny).\n"
                              "- Save and load a project. All values in the project file should be represented by a json data structure (Links to an external site.). The json data structure should have the following fields:\n"
                              "     - height with an integer value\n"
                              "     - width with an integer value\n"
                              "     - numberOfFrames with an integer value\n"
                              "     - frames with an array of frames, with each frame being a field with a name framen, where n is the frame number starting at 0. Each frame is an array of rows from top to bottom and each row being an array of pixels from left to right and each pixel being an array of [r, g, b, a] 0-255 integer values. A json file does not need to preserve ordering (it is more of a map structure) or newlines, so your file may differ from the example that will be provided at the start of the implementation phase.\n"
                              "     - Use a sprite sheet project .ssp extension when saving. You should be able to add additional fields to this format if you have fancy features that need additional information to be saved but you need to be able to read and save this basic format. You can reject files that do not meet your constraints on sprite images (for example, reject non-square if that is a constraint of yours).\n\n"
                              "Additional Features\n"
                              "The application will also allow for some additional tools:\n\n"

                              "- Fill: Will fill all pixels that are of the same color as the pixel until it is blocked off by pixels of different colors\n"
                              "- Fill All: Will fill all pixels that are of the same color in the entire frame\n"
                              "- Shapes: Allows the user to draw circle's, rectangle's, and square's with minimal effort\n"
                              "- Mirror: Allows for symmetrical drawing on the frame, Mirror will automatically toggle off if the user decides to fill, fill all, or load/create a new sprite\n\n"
                              "Other things of notice\n"
                              "- Canvas is set to fixed sizes of 16x16, 32x32, 64x64, 128x128, and 256x256, the user can freely choose which to use when creating a new sprite, but starts out in a default 32x32 size when first opening the application\n"
                              "- Eyedropper tool only functions inside the canvas and you can't replicate colors outside the canvas\n"
                              "- Cursor image changes according to the tool selected");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
    emit activeTool("brush");
}

//!
//! \brief MainWindow::onButtonNewOk Does some inital setup for a new file
//!
void MainWindow::onButtonNewOk() {
    QString canvasSize = ui->comboBox->currentText();
    QMessageBox msgBox;
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int answer = msgBox.exec();
    if(answer == 16384){
        actionSaveTriggered();
    }
    emit startNewProject(canvasSize, model);
    ui->groupBox->hide();
    ui->frameEditor->show();
    toggleCursor(brushCursor, true);
    emit activeTool("brush");
}

//!
//! \brief MainWindow::onButtonNewCancel Cancels the new file menu
//!
void MainWindow::onButtonNewCancel() {
    ui->groupBox->setVisible(false);
    ui->frameEditor->show();

    // Does nothing as the user cancelled the making of a project
    toggleCursor(brushCursor, true);
    emit activeTool("brush");
}

//!
//! \brief MainWindow::addFrame Sends the signal to add a new frame forward
//!
void MainWindow::addFrame(){
    emit frameAdded(model);
}

//!
//! \brief MainWindow::changeFrame Sends the signal to change the frame forward
//! \param num frame number to change to
//!
void MainWindow::changeFrame(int num){
    emit frameChanged(num, model);
}

//!
//! \brief MainWindow::deleteFrame Sends the signal to delete a frame forward
//!
void MainWindow::deleteFrame(){
    emit frameDeleted(model);
}

//!
//! \brief MainWindow::onPreviewStart Sends the signal to start the preview forward
//!
void MainWindow::onPreviewStart(){
    emit previewPlayed(ui->fpsSpinBox);
}

//!
//! \brief MainWindow::setPreviewFrame Changes the preview frame pixamp
//! \param frame the frame to display
//!
void MainWindow::setPreviewFrame(QPixmap frame){
    QPixmap display = frame;
    if(ui->scaleToggle->isChecked()) display = frame.scaled(ui->previewFrame->size(),Qt::KeepAspectRatio);
    ui->previewFrame->setPixmap(display);
}

//!
//! \brief MainWindow::actionSaveTriggered Displays the save window and allows the user to save
//!
void MainWindow::actionSaveTriggered() {
    // Sets the tool and window
    previousTool = (ui->frameEditor->selectedTool);
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    emit activeTool("none");
    fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("Sprite Sheet Project (*.ssp)"));

    // Save the file
    if (!fileName.isEmpty()) {
        emit saveFile(fileName);
        emit activeMirror(false);
        ui->actionMirror->setChecked(false);
        untoggleActive(ui->actionBrush);
        toggleCursor(brushCursor, true);
        emit activeTool("brush");
    } else {
        emit activeTool(previousTool);
    }
}

//!
//! \brief MainWindow::actionOpenTriggered Displays the open window and allows the user to open a file
//!
void MainWindow::actionOpenTriggered() {
    // Set the tool and window
    previousTool = (ui->frameEditor->selectedTool);
    ui->actionShapes->setChecked(false);
    ui->secondaryToolBar->setVisible(false);
    emit activeTool("none");

    // Open the open dialog
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(), tr("Sprite Sheet Project (*.ssp)"));

    if (!fileName.isEmpty()) {
        // Load the file
        emit loadFile(fileName);
        emit activeMirror(false);
        ui->actionMirror->setChecked(false);
        untoggleActive(ui->actionBrush);
        toggleCursor(brushCursor, true);
        emit activeTool("brush");
    } else {
        emit activeTool(previousTool);
    }
}

//!
//! \brief MainWindow::displayOpenImageSizeError Displays the error popup for a failed open
//!
void MainWindow::displayOpenImageSizeError()
{
    QMessageBox::warning(this, tr("File Error"), tr("Unable to parse file. Try again."));
}
