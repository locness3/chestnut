#ifndef BLANKSCROLLAREA_H
#define BLANKSCROLLAREA_H

#include <QScrollArea>
#include <QMouseEvent>


namespace ui
{

  class BlankScrollArea : public QScrollArea
  {
      Q_OBJECT
    public:
      BlankScrollArea(QWidget* parent=nullptr, const bool reverse=false);

      void containWidget(QWidget* widget);
      QScrollBar* vert_bar_{nullptr};

    public slots:
      void setScroll(const int val);

    protected:
      virtual void wheelEvent(QWheelEvent *event) override;

      virtual void resizeEvent(QResizeEvent *event) override;

    private:
      int scroll_{0};
      bool reverse_scroll_{false};
  };
}

#endif // BLANKSCROLLAREA_H
