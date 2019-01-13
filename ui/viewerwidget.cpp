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
#include "viewerwidget.h"

#include "panels/panels.h"
#include "panels/viewer.h"
#include "panels/timeline.h"
#include "panels/project.h"
#include "project/sequence.h"
#include "project/effect.h"
#include "project/transition.h"
#include "playback/playback.h"
#include "playback/audio.h"
#include "project/footage.h"
#include "io/config.h"
#include "debug.h"
#include "io/math.h"
#include "ui/collapsiblewidget.h"
#include "project/undo.h"
#include "project/media.h"
#include "ui/viewercontainer.h"
#include "io/avtogl.h"
#include "ui/timelinewidget.h"

#include <QPainter>
#include <QAudioOutput>
#include <QOpenGLShaderProgram>
#include <QtMath>
#include <QOpenGLFramebufferObject>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QMenu>
#include <QOffscreenSurface>
#include <QFileDialog>
#include <QPolygon>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QApplication>
#include <QDebug>

extern "C" {
	#include <libavformat/avformat.h>
}

#define GL_DEFAULT_BLEND glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

namespace {
    const int RETRY_INTERVAL = 50;
    const int SURFACE_DEPTH = 24;
    const double DEFAULT_WAVEFORM_ZOOM = 1.0;
}

ViewerWidget::ViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
    default_fbo(nullptr),
	waveform(false),
	dragging(false),
    selected_gizmo(nullptr),
    waveform_zoom(DEFAULT_WAVEFORM_ZOOM),
	waveform_scroll(0)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);

	QSurfaceFormat format;
    format.setDepthBufferSize(SURFACE_DEPTH);
	setFormat(format);

    // error handler - retries if we couldn't get the entire image
    retry_timer.setInterval(RETRY_INTERVAL);
	connect(&retry_timer, SIGNAL(timeout()), this, SLOT(retry()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_context_menu()));
}

void ViewerWidget::delete_function() {
	// destroy all textures as well
    if (viewer->seq != nullptr) {
		makeCurrent();
		closeActiveClips(viewer->seq);
		doneCurrent();
	}
}

void ViewerWidget::set_waveform_scroll(int s) {
	if (waveform) {
		waveform_scroll = s;
		update();
	}
}

void ViewerWidget::show_context_menu() {
	QMenu menu(this);

	QAction* save_frame_as_image = menu.addAction("Save Frame as Image...");
	connect(save_frame_as_image, SIGNAL(triggered(bool)), this, SLOT(save_frame()));

	QAction* show_fullscreen_action = menu.addAction("Show Fullscreen");
	connect(show_fullscreen_action, SIGNAL(triggered()), this, SLOT(show_fullscreen()));

	QMenu zoom_menu("Zoom");
	QAction* fit_zoom = zoom_menu.addAction("Fit");
	connect(fit_zoom, SIGNAL(triggered(bool)), this, SLOT(set_fit_zoom()));
	zoom_menu.addAction("10%")->setData(0.1);
	zoom_menu.addAction("25%")->setData(0.25);
	zoom_menu.addAction("50%")->setData(0.5);
	zoom_menu.addAction("75%")->setData(0.75);
	zoom_menu.addAction("100%")->setData(1.0);
	zoom_menu.addAction("150%")->setData(1.5);
	zoom_menu.addAction("200%")->setData(2.0);
	zoom_menu.addAction("400%")->setData(4.0);
	QAction* custom_zoom = zoom_menu.addAction("Custom");
	connect(custom_zoom, SIGNAL(triggered(bool)), this, SLOT(set_custom_zoom()));
	connect(&zoom_menu, SIGNAL(triggered(QAction*)), this, SLOT(set_menu_zoom(QAction*)));
	menu.addMenu(&zoom_menu);

	menu.exec(QCursor::pos());
}

