// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "pluginterfaces/vst/vsttypes.h"

#include "parameter.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

LogScale<Vst::ParamValue> GlobalParameter::scaleOscGain(0.0, 1.0, 0.75, 0.5);
LinearScale<Vst::ParamValue> GlobalParameter::scaleSemi(-24.0, 24.0);
LinearScale<Vst::ParamValue> GlobalParameter::scaleCent(-100.0, 100.0);
LinearScale<Vst::ParamValue> GlobalParameter::scaleSync(0.01, 16.0);

LogScale<Vst::ParamValue> GlobalParameter::scaleFMToSync(0.0, 1.0, 0.5, 0.1);
LogScale<Vst::ParamValue> GlobalParameter::scaleFMToFreq(0.0, 1.0, 0.5, 0.1);

LogScale<Vst::ParamValue> GlobalParameter::scaleGain(0.0, 4.0, 0.75, 1.0);

LogScale<Vst::ParamValue> GlobalParameter::scaleEnvelopeA(0.0001, 16.0, 0.5, 1.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleEnvelopeD(0.0001, 16.0, 0.5, 1.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleEnvelopeS(0.0, 1.0, 0.5, 0.3);
LogScale<Vst::ParamValue> GlobalParameter::scaleEnvelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<Vst::ParamValue> GlobalParameter::scaleFilterCutoff(20.0, 20000.0, 0.5, 800.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleFilterResonance(0.001, 1.0, 0.5, 0.3);
LogScale<Vst::ParamValue> GlobalParameter::scaleFilterFeedback(0.0, 1.0, 0.5, 0.3);
LogScale<Vst::ParamValue> GlobalParameter::scaleFilterSaturation(0.01, 8.0, 0.3, 0.5);
LinearScale<Vst::ParamValue> GlobalParameter::scaleFilterCutoffAmount(-1.0, 1.0);
LinearScale<Vst::ParamValue> GlobalParameter::scaleFilterKeyMod(-1.0, 1.0);

LogScale<Vst::ParamValue> GlobalParameter::scaleModEnvelopeA(0.0, 4.0, 0.5, 0.5);
LogScale<Vst::ParamValue> GlobalParameter::scaleModEnvelopeCurve(1.0, 96.0, 0.5, 8.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleModLFOFrequency(0.01, 20.0, 0.5, 1.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleModToFreq(0.0, 16.0, 0.5, 1.0);
LogScale<Vst::ParamValue> GlobalParameter::scaleModToSync(0.0, 16.0, 0.5, 1.0);

} // namespace Synth
} // namespace Steinberg
