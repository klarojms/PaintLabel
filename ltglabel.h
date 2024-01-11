#ifndef LTGLABEL_H
#define LTGLABEL_H

#include <QObject>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QJSEngine>

#include "tbarcode.h"
#include "ltgvar.h"

//class LtgObject : public QObject {
class LtgObject: public QObject {
    Q_OBJECT
public:

    LtgObject(QObject *parent = nullptr)
        : QObject{ parent }
        , m_comment{ "" }
        , m_rect{}
        , m_originalRect{}
        , m_angle{0}
        , m_drawBoundingRect{false} {};

    // Para poder ejecutar destructores override
    virtual ~LtgObject() {};

    QRect& rect();
    QRect& originalRect();

    int angle();

    void applyRotation(int angle);

    void setDrawBoundingRect(bool drawRect);
    bool drawBoundingRect() const;

    virtual void paint(QPainter *painter) = 0;

    virtual QJsonObject toJson() const;

    virtual void fromJson(const QJsonObject &json);

protected:
    void setAngle(int angle);

private:
    QString m_comment;
    QRect m_rect;
    QRect m_originalRect;
    int m_angle;
    bool m_drawBoundingRect;
};

class LtgGraphObject : public LtgObject {
public:
    LtgGraphObject(int penWidth = 0) : LtgObject{}, m_penWidth{ penWidth } {};

    int penWidth();
    void setPenWidth(int penWidth);

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    int m_penWidth{ 0 };
};

class LtgLine : public LtgGraphObject {
public:
    LtgLine(int penWidth = 0) : LtgGraphObject{penWidth} {};

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
};


class LtgFigObject : public LtgGraphObject {
public:
    LtgFigObject(int penWidth = 0) : LtgGraphObject{penWidth} {};

    QBrush& brush();

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    QBrush m_brush{ Qt::white, Qt::NoBrush };
};

class LtgRect : public LtgFigObject {
public:
    LtgRect(int penWidth = 0) : LtgFigObject{penWidth} {};

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
};

class LtgEllipse : public LtgFigObject {
public:
    LtgEllipse(int penWidth = 0) : LtgFigObject{penWidth} {};

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
};

class LtgVarBaseObject : public LtgObject {
public:
    LtgVarBaseObject()
        : LtgObject{}
        , m_var{""}
        {};

    QString& var();

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    QString m_var;
};

class LtgText : public LtgVarBaseObject {
public:
    LtgText(QString text="")
        : LtgVarBaseObject{}
        , m_font{"Arial", 10}
        , m_text{text}
    {};

    QFont& font();
    QString& text();

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    QString m_text;
    QFont m_font;
};

class LtgImage : public LtgVarBaseObject {
public:
    LtgImage(QString text = "")
        : LtgVarBaseObject{}
        , m_path{text}
        , m_stretch{false}
        , m_maintainAspectRatio{false}
        , m_encodedImage{}
        {};

    void setPath(QString path);

    void setStretch(bool stretch);

    void setMaintainAspectRatio(bool maintainAspectRatio);

    void setEncodedImage(QByteArray&& image);
    QByteArray& encodedImage();

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    QString m_path;
    bool m_stretch;
    bool m_maintainAspectRatio;
    QByteArray m_encodedImage;
};

class LtgBarcode : public LtgVarBaseObject {
    Q_OBJECT
public:

