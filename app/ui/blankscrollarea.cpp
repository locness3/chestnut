#include "blankscrollarea.h"
#include <QScrollBar>

#include "debug.h"
#include "ui/Forms/timelinetrackarea.h"


using ui::BlankScrollArea;

BlankScrollArea::BlankScrollArea(QWidget* parent, const bool reverse)
  : QScrollArea(parent),
    reverse_scroll_(reverse)
{
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  setMouseTracking(false);
  setAlignment(Qt::AlignBottom);
  setFrameShape(Shape::NoFrame);
}


void BlankScrollArea::containWidget(QWidget* widget)
{
  setWidget(widget);
  setMaximumWidth(widget->maximumWidth());
  verticalScrollBar()->setMaximum(widget->height());
}


void BlankScrollArea::setScroll(const int val)
{
  scroll_ = qAbs(val);
  if (reverse_scroll_) {
    auto max_val = verticalScrollBar()->maximum();
    verticalScrollBar()->setValue(max_val - scroll_);
  } else {
    verticalScrollBar()->setValue(scroll_);
  }
}

void BlankScrollArea::wheelEvent(QWheelEvent *event)
{
  event->accept();
  // do nothing, preventing scrolling
}

void BlankScrollArea::resizeEvent(QResizeEvent *event)
{
  event->accept();
  auto wid = reinterpret_cast<TimelineTrackArea*>(widget());
  wid->setMinimumHeight(vert_bar_->minimumHeight()); //Fudge to force the area to resize and fill out scrollarea
}
