#ifndef COMPILEFORGE_VALIDATION_IMPACT_VALIDATOR_HPP
#define COMPILEFORGE_VALIDATION_IMPACT_VALIDATOR_HPP

#include <compileforge/validation/prediction_model.hpp>

namespace compileforge {

class ImpactValidator {
public:
    static ImpactValidationResult validate(
        const ImpactPrediction& prediction,
        const BuildObservation& observation
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_VALIDATION_IMPACT_VALIDATOR_HPP