    enum class BarcodeType {
        eBC_None        = 0,
        eBC_Code11      = 1,
        eBC_2OF5        = 2,
        eBC_2OF5IL      = 3,
        eBC_2OF5IATA    = 4,
        eBC_2OF5M       = 5,
        eBC_2OF5DL      = 6,
        eBC_2OF5IND     = 7,    // Code 2OF5 Industry
        eBC_3OF9        = 8,    // Code 39
        eBC_3OF9A       = 9,    // Code 39 Full ASCII"
        eBC_EAN8        = 10,   // EAN8
        eBC_EAN8P2      = 11,   // EAN8P2
        eBC_EAN8P5      = 12,   // EAN8P5
        eBC_EAN13       = 13,   // EAN13
        eBC_EAN13P2     = 14,   // EAN13P2
        eBC_EAN13P5     = 15,   // EAN13P5"
        eBC_EAN128      = 16,   // UCC/EAN-128
        eBC_UPC12       = 17,   // UPC12
        eBC_CodaBar2    = 18,   // Codabar 2 Widths
        eBC_CodaBar18   = 19,   // Codabar 18 Widths
        eBC_Scan1       = 19,   // Scan 1
        eBC_Code128     = 20,   // Code128"
        eBC_DPLeit      = 21,   // DP Leitcode
        eBC_DPIdent     = 22,   // DP Identcode
        eBC_ISBN13P5    = 23,   // ISBN13P5
        eBC_ISMN        = 24,   // ISMN
        eBC_9OF3        = 25,   // Code 93
        eBC_ISSN        = 26,   // ISSN"
        eBC_ISSNP2      = 27,   // ISSNP2
        eBC_Flattermarken = 28, // Flattermarken"
        eBC_GS1DataBar  = 29,   // GS1 DataBar (RSS-14)
        eBC_RSS14       = 29,   // GS1 DataBar (RSS-14)
        eBC_GS1DataBarLtd = 30, // GS1 DataBar Limited (RSS)
        BC_RSSLtd      = 30,    // GS1 DataBar Limited (RSS)
        eBC_GS1DataBarExp = 31, // GS1 DataBar Expanded (RSS)
        eBC_RSSExp      = 31,   // GS1 DataBar Expanded (RSS)
        eBC_TelepenAlpha = 32,  // Telepen Alpha
        eBC_UCC128      = 33,   // UCC128"
        eBC_UPCA        = 34,   // UPCA
        eBC_UPCAP2      = 35,   // UPCAP2
        eBC_UPCAP5      = 36,   // UPCAP5
        eBC_UPCE        = 37,   // UPCE
        eBC_UPCEP2      = 38,   // UPCEP2
        eBC_UPCEP5      = 39,   // UPCEP5
        eBC_USPSPostNet5  = 40, // USPS PostNet5
        eBC_USPSPostNet6  = 41, // USPS PostNet6
        eBC_USPSPostNet9  = 42, // USPS PostNet9
        eBC_USPSPostNet10 = 43, // USPS PostNet10
        eBC_USPSPostNet11 = 44, // USPS PostNet11
        eBC_USPSPostNet12 = 45, // USPS PostNet12
        eBC_Plessey     = 46,   // Plessey
        eBC_MSI         = 47,   // MSI
        eBC_SSCC18      = 48,   // SSCC-18
        eBC_FIM         = 49,   // FIM
        eBC_Scan2       = 49,   // Scan 2
        eBC_LOGMARS     = 50,   // LOGMARS
        eBC_Pharma1     = 51,   // Pharmacode One-Track
        eBC_PZN7        = 52,   // PZN7
        eBC_Pharma2     = 53,   // Pharmacode Two-Track
        eBC_CEPNet      = 54,   // Brazilian CEPNet
        eBC_PDF417      = 55,   // PDF417
        eBC_PDF417Trunc = 56,   // PDF417 Truncated
        eBC_MAXICODE    = 57,   // MaxiCode
        eBC_QRCode_JIS   = 58,  // QR-Code (JIS)
        eBC_QRCode      = 58,   // QR-Code (JIS - deprecated)
        EBC_Code128A    = 59,   // Code128A
        eBC_Code128B    = 60,   // Code128B
        eBC_Code128C    = 61,   // Code128C
        eBC_9OF3A       = 62,   // Code 93 Full ASCII
        eBC_AusPostCustom = 63, // Australian Post Custom
        eBC_AusPostCustom2 = 64,    // Australian Post Custom2
        eBC_AusPostCustom3 = 65,    // Australian Post Custom3
        eBC_AusPostReplyPaid = 66,  // Australian Post Reply Paid
        eBC_AusPostRouting = 67,    // Australian Post Routing
        eBC_AusPostRedirect = 68,   // Australian Post Redirect
        eBC_ISBN13 = 69,            // ISBN 13
        eBC_RM4SCC      = 70,       // Royal Mail 4 State (RM4SCC)
        eBC_DataMatrix  = 71,       // Data Matrix
        eBC_EAN14       = 72,       // EAN14
        eBC_VIN         = 73,       // VIN / FIN
        eBC_CODABLOCK_F = 74,       // CODABLOCK-F
        eBC_NVE18       = 75,
        eBC_JapanesePostal  = 76,
        eBC_KoreanPostalAuth= 77,
        eBC_GS1DataBarTrunc = 78,
        eBC_RSS14Trunc   = 78,
        eBC_GS1DataBarStacked = 79,
        eBC_RSS14Stacked = 79,
        eBC_GS1DataBarStackedOmni = 80,
        eBC_RSS14StackedOmni = 80,
        eBC_GS1DataBarExpStacked = 81,
        eBC_RSSExpStacked = 81,
        eBC_Planet12     = 82,
        eBC_Planet14     = 83,
        eBC_MicroPDF417  = 84,
        eBC_USPSIntelligentMail = 85,
        eBC_USPSOneCode4CB = 85,
        eBC_PlesseyBidir = 86,
        eBC_Telepen = 87,
        eBC_GS1_128 = 88,
        eBC_ITF14 = 89,
        BC_KIX = 90,
        eBC_Code32 = 91,
        eBC_Aztec = 92,
        eBC_DAFT = 93,
        eBC_ItalianPostal2Of5 = 94,
        eBC_ItalianPostal3Of9 = 95,
        eBC_DPD = 96,
        eBC_MicroQRCode = 97,
        eBC_HIBCLic128 = 98,
        eBC_HIBCLic3OF9 = 99,
        eBC_HIBCPas128 = 100,   // HIBC PAS 128
        eBC_HIBCPas3OF9 = 101,  // HIBC PAS 39
        eBC_HIBCLicDataMatrix = 102,    // HIBC LIC Data Matrix
        eBC_HIBCPasDataMatrix = 103,    // HIBC PAS Data Matrix
        eBC_HIBCLicQRCode = 104, // HIBC LIC QR-Code
        eBC_HIBCPasQRCode = 105, // HIBC PAS QR-Code
        eBC_HIBCLicPDF417 = 106, // HIBC LIC PDF417
        eBC_HIBCPasPDF417 = 107, // HIBC PAS PDF417
        eBC_HIBCLicMPDF417 = 108,// HIBC LIC MicroPDF417
        eBC_HIBCPasMPDF417 = 109,// HIBC PAS MicroPDF417
        eBC_HIBCLicCODABLOCK_F = 110,   // HIBC LIC CODABLOCK-F
        eBC_HIBCPasCODABLOCK_F = 111,   // HIBC PAS CODABLOCK-F
        eBC_QRCode_ISO = 112,   // QR-Code
        eBC_QRCode2005 = 112,   // QR-Code 2005 (ISO - deprecated)
        eBC_PZN8 = 113,         // PZN8
        eBC_Reserved_114 = 114, // Reserved
        eBC_DotCode = 115,      // DotCode
        eBC_HanXin = 116,       // Han Xin Code
        eBC_USPSIMPackage = 117,    // USPS Intelligent Mail Package (IMpb)
        eBC_SwedishPostal = 118,    // Swedish Postal Shipment Item ID
        eBC_2D_Mailmark = 119,  // Royal Mail Mailmark 2D
        eBC_CMDM_Mailmark = 119,// Royal Mail CMDM Mailmark
        eBC_UpuS10 = 120,       // UPU S10
        eBC_4state_Mailmark = 121,  // Royal Mail Mailmark 4-state
        eBC_HIBCLicAztec = 122, // HIBC LIC Aztec Code
        eBC_HIBCPasAztec = 123, // HIBC PAS Aztec Code
        eBC_PPN = 124,          // PPN
        eBC_NTIN = 125,         // NTIN (Data Matrix)
        eBC_SwissQrCode = 126   // Swiss QR Code
    };
    Q_ENUM(BarcodeType)

