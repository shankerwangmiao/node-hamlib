/**
 * hamlib_shim.h - Pure C interface layer for Hamlib
 *
 * This shim provides a compiler-safe C ABI boundary between the Node.js
 * native addon (compiled with MSVC on Windows) and the Hamlib library
 * (compiled with MinGW on Windows).
 *
 * On Linux/macOS: compiled as static library (.a), linked into addon
 * On Windows: compiled as DLL (MinGW), loaded by addon (MSVC)
 *
 * All Hamlib struct access is encapsulated here to avoid cross-compiler
 * struct layout differences.
 */

#ifndef HAMLIB_SHIM_H
#define HAMLIB_SHIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DLL export/import macros */
#ifdef _WIN32
  #ifdef HAMLIB_SHIM_BUILD
    #define SHIM_API __declspec(dllexport)
  #else
    #define SHIM_API __declspec(dllimport)
  #endif
#else
  #define SHIM_API
#endif

/* Opaque handle type - hides RIG* / ROT* from the addon */
typedef void* hamlib_shim_handle_t;

/* ===== Constants (mirror Hamlib values) ===== */

/* Return codes */
#define SHIM_RIG_OK          0
#define SHIM_RIG_EINVAL     -1
#define SHIM_RIG_EIO        -2
#define SHIM_RIG_ENIMPL     -3
#define SHIM_RIG_ETIMEOUT   -4
#define SHIM_RIG_ENAVAIL    -11
#define SHIM_RIG_EPROTO     -8

/* VFO constants */
#define SHIM_RIG_VFO_NONE    0
#define SHIM_RIG_VFO_CURR    ((int)(1u<<29))    /* RIG_VFO_CURR */
#define SHIM_RIG_VFO_MEM     ((int)0x10000000)  /* RIG_VFO_MEM */
#define SHIM_RIG_VFO_A       ((int)(1<<0))      /* RIG_VFO_A */
#define SHIM_RIG_VFO_B       ((int)(1<<1))      /* RIG_VFO_B */

/* Mode constants */
#define SHIM_RIG_MODE_NONE   0

/* Split constants */
#define SHIM_RIG_SPLIT_OFF   0
#define SHIM_RIG_SPLIT_ON    1

/* PTT constants */
#define SHIM_RIG_PTT_OFF     0
#define SHIM_RIG_PTT_ON      1

/* DCD constants */
#define SHIM_RIG_DCD_OFF     0
#define SHIM_RIG_DCD_ON      1

/* Passband constants */
#define SHIM_RIG_PASSBAND_NORMAL  0
#define SHIM_RIG_PASSBAND_NOCHANGE (-1)

/* Port types (must match hamlib rig_port_e enum) */
#define SHIM_RIG_PORT_NONE     0
#define SHIM_RIG_PORT_SERIAL   1
#define SHIM_RIG_PORT_NETWORK  2

/* Scan types */
#define SHIM_RIG_SCAN_STOP   0
#define SHIM_RIG_SCAN_MEM    1
#define SHIM_RIG_SCAN_VFO    (1<<1)
#define SHIM_RIG_SCAN_PROG   (1<<2)
#define SHIM_RIG_SCAN_DELTA  (1<<3)
#define SHIM_RIG_SCAN_PRIO   (1<<4)

/* Parm constants (bit positions for setting_t / uint64_t) */
#define SHIM_RIG_PARM_ANN        (1ULL << 0)
#define SHIM_RIG_PARM_APO        (1ULL << 1)
#define SHIM_RIG_PARM_BACKLIGHT  (1ULL << 2)
#define SHIM_RIG_PARM_BEEP       (1ULL << 4)
#define SHIM_RIG_PARM_TIME       (1ULL << 5)
#define SHIM_RIG_PARM_BAT        (1ULL << 6)
#define SHIM_RIG_PARM_KEYLIGHT   (1ULL << 7)
#define SHIM_RIG_PARM_SCREENSAVER (1ULL << 8)

/* Transceive mode */
#define SHIM_RIG_TRN_POLL    1

/* Power status */
#define SHIM_RIG_POWER_UNKNOWN -1

/* PTT types (for port config) */
#define SHIM_RIG_PTT_NONE         0
#define SHIM_RIG_PTT_RIG          1
#define SHIM_RIG_PTT_SERIAL_DTR   2
#define SHIM_RIG_PTT_SERIAL_RTS   3
#define SHIM_RIG_PTT_PARALLEL     4
#define SHIM_RIG_PTT_CM108        8
#define SHIM_RIG_PTT_GPIO         16
#define SHIM_RIG_PTT_GPION        32

/* DCD types (for port config) */
#define SHIM_RIG_DCD_NONE         0
#define SHIM_RIG_DCD_RIG          1
#define SHIM_RIG_DCD_SERIAL_DSR   2
#define SHIM_RIG_DCD_SERIAL_CTS   3
#define SHIM_RIG_DCD_SERIAL_CAR   4
#define SHIM_RIG_DCD_PARALLEL     5
#define SHIM_RIG_DCD_CM108        8
#define SHIM_RIG_DCD_GPIO         16
#define SHIM_RIG_DCD_GPION        32

/* Serial parity */
#define SHIM_RIG_PARITY_NONE   0
#define SHIM_RIG_PARITY_ODD    1
#define SHIM_RIG_PARITY_EVEN   2

/* Serial handshake */
#define SHIM_RIG_HANDSHAKE_NONE      0
#define SHIM_RIG_HANDSHAKE_XONXOFF   1
#define SHIM_RIG_HANDSHAKE_HARDWARE  2

/* Serial control state */
#define SHIM_RIG_SIGNAL_UNSET  0
#define SHIM_RIG_SIGNAL_ON     1
#define SHIM_RIG_SIGNAL_OFF    2

/* Repeater shift */
#define SHIM_RIG_RPT_SHIFT_NONE  0

/* Reset types */
#define SHIM_RIG_RESET_NONE     0
#define SHIM_RIG_RESET_SOFT     1
#define SHIM_RIG_RESET_VFO      2
#define SHIM_RIG_RESET_MCALL    4
#define SHIM_RIG_RESET_MASTER   8

