#include "nfcwriter.h"

#include <QContactManager>

#include <QContactAvatar>
#include <QContactGuid>
#include <QContactTimestamp>

#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcmimevcardrecord.h"
#include "ndefnfcsprecord.h"

nfcwriter::nfcwriter(QObject *parent) :
    QObject(parent),
    op(nfcwriter::mode_none),
    text(),
    uri(),
    action(),
    cto(),
    nfc(),
    tag() {
    qDebug() << "nfcwriter::nfcwriter";

    connect(&nfc, SIGNAL(targetDetected(QNearFieldTarget*)),
            this, SLOT(targetDetected(QNearFieldTarget*)));
    connect(&nfc, SIGNAL(targetLost(QNearFieldTarget*)),
            this, SLOT(targetLost(QNearFieldTarget*)));
}

bool nfcwriter::check() {
    qDebug() << "nfcwriter::check";
    if (! nfc.isAvailable() ) {
        qDebug() << "nfc not available";
        return false;
    }
    return true;
}


void nfcwriter::read() {
    qDebug() << "nfcwriter::read";

    nfc.setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_read;
    emit nfcTap();
}

void nfcwriter::writetext(QString t) {
    qDebug() << "nfcwriter::writetext" << t;
    text = t;

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_text;
    emit nfcTap();
}

void nfcwriter::writeuri(QString u) {
    qDebug() << "nfcwriter::writeuri" << u;
    uri = QUrl(u);

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_uri;
    emit nfcTap();
}

void nfcwriter::writeid(QContactLocalId id) {
    qDebug() << "nfcwriter::writeid" << id;

    QContactManager mgr;
    cto = mgr.contact(id);
    cto.removeDetail( new QContactDetail(cto.detail<QContactAvatar>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactGuid>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactTimestamp>()));
    qDebug() << cto.id();

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_vcard;
    emit nfcTap();
}

void nfcwriter::writesp(QString t, QString u, int a) {
    qDebug() << "nfcwriter::writeusp" << t << u << a;
    text = t;
    uri = QUrl(u);
    action = a;

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_smartposter;
    emit nfcTap();
}

void nfcwriter::targetDetected(QNearFieldTarget *target) {
    qDebug() << "nfcwriter::targetDetected" << target;
    QNdefNfcTextRecord ndef_text;
    QNdefNfcUriRecord ndef_uri;
    NdefNfcMimeVcardRecord ndef_vcard;
    NdefNfcSpRecord ndef_sp;

    switch (op) {
    case nfcwriter::mode_none:
        qDebug() << "why op == nfcwriter::mode_none ? ";
        break;
    case nfcwriter::mode_read:
        connect(target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId)),
                this, SLOT(requestCompleted(QNearFieldTarget::RequestId)));
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        if (target->hasNdefMessage()) {
            connect(target, SIGNAL(ndefMessageRead(QNdefMessage)),
                    this, SLOT(ndefMessageRead(QNdefMessage)));
            target->readNdefMessages();
        }

        break;
    case nfcwriter::mode_text:
        ndef_text.setText(text);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_text));
        break;
    case nfcwriter::mode_uri:
        ndef_uri.setUri(uri);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_uri));
        break;
    case nfcwriter::mode_vcard:
        ndef_vcard.setContact(cto);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_vcard));
        break;
    case nfcwriter::mode_smartposter:
        ndef_text.setText(text);
        ndef_uri.setUri(uri);

        ndef_sp.addTitle(ndef_text);
        ndef_sp.setUri(ndef_uri);
        switch (action) {
        case 0: // DoAction
            ndef_sp.setAction(NdefNfcSpRecord::DoAction);
            break;
        case 1: // SaveForLater
            ndef_sp.setAction(NdefNfcSpRecord::SaveForLater);
            break;
        case 2: // OpenForEditing
            ndef_sp.setAction(NdefNfcSpRecord::OpenForEditing);
            break;
        case 3: // RFU
            ndef_sp.setAction(NdefNfcSpRecord::RFU);
            break;
        }
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_sp));
        break;
    }
}

