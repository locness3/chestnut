/*
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "media.h"

#include "footage.h"
#include "sequence.h"
#include "undo.h"
#include "io/config.h"
#include "panels/viewer.h"
#include "panels/project.h"
#include "projectmodel.h"

#include <QPainter>
#include <QCoreApplication>

#include "debug.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace
{
  const int COLUMN_COUNT = 3;
  const char* const FOLDER_ICON = ":/icons/folder.png";
  const char* const SEQUENCE_ICON = ":/icons/sequence.png";
}

QString get_interlacing_name(int interlacing) {
  switch (interlacing) {
    case VIDEO_PROGRESSIVE: return QCoreApplication::translate("InterlacingName", "None (Progressive)");
    case VIDEO_TOP_FIELD_FIRST: return QCoreApplication::translate("InterlacingName", "Top Field First");
    case VIDEO_BOTTOM_FIELD_FIRST: return QCoreApplication::translate("InterlacingName", "Bottom Field First");
    default: return QCoreApplication::translate("InterlacingName", "Invalid");
  }
}

QString get_channel_layout_name(int channels, uint64_t layout) {
  switch (channels) {
    case 0: return QCoreApplication::translate("ChannelLayoutName", "Invalid");
    case 1: return QCoreApplication::translate("ChannelLayoutName", "Mono");
    case 2: return QCoreApplication::translate("ChannelLayoutName", "Stereo");
    default: {
      char buf[50];
      av_get_channel_layout_string(buf, sizeof(buf), channels, layout);
      return QString(buf);
    }
  }
}

int Media::nextID = 0;


Media::Media() : Media(nullptr)
{
  // If no parent, this instance is the root
  _root = true;
}

Media::Media(MediaPtr iparent) :
  throbber(nullptr),
  _root(false),
  _type(MediaType::NONE),
  _parent(iparent),
  _id(nextID++)
{

}

Media::~Media() {
  if (throbber != nullptr) {
    delete throbber;
  }
}


/**
 * @brief Obtain this instance unique-id
 * @return id
 */
int Media::id() const {
  return _id;
}

void Media::clearObject() {
  _type = MediaType::NONE;
  _object = nullptr;
}
bool Media::setFootage(FootagePtr ftg) {
  if ((ftg != nullptr) && (ftg != _object)) {
    _type = MediaType::FOOTAGE;
    _object = ftg;
    return true;
  }
  return false;
}

bool Media::setSequence(SequencePtr sqn) {
  if ( (sqn != nullptr) && (sqn != _object) ) {
    setIcon(QIcon(SEQUENCE_ICON));
    _type = MediaType::SEQUENCE;
    _object = sqn;
    updateTooltip();
    return true;
  }
  return false;
}

void Media::setFolder() {
  if (_folderName.isEmpty()) {
    _folderName = QCoreApplication::translate("Media", "New Folder");
  }
  setIcon(QIcon(FOLDER_ICON));
  _type = MediaType::FOLDER;
  _object = nullptr;
}

void Media::setIcon(const QIcon &ico) {
  _icon = ico;
}

void Media::setParent(MediaWPtr p) {
  _parent = p;
}

