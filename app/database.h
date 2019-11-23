#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlResult>

namespace chestnut
{
  using EffectParamType = QVector<QMap<QString, QVector<QPair<QString, QVariant>>>>;
  struct EffectPreset
  {
    QString effect_name_;
    QString preset_name_;
    EffectParamType parameters_;
  };

  class Database
  {
    public:
      explicit Database(QString file_path);
      static std::shared_ptr<Database> instance(const QString& file_path="");

      bool addNewPreset(const EffectPreset& value);
      QStringList getPresets(QString effect_name);
      EffectParamType getPresetParameters(QString preset_name);

    private:
      QSqlDatabase db_;
      inline static std::shared_ptr<Database> instance_{nullptr};

      void setupEffectsTable();

      const QSqlResult* query(const QString& statement);

      int effectId(const QString& name, const bool recurse=true);
      int effectRowId(const QString& name, const int effect_id, const bool recurse=true);
      int presetId(const QString& name);

      bool addNewParameterPreset(const int preset_id, const int row_id, QString name, QVariant value);

      QString variantAsString(QVariant val) const;
  };
}

#endif // DATABASE_H
