#include "timelineinfo.h"

#include "debug.h"

using project::TimelineInfo;

constexpr const char* const XML_ELEM_NAME = "timelineinfo";


bool TimelineInfo::isVideo() const
{
  return track_ < 0;
}


TimelineInfo& TimelineInfo::operator=(const TimelineInfo& rhs)
{
  if (this != &rhs) {
    enabled = rhs.enabled;
    name_ = rhs.name_;
    clip_in = rhs.clip_in.load();
    in = rhs.in.load();
    out = rhs.out.load();
    track_ = rhs.track_.load();
    color = rhs.color;
    media = rhs.media;
    media_stream = rhs.media_stream.load();
    autoscale = rhs.autoscale;
    speed = rhs.speed.load();
    maintain_audio_pitch = rhs.maintain_audio_pitch;
    reverse = rhs.reverse;
  }
  return *this;
}

bool TimelineInfo::load(QXmlStreamReader& stream)
{
  while (stream.readNextStartElement()) {
    const auto name = stream.name().toString().toLower();
    if (name == "name") {
      name_ = stream.readElementText();
    } else if (name == "clipin") {
      clip_in = stream.readElementText().toInt();
    } else if (name == "enabled") {
      enabled = stream.readElementText() == "true";
    } else if (name == "in") {
      in = stream.readElementText().toInt();
    } else if (name == "out") {
      out = stream.readElementText().toInt();
    } else if (name == "track") {
      track_ = stream.readElementText().toInt();
    } else if (name == "color") {
      color.setRgb(static_cast<QRgb>(stream.readElementText().toUInt()));
    } else if (name == "autoscale") {
      autoscale = stream.readElementText() == "true";
    } else if (name == "speed") {
      speed = stream.readElementText().toDouble();
    } else if (name == "maintainpitch") {
      maintain_audio_pitch = stream.readElementText() == "true";
    } else if (name == "reverse") {
      reverse = stream.readElementText() == "true";
    } else if (name == "stream") {
      media_stream = stream.readElementText().toInt();
    } else {
      qWarning() << "Unhandled element" << name;
    }
  }
  return true;
}

bool TimelineInfo::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement(XML_ELEM_NAME);
  stream.writeTextElement("name", name_);
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
