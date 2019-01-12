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
#include "cacher.h"

#include "project/clip.h"
#include "project/sequence.h"
#include "project/transition.h"
#include "project/footage.h"
#include "playback/audio.h"
#include "playback/playback.h"
#include "project/effect.h"
#include "panels/timeline.h"
#include "panels/project.h"
#include "playback/audio.h"
#include "panels/panels.h"
#include "panels/viewer.h"
#include "project/media.h"
#include "io/config.h"
#include "debug.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavfilter/avfilter.h>
	#include <libavfilter/buffersrc.h>
	#include <libavfilter/buffersink.h>
	#include <libavutil/opt.h>
	#include <libavutil/pixdesc.h>
}

#include <QOpenGLFramebufferObject>
#include <QtMath>
#include <QAudioOutput>
#include <math.h>

// temp debug shit
//#define AUDIOWARNINGS

//int dest_format = AV_PIX_FMT_RGBA;

//double bytes_to_seconds(int nb_bytes, int nb_channels, int sample_rate) {
//	return ((double) (nb_bytes >> 1) / nb_channels / sample_rate);
//}

//void apply_audio_effects(ClipPtr c, double timecode_start, AVFrame* frame, int nb_bytes, QVector<ClipPtr> nests) {
//	// perform all audio effects
//	double timecode_end;
//	timecode_end = timecode_start + bytes_to_seconds(nb_bytes, frame->channels, frame->sample_rate);

//	for (int j=0;j<c->effects.size();j++) {
//        EffectPtr e = c->effects.at(j);
//		if (e->is_enabled()) e->process_audio(timecode_start, timecode_end, frame->data[0], nb_bytes, 2);
//	}
//	if (c->get_opening_transition() != NULL) {
//        if (c->timeline_info.media != NULL && c->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
//            double transition_start = (c->get_clip_in_with_transition() / c->sequence->getFrameRate());
//            double transition_end = (c->get_clip_in_with_transition() + c->get_opening_transition()->get_length()) / c->sequence->getFrameRate();
//			if (timecode_end < transition_end) {
//				double adjustment = transition_end - transition_start;
//				double adjusted_range_start = (timecode_start - transition_start) / adjustment;
//				double adjusted_range_end = (timecode_end - transition_start) / adjustment;
//				c->get_opening_transition()->process_audio(adjusted_range_start, adjusted_range_end, frame->data[0], nb_bytes, TA_OPENING_TRANSITION);
//			}
//		}
//	}
//	if (c->get_closing_transition() != NULL) {
//        if (c->timeline_info.media != NULL && c->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
//			long length_with_transitions = c->get_timeline_out_with_transition() - c->get_timeline_in_with_transition();
//            double transition_start = (c->get_clip_in_with_transition() + length_with_transitions - c->get_closing_transition()->get_length()) / c->sequence->getFrameRate();
//            double transition_end = (c->get_clip_in_with_transition() + length_with_transitions) / c->sequence->getFrameRate();
//			if (timecode_start > transition_start) {
//				double adjustment = transition_end - transition_start;
//				double adjusted_range_start = (timecode_start - transition_start) / adjustment;
//				double adjusted_range_end = (timecode_end - transition_start) / adjustment;
//				c->get_closing_transition()->process_audio(adjusted_range_start, adjusted_range_end, frame->data[0], nb_bytes, TA_CLOSING_TRANSITION);
//			}
//		}
//	}

//	if (!nests.isEmpty()) {
//        ClipPtr next_nest = nests.last();
//		nests.removeLast();
//        apply_audio_effects(next_nest, timecode_start + (((double)c->get_timeline_in_with_transition()-c->get_clip_in_with_transition())/c->sequence->getFrameRate()), frame, nb_bytes, nests);
//	}
//}

//#define AUDIO_BUFFER_PADDING 2048

//void cache_audio_worker(ClipPtr c, bool scrubbing, QVector<ClipPtr>& nests) {
//	long timeline_in = c->get_timeline_in_with_transition();
//	long timeline_out = c->get_timeline_out_with_transition();
//    long target_frame = c->audio_playback.target_frame;

