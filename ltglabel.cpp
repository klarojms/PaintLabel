#include <QFile>
#include <QPixmap>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCborValue>
#include <QCborMap>
#include <QMetaObject>
#include <QMetaEnum>

#include "ltglabel.h"

//**********************************************************************
// LtgLabel
//**********************************************************************

LtgLabel::LtgLabel(QObject *parent)
    : QObject(parent)
    , m_comment{ "" }
    , m_img{ nullptr }
{
    m_engine.installExtensions(QJSEngine::ConsoleExtension);
}

LtgLabel::~LtgLabel()
{
    m_objects.clear();
    m_vars.clear();
}

int LtgLabel::width()
{
    return m_width;
}

int LtgLabel::height()
{
    return m_height;
}

void LtgLabel::setWidth(int width)
{
    m_width = width;
    if(m_img) m_img = nullptr; // Invalidate
}

void LtgLabel::setHeight(int height)
{
    m_height = height;
    if(m_img) m_img = nullptr; // Invalidate
}

QList<std::shared_ptr<LtgObject>> &LtgLabel::objects()
{
    return m_objects;
}

QMap<QString, std::shared_ptr<LtgVar>> &LtgLabel::vars()
{
    return m_vars;
}

void LtgLabel::paint()
{
    if(m_img == nullptr)
        //m_img = QSharedPointer<QImage>( new QImage(m_width, m_height, QImage::Format_Mono) );
        m_img = std::make_shared<QImage>(QImage(m_width, m_height, QImage::Format_RGB32) );

    calcVars();

    QPainter paintOriginalSize;
    paintOriginalSize.begin(m_img.get());

    paintOriginalSize.setRenderHint(QPainter::Antialiasing, false);
    QRect r{0, 0, width(), 512};
    QPen pen(Qt::white, 1, Qt::SolidLine);
    paintOriginalSize.fillRect(r, QBrush(Qt::white, Qt::SolidPattern));

    for(auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it)
    {
        // Mira si tiene variable
        LtgVarBaseObject *obj = dynamic_cast<LtgVarBaseObject *>(it->get());
        if(obj) {
            QString varName = obj->var();
            if(m_vars.contains(varName)) {
                // Intentamos transformar en el tipo
                if(dynamic_cast<LtgText *>(obj)) {
                    LtgText *objText = dynamic_cast<LtgText *>(obj);
                    objText->text() = m_vars[varName]->value();
                    //qDebug() << m_vars[varName]->value();
                } else if(dynamic_cast<LtgImage *>(obj)) {
                    LtgImage *objImage = dynamic_cast<LtgImage *>(obj);
                    objImage->setPath( m_vars[varName]->value() );
                } else if(dynamic_cast<LtgBarcode *>(obj)) {
                    LtgBarcode *objBarcode = dynamic_cast<LtgBarcode *>(obj);
                    objBarcode->text() = m_vars[varName]->value();
                }
            }
        }

        (*it)->paint(&paintOriginalSize);
    }

    if(m_drawBoundingRect) {
        QPen pen(Qt::black, 1, Qt::SolidLine);
        paintOriginalSize.setPen(pen);

        r = QRect{0, 0, width()-1, 511};

        paintOriginalSize.drawRect(r);
    }

    paintOriginalSize.end();

    //QImage scaledImage = img.scaled(QQuickPaintedItem::width() , QQuickPaintedItem::height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    //QRect mRect(scaledImage.rect());
    //painter->drawImage(mRect, scaledImage);
}

QImage &LtgLabel::image()
{
    return *m_img.get();
}

void LtgLabel::setDrawBoundingRect(bool drawBoundingRect)
{
    m_drawBoundingRect = drawBoundingRect;
}

bool LtgLabel::drawBoundingRect() const
{
    return m_drawBoundingRect;
}

QJsonObject LtgLabel::toJson() const
{
    QJsonObject json;
    json["_comment"] = m_comment;
    json["width"] = m_width;
    json["height"] = m_height;
    json["drawBoundingRect"] = m_drawBoundingRect;

    QJsonArray objects;
    for(auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it)
    {
        objects.append((*it)->toJson());
    }
    json["objects"] = objects;

    QJsonArray vars;
    for(auto it = m_vars.constBegin(); it != m_vars.constEnd(); ++it)
    {
        vars.append((*it)->toJson());
    }
    json["vars"] = vars;

    return json;
}

