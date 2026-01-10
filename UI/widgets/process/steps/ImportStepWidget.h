#ifndef IMPORTSTEPWIDGET_H
#define IMPORTSTEPWIDGET_H

#include <QWidget>
#include <QLabel>

class Button;

class DropAreaWidget : public QLabel {
    Q_OBJECT
public:
    explicit DropAreaWidget(QWidget* parent = nullptr);

signals:
    void fileDropped(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

class ImportStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImportStepWidget(QWidget* parent = nullptr);
    Button* getImportButton() const { return m_importButton; }

signals:
    void importClicked();
    void fileDropped(const QString& filePath);

private:
    Button* m_importButton;
    DropAreaWidget* m_dropArea;
};

#endif // IMPORTSTEPWIDGET_H