//	long frame_skip = 0;
//    double last_fr = c->sequence->getFrameRate();
//	if (!nests.isEmpty()) {
//		for (int i=nests.size()-1;i>=0;i--) {
//            timeline_in = refactor_frame_number(timeline_in, last_fr, nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();
//            timeline_out = refactor_frame_number(timeline_out, last_fr, nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();
//            target_frame = refactor_frame_number(target_frame, last_fr, nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();

//			timeline_out = qMin(timeline_out, nests.at(i)->get_timeline_out_with_transition());

//            frame_skip = refactor_frame_number(frame_skip, last_fr, nests.at(i)->sequence->getFrameRate());

//			long validator = nests.at(i)->get_timeline_in_with_transition() - timeline_in;
//			if (validator > 0) {
//				frame_skip += validator;
//				//timeline_in = nests.at(i)->get_timeline_in_with_transition();
//			}

//            last_fr = nests.at(i)->sequence->getFrameRate();
//		}
//	}

//	while (true) {
//        AVFrame* av_frame;
//		int nb_bytes = INT_MAX;

//        if (c->timeline_info.media == NULL) {
//            av_frame = c->media_handling.frame;
//            nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;
//            while ((c->audio_playback.frame_sample_index == -1 || c->audio_playback.frame_sample_index >= nb_bytes) && nb_bytes > 0) {
//				// create "new frame"
//                memset(c->media_handling.frame->data[0], 0, nb_bytes);
//                apply_audio_effects(c, bytes_to_seconds(av_frame->pts, av_frame->channels, av_frame->sample_rate), av_frame, nb_bytes, nests);
//                c->media_handling.frame->pts += nb_bytes;
//                c->audio_playback.frame_sample_index = 0;
//                if (c->audio_playback.buffer_write == 0) {
//                    c->audio_playback.buffer_write = get_buffer_offset_from_frame(last_fr, qMax(timeline_in, target_frame));
//				}
//                int offset = audio_ibuffer_read - c->audio_playback.buffer_write;
//				if (offset > 0) {
//                    c->audio_playback.buffer_write += offset;
//                    c->audio_playback.frame_sample_index += offset;
//				}
//			}
//        } else if (c->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
//            double timebase = av_q2d(c->media_handling.stream->time_base);

//            av_frame = c->queue.at(0);

//			// retrieve frame
//			bool new_frame = false;
//            while ((c->audio_playback.frame_sample_index == -1 || c->audio_playback.frame_sample_index >= nb_bytes) && nb_bytes > 0) {
//				// no more audio left in frame, get a new one
//				if (!c->reached_end) {
//					int loop = 0;

//                    if (c->timeline_info.reverse && !c->audio_playback.just_reset) {
//                        avcodec_flush_buffers(c->media_handling.codecCtx);
//						c->reached_end = false;
//                        int64_t backtrack_seek = qMax(c->audio_playback.reverse_target - static_cast<int64_t>(av_q2d(av_inv_q(c->media_handling.stream->time_base))), static_cast<int64_t>(0));
//                        av_seek_frame(c->media_handling.formatCtx, c->media_handling.stream->index, backtrack_seek, AVSEEK_FLAG_BACKWARD);
//#ifdef AUDIOWARNINGS
//						if (backtrack_seek == 0) {
//							dout << "backtracked to 0";
//						}
//#endif
//					}

//					do {
//                        av_frame_unref(av_frame);

//						int ret;

//                        while ((ret = av_buffersink_get_frame(c->buffersink_ctx, av_frame)) == AVERROR(EAGAIN)) {
//                            ret = retrieve_next_frame(c->media_handling.frame);
//							if (ret >= 0) {
//                                if ((ret = av_buffersrc_add_frame_flags(c->buffersrc_ctx, c->media_handling.frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
//									dout << "[ERROR] Could not feed filtergraph -" << ret;
//									break;
//								}
//							} else {
//								if (ret == AVERROR_EOF) {
//#ifdef AUDIOWARNINGS
//										dout << "reached EOF while reading";
//#endif
//									// TODO revise usage of reached_end in audio
//                                    if (!c->timeline_info.reverse) {
//										c->reached_end = true;
//									} else {
//									}
//								} else {
//									dout << "[WARNING] Raw audio frame data could not be retrieved." << ret;
//									c->reached_end = true;
//								}
//								break;
//							}
//						}

