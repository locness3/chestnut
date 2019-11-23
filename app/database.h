#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlResult>

namespace chestnut
{
  //    field->name, field->value
  using ParamType = QPair<QString, QVariant>;
  //    effectrow->fields
  using ParamsType = QVector<ParamType>;
  //    effectrow->name, effectrow->fields
  using EffectParametersType = QMap<QString, ParamsType>;

  struct EffectPreset
  {
    QString effect_name_;
    QString preset_name_;
    EffectParametersType parameters_;
  };

  class Database
  {
    public:
      explicit Database(QString file_path);
      static std::shared_ptr<Database> instance(const QString& file_path="");
      /**
       * @brief         Add a new Effect preset into the database
       * @param value   Structure containing all preset data
       * @return        true==preset added successfully
       */
      bool addNewPreset(const EffectPreset& value);
      /**
       * @brief             Obtain all the preset names of an Effect
       * @param effect_name
       * @return            List of all the names or empty
       */
      QStringList getPresets(const QString& effect_name);
      /**
       * @brief getPresetParameters
       * @param effect_name
       * @param preset_name
       * @return
       */
      EffectParametersType getPresetParameters(QString effect_name, QString preset_name);

    private:
      QSqlDatabase db_;
      inline static std::shared_ptr<Database> instance_{nullptr};

      void setupEffectsTable();

      const QSqlResult* query(const QString& statement);

      int effectId(const QString& name, const bool recurse=true);
      int effectRowId(const QString& name, const int effect_id, const bool recurse=true);
      int presetId(const QString& name);

      bool addNewParameterPreset(const int preset_id, const int row_id, const QString& name, const QVariant& value);
  };
}

#endif // DATABASE_H
