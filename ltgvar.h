#ifndef LTGVAR_H
#define LTGVAR_H

#include <QObject>
#include <QJsonObject>

class LtgVar : public QObject
{
    Q_OBJECT
public:
    explicit LtgVar(QObject *parent = nullptr);

    enum class PadType {
        padNone,
        padLeft,
        padRight
    };
    Q_ENUM(PadType)

    void setName(QString name);
    QString &name();

    const QString value();

    virtual void calc() = 0;

    virtual void fromJson(const QJsonObject &json);

    virtual QJsonObject toJson() const;
signals:

protected:
    void setValue(QString value);

private:
    QString m_comment;
    QString m_name;
    QString m_prefix;
    QString m_sufix;
    QString m_outputMask;
    PadType m_padType;
    quint16 m_padLength;
    QChar m_padChar;
    QString m_value;
};


class LtgTextVar : public LtgVar
{
    Q_OBJECT
public:
    LtgTextVar(QObject *parent = nullptr) : LtgVar{parent}, m_text{""} {};

    void setText(QString text);
    QString text();

    void calc() override;

    void fromJson(const QJsonObject &json) override;

    QJsonObject toJson() const override;
private:
    QString m_text;
};

class LtgFormulaVar : public LtgVar
{
public:
    LtgFormulaVar(QObject *parent = nullptr) : LtgVar{parent}, m_formula{"return 0;"} {};

    //void setFormula(QStringList formula);
    //QStringList& formula();

    void setFormula(QString formula);
    QString formula();

    void calc() override;

    void fromJson(const QJsonObject &json) override;

    QJsonObject toJson() const override;
private:
    //QStringList m_formula;
    QString m_formula;
};


#endif // LTGVAR_H