//						if (ret < 0) {
//							if (ret != AVERROR_EOF) {
//								dout << "[ERROR] Could not pull from filtergraph";
//								c->reached_end = true;
//								break;
//							} else {
//#ifdef AUDIOWARNINGS
//								dout << "reached EOF while pulling from filtergraph";
//#endif
//                                if (!c->timeline_info.reverse) break;
//							}
//						}

//                        if (c->timeline_info.reverse) {
//							if (loop > 1) {
//								AVFrame* rev_frame = c->queue.at(1);
//								if (ret != AVERROR_EOF) {
//									if (loop == 2) {
//#ifdef AUDIOWARNINGS
//										dout << "starting rev_frame";
//#endif
//										rev_frame->nb_samples = 0;
//                                        rev_frame->pts = c->media_handling.frame->pkt_pts;
//									}
//									int offset = rev_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format)) * rev_frame->channels;
//#ifdef AUDIOWARNINGS
//									dout << "offset 1:" << offset;
//									dout << "retrieved samples:" << frame->nb_samples << "size:" << (frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format)) * frame->channels);
//#endif
//									memcpy(
//											rev_frame->data[0]+offset,
//                                            av_frame->data[0],
//                                            (av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels)
//										);
//#ifdef AUDIOWARNINGS
//                                    dout << "pts:" << c->media_handling.frame->pts << "dur:" << c->media_handling.frame->pkt_duration << "rev_target:" << c->audio_playback.reverse_target << "offset:" << offset << "limit:" << rev_frame->linesize[0];
//#endif
//								}

//                                rev_frame->nb_samples += av_frame->nb_samples;

//                                if ((c->media_handling.frame->pts >= c->audio_playback.reverse_target) || (ret == AVERROR_EOF)) {
///*
//#ifdef AUDIOWARNINGS
//                                    dout << "time for the end of rev cache" << rev_frame->nb_samples << c->rev_target << c->media_handling.frame->pts << c->media_handling.frame->pkt_duration << c->media_handling.frame->nb_samples;
//                                    dout << "diff:" << (c->media_handling.frame->pkt_pts + c->media_handling.frame->pkt_duration) - c->rev_target;
//#endif
//                                    int cutoff = qRound64((((c->media_handling.frame->pkt_pts + c->media_handling.frame->pkt_duration) - c->audio_playback.reverse_target) * timebase) * audio_output->format().sampleRate());
//									if (cutoff > 0) {
//#ifdef AUDIOWARNINGS
//										dout << "cut off" << cutoff << "samples (rate:" << audio_output->format().sampleRate() << ")";
//#endif
//										rev_frame->nb_samples -= cutoff;
//									}
//*/

//#ifdef AUDIOWARNINGS
//                                    dout << "pre cutoff deets::: rev_frame.pts:" << rev_frame->pts << "rev_frame.nb_samples" << rev_frame->nb_samples << "rev_target:" << c->audio_playback.reverse_target;
//#endif
//                                    double playback_speed = c->timeline_info.speed * c->timeline_info.media->get_object<Footage>()->speed;
//                                    rev_frame->nb_samples = qRound64(static_cast<double>(c->audio_playback.reverse_target - rev_frame->pts) / c->media_handling.stream->codecpar->sample_rate * (current_audio_freq() / playback_speed));
//#ifdef AUDIOWARNINGS
//									dout << "post cutoff deets::" << rev_frame->nb_samples;
//#endif

//									int frame_size = rev_frame->nb_samples * rev_frame->channels * av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format));
//									int half_frame_size = frame_size >> 1;

//									int sample_size = rev_frame->channels*av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format));
//									char* temp_chars = new char[sample_size];
//									for (int i=0;i<half_frame_size;i+=sample_size) {
//										for (int j=0;j<sample_size;j++) {
//											temp_chars[j] = rev_frame->data[0][i+j];
//										}
//										for (int j=0;j<sample_size;j++) {
//											rev_frame->data[0][i+j] = rev_frame->data[0][frame_size-i-sample_size+j];
//										}
//										for (int j=0;j<sample_size;j++) {
//											rev_frame->data[0][frame_size-i-sample_size+j] = temp_chars[j];
//										}
//									}
//									delete [] temp_chars;