void ViewerWidget::save_frame() {
	QFileDialog fd(this);
	fd.setAcceptMode(QFileDialog::AcceptSave);
	fd.setFileMode(QFileDialog::AnyFile);
	fd.setWindowTitle("Save Frame");
	fd.setNameFilter("Portable Network Graphic (*.png);;JPEG (*.jpg);;Windows Bitmap (*.bmp);;Portable Pixmap (*.ppm);;X11 Bitmap (*.xbm);;X11 Pixmap (*.xpm)");

	if (fd.exec()) {
		QString fn = fd.selectedFiles().at(0);
		QString selected_ext = fd.selectedNameFilter().mid(fd.selectedNameFilter().indexOf(QRegExp("\\*.[a-z][a-z][a-z]")) + 1, 4);
		if (!fn.endsWith(selected_ext,  Qt::CaseInsensitive)) {
			fn += selected_ext;
		}
        QOpenGLFramebufferObject fbo(viewer->seq->getWidth(), viewer->seq->getHeight(), QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_RECTANGLE);

        e_rendering = true;
		fbo.bind();

		default_fbo = &fbo;

		paintGL();

        QImage img(viewer->seq->getWidth(), viewer->seq->getHeight(), QImage::Format_RGBA8888);
		glReadPixels(0, 0, img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
		img.save(fn);

		fbo.release();
        default_fbo = nullptr;
        e_rendering = false;
	}
}

void ViewerWidget::show_fullscreen() {
	showFullScreen();
}

void ViewerWidget::set_fit_zoom() {
	container->fit = true;
	container->adjust();
}

void ViewerWidget::set_custom_zoom() {
	bool ok;
	double d = QInputDialog::getDouble(this, "Viewer Zoom", "Set Custom Zoom Value:", container->zoom*100, 0, 2147483647, 2, &ok);
	if (ok) {
		container->fit = false;
		container->zoom = d*0.01;
		container->adjust();
	}
}

void ViewerWidget::set_menu_zoom(QAction* action) {
	const QVariant& data = action->data();
	if (!data.isNull()) {
		container->fit = false;
		container->zoom = data.toDouble();
		container->adjust();
	}
}

void ViewerWidget::retry() {
	update();
}

void ViewerWidget::initializeGL() {
	initializeOpenGLFunctions();

	connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(delete_function()), Qt::DirectConnection);

	retry_timer.start();
}

//void ViewerWidget::resizeGL(int w, int h)
//{
//}

void ViewerWidget::paintEvent(QPaintEvent *e) {
    if (!e_rendering) {
        QOpenGLContext* const ctxt = context();
        if (ctxt != nullptr) {
            if (ctxt->thread() == this->thread()) {
		makeCurrent();
		QOpenGLWidget::paintEvent(e);
	}
        } else {
            // FIXME: segfault from now on in
}

	}
}

void ViewerWidget::seek_from_click(int x) {
	viewer->seek(getFrameFromScreenPoint(waveform_zoom, x+waveform_scroll));
}

EffectGizmoPtr ViewerWidget::get_gizmo_from_mouse(const int x_coord, const int y_coord) {
    if (gizmos != nullptr) {
        double multiplier = (double) viewer->seq->getWidth() / (double) width();
        QPoint mouse_pos(qRound(x_coord*multiplier), qRound(y_coord*multiplier));
		int dot_size = 2 * qRound(GIZMO_DOT_SIZE * multiplier);
		int target_size = 2 * qRound(GIZMO_TARGET_SIZE * multiplier);
		for (int i=0;i<gizmos->gizmo_count();i++) {
            EffectGizmoPtr g = gizmos->gizmo(i);

			switch (g->get_type()) {
			case GIZMO_TYPE_DOT:
				if (mouse_pos.x() > g->screen_pos[0].x() - dot_size
						&& mouse_pos.y() > g->screen_pos[0].y() - dot_size
						&& mouse_pos.x() < g->screen_pos[0].x() + dot_size
						&& mouse_pos.y() < g->screen_pos[0].y() + dot_size) {
					return g;
				}
				break;
			case GIZMO_TYPE_POLY:
				if (QPolygon(g->screen_pos).containsPoint(mouse_pos, Qt::OddEvenFill)) {
					return g;
				}
				break;
			case GIZMO_TYPE_TARGET:
				if (mouse_pos.x() > g->screen_pos[0].x() - target_size
						&& mouse_pos.y() > g->screen_pos[0].y() - target_size
						&& mouse_pos.x() < g->screen_pos[0].x() + target_size
						&& mouse_pos.y() < g->screen_pos[0].y() + target_size) {
					return g;
				}
				break;
			}

		}
	}
    return nullptr;
}

void ViewerWidget::move_gizmos(QMouseEvent *event, bool done) {
    if (selected_gizmo != nullptr) {
        double multiplier = (double) viewer->seq->getWidth() / (double) width();

		int x_movement = (event->pos().x() - drag_start_x)*multiplier;
		int y_movement = (event->pos().y() - drag_start_y)*multiplier;

        gizmos->gizmo_move(selected_gizmo, x_movement, y_movement, gizmos->parent_clip->get_timecode(gizmos->parent_clip->sequence->playhead), done);

		gizmo_x_mvmt += x_movement;
		gizmo_y_mvmt += y_movement;

		drag_start_x = event->pos().x();
		drag_start_y = event->pos().y();

		gizmos->field_changed();
	}
}

