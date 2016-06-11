#include "stranslater.h"
#include "ui_stranslater.h"

STranslater::STranslater(QWidget *parent) : QMainWindow(parent), ui(new Ui::STranslater)
{
    ui->setupUi(this);
    trns = new Translator();
}

STranslater::~STranslater()
{
    delete ui;
}



void STranslater::on_bTranslate_clicked()
{
    QString text = ui->pteSourceText->toPlainText();

    Lang from = (Lang)ui->cbSourceLang->currentIndex();
    Lang to = (Lang)ui->cbResultLang->currentIndex();

    if( from == to )
    {
        ui->pteResultText->setPlainText( text );
        return;
    }

    trns->setText(text, from);
    QString trans = trns->translate(from, to);

    ui->pteResultText->setPlainText( trans );
}

void STranslater::on_bSwapLang_clicked()
{
    int source = ui->cbSourceLang->currentIndex();
    int result = ui->cbResultLang->currentIndex();

    QString result_text = ui->pteResultText->toPlainText();
    ui->pteResultText->clear();

    ui->cbSourceLang->setCurrentIndex(result);
    ui->cbResultLang->setCurrentIndex(source);
    ui->pteSourceText->setPlainText(result_text);

    on_bTranslate_clicked();
}
