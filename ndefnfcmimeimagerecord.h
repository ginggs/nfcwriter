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
#ifndef NDEFNFCMIMEIMAGERECORD_H
#define NDEFNFCMIMEIMAGERECORD_H

#include <QString>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QBuffer>
#include <QUrl>
#include <QNdefRecord>
#include <qndefmessage.h>
#include <qndefrecord.h>
#include <QDebug>

QTM_USE_NAMESPACE

// The default type name format - always Mimi
#define IMAGERECORD_DEFAULT_TNF QNdefRecord::Mime
// The default image type, if an instance of this class
// is created without specifying an image type.
#define IMAGERECORD_DEFAULT_TYPE "image/png"
// The default payload of the image record - which is empty.
#define IMAGERECORD_DEFAULT_PAYLOAD QByteArray(0, char(0))

/*!
  \brief Handles all image-related MIME media-type constructs.

  Reads all images supported by Qt and is able to return those
  as a QImage instance. When writing the raw pixel data of a QImage
  to this record, you can choose the MIME type to use for encoding
  the image. Again, all image formats supported by Qt for writing
  can be used; in practical scenarios, mostly png, gif and jpeg
  will make sense.


  The NdefNfcMimeImageRecord class handles all supported images
  and therefore many different record types. To simplify the use,
  there is no extra class for each individual image type
  (image/png, gif, jpg, jpeg, etc.).
  Therefore, the isRecordType<>() method can't be used with this
  generic image handler class. Instead, check if the type name format
  is Mime and the record type starts with "image/" - if this is the
  case, create an instance of the NdefNfcMimeImageRecord based on
  the generic record. Example:

  if (record.typeNameFormat() == QNdefRecord::Mime &&
      record.type().startsWith("image/")) {
      NdefNfcMimeImageRecord imgRecord(record);
      QImage img = imgRecord.image();
  }

  \version 1.0.0
  */
class NdefNfcMimeImageRecord : public QNdefRecord
{
public:
    // "urn:nfc:mime:image/XXX"
    NdefNfcMimeImageRecord();
    NdefNfcMimeImageRecord(const QNdefRecord &other);
    NdefNfcMimeImageRecord(const QByteArray &mimeType);
    NdefNfcMimeImageRecord(const QImage &img, const QByteArray &mimeType);

public:
    QByteArray format() const;
    QByteArray mimeType() const;

    QImage image() const;
    QByteArray imageRawData() const;

    bool setImage(QByteArray &imageRawData);
    bool setImage(const QImage &image, const QByteArray &mimeType);

private:
    QByteArray checkImageFormat(const QByteArray &format);
};

// If you require the use of isRecordType() for images,
// create derived classes from this image record class,
// which specialize in the required formats and can then
// be registered like below.
/*
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImagePngRecord, QNdefRecord::Mime, "image/png")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageGifRecord, QNdefRecord::Mime, "image/gif")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageJpgRecord, QNdefRecord::Mime, "image/jpg")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageJpegRecord, QNdefRecord::Mime, "image/jpeg")
// etc.
*/

#endif // NDEFNFCMIMEIMAGERECORD_H