/* Antenna */
#define SHIM_RIG_ANT_CURR  ((int)0x80000000u)

/* ===== Rotator constants (mirror Hamlib values) ===== */
#define SHIM_ROT_TYPE_OTHER      0
#define SHIM_ROT_TYPE_AZIMUTH    (1 << 1)
#define SHIM_ROT_TYPE_ELEVATION  (1 << 2)
#define SHIM_ROT_TYPE_AZEL       (SHIM_ROT_TYPE_AZIMUTH | SHIM_ROT_TYPE_ELEVATION)

#define SHIM_ROT_MOVE_UP          (1 << 1)
#define SHIM_ROT_MOVE_DOWN        (1 << 2)
#define SHIM_ROT_MOVE_LEFT        (1 << 3)
#define SHIM_ROT_MOVE_CCW         SHIM_ROT_MOVE_LEFT
#define SHIM_ROT_MOVE_RIGHT       (1 << 4)
#define SHIM_ROT_MOVE_CW          SHIM_ROT_MOVE_RIGHT
#define SHIM_ROT_MOVE_UP_LEFT     (1 << 5)
#define SHIM_ROT_MOVE_UP_CCW      SHIM_ROT_MOVE_UP_LEFT
#define SHIM_ROT_MOVE_UP_RIGHT    (1 << 6)
#define SHIM_ROT_MOVE_UP_CW       SHIM_ROT_MOVE_UP_RIGHT
#define SHIM_ROT_MOVE_DOWN_LEFT   (1 << 7)
#define SHIM_ROT_MOVE_DOWN_CCW    SHIM_ROT_MOVE_DOWN_LEFT
#define SHIM_ROT_MOVE_DOWN_RIGHT  (1 << 8)
#define SHIM_ROT_MOVE_DOWN_CW     SHIM_ROT_MOVE_DOWN_RIGHT
#define SHIM_ROT_SPEED_NOCHANGE   (-1)

#define SHIM_ROT_RESET_ALL        1
#define SHIM_ROT_LEVEL_SPEED      (1ULL << 0)

#define SHIM_ROT_STATUS_NONE          0
#define SHIM_ROT_STATUS_BUSY          (1 << 0)
#define SHIM_ROT_STATUS_MOVING        (1 << 1)
#define SHIM_ROT_STATUS_MOVING_AZ     (1 << 2)
#define SHIM_ROT_STATUS_MOVING_LEFT   (1 << 3)
#define SHIM_ROT_STATUS_MOVING_RIGHT  (1 << 4)
#define SHIM_ROT_STATUS_MOVING_EL     (1 << 5)
#define SHIM_ROT_STATUS_MOVING_UP     (1 << 6)
#define SHIM_ROT_STATUS_MOVING_DOWN   (1 << 7)
#define SHIM_ROT_STATUS_LIMIT_UP      (1 << 8)
#define SHIM_ROT_STATUS_LIMIT_DOWN    (1 << 9)
#define SHIM_ROT_STATUS_LIMIT_LEFT    (1 << 10)
#define SHIM_ROT_STATUS_LIMIT_RIGHT   (1 << 11)
#define SHIM_ROT_STATUS_OVERLAP_UP    (1 << 12)
#define SHIM_ROT_STATUS_OVERLAP_DOWN  (1 << 13)
#define SHIM_ROT_STATUS_OVERLAP_LEFT  (1 << 14)
#define SHIM_ROT_STATUS_OVERLAP_RIGHT (1 << 16)

/* Rig type mask and types */
#define SHIM_RIG_TYPE_MASK  0x7F000000

/* Max path length */
#define SHIM_HAMLIB_FILPATHLEN  512

/* Max modes for iteration */
#define SHIM_HAMLIB_MAX_MODES 64

/* ===== Level constants (bit positions for setting_t / uint64_t) ===== */
#define SHIM_RIG_LEVEL_PREAMP       (1ULL << 0)
#define SHIM_RIG_LEVEL_ATT          (1ULL << 1)
#define SHIM_RIG_LEVEL_VOXDELAY     (1ULL << 2)
#define SHIM_RIG_LEVEL_AF           (1ULL << 3)
#define SHIM_RIG_LEVEL_RF           (1ULL << 4)
#define SHIM_RIG_LEVEL_SQL          (1ULL << 5)
#define SHIM_RIG_LEVEL_IF           (1ULL << 6)
#define SHIM_RIG_LEVEL_APF          (1ULL << 7)
#define SHIM_RIG_LEVEL_NR           (1ULL << 8)
#define SHIM_RIG_LEVEL_PBT_IN       (1ULL << 9)
#define SHIM_RIG_LEVEL_PBT_OUT      (1ULL << 10)
#define SHIM_RIG_LEVEL_CWPITCH      (1ULL << 11)
#define SHIM_RIG_LEVEL_RFPOWER      (1ULL << 12)
#define SHIM_RIG_LEVEL_MICGAIN      (1ULL << 13)
#define SHIM_RIG_LEVEL_KEYSPD       (1ULL << 14)
#define SHIM_RIG_LEVEL_NOTCHF       (1ULL << 15)
#define SHIM_RIG_LEVEL_COMP         (1ULL << 16)
#define SHIM_RIG_LEVEL_AGC          (1ULL << 17)
#define SHIM_RIG_LEVEL_BKINDL       (1ULL << 18)
#define SHIM_RIG_LEVEL_BALANCE      (1ULL << 19)
#define SHIM_RIG_LEVEL_METER        (1ULL << 20)
#define SHIM_RIG_LEVEL_VOXGAIN      (1ULL << 21)
#define SHIM_RIG_LEVEL_ANTIVOX      (1ULL << 22)
#define SHIM_RIG_LEVEL_SLOPE_LOW    (1ULL << 23)
#define SHIM_RIG_LEVEL_SLOPE_HIGH   (1ULL << 24)
#define SHIM_RIG_LEVEL_BKIN_DLYMS   (1ULL << 25)
#define SHIM_RIG_LEVEL_RAWSTR       (1ULL << 26)
/* bit 27 reserved (was SQLSTAT, deprecated) */
#define SHIM_RIG_LEVEL_SWR          (1ULL << 28)
#define SHIM_RIG_LEVEL_ALC          (1ULL << 29)
#define SHIM_RIG_LEVEL_STRENGTH     (1ULL << 30)
/* bit 31 reserved */
#define SHIM_RIG_LEVEL_RFPOWER_METER (1ULL << 32)
#define SHIM_RIG_LEVEL_COMP_METER   (1ULL << 33)
#define SHIM_RIG_LEVEL_VD_METER     (1ULL << 34)
#define SHIM_RIG_LEVEL_ID_METER     (1ULL << 35)
#define SHIM_RIG_LEVEL_NOTCHF_RAW   (1ULL << 36)
#define SHIM_RIG_LEVEL_MONITOR_GAIN (1ULL << 37)
#define SHIM_RIG_LEVEL_NB           (1ULL << 38)
#define SHIM_RIG_LEVEL_RFPOWER_METER_WATTS (1ULL << 39)
#define SHIM_RIG_LEVEL_SPECTRUM_MODE (1ULL << 40)
#define SHIM_RIG_LEVEL_SPECTRUM_SPAN (1ULL << 41)
#define SHIM_RIG_LEVEL_SPECTRUM_EDGE_LOW (1ULL << 42)
#define SHIM_RIG_LEVEL_SPECTRUM_EDGE_HIGH (1ULL << 43)
#define SHIM_RIG_LEVEL_SPECTRUM_SPEED (1ULL << 44)
#define SHIM_RIG_LEVEL_SPECTRUM_REF (1ULL << 45)
#define SHIM_RIG_LEVEL_SPECTRUM_AVG (1ULL << 46)
#define SHIM_RIG_LEVEL_SPECTRUM_ATT (1ULL << 47)
#define SHIM_RIG_LEVEL_TEMP_METER   (1ULL << 48)