void Media::updateTooltip(const QString& error) {
  switch (_type) {
    case MediaType::FOOTAGE:
    {
      auto ftg = object<Footage>();
      _toolTip = QCoreApplication::translate("Media", "Name:") + " " + ftg->getName() + "\n"
                 + QCoreApplication::translate("Media", "Filename:") + " " + ftg->url + "\n";

      if (error.isEmpty()) {
        if (ftg->video_tracks.size() > 0) {
          _toolTip += QCoreApplication::translate("Media", "Video Dimensions:") + " ";
          for (int i=0;i<ftg->video_tracks.size();i++) {
            if (i > 0) {
              _toolTip += ", ";
            }
            _toolTip += QString::number(ftg->video_tracks.at(i)->video_width) + "x"
                        + QString::number(ftg->video_tracks.at(i)->video_height);
          }
          _toolTip += "\n";

          if (!ftg->video_tracks.front()->infinite_length) {
            _toolTip += QCoreApplication::translate("Media", "Frame Rate:") + " ";
            for (int i=0;i<ftg->video_tracks.size();i++) {
              if (i > 0) {
                _toolTip += ", ";
              }
              if (ftg->video_tracks.at(i)->video_interlacing == VIDEO_PROGRESSIVE) {
                _toolTip += QString::number(ftg->video_tracks.at(i)->video_frame_rate * ftg->speed);
              } else {
                _toolTip += QCoreApplication::translate("Media", "%1 fields (%2 frames)").arg(
                              QString::number(ftg->video_tracks.at(i)->video_frame_rate * ftg->speed * 2),
                              QString::number(ftg->video_tracks.at(i)->video_frame_rate * ftg->speed)
                              );
              }
            }
            _toolTip += "\n";
          }

          _toolTip += QCoreApplication::translate("Media", "Interlacing:") + " ";
          for (int i=0;i<ftg->video_tracks.size();i++) {
            if (i > 0) {
              _toolTip += ", ";
            }
            _toolTip += get_interlacing_name(ftg->video_tracks.at(i)->video_interlacing);
          }
        }

        if (ftg->audio_tracks.size() > 0) {
          _toolTip += "\n";

          _toolTip += QCoreApplication::translate("Media", "Audio Frequency:") + " ";
          for (int i=0;i<ftg->audio_tracks.size();i++) {
            if (i > 0) {
              _toolTip += ", ";
            }
            _toolTip += QString::number(ftg->audio_tracks.at(i)->audio_frequency * ftg->speed);
          }
          _toolTip += "\n";

          _toolTip += QCoreApplication::translate("Media", "Audio Channels:") + " ";
          for (int i=0;i<ftg->audio_tracks.size();i++) {
            if (i > 0) {
              _toolTip += ", ";
            }
            _toolTip += get_channel_layout_name(ftg->audio_tracks.at(i)->audio_channels,
                                                ftg->audio_tracks.at(i)->audio_layout);
          }
          // tooltip += "\n";
        }
      } else {
        _toolTip += error;
      }
    }
      break;
    case MediaType::SEQUENCE:
    {
      auto sqn = object<Sequence>();

      _toolTip = QCoreApplication::translate("Media", "Name: %1"
                                                      "\nVideo Dimensions: %2x%3"
                                                      "\nFrame Rate: %4"
                                                      "\nAudio Frequency: %5"
                                                      "\nAudio Layout: %6").arg(
                   sqn->getName(),
                   QString::number(sqn->width()),
                   QString::number(sqn->height()),
                   QString::number(sqn->frameRate()),
                   QString::number(sqn->audioFrequency()),
                   get_channel_layout_name(av_get_channel_layout_nb_channels(sqn->audioLayout()), sqn->audioLayout())
                   );
    }
      break;
    default:
      throw UnhandledMediaTypeException();
      break;
  }//switch

}

MediaType Media::type() const {
  return _type;
}

const QString &Media::name() {
  switch (_type) {
    case MediaType::FOOTAGE: return object<Footage>()->getName();
    case MediaType::SEQUENCE: return object<Sequence>()->getName();
    default: return _folderName;
  }
}

void Media::setName(const QString &name) {
  switch (_type) {
    case MediaType::FOOTAGE:
      object<Footage>()->setName(name);
      break;
    case MediaType::SEQUENCE:
      object<Sequence>()->setName(name);
      break;
    case MediaType::FOLDER:
      _folderName = name;
      break;
    default:
      throw UnhandledMediaTypeException();
      break;
  }//switch
}

double Media::frameRate(const int stream) {
  switch (type()) {
    case MediaType::FOOTAGE:
    {
      if (auto ftg = object<Footage>()) {
        if ( (stream < 0) && !ftg->video_tracks.empty()) {
          return ftg->video_tracks.front()->video_frame_rate * ftg->speed;
        }
        if (FootageStreamPtr ms = ftg->video_stream_from_file_index(stream)) {
          return ms->video_frame_rate * ftg->speed;
        }
      }
      break;
    }
    case MediaType::SEQUENCE:
      if (auto sqn = object<Sequence>()) {
        return sqn->frameRate();
      }
      break;
    default:
      throw UnhandledMediaTypeException();
      break;
  }//switch

  return 0.0;
}

