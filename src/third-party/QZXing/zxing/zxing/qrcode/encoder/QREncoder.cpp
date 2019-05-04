#include "Encoder.h"
#include "MaskUtil.h"

#include <zxing/common/CharacterSetECI.h>
#include <zxing/UnsupportedEncodingException.h>
#include <zxing/WriterException.h>
#include <zxing/common/reedsolomon/ReedSolomonEncoder.h>
#include "BlockPair.h"
#include <QList>
#include <math.h>
#include <limits>
#include "MatrixUtil.h"
#include <string>
#include "zxing/common/StringUtils.h"

namespace zxing {
namespace qrcode {

const int Encoder::ALPHANUMERIC_TABLE_SIZE = 96;
// The original table is defined in the table 5 of JISX0510:2004 (p.19).
const int Encoder::ALPHANUMERIC_TABLE[Encoder::ALPHANUMERIC_TABLE_SIZE] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x00-0x0f
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x10-0x1f
    36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  // 0x20-0x2f
    0,   1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1,  // 0x30-0x3f
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // 0x40-0x4f
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,  // 0x50-0x5f
};

const std::string Encoder::DEFAULT_BYTE_MODE_ENCODING = "ISO-8859-1";

int Encoder::calculateMaskPenalty(const ByteMatrix& matrix)
{
    return MaskUtil::applyMaskPenaltyRule1(matrix)
            + MaskUtil::applyMaskPenaltyRule2(matrix)
            + MaskUtil::applyMaskPenaltyRule3(matrix)
            + MaskUtil::applyMaskPenaltyRule4(matrix);
}

Ref<QRCode> Encoder::encode(const std::string& content, ErrorCorrectionLevel &ecLevel)
{
    return encode(content, ecLevel, ZXING_NULLPTR);
}

Ref<QRCode> Encoder::encode(const std::string& content, ErrorCorrectionLevel &ecLevel, const EncodeHint* hints)
{
    // Determine what character encoding has been specified by the caller, if any
    std::string encoding = hints == ZXING_NULLPTR ? "" : hints->getCharacterSet();
    if (encoding == "")
        encoding = DEFAULT_BYTE_MODE_ENCODING;

    // Pick an encoding mode appropriate for the content. Note that this will not attempt to use
    // multiple modes / segments even if that were more efficient. Twould be nice.
    Mode mode = chooseMode(content, encoding);

    // This will store the header information, like mode and
    // length, as well as "header" segments like an ECI segment.
    BitArray headerBits;

    // Append ECI segment if applicable
    if (mode == Mode::BYTE && DEFAULT_BYTE_MODE_ENCODING != encoding) {
        zxing::common::CharacterSetECI const * eci =
                zxing::common::CharacterSetECI::getCharacterSetECIByName(encoding);
        if (eci != ZXING_NULLPTR) {
            appendECI(*eci, headerBits);
        }
    }

    // (With ECI in place,) Write the mode marker
    appendModeInfo(mode, headerBits);

    // Collect data within the main segment, separately, to count its size if needed. Don't add it to
    // main payload yet.
    BitArray dataBits;
    appendBytes(content, mode, dataBits, encoding);

    Ref<Version> version;
    if (hints != ZXING_NULLPTR/* && hints->containsKey(EncodeHintType.QR_VERSION)*/) {
        version = Version::getVersionForNumber(1);
        int bitsNeeded = calculateBitsNeeded(mode, headerBits, dataBits, version);
        if (!willFit(bitsNeeded, version, ecLevel)) {
            throw new WriterException("Data too big for requested version");
        }
    } else {
        version = recommendVersion(ecLevel, mode, headerBits, dataBits);
    }

    BitArray headerAndDataBits;
    headerAndDataBits.appendBitArray(headerBits);
    // Find "length" of main segment and write it
    int numLetters = (mode == Mode::BYTE) ? dataBits.getSizeInBytes() : int(content.length());
    appendLengthInfo(numLetters, *version, mode, headerAndDataBits);
    // Put data together into the overall payload
    headerAndDataBits.appendBitArray(dataBits);

    zxing::qrcode::ECBlocks &ecBlocks = version->getECBlocksForLevel(ecLevel);
    int numDataBytes = version->getTotalCodewords() - ecBlocks.getTotalECCodewords();

    // Terminate the bits properly.
    terminateBits(numDataBytes, headerAndDataBits);

    // Interleave data bits with error correction code.
    Ref<BitArray> finalBits(interleaveWithECBytes(headerAndDataBits,
                                                  version->getTotalCodewords(),
                                                  numDataBytes,
                                                  int(ecBlocks.getECBlocks().size())));

    Ref<QRCode> qrCode(new QRCode);

    qrCode->setECLevel(Ref<ErrorCorrectionLevel>(new ErrorCorrectionLevel(ecLevel)));
    qrCode->setMode(mode);
    qrCode->setVersion(version);

    //  Choose the mask pattern and set to "qrCode".
    int dimension = version->getDimensionForVersion();
    Ref<ByteMatrix> matrix(new ByteMatrix(size_t(dimension), size_t(dimension)));
    int maskPattern = chooseMaskPattern(finalBits, ecLevel, version, matrix);
    qrCode->setMaskPattern(maskPattern);

    // Build the matrix and set it to "qrCode".
    MatrixUtil::buildMatrix(*finalBits, ecLevel, *version, maskPattern, *matrix);
    qrCode->setMatrix(matrix);

    return qrCode;

    //return NULL;
}

bool Encoder::willFit(int numInputBits, Ref<Version> version, const ErrorCorrectionLevel &ecLevel) {
      // In the following comments, we use numbers of Version 7-H.
      // numBytes = 196
      int numBytes = version->getTotalCodewords();
      // getNumECBytes = 130
      ECBlocks& ecBlocks = version->getECBlocksForLevel(ecLevel);
      int numEcBytes = ecBlocks.getTotalECCodewords();
      // getNumDataBytes = 196 - 130 = 66
      int numDataBytes = numBytes - numEcBytes;
      int totalInputBytes = (numInputBits + 7) / 8;
      return numDataBytes >= totalInputBytes;
  }

/**
   * @return the code point of the table used in alphanumeric mode or
   *  -1 if there is no corresponding code in the table.
   */
int Encoder::getAlphanumericCode(int code)
{
    if (code < ALPHANUMERIC_TABLE_SIZE) {
        return ALPHANUMERIC_TABLE[code];
    }
    return -1;
}

/**
   * Choose the best mode by examining the content. Note that 'encoding' is used as a hint;
   * if it is Shift_JIS, and the input is only double-byte Kanji, then we return {@link Mode#KANJI}.
   */
Mode Encoder::chooseMode(const std::string& content, const std::string& encoding)
{
    if (encoding == "Shift_JIS") 
	{
        std::cout << "DEBUG: Shift_JIS detected...be aware!" << std::endl;
        return Mode::BYTE;
	}

    bool hasNumeric = false;
    bool hasAlphanumeric = false;
    for (size_t i = 0; i < content.size(); i++) {
        char c = content.at(i);
        if (c >= '0' && c <= '9') {
            hasNumeric = true;
        } else if (getAlphanumericCode(c) != -1) {
            hasAlphanumeric = true;
        } else {
            return Mode::BYTE;
        }
    }
    if (hasAlphanumeric) {
        return Mode::ALPHANUMERIC;
    }
    if (hasNumeric) {
        return Mode::NUMERIC;
    }
    return Mode::BYTE;
}

//bool Encoder::isOnlyDoubleByteKanji(const std::string& content)
//{
//    std::vector<zxing::byte> bytes;
//    try {
//        bytes = content.getBytes("Shift_JIS");
//    } catch (UnsupportedEncodingException ignored) {
//        return false;
//    }
//    int length = bytes.length;
//    if (length % 2 != 0) {
//        return false;
//    }
//    for (int i = 0; i < length; i += 2) {
//        int byte1 = bytes[i] & 0xFF;
//        if ((byte1 < 0x81 || byte1 > 0x9F) && (byte1 < 0xE0 || byte1 > 0xEB)) {
//            return false;
//        }
//    }
//    return true;
//}

int Encoder::chooseMaskPattern(Ref<BitArray> bits,
                               ErrorCorrectionLevel& ecLevel,
                               Ref<Version> version,
                               Ref<ByteMatrix> matrix)
{

    int minPenalty = std::numeric_limits<int>::max();  // Lower penalty is better.
    int bestMaskPattern = -1;
    // We try all mask patterns to choose the best one.
    for (int maskPattern = 0; maskPattern < QRCode::NUM_MASK_PATTERNS; maskPattern++) {
        MatrixUtil::buildMatrix(*bits, ecLevel, *version, maskPattern, *matrix);
        int penalty = calculateMaskPenalty(*matrix);
        if (penalty < minPenalty) {
            minPenalty = penalty;
            bestMaskPattern = maskPattern;
        }
    }
    return bestMaskPattern;
}

Ref<Version> Encoder::chooseVersion(int numInputBits, const ErrorCorrectionLevel &ecLevel)
{
    // In the following comments, we use numbers of Version 7-H.
    for (int versionNum = 1; versionNum <= 40; versionNum++) {
        Ref<Version> version = Version::getVersionForNumber(versionNum);
        if (willFit(numInputBits, version, ecLevel)) {
            return version;
        }
    }

    throw WriterException("Data too big");
}

/**
   * Terminate bits as described in 8.4.8 and 8.4.9 of JISX0510:2004 (p.24).
   */
void Encoder::terminateBits(int numDataBytes, BitArray& bits)
{
    int capacity = numDataBytes << 3;
    if (bits.getSize() > capacity) {
        std::string message("data bits cannot fit in the QR Code");
        message += zxing::common::StringUtils::intToStr(bits.getSize());
        message += " > ";
        message += zxing::common::StringUtils::intToStr(capacity);
        throw WriterException(message.c_str());
    }
    for (int i = 0; i < 4 && bits.getSize() < capacity; ++i) {
        bits.appendBit(false);
    }
    // Append termination bits. See 8.4.8 of JISX0510:2004 (p.24) for details.
    // If the last byte isn't 8-bit aligned, we'll add padding bits.
    int numBitsInLastByte = bits.getSize() & 7;//% 8;
    if (numBitsInLastByte > 0) {
        for (int i = numBitsInLastByte; i < 8; i++) {
            bits.appendBit(false);
        }
    }
    // If we have more space, we'll fill the space with padding patterns defined in 8.4.9 (p.24).
    int bitSizeInBytes = bits.getSizeInBytes();
    int numPaddingBytes = numDataBytes - bitSizeInBytes;
    for (int i = 0; i < numPaddingBytes; i++) {
        bits.appendBits((i & 0x01) == 0 ? 0xEC : 0x11, 8);
    }
    if (bits.getSize() != capacity) {
        throw WriterException("Bits size does not equal capacity");
    }
}

/**
   * Get number of data bytes and number of error correction bytes for block id "blockID". Store
   * the result in "numDataBytesInBlock", and "numECBytesInBlock". See table 12 in 8.5.1 of
   * JISX0510:2004 (p.30)
   */
void Encoder::getNumDataBytesAndNumECBytesForBlockID(int numTotalBytes,
                                                     int numDataBytes,
                                                     int numRSBlocks,
                                                     int blockID,
                                                     std::vector<int>& numDataBytesInBlock,
                                                     std::vector<int>& numECBytesInBlock)
{
    if (blockID >= numRSBlocks) {
        throw WriterException("Block ID too large");
    }
    // numRsBlocksInGroup2 = 196 % 5 = 1
    int numRsBlocksInGroup2 = numTotalBytes % numRSBlocks;
    // numRsBlocksInGroup1 = 5 - 1 = 4
    int numRsBlocksInGroup1 = numRSBlocks - numRsBlocksInGroup2;
    // numTotalBytesInGroup1 = 196 / 5 = 39
    int numTotalBytesInGroup1 = numTotalBytes / numRSBlocks;
    // numTotalBytesInGroup2 = 39 + 1 = 40
    int numTotalBytesInGroup2 = numTotalBytesInGroup1 + 1;
    // numDataBytesInGroup1 = 66 / 5 = 13
    int numDataBytesInGroup1 = numDataBytes / numRSBlocks;
    // numDataBytesInGroup2 = 13 + 1 = 14
    int numDataBytesInGroup2 = numDataBytesInGroup1 + 1;
    // numEcBytesInGroup1 = 39 - 13 = 26
    int numEcBytesInGroup1 = numTotalBytesInGroup1 - numDataBytesInGroup1;
    // numEcBytesInGroup2 = 40 - 14 = 26
    int numEcBytesInGroup2 = numTotalBytesInGroup2 - numDataBytesInGroup2;
    // Sanity checks.
    // 26 = 26
    if (numEcBytesInGroup1 != numEcBytesInGroup2) {
        throw WriterException("EC bytes mismatch");
    }
    // 5 = 4 + 1.
    if (numRSBlocks != numRsBlocksInGroup1 + numRsBlocksInGroup2) {
        throw WriterException("RS blocks mismatch");
    }
    // 196 = (13 + 26) * 4 + (14 + 26) * 1
    if (numTotalBytes !=
            ((numDataBytesInGroup1 + numEcBytesInGroup1) *
             numRsBlocksInGroup1) +
            ((numDataBytesInGroup2 + numEcBytesInGroup2) *
             numRsBlocksInGroup2)) {
        throw WriterException("Total bytes mismatch");
    }

    if (numDataBytesInBlock.size() < 1 )
        numDataBytesInBlock.resize(1);

    if (numECBytesInBlock.size() < 1 )
        numECBytesInBlock.resize(1);

    if (blockID < numRsBlocksInGroup1) {
        numDataBytesInBlock[0] = numDataBytesInGroup1;
        numECBytesInBlock[0] = numEcBytesInGroup1;
    } else {
        numDataBytesInBlock[0] = numDataBytesInGroup2;
        numECBytesInBlock[0] = numEcBytesInGroup2;
    }
}

/**
   * Interleave "bits" with corresponding error correction bytes. On success, store the result in
   * "result". The interleave rule is complicated. See 8.6 of JISX0510:2004 (p.37) for details.
   */
BitArray* Encoder::interleaveWithECBytes(const BitArray& bits,
                                         int numTotalBytes,
                                         int numDataBytes,
                                         int numRSBlocks)
{

    // "bits" must have "getNumDataBytes" bytes of data.
    if (bits.getSizeInBytes() != numDataBytes) {
        std::string message("Encoder::interleaveWithECBytes: Number of bits [");
        message += zxing::common::StringUtils::intToStr(bits.getSizeInBytes());
        message += "] and data bytes [";
        message += zxing::common::StringUtils::intToStr(numDataBytes);
        message += "] does not match";
        throw WriterException( message.c_str());
    }

    // Step 1.  Divide data bytes into blocks and generate error correction bytes for them. We'll
    // store the divided data bytes blocks and error correction bytes blocks into "blocks".
    int dataBytesOffset = 0;
    int maxNumDataBytes = 0;
    int maxNumEcBytes = 0;

    // Since, we know the number of reedsolmon blocks, we can initialize the vector with the number.
    std::vector< BlockPair > blocks;

    for (int i = 0; i < numRSBlocks; i++) {
        std::vector<int> numDataBytesInBlock;
        std::vector<int> numEcBytesInBlock;
        getNumDataBytesAndNumECBytesForBlockID(
                    numTotalBytes, numDataBytes, numRSBlocks, i,
                    numDataBytesInBlock, numEcBytesInBlock);

        int size = numDataBytesInBlock[0];
        std::vector<zxing::byte> dataBytes;
        dataBytes.resize(size_t(size));
        bits.toBytes(8*dataBytesOffset, dataBytes, 0, size);
        ArrayRef<zxing::byte> ecBytes = generateECBytes(dataBytes, numEcBytesInBlock[0]);
        blocks.push_back(BlockPair(ArrayRef<zxing::byte>(dataBytes.data(), int(dataBytes.size())), ecBytes)); //?? please revisit

        maxNumDataBytes = max(maxNumDataBytes, size);
        maxNumEcBytes = max(maxNumEcBytes, ecBytes->size());
        dataBytesOffset += numDataBytesInBlock[0];
    }
    if (numDataBytes != dataBytesOffset) {
        throw WriterException("Data bytes does not match offset");
    }

    BitArray* result = new BitArray;

    // First, place data blocks.
    for (int i = 0; i < maxNumDataBytes; i++) {
        for (std::vector< BlockPair >::iterator it=blocks.begin(); it != blocks.end(); it++) {
            ArrayRef<zxing::byte> dataBytes = it->getDataBytes();
            if (i < dataBytes.array_->size()) {
                result->appendBits(dataBytes[i], 8);  ///????? are we sure?
            }
        }
    }
    // Then, place error correction blocks.
    for (int i = 0; i < maxNumEcBytes; i++) {
        for (std::vector< BlockPair >::iterator it=blocks.begin(); it != blocks.end(); it++) {
            ArrayRef<zxing::byte> ecBytes = it->getErrorCorrectionBytes();
            if (i < ecBytes.array_->size()) {
                result->appendBits(ecBytes[i], 8);
            }
        }
    }
    if (numTotalBytes != result->getSizeInBytes()) {  // Should be same.
        std::string message("Interleaving error: ");
        message += zxing::common::StringUtils::intToStr(numTotalBytes);
        message += " and ";
        message += zxing::common::StringUtils::intToStr(result->getSizeInBytes());
        message += " differ.";
        throw WriterException(message.c_str());
    }

    return result;
}

ArrayRef<zxing::byte> Encoder::generateECBytes(const std::vector<zxing::byte>& dataBytes, int numEcBytesInBlock)
{
    size_t numDataBytes = dataBytes.size();
    std::vector<zxing::byte> dataBytesCopy(dataBytes);

    zxing::ReedSolomonEncoder encoder(GenericGF::QR_CODE_FIELD_256);
    encoder.encode(dataBytesCopy, numEcBytesInBlock);

    ArrayRef<zxing::byte> ecBytes(numEcBytesInBlock);
    for (int i = 0; i < numEcBytesInBlock; i++) {
        ecBytes[i] = dataBytesCopy[numDataBytes + size_t(i)];
    }
    return ecBytes;
}

/**
   * Append mode info. On success, store the result in "bits".
   */
void Encoder::appendModeInfo(const Mode& mode, BitArray& bits)
{
    bits.appendBits(mode.getBits(), 4);
}



/**
   * Append length info. On success, store the result in "bits".
   */
void Encoder::appendLengthInfo(int numLetters, const Version& version, const Mode& mode, BitArray& bits)
{
    int numBits = mode.getCharacterCountBits(&version);
    if (numLetters >= (1 << numBits)) {
        std::string message = zxing::common::StringUtils::intToStr(numLetters);
        message += " is bigger than ";
        message += zxing::common::StringUtils::intToStr((1 << numBits) - 1);

        throw WriterException(message.c_str());
    }
    bits.appendBits(numLetters, numBits);
}

/**
   * Append "bytes" in "mode" mode (encoding) into "bits". On success, store the result in "bits".
   */
void Encoder::appendBytes(const std::string& content,
                          Mode& mode,
                          BitArray& bits,
                          const std::string& encoding)
{
    if (mode == Mode::NUMERIC)
        appendNumericBytes(content, bits);
    else if (mode == Mode::ALPHANUMERIC)
        appendAlphanumericBytes(content, bits);
    else if (mode == Mode::BYTE)
        append8BitBytes(content, bits, encoding);
    else if (mode == Mode::KANJI)
        appendKanjiBytes(content, bits);
    else {
        std::string message("Invalid mode: ");
        message += mode.getName();
        throw WriterException(message.c_str());
    }
}

void Encoder::appendNumericBytes( const std::string& content, BitArray& bits)
{
    size_t length = content.size();
    size_t i = 0;
    while (i < length) {
        int num1 = content.at(i) - '0';
        if (i + 2 < length) {
            // Encode three numeric letters in ten bits.
            int num2 = content.at(i + 1) - '0';
            int num3 = content.at(i + 2) - '0';
            bits.appendBits(num1 * 100 + num2 * 10 + num3, 10);
            i += 3;
        } else if (i + 1 < length) {
            // Encode two numeric letters in seven bits.
            int num2 = content.at(i + 1) - '0';
            bits.appendBits(num1 * 10 + num2, 7);
            i += 2;
        } else {
            // Encode one numeric letter in four bits.
            bits.appendBits(num1, 4);
            i++;
        }
    }
}

void Encoder::appendAlphanumericBytes(const std::string& content, BitArray& bits)
{
    size_t length = content.length();
    size_t i = 0;
    while (i < length) {
        int code1 = getAlphanumericCode(content.at(i));
        if (code1 == -1) {
            throw WriterException();
        }
        if (i + 1 < length) {
            int code2 = getAlphanumericCode(content.at(i + 1));
            if (code2 == -1) {
                throw WriterException();
            }
            // Encode two alphanumeric letters in 11 bits.
            bits.appendBits(code1 * 45 + code2, 11);
            i += 2;
        } else {
            // Encode one alphanumeric letter in six bits.
            bits.appendBits(code1, 6);
            i++;
        }
    }
}

void Encoder::append8BitBytes(const std::string& content, BitArray& bits, const std::string& /*encoding*/)
{
    // For now we will suppose that all the encoding has been handled by std::string class.
    //    byte[] bytes;
    //    try {
    //        bytes = content.getBytes(encoding);
    //    } catch (UnsupportedEncodingException uee) {
    //        throw WriterException(uee);
    //    }

    for (size_t i=0; i<content.size(); i++) {
        bits.appendBits(content.at(i), 8);
    }
}

void Encoder::appendKanjiBytes(const std::string& content, BitArray& bits)
{
    // For now we will suppose that all the encoding has been handled by std::string class.
    //    try {
    //        bytes = content.getBytes("Shift_JIS");
    //    } catch (UnsupportedEncodingException uee) {
    //        throw WriterException(uee);
    //    }
    size_t length = content.size();
    for (size_t i = 0; i < length; i += 2) {
        int byte1 = content.at(i) & 0xFF;
        int byte2 = content.at(i + 1) & 0xFF;
        int code = (byte1 << 8) | byte2;
        int subtracted = -1;
        if (code >= 0x8140 && code <= 0x9ffc) {
            subtracted = code - 0x8140;
        } else if (code >= 0xe040 && code <= 0xebbf) {
            subtracted = code - 0xc140;
        }
        if (subtracted == -1) {
            throw WriterException("Invalid byte sequence");
        }
        int encoded = ((subtracted >> 8) * 0xc0) + (subtracted & 0xff);
        bits.appendBits(encoded, 13);
    }
}

void Encoder::appendECI(const zxing::common::CharacterSetECI& eci, BitArray& bits) {
    bits.appendBits(Mode::ECI.getBits(), 4);
    // This is correct for values up to 127, which is all we need now.
    bits.appendBits(eci.getValue(), 8);
}

int Encoder::calculateBitsNeeded(const Mode &mode, const BitArray &headerBits, const BitArray &dataBits, const
                                 Ref<Version> version)
{
    return headerBits.getSize() + mode.getCharacterCountBits(&(*version)) + dataBits.getSize();
}

Ref<Version> Encoder::recommendVersion(ErrorCorrectionLevel &ecLevel,
                                          Mode &mode,
                                          BitArray &headerBits,
                                          BitArray &dataBits)
{
    // Hard part: need to know version to know how many bits length takes. But need to know how many
    // bits it takes to know version. First we take a guess at version by assuming version will be
    // the minimum, 1:
    int provisionalBitsNeeded = calculateBitsNeeded(mode, headerBits, dataBits, Version::getVersionForNumber(1));
    Ref<Version> provisionalVersion = chooseVersion(provisionalBitsNeeded, ecLevel);

    // Use that guess to calculate the right version. I am still not sure this works in 100% of cases.
    int bitsNeeded = calculateBitsNeeded(mode, headerBits, dataBits, provisionalVersion);
    return chooseVersion(bitsNeeded, ecLevel);
}

}
}
