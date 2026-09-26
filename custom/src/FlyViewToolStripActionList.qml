import QtQml.Models

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlyView

ToolStripActionList {
    signal displayPreFlightChecklist

    model: [
        PreFlightCheckListShowAction { onTriggered: displayPreFlightChecklist() },
        GuidedActionTakeoff { },
        GuidedActionPause { },
        GuidedActionRTL { },
        GuidedActionLand { },
        FlyViewAdditionalActionsButton { }
    ]
}
