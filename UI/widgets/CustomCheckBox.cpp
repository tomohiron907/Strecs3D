#include "CustomCheckBox.h"
#include <QDir>

CustomCheckBox::CustomCheckBox(QWidget* parent)
    : QPushButton(parent), checked(true) {
    // アイコンのパスを設定
    iconChecked = QIcon(":/resources/icons/eye.png");
    iconUnchecked = QIcon(":/resources/icons/eye_slash.png");
    setCheckable(false); // 独自で管理
    setFlat(true);
    setFixedSize(32, 32); // 必要に応じてサイズ調整
    updateIcon();
    connect(this, &QPushButton::clicked, this, &CustomCheckBox::handleClicked);
}

void CustomCheckBox::setChecked(bool checked) {
    if (this->checked != checked) {
        this->checked = checked;
        updateIcon();
        emit toggled(this->checked);
    }
}

bool CustomCheckBox::isChecked() const {
    return checked;
}

void CustomCheckBox::handleClicked() {
    setChecked(!checked);
}

void CustomCheckBox::updateIcon() {
    setIcon(checked ? iconChecked : iconUnchecked);
    setIconSize(QSize(24, 24));
    setToolTip(checked ? "表示" : "非表示");
} 