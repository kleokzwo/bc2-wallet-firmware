#ifndef BC2_SIMULATOR_BC2HEADER_H
#define BC2_SIMULATOR_BC2HEADER_H

#include <QWidget>

class QLabel;

class Bc2Header final : public QWidget {
public:
    explicit Bc2Header(const QString &title, const QString &subtitle = {},
                       QWidget *parent = nullptr);

    QLabel *titleLabel() const;
    QLabel *subtitleLabel() const;

private:
    QLabel *titleLabel_ = nullptr;
    QLabel *subtitleLabel_ = nullptr;
};

#endif
