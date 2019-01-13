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

#include "debug.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
}

QString get_interlacing_name(int interlacing) {
	switch (interlacing) {
	case VIDEO_PROGRESSIVE: return "None (Progressive)";
	case VIDEO_TOP_FIELD_FIRST: return "Top Field First";
	case VIDEO_BOTTOM_FIELD_FIRST: return "Bottom Field First";
	default: return "Invalid";
	}
}

QString get_channel_layout_name(int channels, uint64_t layout) {
	switch (channels) {
	case 0: return "Invalid"; break;
	case 1: return "Mono"; break;
	case 2: return "Stereo"; break;
	default: {
		char buf[50];
		av_get_channel_layout_string(buf, sizeof(buf), channels, layout);
		return QString(buf);
	}
	}
}

Media::Media(MediaPtr iparent) :
    throbber(nullptr),
    root(false),
    type(MEDIA_TYPE_NONE),
    parent(iparent)
{

}

Media::~Media() {
    if (throbber != nullptr) {
        delete throbber;
    }
}

void Media::clear_object() {
    type = MEDIA_TYPE_NONE;
    object = nullptr;
}
void Media::set_footage(FootagePtr ftg) {
	type = MEDIA_TYPE_FOOTAGE;
    object = ftg;
}

void Media::set_sequence(SequencePtr sqn) {
	set_icon(QIcon(":/icons/sequence.png"));
	type = MEDIA_TYPE_SEQUENCE;
    object = sqn;
    if (sqn != nullptr) update_tooltip();
}

