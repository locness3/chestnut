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
#include "ui/mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QLoggingCategory>

#include <unistd.h>

#include "chestnut.h"
#include "debug.h"
#include "project/effect.h"
#include "panels/panelmanager.h"
#include "io/path.h"
#include "database.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

constexpr auto APP_NAME = "Chestnut";
constexpr auto DB_FILENAME = "chestnut.db";

int main(int argc, char *argv[])
{
  auto launch_fullscreen = false;
  QString load_proj;
  int c;

  av_log_set_level(AV_LOG_PANIC);

  while ((c = getopt(argc, argv, "fhi:l:sv")) != -1)
  {
    switch (c) {
      case 'f':
        // fullscreen
        launch_fullscreen = true;
        break;
      case 'h':
        printf("Usage: %s [options] \n\nOptions:"
               "\n\t-f \t\tStart in full screen mode"
               "\n\t-h \t\tShow this help and exit"
               "\n\t-i <filename> \tLoad a project file"
               "\n\t-l <level> \t\tSet the logging level (fatal, critical, warning, info, debug)"
               "\n\t-s  \t\tDisable shaders\n\n",
               argv[0]);
        return 0;
      case 'i':
        // input file to load
        load_proj = optarg;
        break;
      case 'l':
        // log level
        {
          QString val = optarg;
          if (val == "fatal") {
            chestnut::debug::debug_level = QtMsgType::QtFatalMsg;
            av_log_set_level(AV_LOG_FATAL);
          } else if (val == "critical") {
            chestnut::debug::debug_level = QtMsgType::QtCriticalMsg;
            av_log_set_level(AV_LOG_ERROR);
          } else if (val == "warning") {
            chestnut::debug::debug_level = QtMsgType::QtWarningMsg;
            av_log_set_level(AV_LOG_WARNING);
          } else if (val == "info") {
            chestnut::debug::debug_level = QtMsgType::QtInfoMsg;
            av_log_set_level(AV_LOG_INFO);
          } else if (val == "debug") {
            chestnut::debug::debug_level = QtMsgType::QtDebugMsg;
            av_log_set_level(AV_LOG_VERBOSE);
          } else {
            std::cerr << "Unknown logging level:" << val.toUtf8().toStdString() << std::endl;
          }
        }
        break;
      case 's':
        // Disable shader effects
        shaders_are_enabled = false;
        break;
      case 'v':
        std::cout << APP_NAME << "-" << chestnut::version::MAJOR << "." << chestnut::version::MINOR << "." << chestnut::version::PATCH;
        if (!std::string(chestnut::version::PREREL).empty()) {
          std::cout << "-" << chestnut::version::PREREL;
        }
        std::cout << std::endl;
        return 0;
      default:
        std::cerr << "Ignoring:" << c << std::endl;
        break;
    }
  }

  qInstallMessageHandler(debug_message_handler);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
  // init ffmpeg subsystem
  av_register_all();
#endif

#if LIBAVFILTER_VERSION_INT < AV_VERSION_INT(7, 14, 100)
  avfilter_register_all();
#endif

  QApplication a(argc, argv);
  QApplication::setWindowIcon(QIcon(":/icons/chestnut.png"));

  const auto path(QDir(chestnut::paths::dataPath()).filePath(DB_FILENAME));
  Q_ASSERT(chestnut::Database::instance(path) != nullptr);

  const QString name(APP_NAME);
  MainWindow& w = MainWindow::instance(nullptr, name);
  w.initialise();
  w.updateTitle("");

  if (!load_proj.isEmpty()) {
    w.launch_with_project(load_proj);
  }
  if (launch_fullscreen) {
    w.showFullScreen();
  } else {
    w.showMaximized();
  }

  int retCode = QApplication::exec();

  MainWindow::tearDown();
  return retCode;
}