//                                    c->audio_playback.reverse_target = rev_frame->pts;
//                                    av_frame = rev_frame;
//									break;
//								}
//							}

//							loop++;

//#ifdef AUDIOWARNINGS
//							dout << "loop" << loop;
//#endif
//						} else {
//                            av_frame->pts = c->media_handling.frame->pts;
//							break;
//						}
//					} while (true);
//				} else {
//					// if there is no more data in the file, we flush the remainder out of swresample
//					break;
//				}

//				new_frame = true;

//                if (c->audio_playback.frame_sample_index < 0) {
//                    c->audio_playback.frame_sample_index = 0;
//				} else {
//                    c->audio_playback.frame_sample_index -= nb_bytes;
//				}

//                nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;

//                if (c->audio_playback.just_reset) {
//					// get precise sample offset for the elected clip_in from this audio frame
//                    double target_sts = playhead_to_clip_seconds(c, c->audio_playback.target_frame);
//                    double frame_sts = ((av_frame->pts - c->media_handling.stream->start_time) * timebase);
//					int nb_samples = qRound64((target_sts - frame_sts)*current_audio_freq());
//                    c->audio_playback.frame_sample_index = nb_samples * 4;
//#ifdef AUDIOWARNINGS
//                    dout << "fsts:" << frame_sts << "tsts:" << target_sts << "nbs:" << nb_samples << "nbb:" << nb_bytes << "rev_targetToSec:" << (c->audio_playback.reverse_target * timebase);
//                    dout << "fsi-calc:" << c->audio_playback.frame_sample_index;
//#endif
//                    if (c->timeline_info.reverse) c->audio_playback.frame_sample_index = nb_bytes - c->audio_playback.frame_sample_index;
//                    c->audio_playback.just_reset = false;
//				}

//#ifdef AUDIOWARNINGS
//                dout << "fsi-post-post:" << c->audio_playback.frame_sample_index;
//#endif
//                if (c->audio_playback.buffer_write == 0) {
//                    c->audio_playback.buffer_write = get_buffer_offset_from_frame(last_fr, qMax(timeline_in, target_frame));

//					if (frame_skip > 0) {
//						int target = get_buffer_offset_from_frame(last_fr, qMax(timeline_in + frame_skip, target_frame));
//                        c->audio_playback.frame_sample_index += (target - c->audio_playback.buffer_write);
//                        c->audio_playback.buffer_write = target;
//					}
//				}

//                int offset = audio_ibuffer_read - c->audio_playback.buffer_write;
//				if (offset > 0) {
//                    c->audio_playback.buffer_write += offset;
//                    c->audio_playback.frame_sample_index += offset;
//				}

//				// try to correct negative fsi
//                if (c->audio_playback.frame_sample_index < 0) {
//                    c->audio_playback.buffer_write -= c->audio_playback.frame_sample_index;
//                    c->audio_playback.frame_sample_index = 0;
//				}
//			}

//            if (c->timeline_info.reverse) av_frame = c->queue.at(1);

//#ifdef AUDIOWARNINGS
//            dout << "j" << c->audio_playback.frame_sample_index << nb_bytes;
//#endif

//			// apply any audio effects to the data
//            if (nb_bytes == INT_MAX) nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;
//			if (new_frame) {
//                apply_audio_effects(c, bytes_to_seconds(c->audio_playback.buffer_write, 2, current_audio_freq()) + audio_ibuffer_timecode + ((double)c->get_clip_in_with_transition()/c->sequence->getFrameRate()) - ((double)timeline_in/last_fr), av_frame, nb_bytes, nests);
//			}
//		} else {
//			// shouldn't ever get here
//			dout << "[ERROR] Tried to cache a non-footage/tone clip";
//			return;
//		}

//		// mix audio into internal buffer
//        if (av_frame->nb_samples == 0) {
//			break;
//		} else {
//            long buffer_timeline_out = get_buffer_offset_from_frame(c->sequence->getFrameRate(), timeline_out);
//			audio_write_lock.lock();