/* ===== Function constants (bit positions for setting_t / uint64_t) ===== */
#define SHIM_RIG_FUNC_FAGC    (1ULL << 0)
#define SHIM_RIG_FUNC_NB      (1ULL << 1)
#define SHIM_RIG_FUNC_COMP    (1ULL << 2)
#define SHIM_RIG_FUNC_VOX     (1ULL << 3)
#define SHIM_RIG_FUNC_TONE    (1ULL << 4)
#define SHIM_RIG_FUNC_TSQL    (1ULL << 5)
#define SHIM_RIG_FUNC_SBKIN   (1ULL << 6)
#define SHIM_RIG_FUNC_FBKIN   (1ULL << 7)
#define SHIM_RIG_FUNC_ANF     (1ULL << 8)
#define SHIM_RIG_FUNC_NR      (1ULL << 9)
#define SHIM_RIG_FUNC_AIP     (1ULL << 10)
#define SHIM_RIG_FUNC_APF     (1ULL << 11)
#define SHIM_RIG_FUNC_MON     (1ULL << 12)
#define SHIM_RIG_FUNC_MN      (1ULL << 13)
#define SHIM_RIG_FUNC_RF      (1ULL << 14)
#define SHIM_RIG_FUNC_ARO     (1ULL << 15)
#define SHIM_RIG_FUNC_LOCK    (1ULL << 16)
#define SHIM_RIG_FUNC_MUTE    (1ULL << 17)
#define SHIM_RIG_FUNC_VSC     (1ULL << 18)
#define SHIM_RIG_FUNC_REV     (1ULL << 19)
#define SHIM_RIG_FUNC_SQL     (1ULL << 20)
#define SHIM_RIG_FUNC_ABM     (1ULL << 21)
#define SHIM_RIG_FUNC_BC      (1ULL << 22)
#define SHIM_RIG_FUNC_MBC     (1ULL << 23)
#define SHIM_RIG_FUNC_RIT     (1ULL << 24)
#define SHIM_RIG_FUNC_AFC     (1ULL << 25)
#define SHIM_RIG_FUNC_SATMODE (1ULL << 26)
#define SHIM_RIG_FUNC_SCOPE   (1ULL << 27)
#define SHIM_RIG_FUNC_RESUME  (1ULL << 28)
#define SHIM_RIG_FUNC_TBURST  (1ULL << 29)
#define SHIM_RIG_FUNC_TUNER   (1ULL << 30)
#define SHIM_RIG_FUNC_XIT     (1ULL << 31)
#define SHIM_RIG_FUNC_CSQL    (1ULL << 33)
#define SHIM_RIG_FUNC_TRANSCEIVE (1ULL << 42)
#define SHIM_RIG_FUNC_SPECTRUM (1ULL << 43)
#define SHIM_RIG_FUNC_SPECTRUM_HOLD (1ULL << 44)
#define SHIM_RIG_FUNC_SEND_MORSE (1ULL << 45)
#define SHIM_RIG_FUNC_SEND_VOICE_MEM (1ULL << 46)
#define SHIM_RIG_FUNC_OVF_STATUS (1ULL << 47)

/* ===== VFO operation constants ===== */
#define SHIM_RIG_OP_CPY       (1<<0)
#define SHIM_RIG_OP_XCHG      (1<<1)
#define SHIM_RIG_OP_FROM_VFO   (1<<2)
#define SHIM_RIG_OP_TO_VFO     (1<<3)
#define SHIM_RIG_OP_MCL        (1<<4)
#define SHIM_RIG_OP_UP         (1<<5)
#define SHIM_RIG_OP_DOWN       (1<<6)
#define SHIM_RIG_OP_BAND_UP    (1<<7)
#define SHIM_RIG_OP_BAND_DOWN  (1<<8)
#define SHIM_RIG_OP_LEFT       (1<<9)
#define SHIM_RIG_OP_RIGHT      (1<<10)
#define SHIM_RIG_OP_TUNE       (1<<11)
#define SHIM_RIG_OP_TOGGLE     (1<<12)

