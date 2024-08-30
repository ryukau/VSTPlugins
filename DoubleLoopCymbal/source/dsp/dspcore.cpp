// (c) 2024 Takamitsu Endo
//
// This file is part of DoubleLoopCymbal.
//
// DoubleLoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DoubleLoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DoubleLoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

constexpr double defaultTempo = double(120);
constexpr double releaseTimeSecond = double(4.0);

constexpr const std::array<double, 256> circularModes = {
  double(1.000000000000000),  double(1.5933405056951118), double(2.135548786649403),
  double(2.295417267427694),  double(2.6530664045492145), double(2.9172954551172228),
  double(3.1554648154083624), double(3.5001474903090264), double(3.5984846739581138),
  double(3.6474511791052775), double(4.058931883331434),  double(4.131738159726708),
  double(4.230439127905234),  double(4.6010445344331075), double(4.610051645437306),
  double(4.831885262930598),  double(4.903280573212368),  double(5.083567173877822),
  double(5.1307689067016575), double(5.412118429982582),  double(5.5403985098530635),
  double(5.5531264771782425), double(5.650842376925684),  double(5.976540221648715),
  double(6.019355807422682),  double(6.152609171589256),  double(6.1631367313038865),
  double(6.208732130572546),  double(6.482735446055879),  double(6.528612451522295),
  double(6.668996900654445),  double(6.746213299505839),  double(6.848991602808508),
  double(6.9436429101526915), double(7.0707081490386905), double(7.169426625276353),
  double(7.325257332462771),  double(7.4023810568360755), double(7.468242109085181),
  double(7.514500962483965),  double(7.604536126938166),  double(7.665197838561287),
  double(7.85919706013246),   double(7.892520026843893),  double(8.071028338967128),
  double(8.131374173240902),  double(8.1568737689496),    double(8.156918842280733),
  double(8.314295631893737),  double(8.45000551018646),   double(8.645078764049176),
  double(8.652206694443466),  double(8.66047555520746),   double(8.781093075730398),
  double(8.820447105611922),  double(8.999214496283312),  double(9.130077646411111),
  double(9.167810652271394),  double(9.238840557670077),  double(9.390589484063241),
  double(9.464339027734203),  double(9.541304590034361),  double(9.612247455238109),
  double(9.678811692506123),  double(9.807815107462856),  double(9.98784275554081),
  double(10.077190497330994), double(10.091867141275257), double(10.09225481486813),
  double(10.126502295693772), double(10.18572218907702),  double(10.368705458854519),
  double(10.57471344349369),  double(10.607609550950203), double(10.68896784287112),
  double(10.706875023386747), double(10.77153891878896),  double(10.922544696482962),
  double(11.133166170756637), double(11.152639282954734), double(11.188906775410308),
  double(11.310212368186301), double(11.402312929615599), double(11.432629299891351),
  double(11.4701662560518),   double(11.654362978754861), double(11.685843549747782),
  double(11.722758172320448), double(11.903823217314876), double(12.012253849800821),
  double(12.020976194473256), double(12.078559478862408), double(12.17162315503707),
  double(12.285988718162267), double(12.488940118944772), double(12.549376432817636),
  double(12.6291936518746),   double(12.685306868214534), double(12.711609953449944),
  double(12.738806093605008), double(12.84308496674913),  double(13.066558649839825),
  double(13.08201334381275),  double(13.195723591186585), double(13.228284530761863),
  double(13.333546087983708), double(13.385453180985621), double(13.394674759934396),
  double(13.610572794452606), double(13.637496463055456), double(13.819314942198952),
  double(13.941287328845805), double(13.945767336219362), double(14.020359772593565),
  double(14.04501881871901),  double(14.1354057370185),   double(14.202434689932657),
  double(14.40316086180383),  double(14.483373598068052), double(14.549405125286688),
  double(14.645000185525108), double(14.656816446830334), double(14.692253846444542),
  double(14.761947739522833), double(14.980552310159315), double(15.021321422191345),
  double(15.145389465652915), double(15.260566826272614), double(15.31652523569637),
  double(15.328702904590145), double(15.351258321221781), double(15.552105165163068),
  double(15.555467218371973), double(15.734495694194743), double(15.866588486044524),
  double(15.868040174411112), double(15.955615704418207), double(15.998984255488747),
  double(16.08610498399543),  double(16.118344590042522), double(16.317378143958635),
  double(16.41250306033092),  double(16.4682379099555),   double(16.574020171496844),
  double(16.636735502683617), double(16.657518312060414), double(16.67972262794317),
  double(16.89459494845585),  double(16.954588545112223), double(17.061850311878718),
  double(17.184773806897258), double(17.236631644773766), double(17.265584831105425),
  double(17.305660312713336), double(17.466626675790522), double(17.493126212017213),
  double(17.649466343804967), double(17.788600705902546), double(17.789414757566867),
  double(17.886427390005295), double(17.94452559782329),  double(17.963794328004976),
  double(18.033890570040306), double(18.23159325633044),  double(18.338374034935857),
  double(18.38611884498893),  double(18.500019255387453), double(18.57504506872356),
  double(18.596751602241905), double(18.612293459048136), double(18.808671572367395),
  double(18.883777024661043), double(18.977860592070737), double(19.107005849550887),
  double(19.155531162815112), double(19.198005179702488), double(19.252122686852257),
  double(19.2700831140802),   double(19.381086718378597), double(19.564288389688688),
  double(19.70794330753125),  double(19.710513982544636), double(19.814077430361806),
  double(19.884097430065065), double(19.918892134980094), double(19.9491781754168),
  double(20.145806930665653), double(20.26195370400539),  double(20.303314818533995),
  double(20.423840543272263), double(20.50891405763646),  double(20.51324676693419),
  double(20.55956364576846),  double(20.57638224071873),  double(20.722772739552344),
  double(20.893543315236908), double(21.02779742920247),  double(21.073560526792708),
  double(21.12717283813639),  double(21.19281780880533),  double(21.225462703614895),
  double(21.29550180478402),  double(21.47900146140261),  double(21.626388363286946),
  double(21.630359471363427), double(21.739395498479965), double(21.819275510667225),
  double(21.86427572516472),  double(21.86687651357224),  double(22.060019609059026),
  double(22.220001357704295), double(22.346038863398324), double(22.429346709375974),
  double(22.43947648746872),  double(22.501264341880123), double(22.636892207884554),
  double(22.808980427054518), double(22.94750559461174),  double(22.990941679585436),
  double(23.053893406711193), double(23.129183209056905), double(23.209883019451365),
  double(23.39363224708021),  double(23.5441527495905),   double(23.66294295645443),
  double(23.75112099034419),  double(23.779229397791347), double(23.97423157260064),
  double(24.136298677667078), double(24.266994684298997), double(24.34514583273217),
  double(24.367507840105443), double(24.551025685963022), double(24.7242286319338),
  double(24.86637812027331),  double(24.978725299241102), double(25.124238079880623),
  double(25.308199377586597), double(25.46138857404755),  double(25.585113635339905),
  double(25.6940715297246),   double(25.88844300764359),  double(26.052291896063057),
  double(26.186977817460633), double(26.46517012649518),  double(26.63932842024234),
  double(26.784592423677555), double(27.03857252471371),  double(27.22271625388057),
  double(27.37820570550269),  double(27.802654042118437), double(27.968042979710326),
  double(28.379323306016918), double(28.554309478765987), double(29.137192761846382),
  double(29.716864766481052),
};

