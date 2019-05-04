#include "ReedSolomonEncoder.h"

#include <zxing/Exception.h>
#include <stdio.h>
#include <string.h>

namespace zxing {

ReedSolomonEncoder::ReedSolomonEncoder(Ref<GenericGF> field) :
    field_(field), cachedGenerators_()
{
    ArrayRef<int> arrayRef(1); //will this work?
    arrayRef[0] = 1;
    Ref< GenericGFPoly > tmpGeneratorRef(new GenericGFPoly(field, arrayRef));
    cachedGenerators_.push_back(tmpGeneratorRef);
}

Ref<GenericGFPoly> ReedSolomonEncoder::buildGenerator(int degree)
{
    if (degree >= int(cachedGenerators_.size())) {
        Ref<GenericGFPoly> lastGenerator = cachedGenerators_.at(cachedGenerators_.size() - 1);
        for (int d = int(cachedGenerators_.size()); d <= degree; d++)
        {
            ArrayRef<int> arrayRef(2); //will this work?
            arrayRef[0] = 1;
            arrayRef[1] = field_->exp(d - 1 + field_->getGeneratorBase());
            Ref<GenericGFPoly> tmpGFRef(new GenericGFPoly(field_, arrayRef));
            Ref<GenericGFPoly> nextGenerator = (*lastGenerator).multiply(tmpGFRef);
            cachedGenerators_.push_back(nextGenerator);
            lastGenerator = nextGenerator;
        }
    }

    // ??? wont this through exception?
    // No the elements up to index degree are added above
    return cachedGenerators_.at(size_t(degree));
}

void ReedSolomonEncoder::encode(std::vector<zxing::byte> &toEncode, int ecBytes)
{
    if (ecBytes == 0) {
        throw Exception("No error correction bytes");
    }

    int dataBytes = int(toEncode.size());// - ecBytes;
    toEncode.resize(toEncode.size() + size_t(ecBytes));
    if (dataBytes <= 0) {
        throw Exception("No data bytes provided");
    }
    Ref<GenericGFPoly> generator = buildGenerator(ecBytes);
    ArrayRef<int> infoCoefficients(dataBytes);

    //to-do optimize the following loop
    for(int i=0; i< dataBytes; i++)
        infoCoefficients[i] = toEncode[size_t(i)];

    Ref<GenericGFPoly> info(new GenericGFPoly(field_, infoCoefficients));
    info = info->multiplyByMonomial(ecBytes, 1);
    Ref<GenericGFPoly> remainder = info->divide(generator)[1];
    ArrayRef<int> coefficients = remainder->getCoefficients();
    int numZeroCoefficients = ecBytes - coefficients->size();
    for (int i = 0; i < numZeroCoefficients; i++) {
        toEncode[size_t(dataBytes + i)] = 0;
    }

    for (int i = 0; i < coefficients->size(); i++)
      toEncode[size_t(dataBytes + numZeroCoefficients + i)] = zxing::byte(coefficients[int(i)]);
}

}