void LtgLabel::fromJson(const QJsonObject &json)
{
    m_comment = "";
    QJsonValue v = json["_comment"];
    if(v.isString())
        m_comment = v.toString();

    v = json["width"];
    if(v.isDouble())
        m_width = v.toInt(0);
    v = json["height"];
    if(v.isDouble())
        m_height = v.toInt(0);
    v = json["drawBoundingRect"];
    if(v.isBool())
        m_drawBoundingRect = v.toBool();

    if (const QJsonValue v = json["objects"]; v.isArray()) {
        const QJsonArray objects = v.toArray();
        m_objects.clear();
        //m_objects.reserve(objects.size());
        for (const QJsonValue &obj : objects) {
            if(obj.isObject()) {
                if(obj.toObject().contains("type")) {
                    std::shared_ptr<LtgObject> objGr{ nullptr };

                    QJsonValue t = obj.toObject()["type"];
                    if(t.toString() == "line") {
                        objGr = std::make_shared<LtgLine>();
                    } else if(t.toString() == "rect") {
                        objGr = std::make_shared<LtgRect>();
                    } else if(t.toString() == "ellipse") {
                        objGr = std::make_shared<LtgEllipse>();
                    } else if(t.toString() == "text") {
                        objGr = std::make_shared<LtgText>();
                    } else if(t.toString() == "image") {
                        objGr = std::make_shared<LtgImage>();
                    } else if(t.toString() == "barcode") {
                        objGr = std::make_shared<LtgBarcode>();
                    }
                    if(objGr) {
                        objGr->fromJson(obj.toObject());
                        m_objects.push_back( objGr );
                    }
                }
            }
        }
    }

    if (const QJsonValue v = json["vars"]; v.isArray()) {
        const QJsonArray vars = v.toArray();
        m_vars.clear();
        for (const QJsonValue &var : vars) {
            if(var.isObject()) {
                if(var.toObject().contains("name")) {
                    QJsonValue name = var.toObject()["name"];

                    if(!m_vars.contains(name.toString())) {
                        if(var.toObject().contains("type")) {
                            std::shared_ptr<LtgVar> objVar{ nullptr };

                            QJsonValue t = var.toObject()["type"];
                            if(t.toString() == "text") {
                                objVar = std::make_shared<LtgTextVar>(this);
                            } else if(t.toString() == "formula") {
                                objVar = std::make_shared<LtgFormulaVar>(this);
                            }

                            if(objVar) {
                                objVar->fromJson(var.toObject());
                                m_vars[name.toString()] = objVar;
                            }
                        }
                    }

                }
            }
        }
    }
}

bool LtgLabel::save(QString fileName) const
{
    QFile saveFile(fileName);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file");
        return false;
    }

    QJsonObject json = toJson();
    if(fileName.endsWith("json", Qt::CaseInsensitive))
        saveFile.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    else
        saveFile.write(QCborValue::fromJsonValue(json).toCbor());
    return true;
}

