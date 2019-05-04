#ifndef REEDSOLOMONENCODER_H
#define REEDSOLOMONENCODER_H

#include <zxing/common/reedsolomon/GenericGFPoly.h>
#include <zxing/common/reedsolomon/GenericGF.h>
#include <zxing/common/Array.h>
#include <zxing/common/Types.h>

namespace zxing {

class ReedSolomonEncoder
{
private:
    Ref<GenericGF> field_;
    std::vector< Ref< GenericGFPoly > >cachedGenerators_;

    Ref<GenericGFPoly> buildGenerator(int degree);

public:
    ReedSolomonEncoder(Ref<GenericGF> field);

    void encode(std::vector<zxing::byte> &toEncode, int ecBytes);
};

}

#endif // REEDSOLOMONENCODER_H
