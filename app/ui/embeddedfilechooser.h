#ifndef EMBEDDEDFILECHOOSER_H
#define EMBEDDEDFILECHOOSER_H

#include <QWidget>
#include <QLabel>

#include "ui/IEffectFieldWidget.h"

class EmbeddedFileChooser : public QWidget, chestnut::ui::IEffectFieldWidget
{
    Q_OBJECT
  public:
    explicit EmbeddedFileChooser(QWidget* parent = nullptr);

    EmbeddedFileChooser(const EmbeddedFileChooser& ) = delete;
    EmbeddedFileChooser& operator=(const EmbeddedFileChooser&) = delete;

    QString getFilename() const;
    QString getPreviousValue() const;
    void setFilename(QString name);

    QVariant getValue() const override;
    void setValue(QVariant val) override;
  signals:
    void changed();
  private:
    QLabel* file_label;
    QString filename;
    QString old_filename;
    void update_label();
  private slots:
    void browse();
};

#endif // EMBEDDEDFILECHOOSER_H
