#ifndef ENCODEHINTTYPE_H
#define ENCODEHINTTYPE_H

#include <string>
#include <zxing/qrcode/ErrorCorrectionLevel.h>

namespace zxing {

class EncodeHint
{
private:
    /**
   * Specifies what degree of error correction to use, for example in QR Codes.
   * Type depends on the encoder. For example for QR codes it's type
   * {@link com.google.zxing.qrcode.decoder.ErrorCorrectionLevel ErrorCorrectionLevel}.
   * For Aztec it is of type {@link Integer}, representing the minimal percentage of error correction words.
   * Note: an Aztec symbol should have a minimum of 25% EC words.
   */
    zxing::qrcode::ErrorCorrectionLevel* errorCorrectionLevel_;

    /**
   * Specifies what character encoding to use where applicable (type {@link String})
   */
    std::string characterSet_;

    /**
   * Specifies margin, in pixels, to use when generating the barcode. The meaning can vary
   * by format; for example it controls margin before and after the barcode horizontally for
   * most 1D formats. (Type {@link Integer}).
   */
    int margin_;

public:
    EncodeHint();

    const std::string getCharacterSet() const { return characterSet_; }
    const zxing::qrcode::ErrorCorrectionLevel* getErrorCorrectionLevel() { return errorCorrectionLevel_; }

    void setCharacterSet(const std::string& characterSet) { characterSet_ = characterSet; }
    void setErrorCorrectionLevel(zxing::qrcode::ErrorCorrectionLevel* errorCorectionLevel)
        { errorCorrectionLevel_ = errorCorectionLevel; }
};

}

#endif //ENCODEHINTTYPE_H