    enum class SizeMode {
        eSizeMode_Default = -1,
        eSizeMode_FitToBoundingRect = 0,
        eSizeMode_CustomModuleWidth = 1,
        eSizeMode_MinimalModuleWidth = 2,
    };
    Q_ENUM(SizeMode)

    enum class BearerBarType {
        eBearerBar_None = 0,
        eBearerBar_TopAndBottom = 1,
        eBearerBar_Rectangle = 2,
        eBearerBar_Top = 3,
        eBearerBar_Bottom = 4,
    };
    Q_ENUM(BearerBarType)


    LtgBarcode(BarcodeType typeBarcode = BarcodeType::eBC_None, QString text = "")
        : LtgVarBaseObject{}
        , m_text{text}
        , m_barCodeHandle{ nullptr }
        , m_typeBarcode{ typeBarcode }
        , m_sizeMode{ eSizeMode_Default }
        , m_bearerBarType{ eBearerBar_None }
        , m_bearerBarWidth{ -1 }
        , m_setPrintText{ true }
        , m_fontSize{ 10 }
    {
        m_errorCode = BCAlloc(&m_barCodeHandle);
    };

    ~LtgBarcode() override
    {
        // Recordar definir el destructor padre como virtual
        m_errorCode = BCFree(m_barCodeHandle);
    }

