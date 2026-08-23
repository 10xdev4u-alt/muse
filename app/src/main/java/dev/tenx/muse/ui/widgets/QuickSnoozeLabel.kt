package dev.tenx.muse.ui.widgets

import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource
import dev.tenx.muse.R
import dev.tenx.muse.domain.model.QuickSnoozePreset

@Composable
fun quickSnoozeLabel(preset: QuickSnoozePreset): String = when (preset) {
    QuickSnoozePreset.TONIGHT -> stringResource(R.string.settings_snooze_tonight)
    QuickSnoozePreset.TOMORROW_MORNING -> stringResource(R.string.settings_snooze_tomorrow)
    QuickSnoozePreset.NEXT_WEEKDAY -> stringResource(R.string.snooze_next_weekday)
}
