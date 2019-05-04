#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include <zxing/common/CharacterSetECI.h>
#include <zxing/qrcode/ErrorCorrectionLevel.h>
#include <zxing/qrcode/encoder/QRCode.h>
#include <zxing/EncodeHint.h>

#include <vector>
#include <string>

namespace zxing {
namespace qrcode {

class Encoder {

public:
  static Ref<QRCode> encode(const std::string& content, ErrorCorrectionLevel &ecLevel);

  static Ref<QRCode> encode(const std::string& content, ErrorCorrectionLevel &ecLevel, const EncodeHint* hints);

  /**
   * @return the code point of the table used in alphanumeric mode or
   *  -1 if there is no corresponding code in the table.
   */
  static int getAlphanumericCode(int code);

  /**
   * Choose the best mode by examining the content. Note that 'encoding' is used as a hint;
   * if it is Shift_JIS, and the input is only double-byte Kanji, then we return {@link Mode#KANJI}.
   */
  static Mode chooseMode(const std::string& content, const std::string& encoding = "");

  /**
   * Append mode info. On success, store the result in "bits".
   */
  static void appendModeInfo(const Mode& mode, BitArray &bits);

  /**
   * Append length info. On success, store the result in "bits".
   */
  static void appendLengthInfo(int numLetters, const Version& version, const Mode& mode, BitArray& bits);

  /**
   * Append "bytes" in "mode" mode (encoding) into "bits". On success, store the result in "bits".
   */
  static void appendBytes(const std::string& content,
                          Mode& mode,
                          BitArray& bits,
                          const std::string& encoding);

protected:
  /**
   * Terminate bits as described in 8.4.8 and 8.4.9 of JISX0510:2004 (p.24).
   */
  static void terminateBits(int numDataBytes, BitArray& bits);

  /**
   * Get number of data bytes and number of error correction bytes for block id "blockID". Store
   * the result in "numDataBytesInBlock", and "numECBytesInBlock". See table 12 in 8.5.1 of
   * JISX0510:2004 (p.30)
   */
  static void getNumDataBytesAndNumECBytesForBlockID(int numTotalBytes,
                                                     int numDataBytes,
                                                     int numRSBlocks,
                                                     int blockID,
                                                     std::vector<int>& numDataBytesInBlock,
                                                     std::vector<int>& numECBytesInBlock);

  /**
   * Interleave "bits" with corresponding error correction bytes. On success, store the result in
   * "result". The interleave rule is complicated. See 8.6 of JISX0510:2004 (p.37) for details.
   */
  static BitArray* interleaveWithECBytes(const BitArray& bits,
                                        int numTotalBytes,
                                        int numDataBytes,
                                        int numRSBlocks);

  static ArrayRef<zxing::byte> generateECBytes(const std::vector<zxing::byte> &dataBytes, int numEcBytesInBlock);

  static void appendNumericBytes(const std::string& content, BitArray& bits);

  static void appendAlphanumericBytes(const std::string& content, BitArray& bits);

  static void append8BitBytes(const std::string& content, BitArray& bits, const std::string& encoding);

  static void appendKanjiBytes(const std::string& content, BitArray& bits);

  //static bool isOnlyDoubleByteKanji(const std::string& content);

private:
  static int chooseMaskPattern(Ref<BitArray> bits,
                                       ErrorCorrectionLevel& ecLevel,
                                       Ref<Version> version,
                                       Ref<ByteMatrix> matrix);

  static Ref<Version> chooseVersion(int numInputBits, const ErrorCorrectionLevel &ecLevel) ;

  static void appendECI(const zxing::common::CharacterSetECI& eci, BitArray& bits);

  /**
   * The mask penalty calculation is complicated.  See Table 21 of JISX0510:2004 (p.45) for details.
   * Basically it applies four rules and summate all penalties.
   */
  static int calculateMaskPenalty(const ByteMatrix& matrix);

  static int calculateBitsNeeded(const Mode &mode, const BitArray &headerBits, const BitArray &dataBits, const
                                 Ref<Version> version);
  static bool willFit(int numInputBits, Ref<Version> version, const ErrorCorrectionLevel &ecLevel);
  static Ref<Version> recommendVersion(ErrorCorrectionLevel &ecLevel,
                                            Mode &mode,
                                            BitArray &headerBits,
                                            BitArray &dataBits);

  /**
   *  Encode "bytes" with the error correction level "ecLevel". The encoding mode will be chosen
   * internally by chooseMode(). On success, store the result in "qrCode".
   *
   * We recommend you to use QRCode::EC_LEVEL_L (the lowest level) for
   * "getECLevel" since our primary use is to show QR code on desktop screens. We don't need very
   * strong error correction for this purpose.
   *
   * Note that there is no way to encode bytes in MODE_KANJI. We might want to add EncodeWithMode()
   * with which clients can specify the encoding mode. For now, we don't need the functionality.
   */

  static const int ALPHANUMERIC_TABLE_SIZE;
  static const int ALPHANUMERIC_TABLE[];

public:  //should not be public, temp solution for tests
  static const std::string DEFAULT_BYTE_MODE_ENCODING;
};

}
}

#endif // ENCODER_H
