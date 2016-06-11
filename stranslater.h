#ifndef STRANSLATER_H
#define STRANSLATER_H

#include <QMainWindow>
#include <Translator/translator.h>

namespace Ui {
class STranslater;
}

class STranslater : public QMainWindow
{
    Q_OBJECT

public:
    explicit STranslater(QWidget *parent = 0);
    ~STranslater();

private slots:
    void on_bTranslate_clicked();

    void on_bSwapLang_clicked();

private:
    Ui::STranslater *ui;
    Translator * trns;
};

#endif // STRANSLATER_H