template<size_t nAllpass, typename Rng>
inline auto prepareSerialAllpassTime(double upRate, double allpassMaxTimeHz, Rng &rng)
{
  std::array<double, nAllpass> delaySamples{};
  const auto scaler = std::max(
    double(0), std::ceil(upRate * nAllpass / allpassMaxTimeHz) - double(2) * nAllpass);
  double sumSamples = 0;
  std::uniform_real_distribution<double> dist{double(0), double(1)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    delaySamples[idx] = dist(rng);
    sumSamples += delaySamples[idx];
  }
  double sumFraction = 0;
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    const auto samples = double(2) + scaler * delaySamples[idx] / sumSamples;
    delaySamples[idx] = std::floor(samples);
    sumFraction += samples - delaySamples[idx];
  }
  delaySamples[0] += std::round(sumFraction);
  return delaySamples;
}

template<typename Rng>
inline double pitchRatio(double pitch, double spread, double rndCent, Rng &rng)
{
  const auto rndRange = rndCent / double(1200);
  std::uniform_real_distribution<double> dist{-rndRange, rndRange};
  return std::lerp(double(1), pitch, spread) * std::exp2(dist(rng));
}

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = sampleRate;
  upRate = sampleRate * upFold;

  SmootherCommon<double>::setTime(double(0.2));

  releaseSmoother.setup(double(2) * upRate);

  const auto maxDelayTimeSamples = upRate * 2 * Scales::delayTimeSecond.getMax();
  for (auto &x : serialAllpass1) x.setup(maxDelayTimeSamples);
  for (auto &x : serialAllpass2) x.setup(maxDelayTimeSamples);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  useExternalInput = pv[ID::useExternalInput]->getInt();                                 \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  externalInputGain.METHOD(pv[ID::externalInputGain]->getDouble());                      \
  delayTimeModAmount.METHOD(                                                             \
    pv[ID::delayTimeModAmount]->getDouble() * upRate / double(48000));                   \
  allpassFeed1.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed1]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassFeed2.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed2]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassMixSpike.METHOD(pv[ID::allpassMixSpike]->getDouble());                          \
  allpassMixAltSign.METHOD(pv[ID::allpassMixAltSign]->getDouble());                      \
  highShelfCutoff.METHOD(EMAFilter<double>::cutoffToP(std::clamp(                        \
    pv[ID::highShelfFrequencyHz]->getDouble() / upRate, double(0), double(0.5))));       \
  highShelfGain.METHOD(pv[ID::highShelfGain]->getDouble());                              \
  lowShelfCutoff.METHOD(EMAFilter<double>::cutoffToP(std::clamp(                         \
    pv[ID::lowShelfFrequencyHz]->getDouble() / upRate, double(0), double(0.5))));        \
  lowShelfGain.METHOD(pv[ID::lowShelfGain]->getDouble());                                \
  notchMix.METHOD(pv[ID::adaptiveNotchMix]->getDouble());                                \
  notchNarrowness.METHOD(pv[ID::adaptiveNotchNarrowness]->getDouble());                  \
  stereoBalance.METHOD(pv[ID::stereoBalance]->getDouble());                              \
  stereoMerge.METHOD(pv[ID::stereoMerge]->getDouble() / double(2));                      \
                                                                                         \
  auto gain = pv[ID::outputGain]->getDouble();                                           \
  outputGain.METHOD(gain);                                                               \
                                                                                         \
  envelopeNoise.setTime(pv[ID::noiseDecaySeconds]->getDouble() * upRate);                \
  if (!pv[ID::release]->getInt() && noteStack.empty()) {                                 \
    envelopeRelease.setTime(releaseTimeSecond *upRate);                                  \
  }                                                                                      \
                                                                                         \
  updateDelayTime();

