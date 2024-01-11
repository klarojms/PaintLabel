#include <QGraphicsScene>
#include <QPixmap>
#include <QDebug>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonDocument>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ltglabel.h"
//#include <variant>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_measured{ false }
{
    ui->setupUi(this);

/*
    m_lbl.setWidth(1200);
    m_lbl.setHeight(512);
    m_lbl.setDrawBoundingRect(true);

    LtgObject *obj = new LtgLine(10);
    obj->rect() = QRect(200, 150, 400, 300);
    //obj->setDrawBoundingRect(true);
    m_lbl.objects().append(obj);

    obj = new LtgRect(5);
    obj->rect() = QRect(250, 150, 100, 200);
    //static_cast<LtgFigObject*>(obj)->brush().setColor(Qt::GlobalColor::black);
    //static_cast<LtgFigObject*>(obj)->brush().setStyle(Qt::CrossPattern);
    m_lbl.objects().append(obj);

    obj = new LtgEllipse(5);
    obj->rect() = QRect(400, 350, 200, 110);
    //obj->setDrawBoundingRect(true);
    m_lbl.objects().append(obj);

    obj = new LtgText("Limitronic");
    obj->rect() = QRect(10, 400, 350, 100);
    static_cast<LtgText*>(obj)->font().setPointSize(50);
    static_cast<LtgText*>(obj)->font().setFamily("Arial");
    static_cast<LtgText*>(obj)->font().setBold(true);
    //obj->setDrawBoundingRect(true);
    obj->applyRotation(270);
    m_lbl.objects().append(obj);

    obj = new LtgText("Wellcome to the new age!\nWellcome to the future!");
    obj->rect() = QRect(10, 10, 670, 150);
    //static_cast<LtgText*>(obj)->font().setFamily("Arial");
    //static_cast<LtgText*>(obj)->font().setBold(true);
    static_cast<LtgText*>(obj)->font().setPointSize(30);
    //obj->setDrawBoundingRect(true);
    m_lbl.objects().append(obj);

    // Si ponemos una fuente unicode o del tipo que estamos usando, funciona
    // mucho más rápido (de 1300 ms a 45 ms)
    obj = new LtgText("èáñôç車B1234 こんにちは");
    obj->rect() = QRect(10, 80, 670, 150);
    static_cast<LtgText*>(obj)->font().setFamily("Arial Unicode MS");
    //static_cast<LtgText*>(obj)->font().setBold(true);
    static_cast<LtgText*>(obj)->font().setPointSize(30);
    obj->setDrawBoundingRect(true);
    m_lbl.objects().append(obj);

    obj = new LtgImage();
    //obj->rect() = QRect(700, 100, 300, 300);
    obj->rect() = QRect(800, 10, 500, 240);
    static_cast<LtgImage*>(obj)->setEncodedImage(
        "iVBORw0KGgoAAAANSUhEUgAAAXkAAAF5CAIAAAD1Yc1qAAAAA3NCSVQICAjb4U/gAAAMNElEQVR4nO3d23LruBVFUSrV///LzINTbsWyKYqXBWBjjKeudOcciticAilflgXgZuu6/qf1MQBT0BogQWuABK0BErQGSNAaIEFrgAStARK0BkjQGiBBa4AErQEStAZI0BogQWuABK0BErQGSNAaIEFrgAStARK0BkjQGiBBa4AErQEStAZI0BogQWuABK0BErQGSNAaIEFrgIR/7v4L1nW9+68ATno8Hnf/FfY1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVrDEY/HI/BbPqjk9t8PRTHPifn6Z78CjD20hr3+2sgoDntoDe/tuV1SHLZpDVs+fSijOPxFa/jdmUe/isMrreGnqz5gUhyeaQ3/uuNjbMXhi9awLPdU5vXPV5yZac3skl+Spzgz05p5tfrCX8WZk9bMqIdvL1Cc2WjNXHqozDPFmYfWzKK3yjxTnBloTX09V+aZ4tSmNZWNUplnilOV1tQ0YmWeKU49WlPN6JV5pjiVaE0dlSrzTHFq0JoKqlbmmeKMTmvGNkNlninOuLRmVLNV5pnijEhrxjNzZZ4pzljqtObHFVhyBFXmVeHiPC93gRd4++zefY72XH4V1klldphkoe94mXcP2LquA7fmo7Mz7hSqzKcmWetrX6bW/OnYqRlrClXmjBnW+sLXGGjNkL9j9/B5GeXq9RtszxvoHJaf5y/jtebk+e18eQa6QobQ//msPc/PBvscaqAz+6nCL625wp9VDWS8fc15vV3V/b/31tDhSb7kkDp8Xb+asTW98X6b4Ty3NVJrRun3Aeu6uhLu0+fpvXCeh7g0RmrNhfpcmz4viaE5pf2YtDU9c21cxZnsitY08PZhsHfjk/acwD73toUN9pl3JW8/iP36Vy6Jj+xptFPahH1NY2/n3gZnp517GaFpxb6mPRuc89wx9U9reqE4x7hpGoXW9OXxeGxfPIrzTWXG4nlNd/Y8U/AQx6OZ4djXdMot1V9sZwalNV1TnGcqMzT3UAPYc0tV/q7KTdPo7GvGsOcnsKzrWvJi83l2DVozktluqdw0VaI145nhc3GVqcfzmiHV/lzco5mS7GsGVu+WynamMK0ZXo3iqEx57qGKGPpzcTdNM7CvqWPEz8V9nj0PralmlFsqN02z0Zqaei6OyszJ85rKOnyI49HMtOxriuvnIY5HM5PTmim0vaVy08TiHmoqTW6p3DTxxb5mLslbKjdNPNOaGd19S+WmiVdaM6+bimM7w6+0ZnYX/oQKlWGD1nDBQxyV4S2t4X+O3VJ5NMNOPvPm/3z0ubjPs9nPvoafdt5S7flD4Jt9Db87syURGl7Z17Blzx7n9b+HV/Y1vLenIB7NsM2+hl02NjgSwx72NXxAVjhMa4AErQEStAZI0BogQWuABK0BErQGSNAaIEFrgAStARK0BkjQGiBBa4AErQEStAZI0BogQWuABK0BErQGSNAaIGHS1uz/hUdwk9mGcLrWPP86amhrqlGcqzVTLS1DmOfNb6LWTLKijGiG4ZylNTOsJUMrP6JTtKb8KlJD7UGt35ra60cxhce1eGsKrxxVVR3ayq2pumaUV3J0y7am5Goxj3oDXLM19daJCRUb439aH0Bjj8fj1//91mVe1/Wvv5eYwJXcZLq6VbA1Oxdy+2r/+rf3zcTXn6w4rdx9tV81XZXelqq1Zs/67V+8QHHKTNIo2lbm9b+cZ49T83nNhgPX9q05mOfbYXrQT2j2/1/KjMdcrTlcjbt3H2XmqVuBptuibivVmluHKZAbxblD5sSeGY9JtjalWrPtfCwCb1yKc63MyRxitJqbqDWXyMyE3JwXq/YMmbjELK25cCBiuVGcw2Knbri5amiW1lwrNhaK86nkGStfh2tpzUHJOZObPcJdFppPac1x4dwozobwyRGaA7TmlPDMyc2rfIWF5hitOSufG8X50uRUCM1hWnOB/PwpTpPKCM0ZWnONJlM4Z26qbmfKr2ap1mwMROar1G1w7tbkxTbfzjQ/gEuUak0PbHBu0qqqNa7zHkzUmtpf4lV4g9PwpSW/aDPzFzVUrTWdvAu1Oox6xWn4cjoJTScjfV611myb5OtKa+SmbTfLXOH9KNia7SmZJzdDF6ftwffzDSg7j2SItR6pNVed0Elys4xZnObHPFxoRjFSa/br6gedNZ+YUXLTvDJLT6Gpp2ZrFrn5fz1cxtt6OLyuQtN8Zi5XtjWL3Lzo4Xp+1UkHheZug7Xm06GUmx86ubC/9XAw4S/4vjw0PZzDPQZrzR1my83SR3F6OIalv58K0smE3KF+a/YsXjg3ncxTq0u9k8osQpNVvzVLf7lZupmq/GXfSWWW/kLT1R97h/Fac+zkys2GzAvvZzuzdBmafubhJuO15jC52XBrCLqqzCI0jQzZmsODKzfb7ohCV5VZaoWmt3O7bcjWnCE3b134vSC9XQyVQjOc6VqzyM0O5zPRW2UWoWlt1NacHGW52ePYGehwO7NUDE2HJ3nbqK05T272+CgcfVZmqRiaEQ3cmvNjLTc77YlIt5UpGZo+z/a2gVtzCbnZ76/zYDvzzY5mw/C/9eaSletzRPq8gEfR4XpddUh3DMbdp2td19n3NV863N0sE78Bnic0HRq+NVederkpo3BohjZ8axa54Unt0Iy7qVlqtOZCcjO02qEZXZHWXHj9y82gyodm6E3NUqY1i9zMTWj6V6c119rzNWBy04nyoamhVGsuv/jlpn8zhKbApmYp1ppFbiYjNAOp1ppFbqYhNGMp2Jo7yE1vhGY4NVvT5BtG5CZmhtDUU7M1i9zUNUloim1qlsKtWcb8dtgDOjyk+wjNuCq3ZmmRmyZTMkluOnyZQrNf8dYs0+xuyuvwnAvNR+q35ia9jX5vx1Oe0HxqitbctIS/TlvDa75wbno7q4VP9X2maM2Syk3zEWx+AHdo/qIyq1x7U7MU+HnDH2k+tRldnfOTLFmGnzd8seYrCq8mGcu5WrPMsa5l9gJlXsiGGQbyy3StWeZY3QJXaYGX8NYMo/htxtZAD6YKzTJta2ZY5qH3BUMfPL+atDWL3HRs0MP+yAzj98O8rVmmXG96MOfgTd2aZYJVH26PMNwBf6r8yP1l9tYsE689eTMPm9YsS/UJGGinMNChHlB7zN7SGkiYPDSL1nyrPQpD7BeGOEgO05p/1c4NDRmtRWt+KDwTne8aOj+8MwoP1Ue05ieTwYWM0zet+UXV+eh279DtgZ1UdZCO0ZrfmRJOMkI/aA2NldzUCM0rrflTyXEpeWH3puTknKc1WwwNXEVr3qiXm662Nl0dzCXqDcxVtOY908NORmWD1uxihnjLkGzTmr0qTVIndy6dHMYlKo3HTbTmA+aJXxmMPbQGThGanbTmMwYLjtGaj9XITfNnJc0P4BI1hiFDa44wYSzG4ENac5A5m5wB+JTWHGfapmXpD9CaeTV8YjL0wxqhOUZrTjF2sJPWnCU3U7Hch2nNBczfJCz0GVpzDVNYniU+SWvgPaE5T2suYxx3GvpDKA7TmisNlxuX/R7DLWuftAZI0JqLeQ8sxoJeRWuABK25nnfCMizlhbQGSNAa+J1NzbW05hbGFH7QGiBBa4AErQEStAZ+4Ynb5bTmLoYVnmkNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUDCP60PoLJ1XVsfQo+cljnZ1wAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1AEAJ67ra1wAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkKA1QILWAAlaAyRoDZCgNUCC1gAJWgMkaA2QoDVAgtYACVoDJGgNkPBY17X1MQD1/RcCfvP7V/2XhAAAAABJRU5ErkJggg=="
    );
    //qDebug() << static_cast<LtgImage*>(obj)->encodedImage();
    //static_cast<LtgImage*>(obj)->setPath("PA008_MONO.BMP");
    //static_cast<LtgImage*>(obj)->setStretch(true);
    //static_cast<LtgImage*>(obj)->setMaintainAspectRatio(true);
    //static_cast<LtgImage*>(obj)->applyRotation(180);
    m_lbl.objects().append(obj);

    obj = new LtgBarcode(LtgBarcode::BarcodeType::eBC_Code128, "012345");
    obj->rect() = QRect(650, 300, 350, 120);
    //obj->setDrawBoundingRect(true);
    static_cast<LtgBarcode*>(obj)->applyRotation(270);
    m_lbl.objects().append(obj);
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    if(m_lbl.objects().size()>0) {
        QElapsedTimer timer;

        if(!m_measured)
            timer.start();

        m_lbl.paint();

        if(!m_measured) {
            qDebug() << "Pintar " << timer.elapsed() << " ms";
            m_measured = true;
        }

        if(&m_lbl.image() != nullptr) {
            delete ui->graphicsView->scene();

            QPixmap pxm = QPixmap::fromImage(m_lbl.image().scaled(ui->graphicsView->width()-10,ui->graphicsView->height()-10, Qt::AspectRatioMode::KeepAspectRatio));
            QGraphicsScene *scn = new QGraphicsScene;
            scn->addPixmap(pxm);
            //scn->setSceneRect(0, 0, pxm.width(), pxm.height());

            ui->graphicsView->setScene(scn);
            ui->graphicsView->show();
            //qDebug() << "Total " << timer.elapsed() << " ms";
        }
    }
}

void MainWindow::on_readButton_clicked()
{
    m_lbl.read("label1.json");
    m_measured = false;

    //QJsonObject jsonObject;
    //QString textoMultilineal = "Linea 1\n" "Linea 2\n" "Linea 3";
    // Asignar el texto multilineal al objeto JSON
    //jsonObject["texto_multilineal"] = textoMultilineal;
    // Convertir el objeto JSON a un documento JSON
    //QJsonDocument jsonDocument(jsonObject);
    //qDebug() <<jsonDocument.toJson(QJsonDocument::Indented);
}


void MainWindow::on_saveButton_clicked()
{
    if(m_lbl.objects().size()>0) {
        if(&m_lbl.image() != nullptr) {
            m_lbl.image().save("salida.bmp");
            qDebug() << "Image saved";
            m_lbl.save("label1.json");
            qDebug() << "JSON saved";
            //qDebug() << m_lbl.toJson();
        }
    }
    //qDebug() << m_lbl.toJson();
}

