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
#include "playback.h"

#include <QtMath>
#include <QObject>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLFramebufferObject>

#include "project/clip.h"
#include "project/sequence.h"
#include "project/footage.h"
#include "playback/audio.h"
#include "panels/panelmanager.h"
#include "project/effect.h"
#include "project/media.h"
#include "io/config.h"
#include "io/avtogl.h"
#include "debug.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}


#ifdef QT_DEBUG
//#define GCF_DEBUG
#endif

using panels::PanelManager;

bool e_texture_failed = false;
bool e_rendering = false;

long refactor_frame_number(const long framenumber, const double source_frame_rate, const double target_frame_rate)
{
    return qRound((static_cast<double>(framenumber) / source_frame_rate) * target_frame_rate);
}



void set_sequence(const SequencePtr& s)
{
    PanelManager::fxControls().clear_effects(true);
    global::sequence = s;
    PanelManager::sequenceViewer().set_main_sequence();
    PanelManager::timeLine().setSequence(s);
    PanelManager::timeLine().update_sequence();
    PanelManager::timeLine().setFocus();
}

