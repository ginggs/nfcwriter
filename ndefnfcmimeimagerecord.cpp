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

#include "ndefnfcmimeimagerecord.h"

/*!
  \brief Construct a new Mime/Image record using the default type (png)
  and an empty payload.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord()
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, IMAGERECORD_DEFAULT_TYPE)
{
    setPayload(IMAGERECORD_DEFAULT_PAYLOAD);
}

/*!
  \brief Create a new Mime/Image record based on the record passed
  through the argument.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QNdefRecord &other)
    : QNdefRecord(other)
{
}

/*!
  \brief Create a new Mime/Image record using the specified mime type
  and an empty payload.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QByteArray &mimeType)
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, mimeType)
{
    setPayload(IMAGERECORD_DEFAULT_PAYLOAD);
}

/*!
  \brief Create a new Mime/Image record using the pixel data of the \a img,
  converted and encoded in the specified mime type (if supported).

  The mime type can either be a Qt image format or a mime type - the method
  will automatically convert to a mime type (\see setImage() ).
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QImage &img, const QByteArray &mimeType)
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, mimeType)
{
    setImage(img, mimeType);
}

/*!
  \brief Decode the payload and return it as a pixel image (QImage).

  If the payload is empty or Qt cannot parse the decode the image format,
  an empty image is returned.

  \return the decoded image if possible, or an empty image otherwise.
  */
QImage NdefNfcMimeImageRecord::image() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QImage();

    QImage img;
    if (img.loadFromData(p))
    {
        return img;
    }
    return QImage();
}

/*!
  \brief Get the raw data of the image.

  No decoding is performed, so this will retrieve the encoded version
  of the image (e.g., a png). This equals retrieving the payload.
  \return the encoded image / payload of the record.
  */
QByteArray NdefNfcMimeImageRecord::imageRawData() const
{
    return payload();
}

/*!
  \brief Set the image (= payload) of the record to the byte array.
  This allows passing an encoded image (e.g., png) directly to the class.

  The method will attempt to identify the mime type of the raw image data
  and automatically modify the mime type stored in the record if successful.
  If this is not possible or the passed byte array is empty, the payload
  will not be set.
  \param imageRawData the encoded image data to use as payload of the record.
  */
bool NdefNfcMimeImageRecord::setImage(QByteArray& imageRawData)
{
    if (imageRawData.isEmpty())
        return false;

    // Check image mime type
    QBuffer buffer(&imageRawData);  // compiler warning: taking address of temporary
    buffer.open(QIODevice::ReadOnly);
    QByteArray imgFormat = QImageReader::imageFormat(&buffer);

    // Check if Qt supports the image format (needed to set the mime type automatically)
    if (imgFormat.isEmpty())
        return false;

    // Add "image/" to the Qt image type in order to get to a mime type
    imgFormat = imgFormat.toLower();
    imgFormat = imgFormat.prepend("image/");
    setType(imgFormat);
    setPayload(imageRawData);
    return true;
}

/*!
  \brief Encode the image data into the payload using the specified mime type.

  \param image the pixel image data to encode and set as the payload.
  \param mimeType the mime type can either be specified using a supported mime
  type or a Qt image format. The image format is automatically converted to a
  mime type.
  */
bool NdefNfcMimeImageRecord::setImage(const QImage &image, const QByteArray &mimeType)
{
    // See if we support writing the image in the specified type
    QByteArray checkedFormat = checkImageFormat(mimeType);
    if (checkedFormat.isEmpty())
        return false;

    QByteArray p;
    QBuffer buffer(&p);
    buffer.open(QIODevice::WriteOnly);
    bool success = image.save(&buffer, checkedFormat.constData()); // writes image into p in the selected format
    if (!success) {
        return false;
    }

    setPayload(p);
    return true;
}

/*!
  \brief Retrieve the Qt image format of the image data stored in the payload.

  \see QImageReader::imageFormat()
  \return the Qt image format of the payload.
  */
QByteArray NdefNfcMimeImageRecord::format() const
{
    QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QBuffer buffer(&p);  // compiler warning: taking address of temporary
    buffer.open(QIODevice::ReadOnly);
    return QImageReader::imageFormat(&buffer);
}

/*!
  \brief return the mime type of the image, as set in the record type.

  \return the mime type of the record.
  */
QByteArray NdefNfcMimeImageRecord::mimeType() const
{
    return type();
}

/*!
  \brief Check if the specified Qt image \a format / mime type is supported by Qt
  for encoding and return it as a Qt image format name.

  \param format either a Qt image format (e.g., "png") or a mime type
  (e.g., "image/png").
  \return if the image format is supported by Qt for encoding, the Qt image format
  name. If the type is not supported, an empty byte array is returned.
  */
QByteArray NdefNfcMimeImageRecord::checkImageFormat(const QByteArray& format)
{
    // Convert MIME types to Qt image type names
    QByteArray checkFormat = format.toUpper();
    if (checkFormat.startsWith("IMAGE/")) {
        // Remove leading "image/" from the mime type so that only the image
        // type is left
        checkFormat = checkFormat.right(checkFormat.size() - 6);
    }

    // Check if the image format is supported by Qt
    bool supported = false;
    foreach (QByteArray supportedFormat, QImageWriter::supportedImageFormats()) {
        if (checkFormat == supportedFormat) {
            supported = true;
            break;
        }
    }
    if (supported) {
        return checkFormat;
    } else {
        return QByteArray();
    }
}
