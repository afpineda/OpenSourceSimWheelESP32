/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `language` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheelTypes.h"
#include "SimWheel.h"
#include "strings.h"
#include "Preferences.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define PREFS_NAMESPACE_KEY "lang"
static language_t currentLang = LANG_EN;

// ----------------------------------------------------------------------------
// Global strings
// ----------------------------------------------------------------------------

// English
static const char str_en_clutch_function[] = "Clutch paddles";
static const char str_en_clutch_function_value[3][17] = {"Clutch", "ALT", "Button"};
static const char str_en_clutch_cal[] = "Bite point";
static const char str_en_lang[] = "Language";
static const char str_en_slot[] = "Slot %02.2d";
static const char str_en_load_preset[] = "Load preset";
static const char str_en_save_preset[] = "Save preset";
static const char str_en_exit[] = "Exit";
static const char str_en_saved[] = "Saved!";
static const char str_en_connected[] = "Connected!";
static const char str_en_alt_function[] = "ALT buttons";
static const char str_en_powerOff[] = "Power off";
static const char str_en_battery[] = "Battery";
static const char str_en_recalibrate[] = "Recalibrate";
static const char str_en_on[] = "On";
static const char str_en_off[] = "Off";
static const char str_en_wellcome[] = "Wellcome";

// Spanish
static const char str_es_clutch_function[] = "Levas embrague";
static const char str_es_clutch_function_value[3][17] = {"Embragar", "ALT", "Boton"};
static const char str_es_clutch_cal[] = "P. mordida";
static const char str_es_lang[] = "Idioma";
static const char str_es_slot[] = "Ranura %02.2d";
static const char str_es_load_preset[] = "Cargar ajustes";
static const char str_es_save_preset[] = "Salvar ajustes";
static const char str_es_exit[] = "Salir";
static const char str_es_saved[] = "Guardado!";
static const char str_es_connected[] = "Conectado!";
static const char str_es_alt_function[] = "Botones ALT";
static const char str_es_powerOff[] = "Apagar";
static const char str_es_battery[] = "Bateria";
static const char str_es_recalibrate[] = "Recalibrar";
static const char str_es_on[] = "Si";
static const char str_es_off[] = "No";
static const char str_es_wellcome[] = "Bienvenido";

// Language-agnostic strings
static const char str_xx_lang_en[] = "English";
static const char str_xx_lang_es[] = "Espanol";
static const char str_xx_menu[] = "Menu";
static const char str_xx_frameserver[] = "Frameserver";

// Current language
char *str_clutch_function = (char *)str_en_clutch_function;
char *str_clutch_function_value[3] = {
    (char *)str_en_clutch_function_value[0],
    (char *)str_en_clutch_function_value[1],
    (char *)str_en_clutch_function_value[2]};
char *str_clutch_cal = (char *)str_en_clutch_cal;
char *str_lang = (char *)str_en_lang;
char *str_slot = (char *)str_en_slot;
char *str_load_preset = (char *)str_en_load_preset;
char *str_save_preset = (char *)str_en_save_preset;
char *str_exit = (char *)str_en_exit;
char *str_saved = (char *)str_en_saved;
char *str_connected = (char *)str_en_connected;
char *str_alt_function = (char *)str_en_alt_function;
char *str_powerOff = (char *)str_en_powerOff;
char *str_battery = (char *)str_en_battery;
char *str_recalibrate = (char *)str_en_recalibrate;
char *str_on = (char *)str_en_on;
char *str_off = (char *)str_en_off;
char *str_wellcome = (char *)str_en_wellcome;

char *str_lang_en = (char *)str_xx_lang_en;
char *str_lang_es = (char *)str_xx_lang_es;
char *str_menu = (char *)str_xx_menu;
char *str_frameserver = (char *)str_xx_frameserver;

// ----------------------------------------------------------------------------
// Auxiliary
// ---------------------------------------------------------------------------

void setEnglish()
{
    str_clutch_function = (char *)str_en_clutch_function;
    for (int i = 0; i < 3; i++)
        str_clutch_function_value[i] = (char *)str_en_clutch_function_value[i];
    str_clutch_cal = (char *)str_en_clutch_cal;
    str_lang = (char *)str_en_lang;
    str_slot = (char *)str_en_slot;
    str_load_preset = (char *)str_en_load_preset;
    str_save_preset = (char *)str_en_save_preset;
    str_exit = (char *)str_en_exit;
    str_saved = (char *)str_en_saved;
    str_connected = (char *)str_en_connected;
    str_alt_function = (char *)str_en_alt_function;
    str_powerOff = (char *)str_en_powerOff;
    str_battery = (char *)str_en_battery;
    str_recalibrate = (char *)str_en_recalibrate;
    str_on = (char *)str_en_on;
    str_off = (char *)str_en_off;
    str_wellcome = (char *)str_en_wellcome;
}

void setSpanish()
{
    str_clutch_function = (char *)str_es_clutch_function;
    for (int i = 0; i < 3; i++)
        str_clutch_function_value[i] = (char *)str_es_clutch_function_value[i];
    str_clutch_cal = (char *)str_es_clutch_cal;
    str_lang = (char *)str_es_lang;
    str_slot = (char *)str_es_slot;
    str_load_preset = (char *)str_es_load_preset;
    str_save_preset = (char *)str_es_save_preset;
    str_exit = (char *)str_es_exit;
    str_saved = (char *)str_es_saved;
    str_connected = (char *)str_es_connected;
    str_alt_function = (char *)str_es_alt_function;
    str_powerOff = (char *)str_es_powerOff;
    str_battery = (char *)str_es_battery;
    str_recalibrate = (char *)str_es_recalibrate;
    str_on = (char *)str_es_on;
    str_off = (char *)str_es_off;
    str_wellcome = (char *)str_es_wellcome;
}

// ----------------------------------------------------------------------------
// Namespace
// ---------------------------------------------------------------------------

void language::begin()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE_KEY, true))
    {
        uint8_t value = prefs.getUChar(PREFS_NAMESPACE_KEY, LANG_EN);
        if ((value >= LANG_EN) && (value <= LANG_ES))
        {
            currentLang = (language_t)value;
            if (currentLang == LANG_ES)
            {
                setSpanish();
            }
        }
    }
}

void language::setLanguage(language_t lang)
{
    if (lang != currentLang)
    {
        currentLang = lang;
        switch (currentLang)
        {
        case LANG_EN:
            setEnglish();
            break;
        case LANG_ES:
            setSpanish();
            break;
        default:
            return;
        }
        Preferences prefs;
        if (prefs.begin(PREFS_NAMESPACE_KEY, false))
        {
            prefs.putUChar(PREFS_NAMESPACE_KEY, (uint8_t)currentLang);
            prefs.end();
        }
    }
}

language_t language::getLanguage()
{
    return currentLang;
}