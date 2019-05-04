#include "CameraImageWrapper.h"
#include <QColor>

//values based on http://entropymine.com/imageworsener/grayscale/
//round(0,2127*R)
const zxing::byte CameraImageWrapper::R_TO_GREYSCALE[256] =  {
    0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 9,
    10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
    18, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21,
    22, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25,
    26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29,
    30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 33,
    34, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37,
    38, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41,
    42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45,
    46, 46, 46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49,
    50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 53,
    54, 54, 54, 54
};

//values based on http://entropymine.com/imageworsener/grayscale/
//round(0,7152*G)
const zxing::byte CameraImageWrapper::G_TO_GREYSCALE[256] = {
    0, 1, 1, 2, 3, 4, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 11, 12, 13,
    14, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 21, 22, 23, 24, 24,
    25, 26, 26, 27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35,
    36, 36, 37, 38, 39, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46, 46, 47,
    48, 49, 49, 50, 51, 51, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 59,
    60, 61, 62, 62, 63, 64, 64, 65, 66, 67, 67, 68, 69, 69, 70, 71,
    72, 72, 73, 74, 74, 75, 76, 77, 77, 78, 79, 79, 80, 81, 82, 82,
    83, 84, 84, 85, 86, 87, 87, 88, 89, 89, 90, 91, 92, 92, 93, 94, 94,
    95, 96, 97, 97, 98, 99, 99, 100, 101, 102, 102, 103, 104, 104, 105, 106,
    107, 107, 108, 109, 109, 110, 111, 112, 112, 113, 114, 114, 115, 116,
    117, 117, 118, 119, 119, 120, 121, 122, 122, 123, 124, 124, 125, 126,
    127, 127, 128, 129, 129, 130, 131, 132, 132, 133, 134, 134, 135, 136,
    137, 137, 138, 139, 139, 140, 141, 142, 142, 143, 144, 144, 145, 146,
    147, 147, 148, 149, 149, 150, 151, 152, 152, 153, 154, 154, 155, 156,
    157, 157, 158, 159, 159, 160, 161, 162, 162, 163, 164, 164, 165, 166,
    167, 167, 168, 169, 170, 170, 171, 172, 172, 173, 174, 175, 175, 176,
    177, 177, 178, 179, 180, 180, 181, 182, 182
};

//values based on http://entropymine.com/imageworsener/grayscale/
//round(0,0722*B)
const zxing::byte CameraImageWrapper::B_TO_GREYSCALE[256] = {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18
};



CameraImageWrapper::CameraImageWrapper() : LuminanceSource(0,0)
{
}

CameraImageWrapper::CameraImageWrapper(const QImage &sourceImage) : LuminanceSource(sourceImage.width(), sourceImage.height())
{
    updateImageAsGrayscale( sourceImage );

    delegate = Ref<GreyscaleLuminanceSource>(
                new GreyscaleLuminanceSource(getMatrixP(), sourceImage.width(), sourceImage.height(),0, 0, sourceImage.width(), sourceImage.height()));
}

CameraImageWrapper::CameraImageWrapper(CameraImageWrapper& otherInstance) : LuminanceSource(otherInstance.getWidth(), otherInstance.getHeight())
{
    imageBytesPerRow = otherInstance.getOriginalImage();
    delegate = otherInstance.getDelegate();
}

CameraImageWrapper::~CameraImageWrapper()
{
}

CameraImageWrapper *CameraImageWrapper::Factory(const QImage &sourceImage, int maxWidth, int maxHeight, bool smoothTransformation)
{
    if((maxWidth != -1 && sourceImage.width() > maxWidth) || (maxHeight != -1 && sourceImage.height() > maxHeight))
    {
        QImage image;
        image = sourceImage.scaled(
                    maxWidth != -1 ? maxWidth : sourceImage.width(),
                    maxHeight != -1 ? maxHeight : sourceImage.height(),
                    Qt::KeepAspectRatio,
                    smoothTransformation ? Qt::SmoothTransformation : Qt::FastTransformation);
        return new CameraImageWrapper(image);
    }
    else
        return new CameraImageWrapper(sourceImage);
}

ArrayRef<ArrayRef<zxing::byte> > CameraImageWrapper::getOriginalImage()
{
    return imageBytesPerRow;
}

ArrayRef<zxing::byte> CameraImageWrapper::getRow(int y, ArrayRef<zxing::byte> row) const
{
    if(delegate)
        return delegate->getRow(y, row);
    else
        return getRowP(y, row);
}

ArrayRef<zxing::byte> CameraImageWrapper::getMatrix() const
{
    if(delegate)
        return delegate->getMatrix();
    else
        return getMatrixP();
}

bool CameraImageWrapper::isCropSupported() const
{
    if(delegate)
        return delegate->isCropSupported();
    else
        return LuminanceSource::isCropSupported();
}

Ref<LuminanceSource> CameraImageWrapper::crop(int left, int top, int width, int height) const
{
    if(delegate)
        return delegate->crop(left, top, width, height);
    else
        return LuminanceSource::crop(left, top, width, height);
}

bool CameraImageWrapper::isRotateSupported() const
{
    if(delegate)
        return delegate->isRotateSupported();
    else
        return LuminanceSource::isRotateSupported();
}

Ref<LuminanceSource> CameraImageWrapper::invert() const
{
    if(delegate)
        return delegate->invert();
    else
        return LuminanceSource::invert();
}

Ref<LuminanceSource> CameraImageWrapper::rotateCounterClockwise() const
{
    if(delegate)
        return delegate->rotateCounterClockwise();
    else
        return LuminanceSource::rotateCounterClockwise();
}

ArrayRef<zxing::byte> CameraImageWrapper::getRowP(int y, ArrayRef<zxing::byte> row) const
{
    int width = getWidth();

    if (row->size() != width)
        row.reset(ArrayRef<zxing::byte>(width));

    Q_ASSERT(y >= 0 && y < getHeight());

    return imageBytesPerRow[y];
}

ArrayRef<zxing::byte> CameraImageWrapper::getMatrixP() const
{
    return imageBytes;
}

zxing::byte CameraImageWrapper::gray(unsigned int r, unsigned int g, unsigned int b)
{
    //the values are not masked with (x & 0xFF) because functions qRed, qGreen, qBlue already do it
    return R_TO_GREYSCALE[r] + G_TO_GREYSCALE[g] + B_TO_GREYSCALE[b];
}

void CameraImageWrapper::updateImageAsGrayscale(const QImage &origin)
{
    bool needsConvesionToGrayscale = origin.format() != QImage::Format_Grayscale8;

    QRgb pixel;
    zxing::byte pixelGrayscale;

    const int width = getWidth();
    const int height = getHeight();

    imageBytes = ArrayRef<zxing::byte>(height*width);
    imageBytesPerRow = ArrayRef<ArrayRef<zxing::byte>>(height);
    zxing::byte* m = &imageBytes[0];

    for(int j=0; j<height; j++)
    {
        ArrayRef<zxing::byte> line(width);
        for(int i=0; i<width; i++)
        {
            pixel = origin.pixel(i,j);
            if(needsConvesionToGrayscale)
                pixelGrayscale = gray(qRed(pixel),qGreen(pixel),qBlue(pixel));
            else
                pixelGrayscale = pixel & 0xFF;
            line[i] = pixelGrayscale;
        }
        imageBytesPerRow[j] = line;

#if __cplusplus > 199711L
        memcpy(m, line->values().data(), width);
#else
        memcpy(m, &line[0], width);
#endif
        m += width * sizeof(zxing::byte);
    }
}

