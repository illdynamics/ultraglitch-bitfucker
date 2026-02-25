#include "EffectBase.h"

namespace ultraglitch::dsp {
void EffectBase::setEnabled(bool shouldBeEnabled)
{
    enabled.store(shouldBeEnabled);
}

bool EffectBase::isEnabled() const
{
    return enabled.load();
}

void EffectBase::setMix(float newMix)
{
    dryWetMix.store(juce::jlimit(0.0f, 1.0f, newMix));
}

float EffectBase::getMix() const
{
    return dryWetMix.load();
}
}