#include "blankscrollarea.h"
#include <QScrollBar>

#include "debug.h"
#include "ui/Forms/timelinetrackarea.h"


using ui::BlankScrollArea;

BlankScrollArea::BlankScrollArea(QWidget* parent) : QScrollArea(parent)
{
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //TODO: turn off once resizing/scrolling is fixed
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
  auto max_val = verticalScrollBar()->maximum();
  // INFO: needs some sort of offset
  verticalScrollBar()->setValue(max_val - scroll_);
}

void BlankScrollArea::wheelEvent(QWheelEvent *event)
{
  event->accept();
  // do nothing, preventing scrolling
}

void BlankScrollArea::resizeEvent(QResizeEvent *event)
{


}