void ViewerWidget::mousePressEvent(QMouseEvent* event) {
	if (waveform) {
		seek_from_click(event->x());
    } else if (event->buttons() & Qt::MiddleButton || e_panel_timeline->tool == TIMELINE_TOOL_HAND) {
		container->dragScrollPress(event->pos());
	} else {
		drag_start_x = event->pos().x();
		drag_start_y = event->pos().y();

		gizmo_x_mvmt = 0;
		gizmo_y_mvmt = 0;

		selected_gizmo = get_gizmo_from_mouse(event->pos().x(), event->pos().y());

        if (selected_gizmo != nullptr) {
			selected_gizmo->set_previous_value();
		}
	}
	dragging = true;
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* event) {
	unsetCursor();
    if (e_panel_timeline->tool == TIMELINE_TOOL_HAND) {
		setCursor(Qt::OpenHandCursor);
	}
	if (dragging) {
		if (waveform) {
			seek_from_click(event->x());
        } else if (event->buttons() & Qt::MiddleButton || e_panel_timeline->tool == TIMELINE_TOOL_HAND) {
			container->dragScrollMove(event->pos());
        } else if (gizmos == nullptr) {
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;
			mimeData->setText("h"); // QMimeData will fail without some kind of data
			drag->setMimeData(mimeData);
			drag->exec();
			dragging = false;
		} else {
			move_gizmos(event, false);
		}
	} else {
        EffectGizmoPtr g = get_gizmo_from_mouse(event->pos().x(), event->pos().y());
        if (g != nullptr) {
			if (g->get_cursor() > -1) {
				setCursor(static_cast<enum Qt::CursorShape>(g->get_cursor()));
			}
		}
	}
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (dragging) move_gizmos(event, true);
	dragging = false;
}

void ViewerWidget::drawTitleSafeArea() {
	double halfWidth = 0.5;
	double halfHeight = 0.5;
	double viewportAr = (double) width() / (double) height();
	double halfAr = viewportAr*0.5;

    if (e_config.use_custom_title_safe_ratio && e_config.custom_title_safe_ratio > 0) {
        if (e_config.custom_title_safe_ratio > viewportAr) {
            halfHeight = (e_config.custom_title_safe_ratio/viewportAr)*0.5;
		} else {
            halfWidth = (viewportAr/e_config.custom_title_safe_ratio)*0.5;
		}
	}

	glLoadIdentity();
	glOrtho(-halfWidth, halfWidth, halfHeight, -halfHeight, 0, 1);

	glColor4f(0.66f, 0.66f, 0.66f, 1.0f);
	glBegin(GL_LINES);

	// action safe rectangle
	glVertex2d(-0.45, -0.45);
	glVertex2d(0.45, -0.45);
	glVertex2d(0.45, -0.45);
	glVertex2d(0.45, 0.45);
	glVertex2d(0.45, 0.45);
	glVertex2d(-0.45, 0.45);
	glVertex2d(-0.45, 0.45);
	glVertex2d(-0.45, -0.45);

	// title safe rectangle
	glVertex2d(-0.4, -0.4);
	glVertex2d(0.4, -0.4);
	glVertex2d(0.4, -0.4);
	glVertex2d(0.4, 0.4);
	glVertex2d(0.4, 0.4);
	glVertex2d(-0.4, 0.4);
	glVertex2d(-0.4, 0.4);
	glVertex2d(-0.4, -0.4);

	// horizontal centers
	glVertex2d(-0.45, 0);
	glVertex2d(-0.375, 0);
	glVertex2d(0.45, 0);
	glVertex2d(0.375, 0);

	// vertical centers
	glVertex2d(0, -0.45);
	glVertex2d(0, -0.375);
	glVertex2d(0, 0.45);
	glVertex2d(0, 0.375);

	glEnd();

	// center cross
	glLoadIdentity();
	glOrtho(-halfAr, halfAr, 0.5, -0.5, -1, 1);

	glBegin(GL_LINES);

	glVertex2d(-0.05, 0);
	glVertex2d(0.05, 0);
	glVertex2d(0, -0.05);
	glVertex2d(0, 0.05);

	glEnd();
}

GLuint ViewerWidget::draw_clip(QOpenGLFramebufferObject* fbo, GLuint texture, bool clear) {
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);

	fbo->bind();

	if (clear) glClear(GL_COLOR_BUFFER_BIT);

	// get current blend mode
	GLint src_rgb, src_alpha, dst_rgb, dst_alpha;
	glGetIntegerv(GL_BLEND_SRC_RGB, &src_rgb);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &src_alpha);
	glGetIntegerv(GL_BLEND_DST_RGB, &dst_rgb);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &dst_alpha);

	GL_DEFAULT_BLEND

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); // top left
	glVertex2f(0, 0); // top left
	glTexCoord2f(1, 0); // top right
	glVertex2f(1, 0); // top right
	glTexCoord2f(1, 1); // bottom right
	glVertex2f(1, 1); // bottom right
	glTexCoord2f(0, 1); // bottom left
	glVertex2f(0, 1); // bottom left
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	fbo->release();

	// restore previous blendFunc
	glBlendFuncSeparate(src_rgb, dst_rgb, src_alpha, dst_alpha);

    if (default_fbo != nullptr) default_fbo->bind();

	glPopMatrix();
	return fbo->texture();
}

