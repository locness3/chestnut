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


Media::Media() :
    throbber(nullptr),
    root(false),
    type(MediaType::NONE),
    parent(std::weak_ptr<Media>()),
    id(nextID++)
{

}

Media::Media(MediaPtr iparent) :
    throbber(nullptr),
    root(false),
    type(MediaType::NONE),
    parent(iparent),
    id(nextID++)
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
int Media::getId() const {
    return id;
}

void Media::clear_object() {
    type = MediaType::NONE;
    object = nullptr;
}
void Media::set_footage(FootagePtr ftg) {
    type = MediaType::FOOTAGE;
    object = ftg;
}

void Media::set_sequence(SequencePtr sqn) {
    set_icon(QIcon(":/icons/sequence.png"));
    type = MediaType::SEQUENCE;
    object = sqn;
    if (sqn != nullptr) update_tooltip();
}

void Media::set_folder() {
    if (folder_name.isEmpty()) folder_name = QCoreApplication::translate("Media", "New Folder");
    set_icon(QIcon(":/icons/folder.png"));
    type = MediaType::FOLDER;
    object = nullptr;
}

void Media::set_icon(const QIcon &ico) {
    icon = ico;
}

void Media::set_parent(MediaWPtr p) {
    parent = p;
}

void Media::update_tooltip(const QString& error) {
    switch (type) {
    case MediaType::FOOTAGE:
    {
        FootagePtr f = get_object<Footage>();
        tooltip = QCoreApplication::translate("Media", "Name:") + " " + f->getName() + "\n" + QCoreApplication::translate("Media", "Filename:") + " " + f->url + "\n";

        if (error.isEmpty()) {
            if (f->video_tracks.size() > 0) {
                tooltip += QCoreApplication::translate("Media", "Video Dimensions:") + " ";
                for (int i=0;i<f->video_tracks.size();i++) {
                    if (i > 0) {
                        tooltip += ", ";
                    }
                    tooltip += QString::number(f->video_tracks.at(i).video_width) + "x" + QString::number(f->video_tracks.at(i).video_height);
                }
                tooltip += "\n";

                if (!f->video_tracks.at(0).infinite_length) {
                    tooltip += QCoreApplication::translate("Media", "Frame Rate:") + " ";
                    for (int i=0;i<f->video_tracks.size();i++) {
                        if (i > 0) {
                            tooltip += ", ";
                        }
                        if (f->video_tracks.at(i).video_interlacing == VIDEO_PROGRESSIVE) {
                            tooltip += QString::number(f->video_tracks.at(i).video_frame_rate * f->speed);
                        } else {
                            tooltip += QCoreApplication::translate("Media", "%1 fields (%2 frames)").arg(
                                        QString::number(f->video_tracks.at(i).video_frame_rate * f->speed * 2),
                                        QString::number(f->video_tracks.at(i).video_frame_rate * f->speed)
                                        );
                        }
                    }
                    tooltip += "\n";
                }

                tooltip += QCoreApplication::translate("Media", "Interlacing:") + " ";
                for (int i=0;i<f->video_tracks.size();i++) {
                    if (i > 0) {
                        tooltip += ", ";
                    }
                    tooltip += get_interlacing_name(f->video_tracks.at(i).video_interlacing);
                }
            }

            if (f->audio_tracks.size() > 0) {
                tooltip += "\n";

                tooltip += QCoreApplication::translate("Media", "Audio Frequency:") + " ";
                for (int i=0;i<f->audio_tracks.size();i++) {
                    if (i > 0) {
                        tooltip += ", ";
                    }
                    tooltip += QString::number(f->audio_tracks.at(i).audio_frequency * f->speed);
                }
                tooltip += "\n";

                tooltip += QCoreApplication::translate("Media", "Audio Channels:") + " ";
                for (int i=0;i<f->audio_tracks.size();i++) {
                    if (i > 0) {
                        tooltip += ", ";
                    }
                    tooltip += get_channel_layout_name(f->audio_tracks.at(i).audio_channels, f->audio_tracks.at(i).audio_layout);
                }
                // tooltip += "\n";
            }
        } else {
            tooltip += error;
        }
    }
        break;
    case MediaType::SEQUENCE:
    {
        SequencePtr s = get_object<Sequence>();

        tooltip = QCoreApplication::translate("Media", "Name: %1"
                                                       "\nVideo Dimensions: %2x%3"
                                                       "\nFrame Rate: %4"
                                                       "\nAudio Frequency: %5"
                                                       "\nAudio Layout: %6").arg(
                    s->getName(),
                    QString::number(s->getWidth()),
                    QString::number(s->getHeight()),
                    QString::number(s->getFrameRate()),
                    QString::number(s->getAudioFrequency()),
                    get_channel_layout_name(av_get_channel_layout_nb_channels(s->getAudioLayout()), s->getAudioLayout())
                    );
    }
        break;
    }

}

MediaType Media::get_type() const {
    return type;
}

