#include "timelineinfo.h"

using project::TimelineInfo;

constexpr const char* const XML_ELEM_NAME = "timelineinfo";


bool TimelineInfo::isVideo() const
{
  return track_ < 0;
}

bool TimelineInfo::load(QXmlStreamReader& stream)
{
  //TODO:
  return false;
}

bool TimelineInfo::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement(XML_ELEM_NAME);
  stream.writeTextElement("name", name);
  stream.writeTextElement("clipin", QString::number(clip_in));
  stream.writeTextElement("enabled", enabled ? "true" : "false");
  stream.writeTextElement("in", QString::number(in));
  stream.writeTextElement("out", QString::number(out));
  stream.writeTextElement("track", QString::number(track_));
  stream.writeTextElement("color", QString::number(color.rgb()));
  stream.writeTextElement("autoscale", autoscale ? "true" : "false");
  stream.writeTextElement("speed", QString::number(speed, 'g', 10));
  stream.writeTextElement("maintainpitch", QString::number(maintain_audio_pitch));
  stream.writeTextElement("reverse", QString::number(reverse));
  stream.writeTextElement("stream", QString::number(media_stream));

  stream.writeEndElement();
  return true;
}
