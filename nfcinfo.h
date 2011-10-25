/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Andreas Jakl (andreas.jakl@nokia.com)
**
** This file is part of an NFC example for Qt Mobility.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef NFCINFO_H
#define NFCINFO_H

#include <QObject>
#include <QDebug>
#include <QUrl>

#include <qnearfieldmanager.h>
#include <qndeffilter.h>
#include <qnearfieldtarget.h>
#include <qndefmessage.h>
#include <qndefrecord.h>
#include <qndefnfcurirecord.h>
#include <qndefnfctextrecord.h>
#include <qnearfieldtagtype1.h>
#include <qnearfieldtagtype2.h>
#include "ndefnfcmimeimagerecord.h"
#include "ndefnfcsprecord.h"
#include "ndefnfcmimevcardrecord.h"

// Image handling
#include <QImage>
#include "tagimagecache.h"

// VCard reading
#include <QContact>
#include <QContactDetail>
#include <QVariantMap>
#include <QContactThumbnail>

#ifdef Q_OS_SYMBIAN
#include "nfcsettings.h"
#endif

QTM_USE_NAMESPACE

/*!
  \brief The NfcInfo class manages the interaction with the Nfc hardware,
  parses the tags and emits information signals for the UI.

  Following the use case of this application, the emitted signals
  contain mostly textual descriptions of the events or the information.
  Therefore, it is a good example to understand how the APIs are working
  and what kind of information you can get through the Qt Mobility APIs.

  Additionally, this class demonstrates how to parse tags and how to
  extract the relevant information - e.g., the parameters of a Smart Poster
  record, the image of a Mime record or the QContact coming from the versit
  document stored on a mime / text-vCard record.

  More comprehensive support for writing tags will be added in a later release.
  The focus of this version is on reading and parsing tags - even though
  all classes already work in both directions and it's mostly the UI that is
  missing.
  */
class NfcInfo : public QObject
{
    Q_OBJECT
public:
    explicit NfcInfo(QObject *parent = 0);

signals:
    /*! Update on the NFC status (starting target detection, etc.) */
    void nfcStatusUpdate(const QString& nfcStatusText);
    /*! Error with NFC. */
    void nfcStatusError(const QString& nfcStatusErrorText);
    /*! Generic information about a new tag that was detected */
    void nfcInfoUpdate(const QString& nfcInfoText);
    /*! Contents of the NDEF message found on the tag. */
    void nfcTagContents(const QString& nfcTagContents);
    /*! The tag contained an image. The parameter contains
      the image id that can be used to fetch it from the
      tag image cache class. */
    void nfcTagImage(const int nfcImgId);
    /*! An error occured while reading the tag. */
    void nfcTagError(const QString& nfcTagError);

public slots:
    bool checkNfcStatus();
    bool initAndStartNfc();
    bool nfcWriteTag(const QString& nfcTagText);

public:
    void setImageCache(TagImageCache* tagImageCache);

private slots:
    void targetDetected(QNearFieldTarget *target);
    void ndefMessageRead(const QNdefMessage &message);
    void ndefMessageWritten();
    void requestCompleted(const QNearFieldTarget::RequestId & id);
    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

    void targetLost(QNearFieldTarget *target);

private:
    QString analyzeTarget(QNearFieldTarget *target);

    QString parseUriRecord(const QNdefNfcUriRecord &record);
    QString parseTextRecord(const QNdefNfcTextRecord &record);
    QString textRecordToString(const QNdefNfcTextRecord &textRecord);
    QString parseSpRecord(const NdefNfcSpRecord &record);
    QString parseImageRecord(const NdefNfcMimeImageRecord &record);
    QString parseVcardRecord(NdefNfcMimeVcardRecord &record);

    bool writeCachedNdefMessage();

    QString convertTargetErrorToString(QNearFieldTarget::Error error);
    QString convertTagTypeToString(const QNearFieldTarget::Type type);
    QString convertRecordTypeNameToString(const QNdefRecord::TypeNameFormat typeName);


#ifdef Q_OS_SYMBIAN
private slots:
    // Check for NFC Support
    void handleNfcError(NfcSettings::NfcError nfcError, int error);
    void handleNfcModeChange(NfcSettings::NfcMode nfcMode);
private:
    NfcSettings* nfcSettings;
#endif

private:
    QNearFieldManager *nfcManager;
    QNearFieldTarget *cachedTarget;
    /*! Set to true if the application requested to write an NDEF message
      to the tag on the next opportunity. */
    bool pendingWriteNdef;
    /*! The cached NDEF message that is to be written to the tag. */
    QNdefMessage cachedNdefMessage;
    /*! Currently active request ID for tracking the requests
      to the NFC interface. */
    QNearFieldTarget::RequestId cachedRequestId;
    /*! Used for storing images found on the tags. */
    TagImageCache* imgCache;    // Not owned by this class
};

#endif // NFCINFO_H