//            while (c->audio_playback.frame_sample_index < nb_bytes
//                   && c->audio_playback.buffer_write < audio_ibuffer_read+(audio_ibuffer_size>>1)
//                   && c->audio_playback.buffer_write < buffer_timeline_out) {
//                int upper_byte_index = (c->audio_playback.buffer_write+1)%audio_ibuffer_size;
//                int lower_byte_index = (c->audio_playback.buffer_write)%audio_ibuffer_size;
//				qint16 old_sample = static_cast<qint16>((audio_ibuffer[upper_byte_index] & 0xFF) << 8 | (audio_ibuffer[lower_byte_index] & 0xFF));
//                qint16 new_sample = static_cast<qint16>((av_frame->data[0][c->audio_playback.frame_sample_index+1] & 0xFF) << 8 | (av_frame->data[0][c->audio_playback.frame_sample_index] & 0xFF));
//				qint16 mixed_sample = mix_audio_sample(old_sample, new_sample);

//				audio_ibuffer[upper_byte_index] = static_cast<quint8>((mixed_sample >> 8) & 0xFF);
//				audio_ibuffer[lower_byte_index] = static_cast<quint8>(mixed_sample & 0xFF);

//                c->audio_playback.buffer_write+=2;
//                c->audio_playback.frame_sample_index+=2;
//			}

//#ifdef AUDIOWARNINGS
//            if (c->audio_playback.buffer_write >= buffer_timeline_out) dout << "timeline out at fsi" << c->audio_playback.frame_sample_index << "of frame ts" << c->media_handling.frame->pts;
//#endif

//			audio_write_lock.unlock();

//			if (scrubbing) {
//				if (audio_thread != NULL) audio_thread->notifyReceiver();
//			}

//            if (c->audio_playback.frame_sample_index == nb_bytes) {
//                c->audio_playback.frame_sample_index = -1;
//			} else {
//				// assume we have no more data to send
//				break;
//			}

////			dout << "ended" << c->audio_playback.frame_sample_index << nb_bytes;
//		}
//		if (c->reached_end) {
//            av_frame->nb_samples = 0;
//		}
//		if (scrubbing) {
//			break;
//		}
//	}

//    QMetaObject::invokeMethod(e_panel_footage_viewer, "play_wake", Qt::QueuedConnection);
//    QMetaObject::invokeMethod(e_panel_sequence_viewer, "play_wake", Qt::QueuedConnection);
//}

//void cache_video_worker(ClipPtr c, long playhead) {
//	int read_ret, send_ret, retr_ret;

//	int64_t target_pts = seconds_to_timestamp(c, playhead_to_clip_seconds(c, playhead));

//	int limit = c->max_queue_size;
//	if (c->ignore_reverse) {
//		// waiting for one frame
//		limit = c->queue.size() + 1;
//    } else if (c->timeline_info.reverse) {
//		limit *= 2;
//	}

//	if (c->queue.size() < limit) {
//        bool reverse = (c->timeline_info.reverse && !c->ignore_reverse);
//		c->ignore_reverse = false;

//		int64_t smallest_pts = INT64_MAX;
//		if (reverse && c->queue.size() > 0) {
//            int64_t quarter_sec = qRound64(av_q2d(av_inv_q(c->media_handling.stream->time_base))) >> 2;
//			for (int i=0;i<c->queue.size();i++) {
//				smallest_pts = qMin(smallest_pts, c->queue.at(i)->pts);
//			}
//            avcodec_flush_buffers(c->media_handling.codecCtx);
//			c->reached_end = false;
//			int64_t seek_ts = qMax(static_cast<int64_t>(0), smallest_pts - quarter_sec);
//            av_seek_frame(c->media_handling.formatCtx, c->media_handling.stream->index, seek_ts, AVSEEK_FLAG_BACKWARD);
//		} else {
//			smallest_pts = target_pts;
//		}

//        if (c->multithreaded && c->cache_thread->interrupt) { // ignore interrupts for now
//            c->cache_thread->interrupt = false;
//		}

//		while (true) {
//			AVFrame* frame = av_frame_alloc();

//            FootagePtr media = c->timeline_info.media->get_object<Footage>();
//            const FootageStream* ms = media->get_stream_from_file_index(true, c->timeline_info.media_stream);