int Media::samplingRate(const int stream) {
  switch (type()) {
    case MediaType::FOOTAGE:
    {
      if (auto ftg = object<Footage>()) {
        if (stream < 0) {
          return ftg->audio_tracks.front()->audio_frequency * ftg->speed;
        }
        if (FootageStreamPtr ms = ftg->audio_stream_from_file_index(stream)) {
          return ms->audio_frequency * ftg->speed;
        }
      }
      break;
    }
    case MediaType::SEQUENCE:
      if (auto sqn = object<Sequence>()) {
        return sqn->audioFrequency();
      }
      break;
    default:
      throw UnhandledMediaTypeException();
      break;
  }//switch
  return 0;
}

void Media::appendChild(MediaPtr child) {
  child->setParent(shared_from_this());
  _children.append(child);
}

bool Media::setData(int col, const QVariant &value) {
  if (col == 0) {
    QString n = value.toString();
    if (!n.isEmpty() && name() != n) {
      e_undo_stack.push(new MediaRename(shared_from_this(), value.toString()));
      return true;
    }
  }
  return false;
}

MediaPtr Media::child(const int row) {
  return _children.value(row);
}

int Media::childCount() const {
  return _children.count();
}

int Media::columnCount() const {
  return COLUMN_COUNT;
}

QVariant Media::data(int column, int role) {
  switch (role) {
    case Qt::DecorationRole:
      if (column == 0) {
        if (type() == MediaType::FOOTAGE) {
          FootagePtr f = object<Footage>();
          if (f->video_tracks.size() > 0
              && f->video_tracks.front()->preview_done) {
            return f->video_tracks.front()->video_preview_square;
          }
        }

        return _icon;
      }
      break;
    case Qt::DisplayRole:
      switch (column) {
        case 0: return (_root) ? QCoreApplication::translate("Media", "Name") : name();
        case 1:
          if (_root) return QCoreApplication::translate("Media", "Duration");
          if (type() == MediaType::SEQUENCE) {
            SequencePtr s = object<Sequence>();
            return frame_to_timecode(s->endFrame(), e_config.timecode_view, s->frameRate());
          }
          if (type() == MediaType::FOOTAGE) {
            FootagePtr f = object<Footage>();
            double r = 30;

            if (f->video_tracks.size() > 0 && !qIsNull(f->video_tracks.front()->video_frame_rate))
              r = f->video_tracks.front()->video_frame_rate * f->speed;

            long len = f->get_length_in_frames(r);
            if (len > 0) return frame_to_timecode(len, e_config.timecode_view, r);
          }
          break;
        case 2:
          if (_root) return QCoreApplication::translate("Media", "Rate");
          if (type() == MediaType::SEQUENCE) return QString::number(frameRate()) + " FPS";
          if (type() == MediaType::FOOTAGE) {
            auto ftg = object<Footage>();
            const double rate = frameRate();
            if ( (ftg->video_tracks.size() > 0) && !qIsNull(rate)) {
              return QString::number(rate) + " FPS";
            } else if (ftg->audio_tracks.size() > 0) {
              return QString::number(samplingRate()) + " Hz";
            }
          }
          break;
        default:
          // There's only 3 columns
          break;
      }//switch
      break;
    case Qt::ToolTipRole:
      return _toolTip;
    default:
      qDebug() << "Unhandled role" << role;
      break;
  }//switch
  return QVariant();
}

int Media::row() {
  if (auto parPtr = _parent.lock()) {
    return parPtr->_children.indexOf(shared_from_this());
  }
  return 0;
}

MediaPtr Media::parentItem() {
  return _parent.lock();
}

void Media::removeChild(int i) {
  _children.removeAt(i);
}
