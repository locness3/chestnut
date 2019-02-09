#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <QDialog>

class QPlainTextEdit;

class TextEditDialog : public QDialog {
    Q_OBJECT
  public:
    TextEditDialog(QWidget* parent = nullptr, const QString& s = 0);

    TextEditDialog(const TextEditDialog& ) = delete;
    TextEditDialog& operator=(const TextEditDialog&) = delete;

    const QString& get_string();
  private slots:
    void save();
    void cancel();
  private:
    QString result_str;
    QPlainTextEdit* textEdit;
};

#endif // TEXTEDITDIALOG_H