bool LtgLabel::read(QString fileName)
{
    QFile loadFile(fileName);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    bool isJson{ false };
    if(fileName.endsWith("json", Qt::CaseInsensitive))
        isJson = true;

    const QJsonDocument loadDoc(isJson
                            ? QJsonDocument::fromJson(saveData)
                            : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    fromJson( loadDoc.object() );

    return true;
}

void LtgLabel::calcVars()
{
    QJsonArray vars;
    for(auto it = m_vars.constBegin(); it != m_vars.constEnd(); ++it)
    {
        (*it)->calc();
    }
}

QJSEngine &LtgLabel::engine()
{
    return m_engine;
}

//**********************************************************************
// LtgObject
//**********************************************************************

QRect& LtgObject::rect()
{
    return m_rect;
}

QRect &LtgObject::originalRect()
{
    return m_originalRect;
}

int LtgObject::angle()
{
    return m_angle;
}

void LtgObject::setAngle(int angle)
{
    m_angle = angle;
}

void LtgObject::applyRotation(int angle)
{
    if(m_angle ==  0 || m_angle == 90 || m_angle == 180 || m_angle == 270 )
    {
        if(m_angle == 0) {
            // Solo aplico rotaciones si el ángulo es 0

            m_originalRect = m_rect;

            m_angle = angle;
            if(angle == 180) {
                // Dejo el rectangulo como está
            } if(angle == 90) {
                m_rect = QRect{
                    QPoint{rect().left(), rect().bottom() - rect().width()},
                    QPoint{rect().left() + rect().height(), rect().bottom()}
                };
            } if(angle == 270) {
                m_rect = QRect{
                    QPoint{rect().left(), rect().bottom() - rect().width()},
                    QPoint{rect().left() + rect().height(), rect().bottom()}
                };
            }
        } else if(angle == 0 ){
            m_angle = angle;
            m_rect = m_originalRect;
        }
    }
}

void LtgObject::setDrawBoundingRect(bool drawBoundingRect)
{
    m_drawBoundingRect = drawBoundingRect;
}

bool LtgObject::drawBoundingRect() const
{
    return m_drawBoundingRect;
}

QJsonObject LtgObject::toJson() const
{
    QJsonObject json;
    json["_comment"] = m_comment;
    json["left"] = m_rect.left();
    json["top"] = m_rect.top();
    json["width"] = m_rect.width();
    json["height"] = m_rect.height();
    json["angle"] = m_angle;
    json["drawBoundingRect"] = m_drawBoundingRect;

    return json;
}

void LtgObject::fromJson(const QJsonObject &json)
{
    m_comment = "";
    if (const QJsonValue v = json["_comment"]; v.isDouble())
        m_comment = v.toString();
    if (const QJsonValue v = json["left"]; v.isDouble())
        m_rect.setLeft( v.toInt(0) );
    if (const QJsonValue v = json["top"]; v.isDouble())
        m_rect.setTop( v.toInt(0) );
    if (const QJsonValue v = json["width"]; v.isDouble())
        m_rect.setWidth( v.toInt(0) );
    if (const QJsonValue v = json["height"]; v.isDouble())
        m_rect.setHeight( v.toInt(0) );
    if (const QJsonValue v = json["angle"]; v.isDouble())
        m_angle = v.toInt(0);
    if (const QJsonValue v = json["drawBoundingRect"]; v.isBool())
        m_drawBoundingRect = v.toBool();
}

int LtgGraphObject::penWidth()
{
    return m_penWidth;
}

void LtgGraphObject::setPenWidth(int penWidth)
{
    m_penWidth = penWidth;
}

QJsonObject LtgGraphObject::toJson() const
{
    QJsonObject json = LtgObject::toJson();
    json["penWidth"] = m_penWidth;
    return json;
}

void LtgGraphObject::fromJson(const QJsonObject &json)
{
    LtgObject::fromJson(json);
    if (const QJsonValue v = json["penWidth"]; v.isDouble())
        m_penWidth = v.toInt(1);
}

void LtgLine::paint(QPainter *painter)
{
    painter->save();

    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(pen);

    rect() = rect().normalized();
    if(drawBoundingRect())
        painter->drawRect(rect());

    pen.setWidth(penWidth());

    painter->setPen(pen);
    painter->drawLine(rect().topLeft(), rect().bottomRight());
    painter->restore();
}

QJsonObject LtgLine::toJson() const
{
    QJsonObject json = LtgGraphObject::toJson();
    json["type"] = "line";
    return json;
}

void LtgLine::fromJson(const QJsonObject &json)
{
    LtgGraphObject::fromJson(json);
}

QBrush &LtgFigObject::brush()
{
    return m_brush;
}

QJsonObject LtgFigObject::toJson() const
{
    QJsonObject json = LtgGraphObject::toJson();

    // https://stackoverflow.com/questions/29168588/how-to-convert-a-string-variable-to-qbrushstyle-in-qt
    QVariant const vColor = m_brush.color();
    json["brush.color"] = vColor.toString();

    QVariant const vBrush = QVariant::fromValue(m_brush.style());
    json["brush.style"] = vBrush.toString();
    return json;
}

void LtgFigObject::fromJson(const QJsonObject &json)
{
    LtgGraphObject::fromJson(json);

    if (json.contains("brush.color")) {
        const QJsonValue v = json["brush.color"];
        QVariant vColor = v.toVariant();
        m_brush.setColor(QColor::fromRgba(vColor.toUInt()));
    }

    if (json.contains("brush.style")) {
        const QJsonValue v = json["brush.style"];
        QVariant vStyle = v.toVariant();
        m_brush.setStyle( vStyle.value<Qt::BrushStyle>() );
    }
}

void LtgRect::paint(QPainter *painter)
{
    painter->save();
    painter->setBrush(brush());
    QPen pen(Qt::black, penWidth(), Qt::SolidLine);
    painter->setPen(pen);

    painter->drawRect(rect());
    painter->restore();
}

QJsonObject LtgRect::toJson() const
{
    QJsonObject json = LtgFigObject::toJson();
    json["type"] = "rect";
    return json;
}

void LtgRect::fromJson(const QJsonObject &json)
{
    LtgFigObject::fromJson(json);
}

void LtgEllipse::paint(QPainter *painter)
{
    painter->save();
    painter->setBrush(brush());

    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(pen);

    rect() = rect().normalized();
    if(drawBoundingRect())
        painter->drawRect(rect());

    pen.setWidth(penWidth());

    painter->setPen(pen);
    painter->drawEllipse(rect());
    painter->restore();
}

QJsonObject LtgEllipse::toJson() const
{
    QJsonObject json = LtgFigObject::toJson();
    json["type"] = "ellipse";
    return json;
}

void LtgEllipse::fromJson(const QJsonObject &json)
{
    LtgFigObject::fromJson(json);
}

QString &LtgVarBaseObject::var()
{
    return m_var;
}

QJsonObject LtgVarBaseObject::toJson() const
{
    QJsonObject json = LtgObject::toJson();
    json["var"] = m_var;
    return json;
}

void LtgVarBaseObject::fromJson(const QJsonObject &json)
{
    LtgObject::fromJson(json);
    if (const QJsonValue v = json["var"]; v.isString())
        m_var = v.toString();
}

QFont &LtgText::font()
{
    return m_font;
}

QString &LtgText::text()
{
    return m_text;
}

void LtgText::paint(QPainter *painter)
{
    painter->save();
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(pen);
    painter->setFont(m_font);

    rect() = rect().normalized();
    if(drawBoundingRect())
        painter->drawRect(rect());

    if(angle()) {
        painter->setViewport(rect());
        if(angle() == 180) {
            painter->setWindow(0, 0, rect().width(), rect().height());
            painter->translate(rect().width(), 2);
            painter->rotate(180);
        } else if(angle() == 90 ) {
            painter->setWindow(0, 0, rect().width(), rect().height());
            painter->rotate(90);
        } else if(angle() == 270 ) {
            painter->setWindow(0, 0, rect().width(), rect().height());
            painter->translate(rect().width(), rect().height());
            painter->rotate(270);
        }
        painter->drawText(
            0, 0,
            m_text);
//        painter->drawText(
//            QRect{ 0, 0, rect().height(), rect().width() },
//            //Qt::AlignCenter,
//            m_text);
    } else {
        //painter->drawText(
        //    rect().left(), rect().top(),
        //    m_text);
        painter->drawText(
            QRect{
                rect().left(), rect().top(),
                rect().width(),rect().height()
            }, Qt::AlignVCenter, m_text);
    }

    painter->restore();
}

QJsonObject LtgText::toJson() const
{
    QJsonObject json = LtgVarBaseObject::toJson();
    json["type"] = "text";
    json["text"] = m_text;
    json["font"] = m_font.toString();
    return json;
}

void LtgText::fromJson(const QJsonObject &json)
{
    LtgVarBaseObject::fromJson(json);
    if (const QJsonValue v = json["text"]; v.isString())
        m_text = v.toString();
    if (const QJsonValue v = json["font"]; v.isString())
        m_font.fromString( v.toString() );
}

void LtgImage::setPath(QString path)
{
    if(!path.isEmpty()) {
        m_encodedImage = "";
        m_path = path;
    }
}

void LtgImage::setStretch(bool stretch)
{
    if(stretch)
        m_maintainAspectRatio = false;
    m_stretch = stretch;
}

void LtgImage::setMaintainAspectRatio(bool maintainAspectRatio)
{
    if(maintainAspectRatio)
        m_stretch = false;
    m_maintainAspectRatio = maintainAspectRatio;
}

void LtgImage::setEncodedImage(QByteArray&& encodedImage)
{
    if(!encodedImage.isEmpty()) {
        m_path = "";
        m_encodedImage = std::move(encodedImage);
    }
}

QByteArray &LtgImage::encodedImage()
{
    return m_encodedImage;
}

void LtgImage::paint(QPainter *painter)
{
    painter->save();
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(pen);

    rect() = rect().normalized();
    if(drawBoundingRect())
        painter->drawRect(rect());

    bool mustPaint{ false };
    QImage px;

    if(m_path!="") {
        QFile f(m_path);
        if(f.exists()) {
            px.load(m_path);
            mustPaint = true;
        }
    } else if(!m_encodedImage.isEmpty()) {
        px.loadFromData(QByteArray::fromBase64(m_encodedImage));
        mustPaint = true;
    }

    if( mustPaint ) {
        if(!px.isNull()) {
            QPoint destPoint;

            if(angle()) {
                painter->setViewport(rect());
                if(angle() == 180) {
                    painter->setWindow(0, 0, rect().width(), rect().height());
                    painter->translate(rect().width(), rect().height());
                    painter->rotate(180);
                } else if(angle() == 90 ) {
                    painter->setWindow(0, 0, rect().width(), rect().height());
                    painter->translate(rect().width(), 0);
                    painter->rotate(90);
                } else if(angle() == 270 ) {
                    painter->setWindow(0, 0, rect().width(), rect().height());
                    painter->translate(0, rect().height());
                    painter->rotate(270);
                }
                destPoint = QPoint{ 0, 0 };
            } else {
                destPoint = rect().topLeft();
            }

            if(m_stretch) {
                QRect target;
                if(angle() == 0 || angle() == 180)
                  target = QRect(destPoint.x(), destPoint.y(), rect().width(), rect().height());
                else
                   target = QRect(destPoint.x(), destPoint.y(), rect().height(), rect().width());

                QRect source(0, 0, px.width(), px.height());
                painter->drawImage(target, px, source);
            } else if (m_maintainAspectRatio) {
                QImage tempQImage = px.scaled(rect().width(), rect().height(), Qt::KeepAspectRatio);

                double imageSizeWidth = static_cast<double>(tempQImage.width());
                double imageSizeHeight = static_cast<double>(tempQImage.height());

                int centX, centY;
                if(angle() == 0 || angle() == 180) {
                    centX = rect().width()/2;
                    centY = rect().height()/2;
                } else {
                    centX = rect().height()/2;
                    centY = rect().width()/2;
                }

                QRectF dest{
                    destPoint.x()+centX-imageSizeWidth/2,      // X1
                    destPoint.y()+centY-imageSizeHeight/2,     // Y1
                    imageSizeWidth,      // Width
                    imageSizeHeight};    // Height

                //qDebug() << this->x1() << this->y1() << centX << centY << imageSizeWidth << imageSizeHeight << dest;//deltaX << deltaY, QRect(this->x1(), this->y1(), this->x2(), this->y2());

                painter->drawImage(dest, tempQImage);
            } else {
                QRect r{rect()};

                r.setRight(r.left() + px.width() - 1);
                r.setBottom(r.top() + px.height() - 1);
                painter->drawImage(destPoint, px);
                //qDebug()<<this->x1() << this->y1() << this->image();
            }
        }
    }
    painter->restore();
}

QJsonObject LtgImage::toJson() const
{
    QJsonObject json = LtgVarBaseObject::toJson();
    json["type"] = "image";
    json["path"] = m_path;
    json["stretch"] = m_stretch;
    json["maintainAspectRatio"] = m_maintainAspectRatio;
    json["encodedImage"] = QString(m_encodedImage);
    return json;
}

void LtgImage::fromJson(const QJsonObject &json)
{
    LtgVarBaseObject::fromJson(json);
    if (const QJsonValue v = json["path"]; v.isString())
        m_path = v.toString();
    if (const QJsonValue v = json["stretch"]; v.isBool())
        m_stretch = v.toBool();
    if (const QJsonValue v = json["maintainAspectRatio"]; v.isBool())
        m_maintainAspectRatio = v.toBool();
    if (const QJsonValue v = json["encodedImage"]; v.isString())
        m_encodedImage = v.toString().toUtf8();
}


QString &LtgBarcode::text()
{
    return m_text;
}

void LtgBarcode::setSizeMode(LtgBarcode::SizeMode sizeMode)
{
    m_sizeMode = sizeMode;
}

const LtgBarcode::SizeMode LtgBarcode::sizeMode()
{
    return m_sizeMode;
}

void LtgBarcode::setBearerBarType(BearerBarType bearerBarType)
{
    m_bearerBarType = bearerBarType;
}

const LtgBarcode::BearerBarType LtgBarcode::bearerBarType()
{
    return m_bearerBarType;
}

void LtgBarcode::setBearerBarWidth(int bearerBarWidth)
{
    m_bearerBarWidth = bearerBarWidth;
}

const int LtgBarcode::bearerBarWidth()
{
    return m_bearerBarWidth;
}

void LtgBarcode::paint(QPainter *painter)
{
    painter->save();
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(pen);

    rect() = rect().normalized();
    if(drawBoundingRect())
        painter->drawRect(rect());

    BCSetDecoder(m_barCodeHandle, eDecoder_Software);
    BCSetMustFit(m_barCodeHandle, true);
    BCSetSizeMode(m_barCodeHandle, static_cast<e_SizeMode>(m_sizeMode));

    BCSetBCType(m_barCodeHandle, static_cast<e_BarCType>(m_typeBarcode));

    BCSetBearerBarType(m_barCodeHandle, static_cast<e_BearerBar>(m_bearerBarType));
    BCSetBearerBarWidth(m_barCodeHandle, m_bearerBarWidth);

    BCSetDPI(m_barCodeHandle, 96, 96);
    if(m_setPrintText) {
        BCSetFontHeight(m_barCodeHandle, m_fontSize);
        BCSetPrintText(m_barCodeHandle, true, false);
    } else {
        BCSetPrintText(m_barCodeHandle, false, false);
    }

    BCSetCodePage(m_barCodeHandle, eCodePage_UTF8);


    wchar_t str[m_text.size() + 1];
    int length = m_text.toWCharArray(str);
    str[length] = '\0';

    m_errorCode = BCSetTextW(m_barCodeHandle, str, wcslen(str));

    if(m_errorCode==0) {
        //qDebug() << "PASO 1" << FErrorCode;
        m_errorCode = BCCreate(m_barCodeHandle);
    }

    if(m_errorCode == 0)
    {
        RECT r;
        r.left = 0;
        r.top = 0;
        if(angle() == 90 || angle() == 270) {
            r.right = rect().height();
            r.bottom = rect().width();
        } else {
            r.right = rect().width();
            r.bottom = rect().height();
        }

        HBITMAP hbmp = CreateBitmap(r.right, r.bottom, 1, 0, 0);
        HDC hdcimage = CreateCompatibleDC(NULL);
        SelectObject(hdcimage, hbmp);

        //qDebug() << r.right << r.bottom;

        m_errorCode = BCDraw(m_barCodeHandle, hdcimage, &r);
        QImage image(QImage::fromHBITMAP(hbmp));

        //image.save("salida2.bmp");

        RECT r2{ r };
        BCGetOptimalBitmapSize(m_barCodeHandle, &r2, 0.0, 0.0);

        QPoint destPoint;

        if(angle()) {
            painter->setViewport(rect());
            if(angle() == 180) {
                painter->setWindow(0, 0, rect().width(), rect().height());
                painter->translate(rect().width(), rect().height());
                painter->rotate(180);
            } else if(angle() == 90 ) {
                painter->setWindow(0, 0, rect().width(), rect().height());
                painter->translate(rect().width(), 0);
                painter->rotate(90);
            } else if(angle() == 270 ) {
                painter->setWindow(0, 0, rect().width(), rect().height());
                painter->translate(0, rect().height());
                painter->rotate(270);
            }
            destPoint = QPoint{ 0, 0 };
        } else {
            destPoint = rect().topLeft();
        }

        painter->drawImage(destPoint, image);
    }
    painter->restore();

}

QJsonObject LtgBarcode::toJson() const
{
    QJsonObject json = LtgVarBaseObject::toJson();
    json["type"] = "barcode";
    json["text"] = m_text;

    // https://gist.github.com/ukll/76e21e5ef0c36e1e608a04c0deb99d41
    QMetaObject mo = LtgBarcode::staticMetaObject;

    QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("BarcodeType"));
    json["typeBarcode"] = QString(metaEnum.valueToKey( static_cast<e_BarCType>(m_typeBarcode) ));

    metaEnum = mo.enumerator(mo.indexOfEnumerator("SizeMode"));
    json["sizeMode"] = QString(metaEnum.valueToKey( static_cast<e_SizeMode>(m_sizeMode) ));

    metaEnum = mo.enumerator(mo.indexOfEnumerator("BearerBarType"));
    json["bearerBarType"] = QString(metaEnum.valueToKey( static_cast<e_BearerBar>(m_bearerBarType) ));

    json["bearerBarWidth"] = m_bearerBarWidth;

    json["fontSize"] = m_fontSize;
    json["setPrintText"] = m_setPrintText;

    return json;
}