//			while ((retr_ret = av_buffersink_get_frame(c->buffersink_ctx, frame)) == AVERROR(EAGAIN)) {
//                if (c->multithreaded && c->cache_thread->interrupt) return; // abort

//                AVFrame* send_frame = c->media_handling.frame;
////				qint64 time = QDateTime::currentMSecsSinceEpoch();
//				read_ret = (c->use_existing_frame) ? 0 : retrieve_next_frame(c, send_frame);
////				dout << QDateTime::currentMSecsSinceEpoch() - time;
//				c->use_existing_frame = false;
//				if (read_ret >= 0) {
//					bool send_it = true;

//					/*if (reverse) {
//						send_it = true;
//					} else if (send_frame->pts > target_pts - eighth_second) {
//						send_it = true;
//                    } else if (media->get_stream_from_file_index(true, c->timeline_info.media_stream)->infinite_length) {
//						send_it = true;
//					} else {
//						dout << "skipped adding a frame to the queue - fpts:" << send_frame->pts << "target:" << target_pts;
//					}*/

//					if (send_it) {
//						if ((send_ret = av_buffersrc_add_frame_flags(c->buffersrc_ctx, send_frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
//							dout << "[ERROR] Failed to add frame to buffer source." << send_ret;
//							break;
//						}
//					}

//                    av_frame_unref(c->media_handling.frame);
//				} else {
//					if (read_ret == AVERROR_EOF) {
//						c->reached_end = true;
//					} else {
//						dout << "[ERROR] Failed to read frame." << read_ret;
//					}
//					break;
//				}
//			}

//			if (retr_ret < 0) {
//				if (retr_ret == AVERROR_EOF) {
//					c->reached_end = true;
//				} else {
//					dout << "[ERROR] Failed to retrieve frame from buffersink." << retr_ret;
//				}
//				av_frame_free(&frame);
//				break;
//			} else {
//				if (reverse && ((smallest_pts == target_pts && frame->pts >= smallest_pts) || (smallest_pts != target_pts && frame->pts > smallest_pts))) {
//					av_frame_free(&frame);
//					break;
//				} else {
//					// thread-safety while adding frame to the queue
//					c->queue_lock.lock();
//					c->queue.append(frame);

//					if (!ms->infinite_length && !reverse && c->queue.size() == limit) {
//						// see if we got the frame we needed (used for speed ups primarily)
//						bool found = false;
//						for (int i=0;i<c->queue.size();i++) {
//							if (c->queue.at(i)->pts >= target_pts) {
//								found = true;
//								break;
//							}
//						}
//						if (found) {
//							c->queue_lock.unlock();
//							break;
//						} else {
//							// remove earliest frame and loop to store another
//							c->queue_remove_earliest();
//						}
//					}
//					c->queue_lock.unlock();
//				}
//			}

//            if (c->multithreaded && c->cache_thread->interrupt) { // abort
//				return;
//			}
//		}
//	}
//}

//void reset_cache(ClipPtr c, long target_frame) {
//	// if we seek to a whole other place in the timeline, we'll need to reset the cache with new values
//    if (c->timeline_info.media == NULL) {
//        if (c->timeline_info.track >= 0) {
//			// tone clip
//			c->reached_end = false;
//            c->audio_playback.target_frame = target_frame;
//            c->audio_playback.frame_sample_index = -1;
//            c->media_handling.frame->pts = 0;
//		}
//	} else {
//        const FootageStream* ms = c->timeline_info.media->get_object<Footage>()->get_stream_from_file_index(c->timeline_info.track < 0, c->timeline_info.media_stream);
//		if (ms->infinite_length) {
//            /*avcodec_flush_buffers(c->media_handling.codecCtx);
//            av_seek_frame(c->media_handling.formatCtx, ms->file_index, 0, AVSEEK_FLAG_BACKWARD);*/
//			c->use_existing_frame = false;
//		} else {
//            if (c->media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//				// clear current queue
//				c->queue_clear();

