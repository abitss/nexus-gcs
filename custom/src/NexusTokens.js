.pragma library

// NEXUS operator UI tokens. Keep raw values centralized so field ergonomics
// and sunlight contrast can be tuned without chasing magic numbers.
var space2 = 2
var space4 = 4
var space6 = 6
var space8 = 8
var space12 = 12
var space16 = 16
var space20 = 20
var space24 = 24

var radiusSmall = 8
var radiusMedium = 12
var radiusLarge = 16

var touchMin = 48
var touchComfort = 52

var textCaption = 10
var textBody = 12
var textMetric = 17
var textSection = 14
var textTitle = 20

var motionFast = 120
var motionStandard = 180

var bg = "#081017"
var surface = "#0D171F"
var surfaceRaised = "#12212B"
var surfacePressed = "#19303B"
var border = "#45606F"
var borderSubtle = "#2E414D"

var textPrimary = "#FFFFFF"
var textSecondary = "#C6D3DB"
var textMuted = "#93A6B2"

var success = "#38D6A3"
var successSoft = "#123E34"
var warning = "#FFD166"
var warningSoft = "#4A3A12"
var critical = "#FF6B70"
var criticalSoft = "#481D22"
var info = "#77BDFB"
var infoSoft = "#16364F"

function statusColor(state) {
    if (state === "CRITICAL" || state === "BLOCKED" || state === "FAIL") return critical
    if (state === "WARNING" || state === "DEGRADED") return warning
    if (state === "NOMINAL" || state === "READY" || state === "PASS") return success
    return textMuted
}