void LtgBarcode::fromJson(const QJsonObject &json)
{
    LtgVarBaseObject::fromJson(json);
    if (const QJsonValue v = json["text"]; v.isString())
        m_text = v.toString();

    QMetaObject mo = LtgBarcode::staticMetaObject;

    m_typeBarcode = BarcodeType::eBC_None;
    if (const QJsonValue v = json["typeBarcode"]; v.isString()) {
        QString str = v.toString();
        QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("BarcodeType"));

        BarcodeType enumValue = static_cast<BarcodeType>(metaEnum.keyToValue(str.toLatin1()));
        if(enumValue < BarcodeType::eBC_None || enumValue > BarcodeType::eBC_SwissQrCode)
            enumValue = BarcodeType::eBC_None;
        m_typeBarcode = enumValue;
    }

    m_sizeMode = SizeMode::eSizeMode_Default;
    if (const QJsonValue v = json["sizeMode"]; v.isString()) {
        QString str = v.toString();
        QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("SizeMode"));

        SizeMode enumValue = static_cast<SizeMode>(metaEnum.keyToValue(str.toLatin1()));
        if(enumValue < SizeMode::eSizeMode_Default || enumValue > SizeMode::eSizeMode_MinimalModuleWidth)
            enumValue = SizeMode::eSizeMode_Default;
        m_sizeMode = enumValue;
    }

    m_bearerBarType = BearerBarType::eBearerBar_None;
    if (const QJsonValue v = json["bearerBarType"]; v.isString()) {
        QString str = v.toString();
        QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("BearerBarType"));

        BearerBarType enumValue = static_cast<BearerBarType>(metaEnum.keyToValue(str.toLatin1()));
        if(enumValue < BearerBarType::eBearerBar_None || enumValue > BearerBarType::eBearerBar_Bottom)
            enumValue = BearerBarType::eBearerBar_None;
        m_bearerBarType = enumValue;
    }

    m_bearerBarWidth = -1;
    if (const QJsonValue v = json["bearerBarWidth"]; v.isDouble())
        m_bearerBarWidth = v.toInt(-1);

    m_fontSize = 10;
    if (const QJsonValue v = json["fontSize"]; v.isDouble())
        m_fontSize = v.toInt(10);

    m_setPrintText = true;
    if (const QJsonValue v = json["setPrintText"]; v.isBool())
        m_setPrintText = v.toBool();
}