/* ===== Repeater shift constants ===== */
#define SHIM_RIG_RPT_SHIFT_MINUS 1
#define SHIM_RIG_RPT_SHIFT_PLUS  2

/* ===== Simplified structs for cross-compiler safety ===== */

/* Simplified channel data for set/get memory channel */
#define SHIM_HAMLIB_MAX_CHANNEL_LEVELS 64
#define SHIM_HAMLIB_MAX_CHANNEL_RANGES 256

typedef struct {
    int bank_num;
    int vfo;
    int ant;
    int freq;
    int mode;
    int width;
    int tx_freq;
    int tx_mode;
    int tx_width;
    int split;
    int tx_vfo;
    int rptr_shift;
    int rptr_offs;
    int tuning_step;
    int rit;
    int xit;
    uint64_t funcs;
    uint64_t levels;
    int ctcss_tone;
    int ctcss_sql;
    int dcs_code;
    int dcs_sql;
    int scan_group;
    int flags;
    int channel_desc;
    int tag;
} shim_channel_caps_t;

typedef struct {
    int start;
    int end;
    int type;
    shim_channel_caps_t caps;
} shim_memory_range_t;

typedef struct {
    int channel_num;
    int bank_num;
    int channel_type;
    int ant;
    double freq;
    double tx_freq;
    int mode;          /* rmode_t as int */
    int width;         /* pbwidth_t as int */
    int tx_mode;
    int tx_width;
    int split;         /* split_t as int */
    int tx_vfo;
    int rptr_shift;
    int rptr_offs;
    int tuning_step;
    int rit;
    int xit;
    uint64_t funcs;
    int level_count;
    uint64_t level_tokens[SHIM_HAMLIB_MAX_CHANNEL_LEVELS];
    double level_values[SHIM_HAMLIB_MAX_CHANNEL_LEVELS];
    int ctcss_tone;
    int ctcss_sql;
    int dcs_code;
    int dcs_sql;
    int scan_group;
    unsigned int flags;
    int vfo;
    char channel_desc[64];
    char tag[32];
} shim_channel_t;

/* Rig info for rig list enumeration */
typedef struct {
    unsigned int rig_model;
    const char* model_name;
    const char* mfg_name;
    const char* version;
    int status;
    int rig_type;
} shim_rig_info_t;

typedef struct {
    unsigned int rot_model;
    const char* model_name;
    const char* mfg_name;
    const char* version;
    int status;
    int rot_type;
} shim_rot_info_t;

#define SHIM_CONF_NAME_MAX 64
#define SHIM_CONF_LABEL_MAX 128
#define SHIM_CONF_TOOLTIP_MAX 256
#define SHIM_CONF_DEFAULT_MAX 128
#define SHIM_CONF_COMBO_MAX 16
#define SHIM_CONF_COMBO_VALUE_MAX 64
#define SHIM_PORT_TYPE_MAX 32
#define SHIM_PARITY_MAX 32
#define SHIM_HANDSHAKE_MAX 32

typedef struct {
    int token;
    char name[SHIM_CONF_NAME_MAX];
    char label[SHIM_CONF_LABEL_MAX];
    char tooltip[SHIM_CONF_TOOLTIP_MAX];
    char dflt[SHIM_CONF_DEFAULT_MAX];
    int type;
    double numeric_min;
    double numeric_max;
    double numeric_step;
    int combo_count;
    char combo_options[SHIM_CONF_COMBO_MAX][SHIM_CONF_COMBO_VALUE_MAX];
} shim_confparam_info_t;

typedef struct {
    char port_type[SHIM_PORT_TYPE_MAX];
    int serial_rate_min;
    int serial_rate_max;
    int serial_data_bits;
    int serial_stop_bits;
    char serial_parity[SHIM_PARITY_MAX];
    char serial_handshake[SHIM_HANDSHAKE_MAX];
    int write_delay;
    int post_write_delay;
    int timeout;
    int retry;
} shim_rig_port_caps_t;

typedef struct {
    int rot_type;
    double min_az;
    double max_az;
    double min_el;
    double max_el;
    uint64_t has_get_level;
    uint64_t has_set_level;
    uint64_t has_get_func;
    uint64_t has_set_func;
    uint64_t has_get_parm;
    uint64_t has_set_parm;
    int has_status;
} shim_rot_caps_t;

typedef struct {
    int id;
    char name[64];
} shim_spectrum_scope_t;

typedef struct {
    int id;
    char name[64];
} shim_spectrum_avg_mode_t;

typedef struct {
    int id;
    int data_level_min;
    int data_level_max;
    double signal_strength_min;
    double signal_strength_max;
    int spectrum_mode;
    double center_freq;
    double span_freq;
    double low_edge_freq;
    double high_edge_freq;
    int data_length;
    unsigned char data[2048];
} shim_spectrum_line_t;

/* ===== Callback types ===== */

/* Frequency change callback: (handle, vfo, freq, arg) -> int */
typedef int (*shim_freq_cb_t)(void* handle, int vfo, double freq, void* arg);

/* PTT change callback: (handle, vfo, ptt, arg) -> int */
typedef int (*shim_ptt_cb_t)(void* handle, int vfo, int ptt, void* arg);

/* Spectrum line callback: (handle, line, arg) -> int */
typedef int (*shim_spectrum_cb_t)(void* handle, const shim_spectrum_line_t* line, void* arg);

/* Rig list callback: (info, data) -> int */
typedef int (*shim_rig_list_cb_t)(const shim_rig_info_t* info, void* data);
/* Rotator list callback: (info, data) -> int */
typedef int (*shim_rot_list_cb_t)(const shim_rot_info_t* info, void* data);
/* Rig config callback: (info, data) -> int */
typedef int (*shim_rig_cfg_cb_t)(const shim_confparam_info_t* info, void* data);

/* ===== Lifecycle functions ===== */

SHIM_API hamlib_shim_handle_t shim_rig_init(unsigned int model);
SHIM_API int  shim_rig_open(hamlib_shim_handle_t h);
SHIM_API int  shim_rig_close(hamlib_shim_handle_t h);
SHIM_API int  shim_rig_cleanup(hamlib_shim_handle_t h);
SHIM_API const char* shim_rigerror(int errcode);