    QString& text();

    void setSizeMode(LtgBarcode::SizeMode sizeMode);
    const SizeMode sizeMode();

    void setBearerBarType( BearerBarType bearerBarType);
    const BearerBarType bearerBarType();

    void setBearerBarWidth( int bearerBarWidth );
    const int bearerBarWidth();

    void paint(QPainter *painter) override;

    QJsonObject toJson() const override;

    void fromJson(const QJsonObject &json) override;
private:
    QString m_text;
    t_BarCode* m_barCodeHandle;
    ERRCODE m_errorCode;
    BarcodeType m_typeBarcode;
    SizeMode m_sizeMode;
    BearerBarType m_bearerBarType;
    int m_bearerBarWidth;
    bool m_setPrintText;
    int m_fontSize;

    //QMap<e_BarCType, QString> m_bcToString;
};

class LtgLabel : public QObject
{
    Q_OBJECT
public:
    LtgLabel(QObject *parent = nullptr);
    ~LtgLabel();

    int width();
    int height();
    void setWidth(int width);
    void setHeight(int height);

    QList<std::shared_ptr<LtgObject>> &objects();
    QMap<QString, std::shared_ptr<LtgVar>> &vars();

    void paint();

    QImage &image();

    void setDrawBoundingRect(bool drawBoundingRect);
    bool drawBoundingRect() const;

    QJsonObject toJson() const;

    void fromJson(const QJsonObject &json);

    bool save(QString fileName) const;
    bool read(QString fileName);

    void calcVars();
    QJSEngine &engine();

private:
    QString m_comment;
    int m_width{ 0 };
    int m_height{ 0 };

    QList<std::shared_ptr<LtgObject>> m_objects; // En Qt 6.0 QList y QVector son casi identicos
    QMap<QString, std::shared_ptr<LtgVar>> m_vars;
    std::shared_ptr<QImage> m_img;

    bool m_drawBoundingRect{ false };

    QJSEngine m_engine;
};

#endif // LTGLABEL_H
