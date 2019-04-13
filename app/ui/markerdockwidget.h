#ifndef MARKERDOCKWIDGET_H
#define MARKERDOCKWIDGET_H

#include <QString>
#include <QObject>
#include <tuple>

namespace ui {
  class MarkerDockWidget { //TODO: come up with a better name. its not a dock widget and markerwidget already exists
    public:
      MarkerDockWidget() = default;
      virtual ~MarkerDockWidget() = default;
      /**
       * Create a new marker of an object in the widget
       */
      virtual void setMarker() const = 0;
      /**
       * Get the name of a new marker via a dialog
       * @return name & user accepted
       */
      std::tuple<QString, bool> getName() const;
  };
}
#endif // MARKERDOCKWIDGET_H