void ViewerWidget::process_effect(ClipPtr c, EffectPtr e, double timecode,
                                  GLTextureCoords& coords, GLuint& composite_texture, bool& fbo_switcher, int data) {
	if (e->is_enabled()) {
		if (e->enable_coords) {
			e->process_coords(timecode, coords, data);
		}
		if (e->enable_shader || e->enable_superimpose) {
			e->startEffect();
			if (e->enable_shader && e->is_glsl_linked()) {
				e->process_shader(timecode, coords);
				composite_texture = draw_clip(c->fbo[fbo_switcher], composite_texture, true);
				fbo_switcher = !fbo_switcher;
			}
			if (e->enable_superimpose) {
				GLuint superimpose_texture = e->process_superimpose(timecode);
				if (superimpose_texture == 0) {
                    dout << "[WARNING] Superimpose texture was nullptr, retrying...";
                    e_texture_failed = true;
				} else {
					composite_texture = draw_clip(c->fbo[!fbo_switcher], superimpose_texture, false);
				}
			}
			e->endEffect();
		}
	}
}

GLuint ViewerWidget::compose_sequence(QVector<ClipPtr>& nests, bool render_audio) {
    SequencePtr s = viewer->seq;
	long playhead = s->playhead;

	if (!nests.isEmpty()) {
		for (int i=0;i<nests.size();i++) {
            s = nests.at(i)->timeline_info.media->get_object<Sequence>();
            playhead += nests.at(i)->timeline_info.clip_in - nests.at(i)->get_timeline_in_with_transition();
            playhead = refactor_frame_number(playhead, nests.at(i)->sequence->getFrameRate(), s->getFrameRate());
		}

        if (nests.last()->fbo != nullptr) {
			nests.last()->fbo[0]->bind();
			glClear(GL_COLOR_BUFFER_BIT);
			nests.last()->fbo[0]->release();
		}
	}

	int audio_track_count = 0;

    QVector<ClipPtr> current_clips;

	for (int i=0;i<s->clips.size();i++) {
        ClipPtr c = s->clips.at(i);

		// if clip starts within one second and/or hasn't finished yet
        if (c != nullptr) {
            if (!(!nests.isEmpty() && !same_sign(c->timeline_info.track, nests.last()->timeline_info.track))) {
				bool clip_is_active = false;

                if (c->timeline_info.media != nullptr && c->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
                    FootagePtr m = c->timeline_info.media->get_object<Footage>();
                    if (!m->invalid && !(c->timeline_info.track >= 0 && !is_audio_device_set())) {
						if (m->ready) {
                            const FootageStream* ms = m->get_stream_from_file_index(c->timeline_info.track < 0, c->timeline_info.media_stream);
                            if ( (ms != nullptr) && c->isActive(playhead) ) {
								// if thread is already working, we don't want to touch this,
								// but we also don't want to hang the UI thread
                                if (!c->is_open) {
                                    c->open(!e_rendering);
								}
								clip_is_active = true;
                                if (c->timeline_info.track >= 0) {
                                    audio_track_count++;
                                }
                            } else if (c->is_open) {
                                c->close(false);
							}
						} else {
							//dout << "[WARNING] Media '" + m->name + "' was not ready, retrying...";
                            e_texture_failed = true;
						}
					}
				} else {
                    if (c->isActive(playhead)) {
                        if (!c->is_open) {
                            c->open(!e_rendering);
                        }
						clip_is_active = true;
                    } else if (c->is_open) {
                        c->close(false);
					}
				}
				if (clip_is_active) {
					bool added = false;
					for (int j=0;j<current_clips.size();j++) {
                        if (current_clips.at(j)->timeline_info.track < c->timeline_info.track) {
							current_clips.insert(j, c);
							added = true;
							break;
						}
					}
					if (!added) {
						current_clips.append(c);
					}
				}
			}
		}
	}

    int half_width = s->getWidth()/2;
    int half_height = s->getHeight()/2;
    if (e_rendering || !nests.isEmpty()) half_height = -half_height; // invert vertical

	glPushMatrix();
	glLoadIdentity();
	glOrtho(-half_width, half_width, half_height, -half_height, -1, 10);

	for (int i=0;i<current_clips.size();i++) {
		GL_DEFAULT_BLEND
		glColor4f(1.0, 1.0, 1.0, 1.0);

        ClipPtr clipNow = current_clips.at(i);

        if (clipNow->timeline_info.media != nullptr
                && clipNow->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE
                && !clipNow->finished_opening) {
			dout << "[WARNING] Tried to display clip" << i << "but it's closed";
            e_texture_failed = true;
		} else {
            if (clipNow->timeline_info.track < 0) {
				GLuint textureID = 0;
                int video_width = clipNow->getWidth();
                int video_height = clipNow->getHeight();

                if (clipNow->timeline_info.media != nullptr) {
                    switch (clipNow->timeline_info.media->get_type()) {
					case MEDIA_TYPE_FOOTAGE:
						// set up opengl texture
                        if (clipNow->texture == nullptr) {
                            clipNow->texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
                            clipNow->texture->setSize(clipNow->media_handling.stream->codecpar->width,
                                                      clipNow->media_handling.stream->codecpar->height);
                            clipNow->texture->setFormat(get_gl_tex_fmt_from_av(clipNow->pix_fmt));
                            clipNow->texture->setMipLevels(clipNow->texture->maximumMipLevels());
                            clipNow->texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
                            clipNow->texture->allocateStorage(get_gl_pix_fmt_from_av(clipNow->pix_fmt), QOpenGLTexture::UInt8);
						}
                        clipNow->get_frame(playhead);
                        textureID = clipNow->texture->textureId();
						break;
					case MEDIA_TYPE_SEQUENCE:
						textureID = -1;
						break;
                    default:
                        //TODO: log/something
                        break;
                    }//switch
				}

                if (textureID == 0 && clipNow->timeline_info.media != nullptr) {
					dout << "[WARNING] Texture hasn't been created yet";
                    e_texture_failed = true;
                } else if (playhead >= clipNow->get_timeline_in_with_transition()) {
					glPushMatrix();

					// start preparing cache
                    if (clipNow->fbo == nullptr) {
                        clipNow->fbo = new QOpenGLFramebufferObject* [2];
                        clipNow->fbo[0] = new QOpenGLFramebufferObject(video_width, video_height);
                        clipNow->fbo[1] = new QOpenGLFramebufferObject(video_width, video_height);
					}

					// clear fbos
					/*c->fbo[0]->bind();
					glClear(GL_COLOR_BUFFER_BIT);
					c->fbo[0]->release();
					c->fbo[1]->bind();
					glClear(GL_COLOR_BUFFER_BIT);
					c->fbo[1]->release();*/

					bool fbo_switcher = false;

					glViewport(0, 0, video_width, video_height);

					GLuint composite_texture;

                    if (clipNow->timeline_info.media == nullptr) {
                        clipNow->fbo[fbo_switcher]->bind();
						glClear(GL_COLOR_BUFFER_BIT);
                        clipNow->fbo[fbo_switcher]->release();
                        composite_texture = clipNow->fbo[fbo_switcher]->texture();
					} else {
						// for nested sequences
                        if (clipNow->timeline_info.media->get_type()== MEDIA_TYPE_SEQUENCE) {
                            nests.append(clipNow);
							textureID = compose_sequence(nests, render_audio);
							nests.removeLast();
							fbo_switcher = true;
						}

                        composite_texture = draw_clip(clipNow->fbo[fbo_switcher], textureID, true);
					}

					fbo_switcher = !fbo_switcher;

					// set up default coords
					GLTextureCoords coords;
					coords.grid_size = 1;
					coords.vertexTopLeftX = coords.vertexBottomLeftX = -video_width/2;
					coords.vertexTopLeftY = coords.vertexTopRightY = -video_height/2;
					coords.vertexTopRightX = coords.vertexBottomRightX = video_width/2;
					coords.vertexBottomLeftY = coords.vertexBottomRightY = video_height/2;
					coords.vertexBottomLeftZ = coords.vertexBottomRightZ = coords.vertexTopLeftZ = coords.vertexTopRightZ = 1;
					coords.textureTopLeftY = coords.textureTopRightY = coords.textureTopLeftX = coords.textureBottomLeftX = 0.0;
					coords.textureBottomLeftY = coords.textureBottomRightY = coords.textureTopRightX = coords.textureBottomRightX = 1.0;
					coords.textureTopLeftQ = coords.textureTopRightQ = coords.textureTopLeftQ = coords.textureBottomLeftQ = 1;

					// set up autoscale
                    if (clipNow->timeline_info.autoscale && (video_width != s->getWidth() && video_height != s->getHeight())) {
                        float width_multiplier = (float) s->getWidth() / (float) video_width;
                        float height_multiplier = (float) s->getHeight() / (float) video_height;
						float scale_multiplier = qMin(width_multiplier, height_multiplier);
						glScalef(scale_multiplier, scale_multiplier, 1);
					}

					// EFFECT CODE START
                    double timecode = clipNow->get_timecode(playhead);

                    EffectPtr first_gizmo_effect;
                    EffectPtr selected_effect;

                    for (int j=0;j<clipNow->effects.size();j++) {
                        EffectPtr e = clipNow->effects.at(j);
                        process_effect(clipNow, e, timecode, coords, composite_texture, fbo_switcher, TA_NO_TRANSITION);

						if (e->are_gizmos_enabled()) {
                            if (first_gizmo_effect == nullptr) first_gizmo_effect = e;
							if (e->container->selected) selected_effect = e;
						}
					}

                    if (!e_rendering) {
                        if (selected_effect != nullptr) {
							gizmos = selected_effect;
                        } else if (e_panel_timeline->is_clip_selected(clipNow, true)) {
							gizmos = first_gizmo_effect;
						}
					}

                    if (clipNow->get_opening_transition() != nullptr) {
                        int transition_progress = playhead - clipNow->get_timeline_in_with_transition();
                        if (transition_progress < clipNow->get_opening_transition()->get_length()) {
                            // FIXME:
//                            process_effect(clipNow, clipNow->get_opening_transition(),
//                                           (double)transition_progress/(double)clipNow->get_opening_transition()->get_length(),
//                                           coords, composite_texture, fbo_switcher, TA_OPENING_TRANSITION);
						}
					}

                    if (clipNow->get_closing_transition() != nullptr) {
                        int transition_progress = playhead - (clipNow->get_timeline_out_with_transition() - clipNow->get_closing_transition()->get_length());
                        if (transition_progress >= 0 && transition_progress < clipNow->get_closing_transition()->get_length()) {
                            //FIXME:
//                            process_effect(clipNow, clipNow->get_closing_transition(), (double)transition_progress/(double)clipNow->get_closing_transition()->get_length(), coords, composite_texture, fbo_switcher, TA_CLOSING_TRANSITION);
						}
					}
					// EFFECT CODE END

					if (!nests.isEmpty()) {
						nests.last()->fbo[0]->bind();
                        glViewport(0, 0, s->getWidth(), s->getHeight());
                    } else if (e_rendering) {
                        glViewport(0, 0, s->getWidth(), s->getHeight());
					} else {
						int widget_width = width();
						int widget_height = height();

						widget_width *= QApplication::desktop()->devicePixelRatio();
						widget_height *= QApplication::desktop()->devicePixelRatio();

						glViewport(0, 0, widget_width, widget_height);
					}

					glBindTexture(GL_TEXTURE_2D, composite_texture);

					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					glBegin(GL_QUADS);

					if (coords.grid_size <= 1) {
						float z = 0.0f;

						glTexCoord2f(coords.textureTopLeftX, coords.textureTopLeftY); // top left
						glVertex3f(coords.vertexTopLeftX, coords.vertexTopLeftY, z); // top left
						glTexCoord2f(coords.textureTopRightX, coords.textureTopRightY); // top right
						glVertex3f(coords.vertexTopRightX, coords.vertexTopRightY, z); // top right
						glTexCoord2f(coords.textureBottomRightX, coords.textureBottomRightY); // bottom right
						glVertex3f(coords.vertexBottomRightX, coords.vertexBottomRightY, z); // bottom right
						glTexCoord2f(coords.textureBottomLeftX, coords.textureBottomLeftY); // bottom left
						glVertex3f(coords.vertexBottomLeftX, coords.vertexBottomLeftY, z); // bottom left
					} else {
						float rows = coords.grid_size;
						float cols = coords.grid_size;

						for (float k=0;k<rows;k++) {
							float row_prog = k/rows;
							float next_row_prog = (k+1)/rows;
							for (float j=0;j<cols;j++) {
								float col_prog = j/cols;
								float next_col_prog = (j+1)/cols;

								float vertexTLX = float_lerp(coords.vertexTopLeftX, coords.vertexBottomLeftX, row_prog);
								float vertexTRX = float_lerp(coords.vertexTopRightX, coords.vertexBottomRightX, row_prog);
								float vertexBLX = float_lerp(coords.vertexTopLeftX, coords.vertexBottomLeftX, next_row_prog);
								float vertexBRX = float_lerp(coords.vertexTopRightX, coords.vertexBottomRightX, next_row_prog);

								float vertexTLY = float_lerp(coords.vertexTopLeftY, coords.vertexTopRightY, col_prog);
								float vertexTRY = float_lerp(coords.vertexTopLeftY, coords.vertexTopRightY, next_col_prog);
								float vertexBLY = float_lerp(coords.vertexBottomLeftY, coords.vertexBottomRightY, col_prog);
								float vertexBRY = float_lerp(coords.vertexBottomLeftY, coords.vertexBottomRightY, next_col_prog);

								glTexCoord2f(float_lerp(coords.textureTopLeftX, coords.textureTopRightX, col_prog), float_lerp(coords.textureTopLeftY, coords.textureBottomLeftY, row_prog)); // top left
								glVertex2f(float_lerp(vertexTLX, vertexTRX, col_prog), float_lerp(vertexTLY, vertexBLY, row_prog)); // top left
								glTexCoord2f(float_lerp(coords.textureTopLeftX, coords.textureTopRightX, next_col_prog), float_lerp(coords.textureTopRightY, coords.textureBottomRightY, row_prog)); // top right
								glVertex2f(float_lerp(vertexTLX, vertexTRX, next_col_prog), float_lerp(vertexTRY, vertexBRY, row_prog)); // top right
								glTexCoord2f(float_lerp(coords.textureBottomLeftX, coords.textureBottomRightX, next_col_prog), float_lerp(coords.textureTopRightY, coords.textureBottomRightY, next_row_prog)); // bottom right
								glVertex2f(float_lerp(vertexBLX, vertexBRX, next_col_prog), float_lerp(vertexTRY, vertexBRY, next_row_prog)); // bottom right
								glTexCoord2f(float_lerp(coords.textureBottomLeftX, coords.textureBottomRightX, col_prog), float_lerp(coords.textureTopLeftY, coords.textureBottomLeftY, next_row_prog)); // bottom left
								glVertex2f(float_lerp(vertexBLX, vertexBRX, col_prog), float_lerp(vertexTLY, vertexBLY, next_row_prog)); // bottom left
							}
						}
					}

					glEnd();

					glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

                    if (gizmos != nullptr && !drawn_gizmos) {
						gizmos->gizmo_draw(timecode, coords); // set correct gizmo coords
						gizmos->gizmo_world_to_screen();

						drawn_gizmos = true;
					}

					if (!nests.isEmpty()) {
						nests.last()->fbo[0]->release();
                        if (default_fbo != nullptr) default_fbo->bind();
					}

					glPopMatrix();

					/*GLfloat motion_blur_frac = (GLfloat) motion_blur_prog / (GLfloat) motion_blur_lim;
					if (motion_blur_prog == 0) {
						glAccum(GL_LOAD, motion_blur_frac);
					} else {
						glAccum(GL_ACCUM, motion_blur_frac);
					}
					motion_blur_prog++;*/
				}
			} else {
                if (render_audio || (e_config.enable_audio_scrubbing && audio_scrub)) {
                    if (clipNow->timeline_info.media != nullptr &&
                            clipNow->timeline_info.media->get_type() == MEDIA_TYPE_SEQUENCE) {
                        nests.append(clipNow);
						compose_sequence(nests, render_audio);
						nests.removeLast();
					} else {
                        if (clipNow->lock.tryLock()) {
							// clip is not caching, start caching audio
                            clipNow->cache(playhead, clipNow->audio_playback.reset, !render_audio, nests);
                            clipNow->lock.unlock();
						}
					}
				}

				// visually update all the keyframe values
                if (clipNow->sequence == viewer->seq) { // only if you can currently see them
                    double ts = (playhead - clipNow->get_timeline_in_with_transition() + clipNow->get_clip_in_with_transition())/s->getFrameRate();
                    for (int i=0;i<clipNow->effects.size();i++) {
                        EffectPtr e = clipNow->effects.at(i);
						for (int j=0;j<e->row_count();j++) {
                            EffectRowPtr r = e->row(j);
							for (int k=0;k<r->fieldCount();k++) {
								r->field(k)->validate_keyframe_data(ts);
							}
						}
					}
				}
			}
		}
	}

	if (audio_track_count == 0) {
		viewer->play_wake();
	}

	glPopMatrix();

    if (!nests.isEmpty() && nests.last()->fbo != nullptr) {
		// returns nested clip's texture
		return nests.last()->fbo[0]->texture();
	}

	return 0;
}