void Media::set_folder() {
	if (folder_name.isEmpty()) folder_name = "New Folder";
	set_icon(QIcon(":/icons/folder.png"));
	type = MEDIA_TYPE_FOLDER;
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
	case MEDIA_TYPE_FOOTAGE:
	{
        FootagePtr f = get_object<Footage>();
        tooltip = "Name: " + f->getName() + "\nFilename: " + f->url + "\n";

		if (error.isEmpty()) {
			if (f->video_tracks.size() > 0) {
				tooltip += "Video Dimensions: ";
				for (int i=0;i<f->video_tracks.size();i++) {
					if (i > 0) {
						tooltip += ", ";
					}
					tooltip += QString::number(f->video_tracks.at(i).video_width) + "x" + QString::number(f->video_tracks.at(i).video_height);
				}
				tooltip += "\n";

				if (!f->video_tracks.at(0).infinite_length) {
					tooltip += "Frame Rate: ";
					for (int i=0;i<f->video_tracks.size();i++) {
						if (i > 0) {
							tooltip += ", ";
						}
						if (f->video_tracks.at(i).video_interlacing == VIDEO_PROGRESSIVE) {
							tooltip += QString::number(f->video_tracks.at(i).video_frame_rate * f->speed);
						} else {
                            tooltip += QString::number(f->video_tracks.at(i).video_frame_rate * f->speed * 2);
							tooltip += " fields (" + QString::number(f->video_tracks.at(i).video_frame_rate * f->speed) + " frames)";
						}
					}
					tooltip += "\n";
				}

				tooltip += "Interlacing: ";
				for (int i=0;i<f->video_tracks.size();i++) {
					if (i > 0) {
						tooltip += ", ";
					}
					tooltip += get_interlacing_name(f->video_tracks.at(i).video_interlacing);
				}
			}

			if (f->audio_tracks.size() > 0) {
				tooltip += "\n";

				tooltip += "Audio Frequency: ";
				for (int i=0;i<f->audio_tracks.size();i++) {
					if (i > 0) {
						tooltip += ", ";
					}
					tooltip += QString::number(f->audio_tracks.at(i).audio_frequency * f->speed);
				}
				tooltip += "\n";

				tooltip += "Audio Channels: ";
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
	case MEDIA_TYPE_SEQUENCE:
	{
        SequencePtr s = get_object<Sequence>();
        tooltip = "Name: " + s->getName()
                       + "\nVideo Dimensions: " + QString::number(s->getWidth()) + "x" + QString::number(s->getHeight())
                       + "\nFrame Rate: " + QString::number(s->getFrameRate())
                       + "\nAudio Frequency: " + QString::number(s->getAudioFrequency())
                       + "\nAudio Layout: " + get_channel_layout_name(av_get_channel_layout_nb_channels(s->getAudioLayout()), s->getAudioLayout());
	}
		break;
	}

}

MediaType_E Media::get_type() const {
	return type;
}

const QString &Media::get_name() {
	switch (type) {
    case MEDIA_TYPE_FOOTAGE: return get_object<Footage>()->getName();
    case MEDIA_TYPE_SEQUENCE: return get_object<Sequence>()->getName();
	default: return folder_name;
	}
}

void Media::set_name(const QString &name) {
	switch (type) {
    case MEDIA_TYPE_FOOTAGE:
        get_object<Footage>()->setName(name);
        break;
    case MEDIA_TYPE_SEQUENCE:
        get_object<Sequence>()->setName(name);
        break;
    case MEDIA_TYPE_FOLDER:
        folder_name = name;
        break;
    default:
        dwarning << "Unknown media type" << type;
        break;
    }//switch
}

double Media::get_frame_rate(const int stream) {
	switch (get_type()) {
	case MEDIA_TYPE_FOOTAGE:
	{
        FootagePtr f = get_object<Footage>();
        if (stream < 0) {
            return f->video_tracks.at(0).video_frame_rate * f->speed;
        }
		return f->get_stream_from_file_index(true, stream)->video_frame_rate * f->speed;
	}
    case MEDIA_TYPE_SEQUENCE:
        return get_object<Sequence>()->getFrameRate();
    default:
        dwarning << "Unknown media type" << get_type();
        break;
    }//switch

    return 0.0;
}

int Media::get_sampling_rate(const int stream) {
	switch (get_type()) {
	case MEDIA_TYPE_FOOTAGE:
	{
        FootagePtr f = get_object<Footage>();
        if (stream < 0) {
            return f->audio_tracks.at(0).audio_frequency * f->speed;
        }
        return get_object<Footage>()->get_stream_from_file_index(false, stream)->audio_frequency * f->speed;
	}
    case MEDIA_TYPE_SEQUENCE:
        return get_object<Sequence>()->getAudioFrequency();
    default:
        dwarning << "Unknown media type" << get_type();
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
			if (get_type() == MEDIA_TYPE_FOOTAGE) {
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
		case 0: return (root) ? "Name" : get_name();
		case 1:
			if (root) return "Duration";
			if (get_type() == MEDIA_TYPE_SEQUENCE) {
                SequencePtr s = get_object<Sequence>();
                return frame_to_timecode(s->getEndFrame(), e_config.timecode_view, s->getFrameRate());
			}
			if (get_type() == MEDIA_TYPE_FOOTAGE) {
                FootagePtr f = get_object<Footage>();
				double r = 30;

				if (f->video_tracks.size() > 0 && !qIsNull(f->video_tracks.at(0).video_frame_rate))
					r = f->video_tracks.at(0).video_frame_rate * f->speed;

				long len = f->get_length_in_frames(r);
				if (len > 0) return frame_to_timecode(len, e_config.timecode_view, r);
			}
			break;
		case 2:
			if (root) return "Rate";
			if (get_type() == MEDIA_TYPE_SEQUENCE) return QString::number(get_frame_rate()) + " FPS";
			if (get_type() == MEDIA_TYPE_FOOTAGE) {
                FootagePtr f = get_object<Footage>();
				double r;
				if (f->video_tracks.size() > 0 && !qIsNull(r = get_frame_rate())) {
					return QString::number(get_frame_rate()) + " FPS";
				} else if (f->audio_tracks.size() > 0) {
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

int Media::row() const {
    if (!parent.expired()) {
        MediaPtr parPtr = parent.lock();
//        return parPtr->children.indexOf(shared_from_this()); //FIXME:
	}
	return 0;
}

MediaWPtr Media::parentItem() {
    return parent;
}

void Media::removeChild(int i) {
	children.removeAt(i);
}
