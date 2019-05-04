#
# Copyright 2011 QZXing authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

CONFIG += qt warn_on

DEFINES += QZXING_LIBRARY \
        ZXING_ICONV_CONST \
        DISABLE_LIBRARY_FEATURES
		 
INCLUDEPATH  += $$PWD \
                $$PWD/zxing

HEADERS += $$PWD/QZXing_global.h \
    $$PWD/CameraImageWrapper.h \
    $$PWD/ImageHandler.h \
    $$PWD/QZXing.h \
    $$PWD/zxing/zxing/ZXing.h \
    $$PWD/zxing/zxing/IllegalStateException.h \
    $$PWD/zxing/zxing/InvertedLuminanceSource.h \
    $$PWD/zxing/zxing/ChecksumException.h \
    $$PWD/zxing/zxing/ResultPointCallback.h \
    $$PWD/zxing/zxing/ResultPoint.h \
    $$PWD/zxing/zxing/Result.h \
    $$PWD/zxing/zxing/ReaderException.h \
    $$PWD/zxing/zxing/Reader.h \
    $$PWD/zxing/zxing/NotFoundException.h \
    $$PWD/zxing/zxing/MultiFormatReader.h \
    $$PWD/zxing/zxing/LuminanceSource.h \
    $$PWD/zxing/zxing/FormatException.h \
    $$PWD/zxing/zxing/Exception.h \
    $$PWD/zxing/zxing/DecodeHints.h \
    $$PWD/zxing/zxing/BinaryBitmap.h \
    $$PWD/zxing/zxing/Binarizer.h \
    $$PWD/zxing/zxing/BarcodeFormat.h \
    $$PWD/zxing/zxing/aztec/AztecReader.h \
    $$PWD/zxing/zxing/aztec/AztecDetectorResult.h \
    $$PWD/zxing/zxing/aztec/decoder/Decoder.h \
    $$PWD/zxing/zxing/aztec/detector/Detector.h \
    $$PWD/zxing/zxing/common/StringUtils.h \
    $$PWD/zxing/zxing/common/Str.h \
    $$PWD/zxing/zxing/common/Point.h \
    $$PWD/zxing/zxing/common/PerspectiveTransform.h \
    $$PWD/zxing/zxing/common/IllegalArgumentException.h \
    $$PWD/zxing/zxing/common/HybridBinarizer.h \
    $$PWD/zxing/zxing/common/GridSampler.h \
    $$PWD/zxing/zxing/common/GreyscaleRotatedLuminanceSource.h \
    $$PWD/zxing/zxing/common/GreyscaleLuminanceSource.h \
    $$PWD/zxing/zxing/common/GlobalHistogramBinarizer.h \
    $$PWD/zxing/zxing/common/DetectorResult.h \
    $$PWD/zxing/zxing/common/DecoderResult.h \
    $$PWD/zxing/zxing/common/Counted.h \
    $$PWD/zxing/zxing/common/CharacterSetECI.h \
    $$PWD/zxing/zxing/common/BitSource.h \
    $$PWD/zxing/zxing/common/BitMatrix.h \
    $$PWD/zxing/zxing/common/BitArray.h \
    $$PWD/zxing/zxing/common/Array.h \
    $$PWD/zxing/zxing/common/detector/MathUtils.h \
    $$PWD/zxing/zxing/common/detector/JavaMath.h \
    $$PWD/zxing/zxing/common/detector/WhiteRectangleDetector.h \
    $$PWD/zxing/zxing/common/detector/MonochromeRectangleDetector.h \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonException.h \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonDecoder.h \
    $$PWD/zxing/zxing/common/reedsolomon/GenericGFPoly.h \
    $$PWD/zxing/zxing/common/reedsolomon/GenericGF.h \
    $$PWD/zxing/zxing/datamatrix/Version.h \
    $$PWD/zxing/zxing/datamatrix/DataMatrixReader.h \
    $$PWD/zxing/zxing/datamatrix/decoder/Decoder.h \
    $$PWD/zxing/zxing/datamatrix/decoder/DecodedBitStreamParser.h \
    $$PWD/zxing/zxing/datamatrix/decoder/DataBlock.h \
    $$PWD/zxing/zxing/datamatrix/decoder/BitMatrixParser.h \
    $$PWD/zxing/zxing/datamatrix/detector/DetectorException.h \
    $$PWD/zxing/zxing/datamatrix/detector/Detector.h \
    $$PWD/zxing/zxing/datamatrix/detector/CornerPoint.h \
    $$PWD/zxing/zxing/oned/UPCEReader.h \
    $$PWD/zxing/zxing/oned/UPCEANReader.h \
    $$PWD/zxing/zxing/oned/UPCAReader.h \
    $$PWD/zxing/zxing/oned/OneDResultPoint.h \
    $$PWD/zxing/zxing/oned/OneDReader.h \
    $$PWD/zxing/zxing/oned/MultiFormatUPCEANReader.h \
    $$PWD/zxing/zxing/oned/MultiFormatOneDReader.h \
    $$PWD/zxing/zxing/oned/ITFReader.h \
    $$PWD/zxing/zxing/oned/EAN13Reader.h \
    $$PWD/zxing/zxing/oned/EAN8Reader.h \
    $$PWD/zxing/zxing/oned/Code128Reader.h \
    $$PWD/zxing/zxing/oned/Code39Reader.h \
    $$PWD/zxing/zxing/oned/CodaBarReader.h \
    $$PWD/zxing/zxing/oned/Code93Reader.h \
    $$PWD/zxing/zxing/qrcode/Version.h \
    $$PWD/zxing/zxing/qrcode/QRCodeReader.h \
    $$PWD/zxing/zxing/qrcode/FormatInformation.h \
    $$PWD/zxing/zxing/qrcode/ErrorCorrectionLevel.h \
    $$PWD/zxing/zxing/qrcode/decoder/Mode.h \
    $$PWD/zxing/zxing/qrcode/decoder/Decoder.h \
    $$PWD/zxing/zxing/qrcode/decoder/DecodedBitStreamParser.h \
    $$PWD/zxing/zxing/qrcode/decoder/DataMask.h \
    $$PWD/zxing/zxing/qrcode/decoder/DataBlock.h \
    $$PWD/zxing/zxing/qrcode/decoder/BitMatrixParser.h \
    $$PWD/zxing/zxing/qrcode/detector/FinderPatternInfo.h \
    $$PWD/zxing/zxing/qrcode/detector/FinderPatternFinder.h \
    $$PWD/zxing/zxing/qrcode/detector/FinderPattern.h \
    $$PWD/zxing/zxing/qrcode/detector/Detector.h \
    $$PWD/zxing/zxing/qrcode/detector/AlignmentPatternFinder.h \
    $$PWD/zxing/zxing/qrcode/detector/AlignmentPattern.h \
    $$PWD/zxing/zxing/multi/MultipleBarcodeReader.h \
    $$PWD/zxing/zxing/multi/GenericMultipleBarcodeReader.h \
    $$PWD/zxing/zxing/multi/ByQuadrantReader.h \
    $$PWD/zxing/zxing/multi/qrcode/QRCodeMultiReader.h \
    $$PWD/zxing/zxing/multi/qrcode/detector/MultiFinderPatternFinder.h \
    $$PWD/zxing/zxing/multi/qrcode/detector/MultiDetector.h \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ErrorCorrection.h \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ModulusGF.h \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ModulusPoly.h \
    $$PWD/zxing/zxing/pdf417/decoder/BitMatrixParser.h \
    $$PWD/zxing/zxing/pdf417/decoder/DecodedBitStreamParser.h \
    $$PWD/zxing/zxing/pdf417/decoder/Decoder.h \
    $$PWD/zxing/zxing/pdf417/detector/Detector.h \
    $$PWD/zxing/zxing/pdf417/detector/LinesSampler.h \
    $$PWD/zxing/zxing/pdf417/PDF417Reader.h \
    $$PWD/zxing/bigint/NumberlikeArray.hh \
    $$PWD/zxing/bigint/BigUnsignedInABase.hh \
    $$PWD/zxing/bigint/BigUnsigned.hh \
    $$PWD/zxing/bigint/BigIntegerUtils.hh \
    $$PWD/zxing/bigint/BigIntegerLibrary.hh \
    $$PWD/zxing/bigint/BigIntegerAlgorithms.hh \
    $$PWD/zxing/bigint/BigInteger.hh \
    $$PWD/zxing/zxing/qrcode/encoder/BlockPair.h \
    $$PWD/zxing/zxing/qrcode/encoder/ByteMatrix.h \
    $$PWD/zxing/zxing/qrcode/encoder/Encoder.h \
    $$PWD/zxing/zxing/qrcode/encoder/MaskUtil.h \
    $$PWD/zxing/zxing/qrcode/encoder/MatrixUtil.h \
    $$PWD/zxing/zxing/qrcode/encoder/QRCode.h \
    $$PWD/zxing/zxing/WriterException.h \
    $$PWD/zxing/zxing/EncodeHint.h \
    $$PWD/zxing/zxing/UnsupportedEncodingException.h \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonEncoder.h \
    $$PWD/zxing/zxing/common/Types.h

