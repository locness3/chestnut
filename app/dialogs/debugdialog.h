#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>
class QTextEdit;

class DebugDialog : public QDialog {
    Q_OBJECT
  public:
    explicit DebugDialog(QWidget* parent = nullptr);

    DebugDialog(const DebugDialog& ) = delete;
    DebugDialog& operator=(const DebugDialog&) = delete;

  public slots:
    void update_log();
  protected:
    void showEvent(QShowEvent* event) override;
  private:
    QTextEdit* textEdit;
};

extern DebugDialog* debug_dialog;

#endif // DEBUGDIALOG_H
