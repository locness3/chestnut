#include "projectfilter.h"

#include "project/media.h"
#include "panels/project.h"

#include <QDebug>

ProjectFilter::ProjectFilter(QObject *parent) :
  QSortFilterProxyModel(parent),
  show_sequences(true)
{

}

bool ProjectFilter::get_show_sequences()
{
  return show_sequences;
}

void ProjectFilter::set_show_sequences(bool b)
{
  show_sequences = b;
  invalidateFilter();
}

bool ProjectFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!show_sequences) {
    // hide sequences if show_sequences is false
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    auto mda = Project::model().getItem(index);

    if (mda != nullptr && mda->type() == MediaType::SEQUENCE) {
      return false;
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
