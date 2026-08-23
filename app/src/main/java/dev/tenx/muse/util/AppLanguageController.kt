package dev.tenx.muse.util

import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import dev.tenx.muse.domain.model.AppLanguage

object AppLanguageController {
    fun current(): AppLanguage =
        AppLanguage.fromTag(AppCompatDelegate.getApplicationLocales().toLanguageTags())

    fun apply(language: AppLanguage) {
        val locales = language.tag
            ?.let(LocaleListCompat::forLanguageTags)
            ?: LocaleListCompat.getEmptyLocaleList()

        AppCompatDelegate.setApplicationLocales(locales)
    }
}
