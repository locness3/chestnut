#include "footagestream.h"

#include <QPainter>

#include "debug.h"

using project::FootageStream;

void FootageStream::make_square_thumb()
{
  // generate square version for QListView?
  const auto max_dimension = qMax(video_preview.width(), video_preview.height());
  QPixmap pixmap(max_dimension, max_dimension);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  const auto diff = (video_preview.width() - video_preview.height()) / 2;
  const auto sqx = (diff < 0) ? -diff : 0;
  const auto sqy = (diff > 0) ? diff : 0;
  p.drawImage(sqx, sqy, video_preview);
  video_preview_square = QIcon(pixmap);
}

bool FootageStream::load(QXmlStreamReader& stream)
{
  auto name = stream.name().toString().toLower();
  if (name == "video") {
    type_ = StreamType::VIDEO;
  } else if (name == "audio") {
    type_ = StreamType::AUDIO;
  } else {
    qCritical() << "Unknown stream type" << name;
    return false;
  }

  for (const auto& attr : stream.attributes()) {
    const auto name = attr.name().toString().toLower();
    if (name == "id") {
      file_index = attr.value().toInt();
    } else if (name == "infinite") {
      infinite_length = attr.value().toString().toLower() == "true";
    } else {
      qWarning() << "Unknown attribute" << name;
    }
  }

  while (stream.readNextStartElement()) {
    const auto name = stream.name().toString().toLower();
    if (name == "width") {
      video_width = stream.readElementText().toInt();
    } else if (name == "height") {
      video_height = stream.readElementText().toInt();
    } else if (name == "framerate") {
      video_frame_rate = stream.readElementText().toDouble();
    } else if (name == "channels") {
      audio_channels = stream.readElementText().toInt();
    } else if (name == "frequency") {
      audio_frequency = stream.readElementText().toInt();
    } else if (name == "layout") {
      audio_layout = stream.readElementText().toInt();
    } else {
      qWarning() << "Unhandled element" << name;
      stream.skipCurrentElement();
    }
  }

  return true;
}

bool FootageStream::save(QXmlStreamWriter& stream) const
{
  switch (type_) {
    case StreamType::VIDEO:
      stream.writeStartElement("video");
      stream.writeAttribute("id", QString::number(file_index));
      stream.writeAttribute("width", QString::number(video_width));
      stream.writeAttribute("height", QString::number(video_height));
      stream.writeAttribute("framerate", QString::number(video_frame_rate, 'f', 10));
      stream.writeAttribute("infinite", QString::number(infinite_length));
      stream.writeEndElement();
      return true;
    case StreamType::AUDIO:
      stream.writeStartElement("audio");
      stream.writeAttribute("id", QString::number(file_index));
      stream.writeAttribute("channels", QString::number(audio_channels));
      stream.writeAttribute("layout", QString::number(audio_layout));
      stream.writeAttribute("frequency", QString::number(audio_frequency));
      stream.writeEndElement();
      return true;
    default:
      qWarning() << "Unknown stream type. Not saving";
      break;
  }
  return false;
}
