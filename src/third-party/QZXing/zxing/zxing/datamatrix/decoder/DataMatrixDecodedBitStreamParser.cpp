// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  DecodedBitStreamParser.cpp
 *  zxing
 *
 *  Created by Luiz Silva on 09/02/2010.
 *  Copyright 2010 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zxing/FormatException.h>
#include <zxing/datamatrix/decoder/DecodedBitStreamParser.h>
#include <iostream>
#include <zxing/common/DecoderResult.h>

namespace zxing {
namespace datamatrix {

using namespace std;

const char DecodedBitStreamParser::C40_BASIC_SET_CHARS[] = {
    '*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};
  
const char DecodedBitStreamParser::C40_SHIFT2_SET_CHARS[] = {
    '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.',
    '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '_'
};
  
const char DecodedBitStreamParser::TEXT_BASIC_SET_CHARS[] = {
    '*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};
  
const char DecodedBitStreamParser::TEXT_SHIFT3_SET_CHARS[] = {
    '\'', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', (zxing::byte) 127
};

Ref<DecoderResult> DecodedBitStreamParser::decode(ArrayRef<zxing::byte> bytes) {
  Ref<BitSource> bits(new BitSource(bytes));
  ostringstream result;
  ostringstream resultTrailer;
  vector<zxing::byte> byteSegments;
  int mode = ASCII_ENCODE;
  do {
    if (mode == ASCII_ENCODE) {
      mode = decodeAsciiSegment(bits, result, resultTrailer);
    } else {
      switch (mode) {
        case C40_ENCODE:
          decodeC40Segment(bits, result);
          break;
        case TEXT_ENCODE:
          decodeTextSegment(bits, result);
          break;
        case ANSIX12_ENCODE:
          decodeAnsiX12Segment(bits, result);
          break;
        case EDIFACT_ENCODE:
          decodeEdifactSegment(bits, result);
          break;
        case BASE256_ENCODE:
          decodeBase256Segment(bits, result, byteSegments);
          break;
        default:
          throw FormatException("Unsupported mode indicator");
      }
      mode = ASCII_ENCODE;
    }
  } while (mode != PAD_ENCODE && bits->available() > 0);

  if (resultTrailer.str().size() > 0) {
    result << resultTrailer.str();
  }
  ArrayRef<zxing::byte> rawBytes(bytes);
  Ref<String> text(new String(result.str()));
  return Ref<DecoderResult>(new DecoderResult(rawBytes, text));
}

int DecodedBitStreamParser::decodeAsciiSegment(Ref<BitSource> bits, ostringstream & result,
  ostringstream & resultTrailer) {
  bool upperShift = false;
  do {
    int oneByte = bits->readBits(8);
    if (oneByte == 0) {
      throw FormatException("Not enough bits to decode");
    } else if (oneByte <= 128) {  // ASCII data (ASCII value + 1)
      oneByte = upperShift ? (oneByte + 128) : oneByte;
      // upperShift = false;
      result << (zxing::byte) (oneByte - 1);
      return ASCII_ENCODE;
    } else if (oneByte == 129) {  // Pad
      return PAD_ENCODE;
    } else if (oneByte <= 229) {  // 2-digit data 00-99 (Numeric Value + 130)
      int value = oneByte - 130;
      if (value < 10) { // padd with '0' for single digit values
        result << '0';
      }
      result << value;
    } else if (oneByte == 230) {  // Latch to C40 encodation
      return C40_ENCODE;
    } else if (oneByte == 231) {  // Latch to Base 256 encodation
      return BASE256_ENCODE;
    } else if (oneByte == 232) {  // FNC1
      result << ((zxing::byte) 29); // translate as ASCII 29
    } else if (oneByte == 233 || oneByte == 234) {
      // Structured Append, Reader Programming
      // Ignore these symbols for now
      // throw FormatException.getInstance();
    } else if (oneByte == 235) {  // Upper Shift (shift to Extended ASCII)
      upperShift = true;
    } else if (oneByte == 236) {  // 05 Macro
        result << ("[)>RS05GS");
        resultTrailer << ("RSEOT");
    } else if (oneByte == 237) {  // 06 Macro
      result << ("[)>RS06GS");
      resultTrailer <<  ("RSEOT");
    } else if (oneByte == 238) {  // Latch to ANSI X12 encodation
      return ANSIX12_ENCODE;
    } else if (oneByte == 239) {  // Latch to Text encodation
      return TEXT_ENCODE;
    } else if (oneByte == 240) {  // Latch to EDIFACT encodation
      return EDIFACT_ENCODE;
    } else if (oneByte == 241) {  // ECI Character
      // TODO(bbrown): I think we need to support ECI
      // throw FormatException.getInstance();
      // Ignore this symbol for now
    } else if (oneByte >= 242) { // Not to be used in ASCII encodation
      // ... but work around encoders that end with 254, latch back to ASCII
      if (oneByte != 254 || bits->available() != 0) {
        throw FormatException("Not to be used in ASCII encodation");
      }
    }
  } while (bits->available() > 0);
  return ASCII_ENCODE;
}

void DecodedBitStreamParser::decodeC40Segment(Ref<BitSource> bits, ostringstream & result) {
  // Three C40 values are encoded in a 16-bit value as
  // (1600 * C1) + (40 * C2) + C3 + 1
  // TODO(bbrown): The Upper Shift with C40 doesn't work in the 4 value scenario all the time
  bool upperShift = false;

  int cValues[3];
  int shift = 0;
  do {
    // If there is only one byte left then it will be encoded as ASCII
    if (bits->available() == 8) {
      return;
    }
    int firstByte = bits->readBits(8);
    if (firstByte == 254) {  // Unlatch codeword
      return;
    }

    parseTwoBytes(firstByte, bits->readBits(8), cValues);

    for (int i = 0; i < 3; i++) {
      int cValue = cValues[i];
      switch (shift) {
        case 0:
          if (cValue < 3) {
            shift = cValue + 1;
          } else {
            if (upperShift) {
              result << (zxing::byte) (C40_BASIC_SET_CHARS[cValue] + 128);
              upperShift = false;
            } else {
              result << C40_BASIC_SET_CHARS[cValue];
            }
          }
          break;
        case 1:
          if (upperShift) {
            result << (zxing::byte) (cValue + 128);
            upperShift = false;
          } else {
            result << (zxing::byte) cValue;
          }
          shift = 0;
          break;
        case 2:
          if (cValue < 27) {
            if (upperShift) {
              result << (zxing::byte) (C40_SHIFT2_SET_CHARS[cValue] + 128);
              upperShift = false;
            } else {
              result << C40_SHIFT2_SET_CHARS[cValue];
            }
          } else if (cValue == 27) {  // FNC1
            result << ((zxing::byte) 29); // translate as ASCII 29
          } else if (cValue == 30) {  // Upper Shift
            upperShift = true;
          } else {
            throw FormatException("decodeC40Segment: Upper Shift");
          }
          shift = 0;
          break;
        case 3:
          if (upperShift) {
            result << (zxing::byte) (cValue + 224);
            upperShift = false;
          } else {
            result << (zxing::byte) (cValue + 96);
          }
          shift = 0;
          break;
        default:
          throw FormatException("decodeC40Segment: no case");
      }
    }
  } while (bits->available() > 0);
}

void DecodedBitStreamParser::decodeTextSegment(Ref<BitSource> bits, ostringstream & result) {
  // Three Text values are encoded in a 16-bit value as
  // (1600 * C1) + (40 * C2) + C3 + 1
  // TODO(bbrown): The Upper Shift with Text doesn't work in the 4 value scenario all the time
  bool upperShift = false;

  int cValues[3];
  int shift = 0;
  do {
    // If there is only one byte left then it will be encoded as ASCII
    if (bits->available() == 8) {
      return;
    }
    int firstByte = bits->readBits(8);
    if (firstByte == 254) {  // Unlatch codeword
      return;
    }

    parseTwoBytes(firstByte, bits->readBits(8), cValues);

    for (int i = 0; i < 3; i++) {
      int cValue = cValues[i];
      switch (shift) {
        case 0:
          if (cValue < 3) {
            shift = cValue + 1;
          } else {
            if (upperShift) {
              result << (zxing::byte) (TEXT_BASIC_SET_CHARS[cValue] + 128);
              upperShift = false;
            } else {
              result << (TEXT_BASIC_SET_CHARS[cValue]);
            }
          }
          break;
        case 1:
          if (upperShift) {
            result << (zxing::byte) (cValue + 128);
            upperShift = false;
          } else {
            result << (zxing::byte) (cValue);
          }
          shift = 0;
          break;
        case 2:
          // Shift 2 for Text is the same encoding as C40
          if (cValue < 27) {
            if (upperShift) {
              result << (zxing::byte) (C40_SHIFT2_SET_CHARS[cValue] + 128);
              upperShift = false;
            } else {
              result << (C40_SHIFT2_SET_CHARS[cValue]);
            }
          } else if (cValue == 27) {  // FNC1
            result << ((zxing::byte) 29); // translate as ASCII 29
          } else if (cValue == 30) {  // Upper Shift
            upperShift = true;
          } else {
            throw FormatException("decodeTextSegment: Upper Shift");
          }
          shift = 0;
          break;
        case 3:
          if (upperShift) {
            result << (zxing::byte) (TEXT_SHIFT3_SET_CHARS[cValue] + 128);
            upperShift = false;
          } else {
            result << (TEXT_SHIFT3_SET_CHARS[cValue]);
          }
          shift = 0;
          break;
        default:
          throw FormatException("decodeTextSegment: no case");
      }
    }
  } while (bits->available() > 0);
}

void DecodedBitStreamParser::decodeAnsiX12Segment(Ref<BitSource> bits, ostringstream & result) {
  // Three ANSI X12 values are encoded in a 16-bit value as
  // (1600 * C1) + (40 * C2) + C3 + 1

  int cValues[3];
  do {
    // If there is only one byte left then it will be encoded as ASCII
    if (bits->available() == 8) {
      return;
    }
    int firstByte = bits->readBits(8);
    if (firstByte == 254) {  // Unlatch codeword
      return;
    }

    parseTwoBytes(firstByte, bits->readBits(8), cValues);

    for (int i = 0; i < 3; i++) {
      int cValue = cValues[i];
      if (cValue == 0) {  // X12 segment terminator <CR>
        result << '\r';
      } else if (cValue == 1) {  // X12 segment separator *
        result << '*';
      } else if (cValue == 2) {  // X12 sub-element separator >
        result << '>';
      } else if (cValue == 3) {  // space
        result << ' ';
      } else if (cValue < 14) {  // 0 - 9
        result << (zxing::byte) (cValue + 44);
      } else if (cValue < 40) {  // A - Z
        result << (zxing::byte) (cValue + 51);
      } else {
        throw FormatException("decodeAnsiX12Segment: no case");
      }
    }
  } while (bits->available() > 0);
}

void DecodedBitStreamParser::parseTwoBytes(int firstByte, int secondByte, int* result) {
  int fullBitValue = (firstByte << 8) + secondByte - 1;
  int temp = fullBitValue / 1600;
  result[0] = temp;
  fullBitValue -= temp * 1600;
  temp = fullBitValue / 40;
  result[1] = temp;
  result[2] = fullBitValue - temp * 40;
}
  
void DecodedBitStreamParser::decodeEdifactSegment(Ref<BitSource> bits, ostringstream & result) {
  do {
    // If there is only two or less bytes left then it will be encoded as ASCII
    if (bits->available() <= 16) {
      return;
    }

    for (int i = 0; i < 4; i++) {
      int edifactValue = bits->readBits(6);

      // Check for the unlatch character
      if (edifactValue == 0x1f) {  // 011111
        // Read rest of byte, which should be 0, and stop
        int bitsLeft = 8 - bits->getBitOffset();
        if (bitsLeft != 8) {
          bits->readBits(bitsLeft);
        }
        return;
      }

      if ((edifactValue & 0x20) == 0) {  // no 1 in the leading (6th) bit
        edifactValue |= 0x40;  // Add a leading 01 to the 6 bit binary value
      }
      result << (zxing::byte)(edifactValue);
    }
  } while (bits->available() > 0);
}
  
void DecodedBitStreamParser::decodeBase256Segment(Ref<BitSource> bits, ostringstream& result, vector<zxing::byte> byteSegments) {
  // Figure out how long the Base 256 Segment is.
  int codewordPosition = 1 + bits->getByteOffset(); // position is 1-indexed
  int d1 = unrandomize255State(bits->readBits(8), codewordPosition++);
  int count;
  if (d1 == 0) {  // Read the remainder of the symbol
    count = bits->available() / 8;
  } else if (d1 < 250) {
    count = d1;
  } else {
    count = 250 * (d1 - 249) + unrandomize255State(bits->readBits(8), codewordPosition++);
  }

  // We're seeing NegativeArraySizeException errors from users.
  if (count < 0) {
    throw FormatException("NegativeArraySizeException");
  }

  char* bytes = new char[count];
  for (int i = 0; i < count; i++) {
    // Have seen this particular error in the wild, such as at
    // http://www.bcgen.com/demo/IDAutomationStreamingDataMatrix.aspx?MODE=3&D=Fred&PFMT=3&PT=F&X=0.3&O=0&LM=0.2
    if (bits->available() < 8) {
      throw FormatException("byteSegments");
    }
    bytes[i] = unrandomize255State(bits->readBits(8), codewordPosition++);
    byteSegments.push_back(bytes[i]);
    result << (zxing::byte)bytes[i];
  }
  delete [] bytes;
}
}
}

