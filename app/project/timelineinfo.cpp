#include "timelineinfo.h"

using project::TimelineInfo;


bool TimelineInfo::isVideo() const
{
  return track_ < 0;
}