SOURCES += $$PWD/CameraImageWrapper.cpp \
    $$PWD/QZXing.cpp \
    $$PWD/ImageHandler.cpp \
    $$PWD/zxing/zxing/ResultIO.cpp \
    $$PWD/zxing/zxing/InvertedLuminanceSource.cpp \
    $$PWD/zxing/zxing/ChecksumException.cpp \
    $$PWD/zxing/zxing/ResultPointCallback.cpp \
    $$PWD/zxing/zxing/ResultPoint.cpp \
    $$PWD/zxing/zxing/Result.cpp \
    $$PWD/zxing/zxing/Reader.cpp \
    $$PWD/zxing/zxing/MultiFormatReader.cpp \
    $$PWD/zxing/zxing/LuminanceSource.cpp \
    $$PWD/zxing/zxing/FormatException.cpp \
    $$PWD/zxing/zxing/Exception.cpp \
    $$PWD/zxing/zxing/DecodeHints.cpp \
    $$PWD/zxing/zxing/BinaryBitmap.cpp \
    $$PWD/zxing/zxing/Binarizer.cpp \
    $$PWD/zxing/zxing/BarcodeFormat.cpp \
    $$PWD/zxing/zxing/ReaderException.cpp \
    $$PWD/zxing/zxing/IllegalStateException.cpp \
    $$PWD/zxing/zxing/NotFoundException.cpp \
    $$PWD/zxing/zxing/UnsupportedEncodingException.cpp \
    $$PWD/zxing/zxing/WriterException.cpp \
    $$PWD/zxing/zxing/aztec/AztecReader.cpp \
    $$PWD/zxing/zxing/aztec/AztecDetectorResult.cpp \
    $$PWD/zxing/zxing/common/StringUtils.cpp \
    $$PWD/zxing/zxing/common/Str.cpp \
    $$PWD/zxing/zxing/common/PerspectiveTransform.cpp \
    $$PWD/zxing/zxing/common/IllegalArgumentException.cpp \
    $$PWD/zxing/zxing/common/HybridBinarizer.cpp \
    $$PWD/zxing/zxing/common/GridSampler.cpp \
    $$PWD/zxing/zxing/common/GreyscaleRotatedLuminanceSource.cpp \
    $$PWD/zxing/zxing/common/GreyscaleLuminanceSource.cpp \
    $$PWD/zxing/zxing/common/GlobalHistogramBinarizer.cpp \
    $$PWD/zxing/zxing/common/DetectorResult.cpp \
    $$PWD/zxing/zxing/common/DecoderResult.cpp \
    $$PWD/zxing/zxing/common/CharacterSetECI.cpp \
    $$PWD/zxing/zxing/common/BitSource.cpp \
    $$PWD/zxing/zxing/common/BitMatrix.cpp \
    $$PWD/zxing/zxing/common/BitArray.cpp \
    $$PWD/zxing/zxing/common/BitArrayIO.cpp \
    $$PWD/zxing/zxing/common/detector/WhiteRectangleDetector.cpp \
    $$PWD/zxing/zxing/common/detector/MonochromeRectangleDetector.cpp \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonException.cpp \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonDecoder.cpp \
    $$PWD/zxing/zxing/common/reedsolomon/GenericGFPoly.cpp \
    $$PWD/zxing/zxing/common/reedsolomon/GenericGF.cpp \
    $$PWD/zxing/zxing/datamatrix/DataMatrixReader.cpp \
    $$PWD/zxing/zxing/oned/UPCEReader.cpp \
    $$PWD/zxing/zxing/oned/UPCEANReader.cpp \
    $$PWD/zxing/zxing/oned/UPCAReader.cpp \
    $$PWD/zxing/zxing/oned/OneDResultPoint.cpp \
    $$PWD/zxing/zxing/oned/OneDReader.cpp \
    $$PWD/zxing/zxing/oned/MultiFormatUPCEANReader.cpp \
    $$PWD/zxing/zxing/oned/MultiFormatOneDReader.cpp \
    $$PWD/zxing/zxing/oned/ITFReader.cpp \
    $$PWD/zxing/zxing/oned/EAN13Reader.cpp \
    $$PWD/zxing/zxing/oned/EAN8Reader.cpp \
    $$PWD/zxing/zxing/oned/Code128Reader.cpp \
    $$PWD/zxing/zxing/oned/Code39Reader.cpp \
    $$PWD/zxing/zxing/oned/CodaBarReader.cpp \
    $$PWD/zxing/zxing/oned/Code93Reader.cpp \
    $$PWD/zxing/zxing/qrcode/QRCodeReader.cpp \
    $$PWD/zxing/zxing/multi/MultipleBarcodeReader.cpp \
    $$PWD/zxing/zxing/multi/GenericMultipleBarcodeReader.cpp \
    $$PWD/zxing/zxing/multi/ByQuadrantReader.cpp \
    $$PWD/zxing/zxing/multi/qrcode/QRCodeMultiReader.cpp \
    $$PWD/zxing/zxing/multi/qrcode/detector/MultiFinderPatternFinder.cpp \
    $$PWD/zxing/zxing/multi/qrcode/detector/MultiDetector.cpp \
    $$PWD/zxing/zxing/aztec/decoder/AztecDecoder.cpp \
    $$PWD/zxing/zxing/aztec/detector/AztecDetector.cpp \
    $$PWD/zxing/zxing/datamatrix/DataMatrixVersion.cpp \
    $$PWD/zxing/zxing/datamatrix/decoder/DataMatrixDecoder.cpp \
    $$PWD/zxing/zxing/datamatrix/decoder/DataMatrixBitMatrixParser.cpp \
    $$PWD/zxing/zxing/datamatrix/decoder/DataMatrixDataBlock.cpp \
    $$PWD/zxing/zxing/datamatrix/decoder/DataMatrixDecodedBitStreamParser.cpp \
    $$PWD/zxing/zxing/datamatrix/detector/DataMatrixCornerPoint.cpp \
    $$PWD/zxing/zxing/datamatrix/detector/DataMatrixDetector.cpp \
    $$PWD/zxing/zxing/datamatrix/detector/DataMatrixDetectorException.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRBitMatrixParser.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRDataBlock.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRDataMask.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRDecodedBitStreamParser.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRDecoder.cpp \
    $$PWD/zxing/zxing/qrcode/decoder/QRMode.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRAlignmentPattern.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRAlignmentPatternFinder.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRDetector.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRFinderPattern.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRFinderPatternFinder.cpp \
    $$PWD/zxing/zxing/qrcode/detector/QRFinderPatternInfo.cpp \
    $$PWD/zxing/zxing/qrcode/QRVersion.cpp \
    $$PWD/zxing/zxing/qrcode/QRFormatInformation.cpp \
    $$PWD/zxing/zxing/qrcode/QRErrorCorrectionLevel.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ErrorCorrection.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ModulusGF.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/ec/ModulusPoly.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/PDF417BitMatrixParser.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/PDF417DecodedBitStreamParser.cpp \
    $$PWD/zxing/zxing/pdf417/decoder/PDF417Decoder.cpp \
    $$PWD/zxing/zxing/pdf417/detector/PDF417Detector.cpp \
    $$PWD/zxing/zxing/pdf417/detector/LinesSampler.cpp \
    $$PWD/zxing/zxing/pdf417/PDF417Reader.cpp \
    $$PWD/zxing/bigint/BigUnsignedInABase.cc \
    $$PWD/zxing/bigint/BigUnsigned.cc \
    $$PWD/zxing/bigint/BigIntegerUtils.cc \
    $$PWD/zxing/bigint/BigIntegerAlgorithms.cc \
    $$PWD/zxing/bigint/BigInteger.cc \
    $$PWD/zxing/zxing/qrcode/encoder/ByteMatrix.cpp \
    $$PWD/zxing/zxing/qrcode/encoder/QREncoder.cpp \
    $$PWD/zxing/zxing/qrcode/encoder/MaskUtil.cpp \
    $$PWD/zxing/zxing/qrcode/encoder/MatrixUtil.cpp \
    $$PWD/zxing/zxing/qrcode/encoder/QRCode.cpp \
    $$PWD/zxing/zxing/EncodeHint.cpp \
    $$PWD/zxing/zxing/common/reedsolomon/ReedSolomonEncoder.cpp