void nfcwriter::targetLost(QNearFieldTarget *target) {
    qDebug() << "nfcwriter::targetLost" << target;
    nfc.stopTargetDetection();
    op = nfcwriter::mode_none;
    target->disconnect();

    emit nfcLost();
}

void nfcwriter::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id) {
    Q_UNUSED(id);

    QString errorString = "Unknown";
    switch (error) {
    case QNearFieldTarget::NoError:
        errorString = "NoError";
        break;
    case QNearFieldTarget::UnknownError:
        errorString = "UnknownError";
        break;
    case QNearFieldTarget::UnsupportedError:
        errorString = "UnsupportedError";
        break;
    case QNearFieldTarget::TargetOutOfRangeError:
        errorString = "TargetOutOfRangeError";
        break;
    case QNearFieldTarget::NoResponseError:
        errorString = "NoResponseError";
        break;
    case QNearFieldTarget::ChecksumMismatchError:
        errorString = "ChecksumMismatchError";
        break;
    case QNearFieldTarget::InvalidParametersError:
        errorString = "InvalidParametersError";
        break;
    case QNearFieldTarget::NdefReadError:
        errorString = "NdefReadError";
        break;
    case QNearFieldTarget::NdefWriteError:
        errorString = "NdefWriteError";
        break;
    }
    qDebug() << "nfcwriter::targetError" << error << errorString;
}

void nfcwriter::requestCompleted(QNearFieldTarget::RequestId id) {
    // qDebug() << "nfcwriter::requestCompleted";
    Q_UNUSED(id);
}

void nfcwriter::ndefMessageRead(QtMobility::QNdefMessage msg) {
    qDebug() << "nfcwriter::ndefMessageRead";
    tag.clear();

    tag.append("Records: " + QString("%1").arg(msg.size()) + QString('\n'));
    tag.append("Bytes: " + QString("%1").arg(msg.toByteArray().size()) + QString('\n'));

    foreach (const QNdefRecord &r, msg) {
        if (r.isRecordType<QNdefNfcTextRecord>()) {
            tag.append("Text: " + QNdefNfcTextRecord(r).text() + QString('\n'));
        } else if (r.isRecordType<QNdefNfcUriRecord>()) {
            tag.append("Uri: " + QNdefNfcUriRecord(r).uri().toString() + QString('\n'));
        } else if (r.isRecordType<NdefNfcSpRecord>()) {
            NdefNfcSpRecord sp(r);
            tag.append("SmartPoster" + QString('\n'));
            foreach (QNdefNfcTextRecord t, sp.titles()) {
                tag.append("Text: " + t.text() + QString('\n'));
            }
            tag.append("Uri: " + sp.uri().toString() + QString('\n'));
            switch (sp.action()) {
            case NdefNfcSpRecord::DoAction:
                tag.append("Action: DoAction" + QString('\n'));
                break;
            case NdefNfcSpRecord::SaveForLater:
                tag.append("Action: SaveForLater" + QString('\n'));
                break;
            case NdefNfcSpRecord::OpenForEditing:
                tag.append("Action: OpenForEditing" + QString('\n'));
                break;
            case NdefNfcSpRecord::RFU:
                tag.append("Action: RFU" + QString('\n'));
                break;
            }
        } else if (r.isRecordType<NdefNfcMimeVcardRecord>()) {
            tag.append("VCard" + QString('\n'));
            foreach (QContact c, NdefNfcMimeVcardRecord(r).contacts()) {
                foreach (QContactDetail d, c.details()) {
                    tag.append(d.definitionName() + ": ");
                    foreach (QVariant dd, d.variantValues()) {
                        tag.append(dd.toString() + ' ');
                    }
                    tag.append(QString('\n'));
                }
            }
        } else {
            tag.append("Unknow Record" + QString('\n'));
        }
    }
    emit nfcRead();
}

void nfcwriter::ndefMessageWritten() {
    qDebug() << "nfcwriter::ndefMessageWritten";
    emit nfcWritten();
}
