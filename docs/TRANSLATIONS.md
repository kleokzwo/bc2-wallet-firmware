# Desktop translations

The desktop wallet currently supports:

- English (`en`) – default
- German (`de`)

The selected language is stored in Qt settings under `ui/language`.

User-facing desktop strings are centralized through `desktop/bc2/i18n.py`. New user-facing UI text should be added there instead of introducing language-specific branches into wallet, USB, signing, recovery, or networking logic.

Language selection is available under **Settings → Language**. A restart is intentionally required after changing the language so an active wallet/session UI is not rebuilt while security-sensitive flows are running.

This translation layer is UI-only and must not change wallet state, signing behavior, USB protocol behavior, address derivation, recovery semantics, PIN rules, or cache/session isolation.