qzxing_multimedia {
    QT += multimedia

    CONFIG += qzxing_qml

    DEFINES += QZXING_MULTIMEDIA

    HEADERS += \
        $$PWD/QZXingFilter.h

    SOURCES += \
        $$PWD/QZXingFilter.cpp
}

qzxing_qml {
    greaterThan(QT_VERSION, 4.7): lessThan(QT_VERSION, 5.0): QT += declarative
    greaterThan(QT_MAJOR_VERSION, 4): QT += quick

    DEFINES += QZXING_QML

    HEADERS +=  \
        $$PWD/QZXingImageProvider.h

    SOURCES +=  \
        $$PWD/QZXingImageProvider.cpp
}

symbian {
    TARGET.UID3 = 0xE618743C
    TARGET.EPOCALLOWDLLDATA = 1

    #TARGET.CAPABILITY = All -TCB -AllFiles -DRM
    TARGET.CAPABILITY += NetworkServices \
        ReadUserData \
        WriteUserData \
        LocalServices \
        UserEnvironment \
        Location
}

!symbian {
    isEmpty(PREFIX) {
        maemo5 {
            target.path = /opt/usr/lib
        } else {
            target.path = /usr/lib
        }
    }

    DEFINES += NOFMAXL

	# Installation
	headers.files = qzxing.h QZXing_global.h
	headers.path = $$PREFIX/include
	target.path = $$PREFIX/lib
	INSTALLS += headers target

	# pkg-config support
	CONFIG += create_pc create_prl no_install_prl
	QMAKE_PKGCONFIG_DESTDIR = pkgconfig
	QMAKE_PKGCONFIG_LIBDIR = ${prefix}/lib
	QMAKE_PKGCONFIG_INCDIR = ${prefix}/include

	unix:QMAKE_CLEAN += -r pkgconfig lib$${TARGET}.prl
}

win32-msvc*{

    DEFINES += __STDC_LIMIT_MACROS

    INCLUDEPATH += $$PWD/zxing/win32/zxing \
	            $$PWD/zxing/win32/zxing/msvc
    HEADERS += $$PWD/zxing/win32/zxing/msvc/stdint.h \
                $$PWD/zxing/win32/zxing/iconv.h

    SOURCES += $$PWD/zxing/win32/zxing/win_iconv.c
}

win32-g++{

    INCLUDEPATH += $$PWD/zxing/win32/zxing

    HEADERS += $$PWD/zxing/win32/zxing/iconv.h

    SOURCES += $$PWD/zxing/win32/zxing/win_iconv.c
}

!win32{
    DEFINES += NO_ICONV
}
winrt {
    DEFINES += NO_ICONV
}
