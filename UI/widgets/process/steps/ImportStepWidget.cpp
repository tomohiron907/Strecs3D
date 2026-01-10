#include "ImportStepWidget.h"
#include "../../Button.h"
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>

// --- DropAreaWidget Implementation ---

DropAreaWidget::DropAreaWidget(QWidget* parent) : QLabel(parent) {
    setText("Drop STEP file here");
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    
    // Default Style
    setStyleSheet(R"(
        QLabel {
            border: 2px dashed #666;
            border-radius: 8px;
            color: #aaa;
            background-color: rgba(255, 255, 255, 10);
            font-size: 14px;
        }
    )");
    
    setMinimumHeight(250);
}

void DropAreaWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1) {
            QString fileName = urls.first().toLocalFile();
            QFileInfo fileInfo(fileName);
            QString ext = fileInfo.suffix().toLower();
            
            if (ext == "step" || ext == "stp") {
                event->acceptProposedAction();
                setStyleSheet(R"(
                    QLabel {
                        border: 2px dashed #0078D7;
                        border-radius: 8px;
                        color: #0078D7;
                        background-color: rgba(0, 120, 215, 40);
                        font-size: 14px;
                    }
                )");
                return;
            }
        }
    }
    event->ignore();
}

void DropAreaWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    Q_UNUSED(event);
    // Revert to default style
    setStyleSheet(R"(
        QLabel {
            border: 2px dashed #666;
            border-radius: 8px;
            color: #aaa;
            background-color: rgba(255, 255, 255, 10);
            font-size: 14px;
        }
    )");
}

void DropAreaWidget::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QString filePath = mimeData->urls().at(0).toLocalFile();
        emit fileDropped(filePath);
    }
    
    // Revert style
    setStyleSheet(R"(
        QLabel {
            border: 2px dashed #666;
            border-radius: 8px;
            color: #aaa;
            background-color: rgba(255, 255, 255, 10);
            font-size: 14px;
        }
    )");
    
    event->acceptProposedAction();
}

// --- ImportStepWidget Implementation ---

ImportStepWidget::ImportStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->addStretch();
    
    // Import Button
    m_importButton = new Button("Import STEP File", this);
    m_importButton->setGlobalIconSize(QSize(24, 24));
    m_importButton->setIcon(":/resources/icons/step.png");
    m_importButton->setMinimumHeight(50);
    
    connect(m_importButton, &Button::clicked, this, &ImportStepWidget::importClicked);
    
    layout->addWidget(m_importButton);
    
    // "or" Separator
    QLabel* orLabel = new QLabel(" or ", this);
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setStyleSheet("color: #888; font-size: 17px; font-weight: bold; margin: 5px 0;");
    layout->addWidget(orLabel);
    
    // Drop Area
    m_dropArea = new DropAreaWidget(this);
    connect(m_dropArea, &DropAreaWidget::fileDropped, this, &ImportStepWidget::fileDropped);
    
    layout->addWidget(m_dropArea);
    
    layout->addStretch();
}
