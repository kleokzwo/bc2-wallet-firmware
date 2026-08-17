#ifndef BC2_SIMULATOR_BC2BUTTON_H
#define BC2_SIMULATOR_BC2BUTTON_H

#include <QPushButton>

class Bc2Button final : public QPushButton {
public:
    enum class Style { Primary, Secondary, Navigation, Danger };

    explicit Bc2Button(const QString &text, Style style = Style::Primary,
                       QWidget *parent = nullptr);

    void setStyle(Style style);
    void setActive(bool active);
    void setResponsiveText(const QString &fullText, const QString &compactText);
    void setCompact(bool compact);

private:
    void refreshStyle();

    QString fullText_;
    QString compactText_;
    bool compact_ = false;
};

#endif
