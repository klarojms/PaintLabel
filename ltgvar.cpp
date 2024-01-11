#include <QJsonValue>
#include <QJsonArray>
#include <QMetaObject>
#include <QMetaEnum>
#include <QJSEngine>
#include "ltgvar.h"
#include "ltglabel.h"

LtgVar::LtgVar(QObject *parent)
    : QObject{ parent }
    , m_comment{ "" }
    , m_name{ "" }
    , m_prefix{ "" }
    , m_sufix{ "" }
    , m_outputMask{ "" }
    , m_padType{ PadType::padNone }
    , m_padLength { 0 }
    , m_padChar{ 0 }
{
}

void LtgVar::setName(QString name)
{
    m_name = name;
}

QString &LtgVar::name()
{
    return m_name;
}

const QString LtgVar::value()
{
    // Hay que aplicar todo lo que necesita antes de retornar
    QString result{ m_prefix };
    result.append(m_value);
    result.append(m_sufix);

    if(m_padType == PadType::padRight)
        result = result.rightJustified(m_padLength, m_padChar, true);
    else if(m_padType == PadType::padLeft)
        result = result.leftJustified(m_padLength, m_padChar, true);

    return result;
}

void LtgVar::fromJson(const QJsonObject &json)
{
    m_comment = "";
    if (const QJsonValue v = json["_comment"]; v.isString())
        m_comment = v.toString();

    m_name = "var";
    if (const QJsonValue v = json["name"]; v.isString())
        m_name = v.toString();

    m_prefix = "";
    if (const QJsonValue v = json["prefix"]; v.isString())
        m_prefix = v.toString();

    m_sufix = "";
    if (const QJsonValue v = json["sufix"]; v.isString())
        m_sufix = v.toString();

    m_outputMask = "";
    if (const QJsonValue v = json["outputMask"]; v.isString())
        m_outputMask = v.toString();

    m_padLength = 0;
    if (const QJsonValue v = json["padLength"]; v.isDouble())
        m_padLength = v.toInt(0);

    m_padChar = QChar(0);
    if (const QJsonValue v = json["padChar"]; v.isString()) {
        if(v.toString().size()>0)
            m_padChar = v.toString().at(0);
    }

    QMetaObject mo = LtgVar::staticMetaObject;

    m_padType = PadType::padNone;
    if (const QJsonValue v = json["padType"]; v.isString()) {
        QString str = v.toString();
        QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PadType"));

        PadType enumValue = static_cast<PadType>(metaEnum.keyToValue(str.toLatin1()));
        if(enumValue < PadType::padNone || enumValue > PadType::padRight)
            enumValue = PadType::padNone;
        m_padType = enumValue;
    }
}

QJsonObject LtgVar::toJson() const
{
    QJsonObject json;
    json["_comment"] = m_comment;
    json["name"] = m_name;
    json["prefix"] = m_prefix;
    json["sufix"] = m_sufix;
    json["outputMask"] = m_outputMask;
    json["padLength"] = m_padLength;
    if(m_padChar != QChar(0))
        json["padChar"] = QString(m_padChar);
    else
        json["padChar"] = "";
    return json;
}

void LtgVar::setValue(QString value)
{
    m_value = value;
}

void LtgTextVar::setText(QString text)
{
    m_text = text;
}

QString LtgTextVar::text()
{
    return m_text;
}

void LtgTextVar::calc()
{
    LtgVar::setValue( m_text );
}

void LtgTextVar::fromJson(const QJsonObject &json)
{
    LtgVar::fromJson(json);
    if (const QJsonValue v = json["text"]; v.isString())
        m_text = v.toString();
}

QJsonObject LtgTextVar::toJson() const
{
    QJsonObject json = LtgVar::toJson();
    json["type"] = "text";
    json["text"] = m_text;
    return json;
}

//void LtgFormulaVar::setFormula(QStringList formula)
void LtgFormulaVar::setFormula(QString formula)
{
    m_formula = formula;
}

//QStringList &LtgFormulaVar::formula()
QString LtgFormulaVar::formula()
{
    return m_formula;
}

void LtgFormulaVar::calc()
{
    // Calculamos los datos de las variables
    // Asigna y compara si el resultado es nullptr o no
    if(auto lbl = qobject_cast<LtgLabel*>(parent())) {
        for(auto it = lbl->vars().constBegin(); it != lbl->vars().constEnd(); ++it)
        {
            if((*it)->name() != name()) {
                //if(!lbl->engine().globalObject().hasProperty((*it)->name()) || lbl->engine().globalObject().property((*it)->name()).toString().isEmpty() ) {
                    //(*it)->calc();
                    lbl->engine().globalObject().setProperty((*it)->name(), (*it)->value());
                //}
            }
        }

        QString completedFormula = "(function() {";
        //for(int i = 0; i < m_formula.count(); i++)
        //    completedFormula += m_formula[i];
        completedFormula += m_formula;
        completedFormula += "})";
        auto fn = lbl->engine().evaluate(completedFormula);
        auto result = fn.call(); //engine.evaluate(completedFormula);
        if(!result.isNull() && !result.isError() && !result.isUndefined()) {
            auto str_result = result.toString();
            LtgVar::setValue( str_result );
        }
    }
}

void LtgFormulaVar::fromJson(const QJsonObject &json)
{
    LtgVar::fromJson(json);
    //m_formula.clear();
    //if (const QJsonValue v = json["lines"]; v.isArray()) {
    //    const QJsonArray lines = v.toArray();
    //    for (const QJsonValue &obj : lines) {
    //        m_formula.append(obj.toString());
    //    }
    //}

    if (const QJsonValue v = json["formula"]; v.isString())
        m_formula = v.toString();
}

QJsonObject LtgFormulaVar::toJson() const
{
    QJsonObject json = LtgVar::toJson();
    json["type"] = "formula";
    json["formula"] = m_formula;
    return json;
}
