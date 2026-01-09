#ifndef IMPORTSTEPWIDGET_H
#define IMPORTSTEPWIDGET_H

#include <QWidget>

class Button;

class ImportStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImportStepWidget(QWidget* parent = nullptr);
    Button* getImportButton() const { return m_importButton; }
signals:
    void importClicked();
private:
    Button* m_importButton;
};

#endif // IMPORTSTEPWIDGET_H
