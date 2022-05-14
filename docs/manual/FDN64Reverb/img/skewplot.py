import math
import numpy as np
import matplotlib.pyplot as plt

def getSplitGain(skew, phase):
    gain = np.exp(skew * np.sin(math.tau * phase))
    return gain / np.sum(gain)

skewMax = 6
phase = np.linspace(0, 1, 64, endpoint=False)

plt.figure(figsize=(6, 3))
cmap = plt.get_cmap("plasma")
for skew in np.linspace(0, skewMax, skewMax + 1):
    splitGain = getSplitGain(skew, phase)
    plt.plot(
        phase,
        splitGain,
        lw=1,
        color=cmap(skew / skewMax),
        label=f"skew={skew}",
    )
plt.title("e^(skew * sin(ωt)), Amplitude is normalized")
plt.ylabel("Amplitude")
plt.xlabel("Normalized Phase [rad/2π]")
plt.legend(ncol=2)
plt.grid()
plt.tight_layout()
plt.savefig("skewplot.svg")
plt.show()