/* ===== Debug / Utility ===== */

SHIM_API void shim_rig_set_debug(int level);
SHIM_API int  shim_rig_get_debug(void);
SHIM_API const char* shim_rig_get_version(void);
SHIM_API int  shim_rig_load_all_backends(void);
SHIM_API int  shim_rig_list_foreach(shim_rig_list_cb_t cb, void* data);
SHIM_API int  shim_rig_cfgparams_foreach(hamlib_shim_handle_t h, shim_rig_cfg_cb_t cb, void* data);
SHIM_API int  shim_rig_get_port_caps(hamlib_shim_handle_t h, shim_rig_port_caps_t* out_caps);
SHIM_API const char* shim_rig_strstatus(int status);
SHIM_API hamlib_shim_handle_t shim_rot_init(unsigned int model);
SHIM_API int  shim_rot_open(hamlib_shim_handle_t h);
SHIM_API int  shim_rot_close(hamlib_shim_handle_t h);
SHIM_API int  shim_rot_cleanup(hamlib_shim_handle_t h);
SHIM_API int  shim_rot_load_all_backends(void);
SHIM_API int  shim_rot_list_foreach(shim_rot_list_cb_t cb, void* data);
SHIM_API int  shim_rot_cfgparams_foreach(hamlib_shim_handle_t h, shim_rig_cfg_cb_t cb, void* data);
SHIM_API int  shim_rot_get_port_caps(hamlib_shim_handle_t h, shim_rig_port_caps_t* out_caps);
SHIM_API int  shim_rot_get_caps(hamlib_shim_handle_t h, shim_rot_caps_t* out_caps);
SHIM_API const char* shim_rot_strstatus(int status);
SHIM_API const char* shim_rot_type_str(int rot_type);

/* ===== Port configuration (before open) ===== */

SHIM_API void shim_rig_set_port_path(hamlib_shim_handle_t h, const char* path);
SHIM_API void shim_rig_set_port_type(hamlib_shim_handle_t h, int type);
SHIM_API void shim_rot_set_port_path(hamlib_shim_handle_t h, const char* path);
SHIM_API void shim_rot_set_port_type(hamlib_shim_handle_t h, int type);

