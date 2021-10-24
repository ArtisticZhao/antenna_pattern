#pragma once
#include <qcombobox.h>
class QComboBoxMoreSignal : public QComboBox {
    Q_OBJECT
signals:
    void combo_box_showpopup(QComboBoxMoreSignal* self);
public:
    explicit QComboBoxMoreSignal(QWidget* parent = nullptr);
private:
    void showPopup() override;
};