void DSPCore::updateUpRate()
{
  upRate = sampleRate * fold[overSampling];
  SmootherCommon<double>::setSampleRate(upRate);
}

void DSPCore::updateDelayTime()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  paramRng.seed(pv[ID::seed]->getInt());
  const auto delayTimeBase = pv[ID::delayTimeBaseSecond]->getDouble() * upRate;
  const auto delayTimeRandom = pv[ID::delayTimeRandomSecond]->getDouble() * upRate;
  const auto shape = pv[ID::delayTimeShape]->getDouble();
  std::uniform_real_distribution<double> delayTimeDist{
    double(0), double(delayTimeRandom)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    static_assert(nAllpass >= 1);
    const auto t1 = delayTimeBase / double(idx + 1);
    const auto t2 = delayTimeBase * (circularModes[idx] / circularModes[nAllpass - 1]);
    const auto timeHarmonics = std::lerp(t1, t2, shape);
    serialAllpass1[0].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass1[1].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass2[0].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass2[1].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
  }
}

void DSPCore::reset()
{
  noteNumber = 57.0;
  velocity = 0;

  overSampling = param.value[ParameterID::ID::overSampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  startup();

  impulse = 0;
  releaseSmoother.reset();
  envelopeNoise.reset();
  envelopeRelease.reset();
  feedbackBuffer1.fill(double(0));
  feedbackBuffer2.fill(double(0));
  for (auto &x : serialAllpass1) x.reset();
  for (auto &x : serialAllpass2) x.reset();

  for (auto &x : halfbandInput) x.fill({});
  for (auto &x : halfbandIir) x.reset();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;
  noiseRng.seed(pv[ID::seed]->getInt());
}

void DSPCore::setParameters()
{
  size_t newOverSampling = param.value[ParameterID::ID::overSampling]->getInt();
  if (overSampling != newOverSampling) {
    overSampling = newOverSampling;
    updateUpRate();
  }
  ASSIGN_PARAMETER(push);
}

std::array<double, 2> DSPCore::processFrame(const std::array<double, 2> &externalInput)
{
  const auto envRelease = envelopeRelease.process();

  const auto extGain = externalInputGain.process();
  auto timeModAmt = delayTimeModAmount.process();
  timeModAmt += (double(1) - envRelease) * (double(1000) * upRate / double(48000));

  auto apGain1 = allpassFeed1.process();
  auto apGain2 = allpassFeed2.process();
  // apGain1 = std::lerp(apGain1 * double(0.5), apGain1, envRelease);
  // apGain2 = std::lerp(apGain2 * double(0.5), apGain2, envRelease);

  const auto apMixSpike = allpassMixSpike.process();
  const auto apMixSign = allpassMixAltSign.process();
  const auto hsCut = highShelfCutoff.process();
  const auto hsGain = highShelfGain.process() * envRelease;
  const auto lsCut = lowShelfCutoff.process();
  const auto lsGain = lowShelfGain.process();
  const size_t nNotch = param.value[ParameterID::nAdaptiveNotch]->getInt();
  const auto ntMix = notchMix.process();
  const auto ntNarrowness = notchNarrowness.process();
  const auto balance = stereoBalance.process();
  const auto merge = stereoMerge.process();
  const auto outGain = outputGain.process() * envRelease;

  std::uniform_real_distribution<double> dist{double(-1), double(1)};
  const auto noiseEnv = releaseSmoother.process() + envelopeNoise.process();
  std::array<double, 2> excitation{
    -apGain1 * feedbackBuffer1[0], -apGain1 * feedbackBuffer1[1]};
  if (impulse != 0) {
    excitation[0] += impulse;
    excitation[1] += impulse;
    impulse = 0;
  } else {
    const auto ipow = [](double v) { return std::copysign(v * v * v, v); };
    excitation[0] += noiseEnv * ipow(dist(noiseRng));
    excitation[1] += noiseEnv * ipow(dist(noiseRng));
  }

  if (useExternalInput) {
    excitation[0] += externalInput[0] * extGain;
    excitation[1] += externalInput[1] * extGain;
  }

  // Normalize amplitude.
  const auto pitchRatio = interpPitch.process(pitchSmoothingKp);
  const auto normalizeGain = nAllpass
    * std::lerp(double(1) / std::sqrt(hsCut / (double(2) - hsCut)), hsGain, hsGain);
  auto ap1Out0
    = std::lerp(serialAllpass1[0].sum(apMixSign), feedbackBuffer1[0], apMixSpike)
    * normalizeGain;
  auto ap1Out1
    = std::lerp(serialAllpass1[1].sum(apMixSign), feedbackBuffer1[1], apMixSpike)
    * normalizeGain;

  feedbackBuffer1[0] = serialAllpass1[0].process(
    excitation[0], hsCut, hsGain, lsCut, lsGain, apGain1, pitchRatio, timeModAmt, nNotch,
    ntMix, ntNarrowness);
  feedbackBuffer1[1] = serialAllpass1[1].process(
    excitation[1], hsCut, hsGain, lsCut, lsGain, apGain1, pitchRatio, timeModAmt, nNotch,
    ntMix, ntNarrowness);

  auto cymbal0
    = std::lerp(serialAllpass2[0].sum(apMixSign), feedbackBuffer2[0], apMixSpike)
    * normalizeGain;
  auto cymbal1
    = std::lerp(serialAllpass2[1].sum(apMixSign), feedbackBuffer2[1], apMixSpike)
    * normalizeGain;
  feedbackBuffer2[0] = serialAllpass2[0].process(
    ap1Out0 - apGain2 * feedbackBuffer2[0], hsCut, hsGain, lsCut, lsGain, apGain2,
    pitchRatio, timeModAmt, nNotch, ntMix, ntNarrowness);
  feedbackBuffer2[1] = serialAllpass2[1].process(
    ap1Out1 - apGain2 * feedbackBuffer2[1], hsCut, hsGain, lsCut, lsGain, apGain2,
    pitchRatio, timeModAmt, nNotch, ntMix, ntNarrowness);

  constexpr auto eps = std::numeric_limits<double>::epsilon();
  if (balance < -eps) {
    cymbal0 *= double(1) + balance;
  } else if (balance > eps) {
    cymbal1 *= double(1) - balance;
  }
  return {
    outGain * std::lerp(cymbal0, cymbal1, merge),
    outGain * std::lerp(cymbal1, cymbal0, merge),
  };
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  std::array<double, 2> prevExtIn = halfbandInput[0];
  std::array<double, 2> frame{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    const double extIn0 = in0 == nullptr ? 0 : in0[i];
    const double extIn1 = in1 == nullptr ? 0 : in1[i];

    if (overSampling) {
      frame = processFrame({
        double(0.5) * (prevExtIn[0] + extIn0),
        double(0.5) * (prevExtIn[1] + extIn1),
      });
      halfbandInput[0][0] = frame[0];
      halfbandInput[1][0] = frame[1];

      frame = processFrame({extIn0, extIn1});
      halfbandInput[0][1] = frame[0];
      halfbandInput[1][1] = frame[1];

      frame[0] = halfbandIir[0].process(halfbandInput[0]);
      frame[1] = halfbandIir[1].process(halfbandInput[1]);
      out0[i] = float(frame[0]);
      out1[i] = float(frame[1]);
    } else {
      frame = processFrame({extIn0, extIn1});
      out0[i] = float(frame[0]);
      out1[i] = float(frame[1]);
    }

    prevExtIn = {extIn0, extIn1};
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto eps = std::numeric_limits<double>::epsilon();

  noteStack.push_back(info);

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  interpPitch.push(notePitch);

  velocity = velocityMap.map(info.velocity);

  if (pv[ID::resetSeedAtNoteOn]->getInt()) noiseRng.seed(pv[ID::seed]->getInt());

  const auto oscGain = velocity * pv[ID::noiseGain]->getDouble();
  releaseSmoother.prepare(
    double(0.1) * envelopeNoise.process(),
    upRate * pv[ID::noiseDecaySeconds]->getDouble());
  impulse = oscGain;
  envelopeNoise.noteOn(oscGain);
  envelopeRelease.noteOn(double(1));
  envelopeRelease.setTime(1, true);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (!noteStack.empty()) {
    noteNumber = noteStack.back().noteNumber;
    interpPitch.push(calcNotePitch(noteNumber));
  } else {
    if (!pv[ID::release]->getInt()) envelopeRelease.setTime(releaseTimeSecond * upRate);
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto centerNote = double(60);
  auto pitchBendSemitone
    = pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble();
  auto cent = pv[ID::transposeSemitone]->getDouble();
  auto notePitchAmount = pv[ID::notePitchAmount]->getDouble();
  return std::exp2(
    (pitchBendSemitone + notePitchAmount * (note - centerNote) + cent) / double(12));
}