/* Serial port configuration */
SHIM_API void shim_rig_set_serial_rate(hamlib_shim_handle_t h, int rate);
SHIM_API int  shim_rig_get_serial_rate(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_data_bits(hamlib_shim_handle_t h, int bits);
SHIM_API int  shim_rig_get_serial_data_bits(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_stop_bits(hamlib_shim_handle_t h, int bits);
SHIM_API int  shim_rig_get_serial_stop_bits(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_parity(hamlib_shim_handle_t h, int parity);
SHIM_API int  shim_rig_get_serial_parity(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_handshake(hamlib_shim_handle_t h, int handshake);
SHIM_API int  shim_rig_get_serial_handshake(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_rts_state(hamlib_shim_handle_t h, int state);
SHIM_API int  shim_rig_get_serial_rts_state(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_serial_dtr_state(hamlib_shim_handle_t h, int state);
SHIM_API int  shim_rig_get_serial_dtr_state(hamlib_shim_handle_t h);

/* Port timing configuration */
SHIM_API void shim_rig_set_port_timeout(hamlib_shim_handle_t h, int ms);
SHIM_API int  shim_rig_get_port_timeout(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_port_retry(hamlib_shim_handle_t h, int count);
SHIM_API int  shim_rig_get_port_retry(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_port_write_delay(hamlib_shim_handle_t h, int ms);
SHIM_API int  shim_rig_get_port_write_delay(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_port_post_write_delay(hamlib_shim_handle_t h, int ms);
SHIM_API int  shim_rig_get_port_post_write_delay(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_port_flushx(hamlib_shim_handle_t h, int enable);
SHIM_API int  shim_rig_get_port_flushx(hamlib_shim_handle_t h);

/* PTT/DCD port type configuration */
SHIM_API void shim_rig_set_ptt_type(hamlib_shim_handle_t h, int type);
SHIM_API int  shim_rig_get_ptt_type(hamlib_shim_handle_t h);

SHIM_API void shim_rig_set_dcd_type(hamlib_shim_handle_t h, int type);
SHIM_API int  shim_rig_get_dcd_type(hamlib_shim_handle_t h);

/* ===== Frequency control ===== */

SHIM_API int shim_rig_set_freq(hamlib_shim_handle_t h, int vfo, double freq);
SHIM_API int shim_rig_get_freq(hamlib_shim_handle_t h, int vfo, double* freq);

/* ===== VFO control ===== */

SHIM_API int shim_rig_set_vfo(hamlib_shim_handle_t h, int vfo);
SHIM_API int shim_rig_get_vfo(hamlib_shim_handle_t h, int* vfo);
SHIM_API int shim_rig_parse_vfo(const char* vfo_str);
SHIM_API const char* shim_rig_strvfo(int vfo);

/* ===== Mode control ===== */

SHIM_API int shim_rig_set_mode(hamlib_shim_handle_t h, int vfo, int mode, int width);
SHIM_API int shim_rig_get_mode(hamlib_shim_handle_t h, int vfo, int* mode, int* width);
SHIM_API int shim_rig_parse_mode(const char* mode_str);
SHIM_API const char* shim_rig_strrmode(int mode);
SHIM_API int shim_rig_passband_narrow(hamlib_shim_handle_t h, int mode);
SHIM_API int shim_rig_passband_wide(hamlib_shim_handle_t h, int mode);

/* ===== PTT control ===== */

SHIM_API int shim_rig_set_ptt(hamlib_shim_handle_t h, int vfo, int ptt);
SHIM_API int shim_rig_get_ptt(hamlib_shim_handle_t h, int vfo, int* ptt);
SHIM_API int shim_rig_get_dcd(hamlib_shim_handle_t h, int vfo, int* dcd);

/* ===== Signal strength ===== */

SHIM_API int shim_rig_get_strength(hamlib_shim_handle_t h, int vfo, int* strength);

/* ===== Level control ===== */

SHIM_API int shim_rig_set_level_f(hamlib_shim_handle_t h, int vfo, uint64_t level, float value);
SHIM_API int shim_rig_set_level_i(hamlib_shim_handle_t h, int vfo, uint64_t level, int value);
SHIM_API int shim_rig_get_level_f(hamlib_shim_handle_t h, int vfo, uint64_t level, float* value);
SHIM_API int shim_rig_get_level_i(hamlib_shim_handle_t h, int vfo, uint64_t level, int* value);
SHIM_API int shim_rig_level_is_float(uint64_t level);
/* Auto-detect int/float level type, returns value as double */
SHIM_API int shim_rig_get_level_auto(hamlib_shim_handle_t h, int vfo, uint64_t level, double* value);

/* ===== Function control ===== */

SHIM_API int shim_rig_set_func(hamlib_shim_handle_t h, int vfo, uint64_t func, int enable);
SHIM_API int shim_rig_get_func(hamlib_shim_handle_t h, int vfo, uint64_t func, int* state);

/* ===== Parameter control ===== */

SHIM_API int shim_rig_set_parm_f(hamlib_shim_handle_t h, uint64_t parm, float value);
SHIM_API int shim_rig_set_parm_i(hamlib_shim_handle_t h, uint64_t parm, int value);
SHIM_API int shim_rig_get_parm_f(hamlib_shim_handle_t h, uint64_t parm, float* value);
SHIM_API int shim_rig_get_parm_i(hamlib_shim_handle_t h, uint64_t parm, int* value);
SHIM_API uint64_t shim_rig_parse_parm(const char* parm_str);

/* ===== Split operations ===== */

SHIM_API int shim_rig_set_split_freq(hamlib_shim_handle_t h, int vfo, double freq);
SHIM_API int shim_rig_get_split_freq(hamlib_shim_handle_t h, int vfo, double* freq);

SHIM_API int shim_rig_set_split_vfo(hamlib_shim_handle_t h, int rx_vfo, int split, int tx_vfo);
SHIM_API int shim_rig_get_split_vfo(hamlib_shim_handle_t h, int vfo, int* split, int* tx_vfo);

SHIM_API int shim_rig_set_split_mode(hamlib_shim_handle_t h, int vfo, int mode, int width);
SHIM_API int shim_rig_get_split_mode(hamlib_shim_handle_t h, int vfo, int* mode, int* width);

SHIM_API int shim_rig_set_split_freq_mode(hamlib_shim_handle_t h, int vfo, double freq, int mode, int width);
SHIM_API int shim_rig_get_split_freq_mode(hamlib_shim_handle_t h, int vfo, double* freq, int* mode, int* width);

/* ===== RIT/XIT control ===== */

SHIM_API int shim_rig_set_rit(hamlib_shim_handle_t h, int vfo, int offset);
SHIM_API int shim_rig_get_rit(hamlib_shim_handle_t h, int vfo, int* offset);
SHIM_API int shim_rig_set_xit(hamlib_shim_handle_t h, int vfo, int offset);
SHIM_API int shim_rig_get_xit(hamlib_shim_handle_t h, int vfo, int* offset);

/* ===== Memory channel operations ===== */

SHIM_API int shim_rig_set_channel(hamlib_shim_handle_t h, int vfo, const shim_channel_t* chan);
SHIM_API int shim_rig_get_channel(hamlib_shim_handle_t h, int vfo, shim_channel_t* chan, int read_only);
SHIM_API int shim_rig_get_memory_range_count(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_memory_range(hamlib_shim_handle_t h, int index, shim_memory_range_t* out);
SHIM_API int shim_rig_lookup_memory_caps(hamlib_shim_handle_t h, int channel_num, shim_memory_range_t* out);
SHIM_API int shim_rig_get_chan_all(hamlib_shim_handle_t h, int vfo, shim_channel_t* chans, int max_count);
SHIM_API int shim_rig_set_chan_all(hamlib_shim_handle_t h, int vfo, const shim_channel_t* chans, int count);
SHIM_API int shim_rig_set_mem(hamlib_shim_handle_t h, int vfo, int ch);
SHIM_API int shim_rig_get_mem(hamlib_shim_handle_t h, int vfo, int* ch);
SHIM_API int shim_rig_set_bank(hamlib_shim_handle_t h, int vfo, int bank);
SHIM_API int shim_rig_mem_count(hamlib_shim_handle_t h);

/* ===== Scanning ===== */

SHIM_API int shim_rig_scan(hamlib_shim_handle_t h, int vfo, int scan_type, int channel);

/* ===== VFO operations ===== */

SHIM_API int shim_rig_vfo_op(hamlib_shim_handle_t h, int vfo, int op);

/* ===== Antenna control ===== */

SHIM_API int shim_rig_set_ant(hamlib_shim_handle_t h, int vfo, int ant, float option);
SHIM_API int shim_rig_get_ant(hamlib_shim_handle_t h, int vfo, int ant, float* option,
                               int* ant_curr, int* ant_tx, int* ant_rx);

/* ===== Tuning step ===== */

SHIM_API int shim_rig_set_ts(hamlib_shim_handle_t h, int vfo, int ts);
SHIM_API int shim_rig_get_ts(hamlib_shim_handle_t h, int vfo, int* ts);

/* ===== Repeater control ===== */

SHIM_API int shim_rig_set_rptr_shift(hamlib_shim_handle_t h, int vfo, int shift);
SHIM_API int shim_rig_get_rptr_shift(hamlib_shim_handle_t h, int vfo, int* shift);
SHIM_API const char* shim_rig_strptrshift(int shift);
SHIM_API int shim_rig_set_rptr_offs(hamlib_shim_handle_t h, int vfo, int offset);
SHIM_API int shim_rig_get_rptr_offs(hamlib_shim_handle_t h, int vfo, int* offset);

/* ===== CTCSS/DCS tone control ===== */

SHIM_API int shim_rig_set_ctcss_tone(hamlib_shim_handle_t h, int vfo, unsigned int tone);
SHIM_API int shim_rig_get_ctcss_tone(hamlib_shim_handle_t h, int vfo, unsigned int* tone);
SHIM_API int shim_rig_set_dcs_code(hamlib_shim_handle_t h, int vfo, unsigned int code);
SHIM_API int shim_rig_get_dcs_code(hamlib_shim_handle_t h, int vfo, unsigned int* code);
SHIM_API int shim_rig_set_ctcss_sql(hamlib_shim_handle_t h, int vfo, unsigned int tone);
SHIM_API int shim_rig_get_ctcss_sql(hamlib_shim_handle_t h, int vfo, unsigned int* tone);
SHIM_API int shim_rig_set_dcs_sql(hamlib_shim_handle_t h, int vfo, unsigned int code);
SHIM_API int shim_rig_get_dcs_sql(hamlib_shim_handle_t h, int vfo, unsigned int* code);

/* ===== DTMF ===== */

SHIM_API int shim_rig_send_dtmf(hamlib_shim_handle_t h, int vfo, const char* digits);
SHIM_API int shim_rig_recv_dtmf(hamlib_shim_handle_t h, int vfo, char* digits, int* length);

/* ===== Morse code ===== */

SHIM_API int shim_rig_send_morse(hamlib_shim_handle_t h, int vfo, const char* msg);
SHIM_API int shim_rig_stop_morse(hamlib_shim_handle_t h, int vfo);
SHIM_API int shim_rig_wait_morse(hamlib_shim_handle_t h, int vfo);

/* ===== Voice memory ===== */

SHIM_API int shim_rig_send_voice_mem(hamlib_shim_handle_t h, int vfo, int ch);
SHIM_API int shim_rig_stop_voice_mem(hamlib_shim_handle_t h, int vfo);

/* ===== Power control ===== */

SHIM_API int shim_rig_set_powerstat(hamlib_shim_handle_t h, int status);
SHIM_API int shim_rig_get_powerstat(hamlib_shim_handle_t h, int* status);

/* ===== Power conversion ===== */

SHIM_API int shim_rig_power2mW(hamlib_shim_handle_t h, unsigned int* mwpower, float power, double freq, int mode);
SHIM_API int shim_rig_mW2power(hamlib_shim_handle_t h, float* power, unsigned int mwpower, double freq, int mode);

/* ===== Reset ===== */

SHIM_API int shim_rig_reset(hamlib_shim_handle_t h, int reset_type);
SHIM_API int shim_rot_set_position(hamlib_shim_handle_t h, double azimuth, double elevation);
SHIM_API int shim_rot_get_position(hamlib_shim_handle_t h, double* azimuth, double* elevation);
SHIM_API int shim_rot_stop(hamlib_shim_handle_t h);
SHIM_API int shim_rot_park(hamlib_shim_handle_t h);
SHIM_API int shim_rot_reset(hamlib_shim_handle_t h, int reset_type);
SHIM_API int shim_rot_move(hamlib_shim_handle_t h, int direction, int speed);
SHIM_API const char* shim_rot_get_info(hamlib_shim_handle_t h);
SHIM_API int shim_rot_get_status(hamlib_shim_handle_t h, int* status);
SHIM_API int shim_rot_set_conf(hamlib_shim_handle_t h, const char* name, const char* val);
SHIM_API int shim_rot_get_conf(hamlib_shim_handle_t h, const char* name, char* buf, int buflen);
SHIM_API uint64_t shim_rot_get_caps_has_get_level(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rot_get_caps_has_set_level(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rot_get_caps_has_get_func(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rot_get_caps_has_set_func(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rot_get_caps_has_get_parm(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rot_get_caps_has_set_parm(hamlib_shim_handle_t h);
SHIM_API int shim_rot_set_level_f(hamlib_shim_handle_t h, uint64_t level, float value);
SHIM_API int shim_rot_set_level_i(hamlib_shim_handle_t h, uint64_t level, int value);
SHIM_API int shim_rot_get_level_f(hamlib_shim_handle_t h, uint64_t level, float* value);
SHIM_API int shim_rot_get_level_i(hamlib_shim_handle_t h, uint64_t level, int* value);
SHIM_API int shim_rot_get_level_auto(hamlib_shim_handle_t h, uint64_t level, double* value);
SHIM_API int shim_rot_set_func(hamlib_shim_handle_t h, uint64_t func, int enable);
SHIM_API int shim_rot_get_func(hamlib_shim_handle_t h, uint64_t func, int* state);
SHIM_API int shim_rot_set_parm_f(hamlib_shim_handle_t h, uint64_t parm, float value);
SHIM_API int shim_rot_set_parm_i(hamlib_shim_handle_t h, uint64_t parm, int value);
SHIM_API int shim_rot_get_parm_f(hamlib_shim_handle_t h, uint64_t parm, float* value);
SHIM_API int shim_rot_get_parm_i(hamlib_shim_handle_t h, uint64_t parm, int* value);
SHIM_API uint64_t shim_rot_parse_level(const char* level_str);
SHIM_API uint64_t shim_rot_parse_func(const char* func_str);
SHIM_API uint64_t shim_rot_parse_parm(const char* parm_str);
SHIM_API const char* shim_rot_strlevel(uint64_t level);
SHIM_API const char* shim_rot_strfunc(uint64_t func);
SHIM_API const char* shim_rot_strparm(uint64_t parm);

/* ===== Callbacks ===== */

SHIM_API int shim_rig_set_freq_callback(hamlib_shim_handle_t h, shim_freq_cb_t cb, void* arg);
SHIM_API int shim_rig_set_ptt_callback(hamlib_shim_handle_t h, shim_ptt_cb_t cb, void* arg);
SHIM_API int shim_rig_set_spectrum_callback(hamlib_shim_handle_t h, shim_spectrum_cb_t cb, void* arg);
SHIM_API int shim_rig_set_trn(hamlib_shim_handle_t h, int trn);

/* ===== Capability queries (replaces direct caps-> access) ===== */

SHIM_API uint64_t shim_rig_get_mode_list(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rig_get_caps_has_get_level(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rig_get_caps_has_set_level(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rig_get_caps_has_get_func(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rig_get_caps_has_set_func(hamlib_shim_handle_t h);
SHIM_API int shim_rig_is_async_data_supported(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_spectrum_scope_count(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_spectrum_scope(hamlib_shim_handle_t h, int index, shim_spectrum_scope_t* out);
SHIM_API int shim_rig_get_caps_spectrum_mode_count(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_spectrum_mode(hamlib_shim_handle_t h, int index, int* out);
SHIM_API int shim_rig_get_caps_spectrum_span_count(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_spectrum_span(hamlib_shim_handle_t h, int index, double* out);
SHIM_API int shim_rig_get_caps_spectrum_avg_mode_count(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_spectrum_avg_mode(hamlib_shim_handle_t h, int index, shim_spectrum_avg_mode_t* out);
SHIM_API const char* shim_rig_str_spectrum_mode(int mode);

/* Format mode bitmask to string list */
SHIM_API int shim_rig_sprintf_mode(uint64_t modes, char* buf, int buflen);

/* Rig type to string conversion */
SHIM_API const char* shim_rig_type_str(int rig_type);

/* ===== Capability Query: Structured data types ===== */

/* ABI-safe struct for frequency range (no Hamlib types cross boundary) */
typedef struct {
    double start_freq;   /* Hz */
    double end_freq;     /* Hz */
    uint64_t modes;      /* rmode_t bitmask */
    int low_power;       /* mW */
    int high_power;      /* mW */
    int vfo;             /* vfo_t */
    int ant;             /* ant_t */
} shim_freq_range_t;

/* ABI-safe struct for tuning step / filter */
typedef struct {
    uint64_t modes;      /* rmode_t bitmask */
    int value;           /* step Hz or filter width Hz */
} shim_mode_value_t;

typedef struct {
    double min_value;
    double max_value;
    double step_value;
    int is_float;
    int is_defined;
} shim_granularity_t;

/* Group A: Simple value queries */
SHIM_API int shim_rig_get_caps_preamp(hamlib_shim_handle_t h, int* out, int max_count);
SHIM_API int shim_rig_get_caps_attenuator(hamlib_shim_handle_t h, int* out, int max_count);
SHIM_API int shim_rig_get_caps_agc_levels(hamlib_shim_handle_t h, int* out, int max_count);
SHIM_API long shim_rig_get_caps_max_rit(hamlib_shim_handle_t h);
SHIM_API long shim_rig_get_caps_max_xit(hamlib_shim_handle_t h);
SHIM_API long shim_rig_get_caps_max_ifshift(hamlib_shim_handle_t h);

/* Group B: Tone/code lists */
SHIM_API int shim_rig_get_caps_ctcss_list(hamlib_shim_handle_t h, unsigned int* out, int max_count);
SHIM_API int shim_rig_get_caps_dcs_list(hamlib_shim_handle_t h, unsigned int* out, int max_count);

/* Group C: Structured data */
SHIM_API int shim_rig_get_caps_rx_range(hamlib_shim_handle_t h, shim_freq_range_t* out, int max_count);
SHIM_API int shim_rig_get_caps_tx_range(hamlib_shim_handle_t h, shim_freq_range_t* out, int max_count);
SHIM_API int shim_rig_get_caps_tuning_steps(hamlib_shim_handle_t h, shim_mode_value_t* out, int max_count);
SHIM_API int shim_rig_get_caps_filters(hamlib_shim_handle_t h, shim_mode_value_t* out, int max_count);
SHIM_API int shim_rig_get_level_granularity(hamlib_shim_handle_t h, uint64_t level, shim_granularity_t* out);
SHIM_API int shim_rig_get_rfpower_metadata(hamlib_shim_handle_t h, int* out_current, int* out_min, int* out_max);

/* ===== Lock Mode (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_set_lock_mode(hamlib_shim_handle_t h, int lock);
SHIM_API int shim_rig_get_lock_mode(hamlib_shim_handle_t h, int *lock);

/* ===== Clock (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_set_clock(hamlib_shim_handle_t h, int year, int month, int day,
                                 int hour, int min, int sec, double msec, int utc_offset);
SHIM_API int shim_rig_get_clock(hamlib_shim_handle_t h, int *year, int *month, int *day,
                                 int *hour, int *min, int *sec, double *msec, int *utc_offset);

/* ===== VFO Info (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_get_vfo_info(hamlib_shim_handle_t h, int vfo,
                                    double *freq, uint64_t *mode,
                                    long *width, int *split, int *satmode);

/* ===== Rig info / Raw / Conf ===== */

SHIM_API const char* shim_rig_get_info(hamlib_shim_handle_t h);
SHIM_API int shim_rig_send_raw(hamlib_shim_handle_t h,
    const unsigned char* send, int send_len,
    unsigned char* reply, int reply_len, const unsigned char* term);
SHIM_API int shim_rig_set_conf(hamlib_shim_handle_t h, const char* name, const char* val);
SHIM_API int shim_rig_get_conf(hamlib_shim_handle_t h, const char* name, char* buf, int buflen);

/* ===== Passband / Resolution ===== */

SHIM_API int shim_rig_passband_normal(hamlib_shim_handle_t h, int mode);
SHIM_API int shim_rig_get_resolution(hamlib_shim_handle_t h, int mode);

/* ===== Capability queries (parm / vfo_ops / scan) ===== */

SHIM_API uint64_t shim_rig_get_caps_has_get_parm(hamlib_shim_handle_t h);
SHIM_API uint64_t shim_rig_get_caps_has_set_parm(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_vfo_ops(hamlib_shim_handle_t h);
SHIM_API int shim_rig_get_caps_has_scan(hamlib_shim_handle_t h);

/* ===== Static info ===== */

SHIM_API const char* shim_rig_copyright(void);
SHIM_API const char* shim_rig_license(void);

#ifdef __cplusplus
}
#endif

#endif /* HAMLIB_SHIM_H */