void ViewerWidget::paintGL() {
	drawn_gizmos = false;
	force_quit = false;
    if (viewer->seq != nullptr) {
        gizmos = nullptr;

        bool render_audio = (viewer->playing || e_rendering);
		bool loop = false;
		do {
			loop = false;

			glClearColor(0, 0, 0, 1);
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glEnable(GL_DEPTH);

            e_texture_failed = false;

            if (!e_rendering) retry_timer.stop();

			glClear(GL_COLOR_BUFFER_BIT);

			// compose video preview
			glClearColor(0, 0, 0, 0);

            QVector<ClipPtr> nests;

			compose_sequence(nests, render_audio);

			if (waveform) {
				QPainter p(this);
				if (viewer->seq->using_workarea) {
					int in_x = getScreenPointFromFrame(waveform_zoom, viewer->seq->workarea_in) - waveform_scroll;
					int out_x = getScreenPointFromFrame(waveform_zoom, viewer->seq->workarea_out) - waveform_scroll;

					p.fillRect(QRect(in_x, 0, out_x - in_x, height()), QColor(255, 255, 255, 64));
					p.setPen(Qt::white);
					p.drawLine(in_x, 0, in_x, height());
					p.drawLine(out_x, 0, out_x, height());
				}
				QRect wr = rect();
				wr.setX(wr.x() - waveform_scroll);

				p.setPen(Qt::green);
                draw_waveform(waveform_clip, waveform_ms, waveform_clip->timeline_info.out, &p, wr, waveform_scroll, width()+waveform_scroll, waveform_zoom);
				p.setPen(Qt::red);
				int playhead_x = getScreenPointFromFrame(waveform_zoom, viewer->seq->playhead) - waveform_scroll;
				p.drawLine(playhead_x, 0, playhead_x, height());
			}

			if (force_quit) break;
            if (e_texture_failed) {
                if (e_rendering) {
					dout << "[INFO] Texture failed - looping";
					loop = true;
				} else {
					retry_timer.start();
				}
			}

            if (e_config.show_title_safe_area && !e_rendering) {
				drawTitleSafeArea();
			}

            if (gizmos != nullptr && drawn_gizmos) {
				float color[4];
				glGetFloatv(GL_CURRENT_COLOR, color);

                float dot_size = GIZMO_DOT_SIZE / width() * viewer->seq->getWidth();
                float target_size = GIZMO_TARGET_SIZE / width() * viewer->seq->getWidth();

				glPushMatrix();
				glLoadIdentity();
                glOrtho(0, viewer->seq->getWidth(), viewer->seq->getHeight(), 0, -1, 10);
				float gizmo_z = 0.0f;
				for (int j=0;j<gizmos->gizmo_count();j++) {
                    EffectGizmoPtr g = gizmos->gizmo(j);
					glColor4f(g->color.redF(), g->color.greenF(), g->color.blueF(), 1.0);
					switch (g->get_type()) {
					case GIZMO_TYPE_DOT: // draw dot
						glBegin(GL_QUADS);
						glVertex3f(g->screen_pos[0].x()-dot_size, g->screen_pos[0].y()-dot_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()+dot_size, g->screen_pos[0].y()-dot_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()+dot_size, g->screen_pos[0].y()+dot_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()-dot_size, g->screen_pos[0].y()+dot_size, gizmo_z);
						glEnd();
						break;
					case GIZMO_TYPE_POLY: // draw lines
						glBegin(GL_LINES);
						for (int k=1;k<g->get_point_count();k++) {
							glVertex3f(g->screen_pos[k-1].x(), g->screen_pos[k-1].y(), gizmo_z);
							glVertex3f(g->screen_pos[k].x(), g->screen_pos[k].y(), gizmo_z);
						}
						glVertex3f(g->screen_pos[g->get_point_count()-1].x(), g->screen_pos[g->get_point_count()-1].y(), gizmo_z);
						glVertex3f(g->screen_pos[0].x(), g->screen_pos[0].y(), gizmo_z);
						glEnd();
						break;
					case GIZMO_TYPE_TARGET: // draw target
						glBegin(GL_LINES);
						glVertex3f(g->screen_pos[0].x()-target_size, g->screen_pos[0].y()-target_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()+target_size, g->screen_pos[0].y()-target_size, gizmo_z);

						glVertex3f(g->screen_pos[0].x()+target_size, g->screen_pos[0].y()-target_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()+target_size, g->screen_pos[0].y()+target_size, gizmo_z);

						glVertex3f(g->screen_pos[0].x()+target_size, g->screen_pos[0].y()+target_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()-target_size, g->screen_pos[0].y()+target_size, gizmo_z);

						glVertex3f(g->screen_pos[0].x()-target_size, g->screen_pos[0].y()+target_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x()-target_size, g->screen_pos[0].y()-target_size, gizmo_z);

						glVertex3f(g->screen_pos[0].x()-target_size, g->screen_pos[0].y(), gizmo_z);
						glVertex3f(g->screen_pos[0].x()+target_size, g->screen_pos[0].y(), gizmo_z);

						glVertex3f(g->screen_pos[0].x(), g->screen_pos[0].y()-target_size, gizmo_z);
						glVertex3f(g->screen_pos[0].x(), g->screen_pos[0].y()+target_size, gizmo_z);
						glEnd();
						break;
					}
				}
				glPopMatrix();

				glColor4f(color[0], color[1], color[2], color[3]);

				drawn_gizmos = true;
			}

			glDisable(GL_DEPTH);
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		} while (loop);
	}
}