const QString &Media::get_name() {
    switch (type) {
    case MediaType::FOOTAGE: return get_object<Footage>()->getName();
    case MediaType::SEQUENCE: return get_object<Sequence>()->getName();
    default: return folder_name;
    }
}

void Media::set_name(const QString &name) {
    switch (type) {
    case MediaType::FOOTAGE:
        get_object<Footage>()->setName(name);
        break;
    case MediaType::SEQUENCE:
        get_object<Sequence>()->setName(name);
        break;
    case MediaType::FOLDER:
        folder_name = name;
        break;
    default:
        qWarning() << "Unknown media type" << static_cast<int>(type);
        break;
    }//switch
}

double Media::get_frame_rate(const int stream) {
    switch (get_type()) {
    case MediaType::FOOTAGE:
    {
        if (auto ftg = get_object<Footage>()) {
            if ( (stream < 0) && !ftg->video_tracks.empty()) {
                return ftg->video_tracks.at(0).video_frame_rate * ftg->speed;
            }
            FootageStream ms;
            if (ftg->get_stream_from_file_index(true, stream, ms)) {
                return ms.video_frame_rate * ftg->speed;
            }
        }
        break;
    }
    case MediaType::SEQUENCE:
        if (auto sqn = get_object<Sequence>()) {
            return sqn->getFrameRate();
        }
        break;
    default:
        qWarning() << "Unknown media type" << static_cast<int>(get_type());
        break;
    }//switch

    return 0.0;
}

int Media::get_sampling_rate(const int stream) {
    switch (get_type()) {
    case MediaType::FOOTAGE:
    {
        if (auto ftg = get_object<Footage>()) {
            if (stream < 0) {
                return ftg->audio_tracks.at(0).audio_frequency * ftg->speed;
            }
            FootageStream ms;
            if (ftg->get_stream_from_file_index(false, stream, ms)) {
                return ms.audio_frequency * ftg->speed;
            }
        }
        break;
    }
    case MediaType::SEQUENCE:
        if (auto sqn = get_object<Sequence>()) {
            return sqn->getAudioFrequency();
        }
        break;
    default:
        qWarning() << "Unknown media type" << static_cast<int>(get_type());
        break;
    }//switch
    return 0;
}

void Media::appendChild(MediaPtr child) {
    child->set_parent(shared_from_this());
    children.append(child);
}

bool Media::setData(int col, const QVariant &value) {
    if (col == 0) {
        QString n = value.toString();
        if (!n.isEmpty() && get_name() != n) {
            e_undo_stack.push(new MediaRename(shared_from_this(), value.toString()));
            return true;
        }
    }
    return false;
}

MediaPtr Media::child(const int row) {
    return children.value(row);
}

int Media::childCount() const {
    return children.count();
}

int Media::columnCount() const {
    return 3;
}

QVariant Media::data(int column, int role) {
    switch (role) {
    case Qt::DecorationRole:
        if (column == 0) {
            if (get_type() == MediaType::FOOTAGE) {
                FootagePtr f = get_object<Footage>();
                if (f->video_tracks.size() > 0
                        && f->video_tracks.at(0).preview_done) {
                    return f->video_tracks.at(0).video_preview_square;
                }
            }

            return icon;
        }
        break;
    case Qt::DisplayRole:
        switch (column) {
        case 0: return (root) ? QCoreApplication::translate("Media", "Name") : get_name();
        case 1:
            if (root) return QCoreApplication::translate("Media", "Duration");
            if (get_type() == MediaType::SEQUENCE) {
                SequencePtr s = get_object<Sequence>();
                return frame_to_timecode(s->getEndFrame(), e_config.timecode_view, s->getFrameRate());
            }
            if (get_type() == MediaType::FOOTAGE) {
                FootagePtr f = get_object<Footage>();
                double r = 30;

                if (f->video_tracks.size() > 0 && !qIsNull(f->video_tracks.at(0).video_frame_rate))
                    r = f->video_tracks.at(0).video_frame_rate * f->speed;

                long len = f->get_length_in_frames(r);
                if (len > 0) return frame_to_timecode(len, e_config.timecode_view, r);
            }
            break;
        case 2:
            if (root) return QCoreApplication::translate("Media", "Rate");
            if (get_type() == MediaType::SEQUENCE) return QString::number(get_frame_rate()) + " FPS";
            if (get_type() == MediaType::FOOTAGE) {
                auto ftg = get_object<Footage>();
                const double rate = get_frame_rate();
                if ( (ftg->video_tracks.size() > 0) && !qIsNull(rate)) {
                    return QString::number(rate) + " FPS";
                } else if (ftg->audio_tracks.size() > 0) {
                    return QString::number(get_sampling_rate()) + " Hz";
                }
            }
            break;
        }
        break;
    case Qt::ToolTipRole:
        return tooltip;
    }
    return QVariant();
}

int Media::row() {
    if (auto parPtr = parent.lock()) {
        return parPtr->children.indexOf(shared_from_this());
    }
    return 0;
}

MediaPtr Media::parentItem() {
    return parent.lock();
}

void Media::removeChild(int i) {
    children.removeAt(i);
}
