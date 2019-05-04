#ifndef ZXING_MULTI_DETECTOR_H
#define ZXING_MULTI_DETECTOR_H

/*
 *  Copyright 2011 ZXing authors
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

#include <zxing/qrcode/detector/Detector.h>
#include <zxing/common/DetectorResult.h>
#include <zxing/DecodeHints.h>

namespace zxing {
namespace multi {

class MultiDetector : public zxing::qrcode::Detector {
  public:
    MultiDetector(Ref<BitMatrix> image);
    virtual ~MultiDetector();
    virtual std::vector<Ref<DetectorResult> > detectMulti(DecodeHints hints);
};

}
}

#endif // ZXING_MULTI_DETECTOR_H