//				// seeks to nearest keyframe (target_frame represents internal clip frame)
//				int64_t target_ts = seconds_to_timestamp(c, playhead_to_clip_seconds(c, target_frame));
//				int64_t seek_ts = target_ts;
//                int64_t timebase_half_second = qRound64(av_q2d(av_inv_q(c->media_handling.stream->time_base)));
//                if (c->timeline_info.reverse) seek_ts -= timebase_half_second;

//				while (true) {
//					// flush ffmpeg codecs
//                    avcodec_flush_buffers(c->media_handling.codecCtx);
//					c->reached_end = false;

//					if (seek_ts > 0) {
//                        av_seek_frame(c->media_handling.formatCtx, ms->file_index, seek_ts, AVSEEK_FLAG_BACKWARD);

//                        av_frame_unref(c->media_handling.frame);
//                        int ret = retrieve_next_frame(c, c->media_handling.frame);
//						if (ret < 0) {
//							dout << "[WARNING] Seeking terminated prematurely";
//							break;
//						}
//                        if (c->media_handling.frame->pts <= target_ts) {
//							c->use_existing_frame = true;
//							break;
//						} else {
//							seek_ts -= timebase_half_second;
//						}
//					} else {
//                        av_frame_unref(c->media_handling.frame);
//                        av_seek_frame(c->media_handling.formatCtx, ms->file_index, 0, AVSEEK_FLAG_BACKWARD);
//						c->use_existing_frame = false;
//						break;
//					}
//				}
//            } else if (c->media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//				// flush ffmpeg codecs
//                avcodec_flush_buffers(c->media_handling.codecCtx);
//				c->reached_end = false;

//				// seek (target_frame represents timeline timecode in frames, not clip timecode)

//				int64_t timestamp = seconds_to_timestamp(c, playhead_to_clip_seconds(c, target_frame));

//                if (c->timeline_info.reverse) {
//                    c->audio_playback.reverse_target = timestamp;
//                    timestamp -= av_q2d(av_inv_q(c->media_handling.stream->time_base));
//#ifdef AUDIOWARNINGS
//                    dout << "seeking to" << timestamp << "(originally" << c->audio_playback.reverse_target << ")";
//				} else {
//					dout << "reset called; seeking to" << timestamp;
//#endif
//				}
//                av_seek_frame(c->media_handling.formatCtx, ms->file_index, timestamp, AVSEEK_FLAG_BACKWARD);
//                c->audio_playback.target_frame = target_frame;
//                c->audio_playback.frame_sample_index = -1;
//                c->audio_playback.just_reset = true;
//			}
//		}
//	}
//}

Cacher::Cacher(ClipPtr c) : clip(c) {}

//AVSampleFormat sample_format = AV_SAMPLE_FMT_S16;

//void cache_clip_worker(ClipPtr clip, long playhead, bool reset, bool scrubbing, QVector<ClipPtr> nests) {
//	if (reset) {
//		// note: for video, playhead is in "internal clip" frames - for audio, it's the timeline playhead
//		reset_cache(clip, playhead);
//        clip->audio_playback.reset = false;
//	}

//    if (clip->timeline_info.media == NULL) {
//        if (clip->timeline_info.track >= 0) {
//			cache_audio_worker(clip, scrubbing, nests);
//		}
//    } else if (clip->timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
//        if (clip->media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//			cache_video_worker(clip, playhead);
//        } else if (clip->media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//			cache_audio_worker(clip, scrubbing, nests);
//		}
//	}
//}


void Cacher::run() {
//	// open_lock is used to prevent the clip from being destroyed before the cacher has closed it properly
//	clip->lock.lock();
//	clip->finished_opening = false;
//    clip->is_open = true;
//	caching = true;
//	interrupt = false;

//    clip->open_worker();

//	while (caching) {
//		clip->can_cache.wait(&clip->lock);
//		if (!caching) {
//			break;
//		} else {
//			while (true) {
//				cache_clip_worker(clip, playhead, reset, scrubbing, nests);
//                if (clip->multithreaded && clip->cache_thread->interrupt && clip->timeline_info.track < 0) {
//                    clip->cache_thread->interrupt = false;
//				} else {
//					break;
//				}
//            }//while
//		}
//    }//while

//    clip->close_worker();

//	clip->lock.unlock();
//	clip->open_lock.unlock();
}
