/**
 * hamlib_shim.c - Pure C implementation of the Hamlib shim layer
 *
 * This file wraps all Hamlib C API calls and struct accesses, providing
 * a clean C ABI boundary. It must be compiled with the same compiler
 * as Hamlib (MinGW on Windows, gcc/clang on Unix).
 */

#ifndef HAMLIB_SHIM_BUILD
#define HAMLIB_SHIM_BUILD
#endif
#include "hamlib_shim.h"
#include <hamlib/rig.h>
#include <hamlib/rig_state.h>
#include <hamlib/rotator.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int shim_setting_to_index(setting_t setting) {
    int index = 0;

    if (setting == 0 || (setting & (setting - 1)) != 0) {
        return -1;
    }

    while ((setting >>= 1) != 0) {
        index++;
    }

    return index;
}

/* ===== Lifecycle ===== */

SHIM_API hamlib_shim_handle_t shim_rig_init(unsigned int model) {
    return (hamlib_shim_handle_t)rig_init((rig_model_t)model);
}

SHIM_API int shim_rig_open(hamlib_shim_handle_t h) {
    return rig_open((RIG*)h);
}

SHIM_API int shim_rig_close(hamlib_shim_handle_t h) {
    return rig_close((RIG*)h);
}

SHIM_API int shim_rig_cleanup(hamlib_shim_handle_t h) {
    return rig_cleanup((RIG*)h);
}

SHIM_API hamlib_shim_handle_t shim_rot_init(unsigned int model) {
    return (hamlib_shim_handle_t)rot_init((rot_model_t)model);
}

SHIM_API int shim_rot_open(hamlib_shim_handle_t h) {
    return rot_open((ROT*)h);
}

SHIM_API int shim_rot_close(hamlib_shim_handle_t h) {
    return rot_close((ROT*)h);
}

SHIM_API int shim_rot_cleanup(hamlib_shim_handle_t h) {
    return rot_cleanup((ROT*)h);
}

SHIM_API const char* shim_rigerror(int errcode) {
    return rigerror(errcode);
}

/* ===== Debug / Utility ===== */

SHIM_API void shim_rig_set_debug(int level) {
    rig_set_debug((enum rig_debug_level_e)level);
}

SHIM_API int shim_rig_get_debug(void) {
#ifdef SHIM_HAS_GET_DEBUG
    enum rig_debug_level_e level;
    rig_get_debug(&level);
    return (int)level;
#else
    return -1;  /* Not available */
#endif
}

SHIM_API const char* shim_rig_get_version(void) {
    return hamlib_version;
}

SHIM_API int shim_rig_load_all_backends(void) {
    return rig_load_all_backends();
}

SHIM_API int shim_rot_load_all_backends(void) {
    return rot_load_all_backends();
}

/* Internal callback adapter for rig_list_foreach */
struct shim_list_adapter {
    shim_rig_list_cb_t user_cb;
    void* user_data;
};

struct shim_rot_list_adapter {
    shim_rot_list_cb_t user_cb;
    void* user_data;
};

struct shim_cfg_adapter {
    RIG* rig;
    shim_rig_cfg_cb_t user_cb;
    void* user_data;
};

struct shim_rot_cfg_adapter {
    ROT* rot;
    shim_rig_cfg_cb_t user_cb;
    void* user_data;
};

static void shim_copy_string(char* dest, size_t dest_size, const char* src) {
    if (!dest || dest_size == 0) return;
    if (!src) src = "";
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

static int shim_list_foreach_adapter(const struct rig_caps *caps, void *data) {
    struct shim_list_adapter *adapter = (struct shim_list_adapter*)data;
    shim_rig_info_t info;
    info.rig_model = caps->rig_model;
    info.model_name = caps->model_name ? caps->model_name : "";
    info.mfg_name = caps->mfg_name ? caps->mfg_name : "";
    info.version = caps->version ? caps->version : "";
    info.status = (int)caps->status;
    info.rig_type = (int)(caps->rig_type & RIG_TYPE_MASK);
    return adapter->user_cb(&info, adapter->user_data);
}

SHIM_API int shim_rig_list_foreach(shim_rig_list_cb_t cb, void* data) {
    struct shim_list_adapter adapter;
    adapter.user_cb = cb;
    adapter.user_data = data;
    return rig_list_foreach(shim_list_foreach_adapter, &adapter);
}

static int shim_rot_list_foreach_adapter(const struct rot_caps *caps, void *data) {
    struct shim_rot_list_adapter *adapter = (struct shim_rot_list_adapter*)data;
    shim_rot_info_t info;
    info.rot_model = caps->rot_model;
    info.model_name = caps->model_name ? caps->model_name : "";
    info.mfg_name = caps->mfg_name ? caps->mfg_name : "";
    info.version = caps->version ? caps->version : "";
    info.status = (int)caps->status;
    info.rot_type = caps->rot_type;
    return adapter->user_cb(&info, adapter->user_data);
}

SHIM_API int shim_rot_list_foreach(shim_rot_list_cb_t cb, void* data) {
    struct shim_rot_list_adapter adapter;
    adapter.user_cb = cb;
    adapter.user_data = data;
    return rot_list_foreach(shim_rot_list_foreach_adapter, &adapter);
}

static int shim_cfg_foreach_adapter(const struct confparams* param, rig_ptr_t data) {
    struct shim_cfg_adapter* adapter = (struct shim_cfg_adapter*)data;
    const struct confparams* lookup;
    shim_confparam_info_t info;
    int i;

    if (!adapter || !adapter->user_cb || !param) {
        return 0;
    }

    if (!adapter->rig || !param->name) {
        return 1;
    }

    lookup = rig_confparam_lookup(adapter->rig, param->name);
    if (!lookup || lookup->token != param->token) {
        return 1;
    }

    memset(&info, 0, sizeof(info));
    info.token = (int)param->token;
    info.type = (int)param->type;

    shim_copy_string(info.name, sizeof(info.name), param->name);
    shim_copy_string(info.label, sizeof(info.label), param->label);
    shim_copy_string(info.tooltip, sizeof(info.tooltip), param->tooltip);
    shim_copy_string(info.dflt, sizeof(info.dflt), param->dflt);

    if (param->type == RIG_CONF_NUMERIC || param->type == RIG_CONF_INT) {
        info.numeric_min = param->u.n.min;
        info.numeric_max = param->u.n.max;
        info.numeric_step = param->u.n.step;
    } else if (param->type == RIG_CONF_COMBO) {
        for (i = 0; i < RIG_COMBO_MAX && i < SHIM_CONF_COMBO_MAX; ++i) {
            if (!param->u.c.combostr[i] || !*param->u.c.combostr[i]) {
                break;
            }

            shim_copy_string(
                info.combo_options[i],
                sizeof(info.combo_options[i]),
                param->u.c.combostr[i]
            );
            info.combo_count++;
        }
    }

    return adapter->user_cb(&info, adapter->user_data);
}

SHIM_API int shim_rig_cfgparams_foreach(hamlib_shim_handle_t h, shim_rig_cfg_cb_t cb, void* data) {
    RIG* rig = (RIG*)h;
    struct shim_cfg_adapter adapter;

    if (!rig || !cb) {
        return -RIG_EINVAL;
    }

    adapter.rig = rig;
    adapter.user_cb = cb;
    adapter.user_data = data;

    return rig_token_foreach(rig, shim_cfg_foreach_adapter, &adapter);
}

static int shim_rot_cfg_foreach_adapter(const struct confparams* param, rig_ptr_t data) {
    struct shim_rot_cfg_adapter* adapter = (struct shim_rot_cfg_adapter*)data;
    const struct confparams* lookup;
    shim_confparam_info_t info;
    int i;

    if (!adapter || !adapter->user_cb || !param) {
        return 0;
    }

    if (!adapter->rot || !param->name) {
        return 1;
    }

    lookup = rot_confparam_lookup(adapter->rot, param->name);
    if (!lookup || lookup->token != param->token) {
        return 1;
    }

    memset(&info, 0, sizeof(info));
    info.token = (int)param->token;
    info.type = (int)param->type;

    shim_copy_string(info.name, sizeof(info.name), param->name);
    shim_copy_string(info.label, sizeof(info.label), param->label);
    shim_copy_string(info.tooltip, sizeof(info.tooltip), param->tooltip);
    shim_copy_string(info.dflt, sizeof(info.dflt), param->dflt);

    if (param->type == RIG_CONF_NUMERIC || param->type == RIG_CONF_INT) {
        info.numeric_min = param->u.n.min;
        info.numeric_max = param->u.n.max;
        info.numeric_step = param->u.n.step;
    } else if (param->type == RIG_CONF_COMBO) {
        for (i = 0; i < RIG_COMBO_MAX && i < SHIM_CONF_COMBO_MAX; ++i) {
            if (!param->u.c.combostr[i] || !*param->u.c.combostr[i]) {
                break;
            }

            shim_copy_string(
                info.combo_options[i],
                sizeof(info.combo_options[i]),
                param->u.c.combostr[i]
            );
            info.combo_count++;
        }
    }

    return adapter->user_cb(&info, adapter->user_data);
}

SHIM_API int shim_rot_cfgparams_foreach(hamlib_shim_handle_t h, shim_rig_cfg_cb_t cb, void* data) {
    ROT* rot = (ROT*)h;
    struct shim_rot_cfg_adapter adapter;

    if (!rot || !cb) {
        return -RIG_EINVAL;
    }

    adapter.rot = rot;
    adapter.user_cb = cb;
    adapter.user_data = data;

    return rot_token_foreach(rot, shim_rot_cfg_foreach_adapter, &adapter);
}

static const char* shim_port_type_name(enum rig_port_e port_type) {
    switch (port_type) {
        case RIG_PORT_NONE: return "none";
        case RIG_PORT_SERIAL: return "serial";
        case RIG_PORT_NETWORK: return "network";
        case RIG_PORT_DEVICE: return "device";
        case RIG_PORT_PACKET: return "packet";
        case RIG_PORT_DTMF: return "dtmf";
        case RIG_PORT_ULTRA: return "ultra";
        case RIG_PORT_RPC: return "rpc";
        case RIG_PORT_PARALLEL: return "parallel";
        case RIG_PORT_USB: return "usb";
        case RIG_PORT_UDP_NETWORK: return "udp-network";
        case RIG_PORT_CM108: return "cm108";
        case RIG_PORT_GPIO: return "gpio";
        case RIG_PORT_GPION: return "gpion";
        default: return "other";
    }
}

static const char* shim_serial_parity_name(enum serial_parity_e parity) {
    switch (parity) {
        case RIG_PARITY_NONE: return "None";
        case RIG_PARITY_ODD: return "Odd";
        case RIG_PARITY_EVEN: return "Even";
        case RIG_PARITY_MARK: return "Mark";
        case RIG_PARITY_SPACE: return "Space";
        default: return "Unknown";
    }
}

static const char* shim_serial_handshake_name(enum serial_handshake_e handshake) {
    switch (handshake) {
        case RIG_HANDSHAKE_NONE: return "None";
        case RIG_HANDSHAKE_XONXOFF: return "XONXOFF";
        case RIG_HANDSHAKE_HARDWARE: return "Hardware";
        default: return "Unknown";
    }
}

SHIM_API int shim_rig_get_port_caps(hamlib_shim_handle_t h, shim_rig_port_caps_t* out_caps) {
    RIG* rig = (RIG*)h;
    const struct rig_caps* caps;

    if (!rig || !out_caps || !rig->caps) {
        return -RIG_EINVAL;
    }

    caps = rig->caps;
    memset(out_caps, 0, sizeof(*out_caps));

    shim_copy_string(out_caps->port_type, sizeof(out_caps->port_type), shim_port_type_name(caps->port_type));
    out_caps->serial_rate_min = caps->serial_rate_min;
    out_caps->serial_rate_max = caps->serial_rate_max;
    out_caps->serial_data_bits = caps->serial_data_bits;
    out_caps->serial_stop_bits = caps->serial_stop_bits;
    shim_copy_string(out_caps->serial_parity, sizeof(out_caps->serial_parity), shim_serial_parity_name(caps->serial_parity));
    shim_copy_string(out_caps->serial_handshake, sizeof(out_caps->serial_handshake), shim_serial_handshake_name(caps->serial_handshake));
    out_caps->write_delay = caps->write_delay;
    out_caps->post_write_delay = caps->post_write_delay;
    out_caps->timeout = caps->timeout;
    out_caps->retry = caps->retry;

    return RIG_OK;
}

SHIM_API const char* shim_rig_strstatus(int status) {
    return rig_strstatus((enum rig_status_e)status);
}

SHIM_API int shim_rot_get_port_caps(hamlib_shim_handle_t h, shim_rig_port_caps_t* out_caps) {
    ROT* rot = (ROT*)h;
    const struct rot_caps* caps;

    if (!rot || !out_caps || !rot->caps) {
        return -RIG_EINVAL;
    }

    caps = rot->caps;
    memset(out_caps, 0, sizeof(*out_caps));

    shim_copy_string(out_caps->port_type, sizeof(out_caps->port_type), shim_port_type_name(caps->port_type));
    out_caps->serial_rate_min = caps->serial_rate_min;
    out_caps->serial_rate_max = caps->serial_rate_max;
    out_caps->serial_data_bits = caps->serial_data_bits;
    out_caps->serial_stop_bits = caps->serial_stop_bits;
    shim_copy_string(out_caps->serial_parity, sizeof(out_caps->serial_parity), shim_serial_parity_name(caps->serial_parity));
    shim_copy_string(out_caps->serial_handshake, sizeof(out_caps->serial_handshake), shim_serial_handshake_name(caps->serial_handshake));
    out_caps->write_delay = caps->write_delay;
    out_caps->post_write_delay = caps->post_write_delay;
    out_caps->timeout = caps->timeout;
    out_caps->retry = caps->retry;

    return RIG_OK;
}

SHIM_API int shim_rot_get_caps(hamlib_shim_handle_t h, shim_rot_caps_t* out_caps) {
    ROT* rot = (ROT*)h;
    const struct rot_caps* caps;

    if (!rot || !out_caps || !rot->caps) {
        return -RIG_EINVAL;
    }

    caps = rot->caps;
    memset(out_caps, 0, sizeof(*out_caps));
    out_caps->rot_type = caps->rot_type;
    out_caps->min_az = caps->min_az;
    out_caps->max_az = caps->max_az;
    out_caps->min_el = caps->min_el;
    out_caps->max_el = caps->max_el;
    out_caps->has_get_level = (uint64_t)caps->has_get_level;
    out_caps->has_set_level = (uint64_t)caps->has_set_level;
    out_caps->has_get_func = (uint64_t)caps->has_get_func;
    out_caps->has_set_func = (uint64_t)caps->has_set_func;
    out_caps->has_get_parm = (uint64_t)caps->has_get_parm;
    out_caps->has_set_parm = (uint64_t)caps->has_set_parm;
    out_caps->has_status = (int)caps->has_status;

    return RIG_OK;
}

SHIM_API const char* shim_rot_strstatus(int status) {
    return rot_strstatus((rot_status_t)status);
}

SHIM_API const char* shim_rot_type_str(int rot_type) {
    switch (rot_type) {
        case ROT_TYPE_AZIMUTH: return "azimuth";
        case ROT_TYPE_ELEVATION: return "elevation";
        case ROT_TYPE_AZEL: return "azel";
        case ROT_TYPE_OTHER:
        default:
            return "other";
    }
}

/* ===== Port configuration ===== */

SHIM_API void shim_rig_set_port_path(hamlib_shim_handle_t h, const char* path) {
    RIG* rig = (RIG*)h;
    strncpy(rig->state.rigport.pathname, path, HAMLIB_FILPATHLEN - 1);
    rig->state.rigport.pathname[HAMLIB_FILPATHLEN - 1] = '\0';
}

SHIM_API void shim_rig_set_port_type(hamlib_shim_handle_t h, int type) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.type.rig = (enum rig_port_e)type;
}

SHIM_API void shim_rot_set_port_path(hamlib_shim_handle_t h, const char* path) {
    ROT* rot = (ROT*)h;
    strncpy(rot->state.rotport.pathname, path, HAMLIB_FILPATHLEN - 1);
    rot->state.rotport.pathname[HAMLIB_FILPATHLEN - 1] = '\0';
}

SHIM_API void shim_rot_set_port_type(hamlib_shim_handle_t h, int type) {
    ROT* rot = (ROT*)h;
    rot->state.rotport.type.rig = (enum rig_port_e)type;
}

/* Serial config */
SHIM_API void shim_rig_set_serial_rate(hamlib_shim_handle_t h, int rate) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.rate = rate;
}

SHIM_API int shim_rig_get_serial_rate(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.parm.serial.rate;
}

SHIM_API void shim_rig_set_serial_data_bits(hamlib_shim_handle_t h, int bits) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.data_bits = bits;
}

SHIM_API int shim_rig_get_serial_data_bits(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.parm.serial.data_bits;
}

SHIM_API void shim_rig_set_serial_stop_bits(hamlib_shim_handle_t h, int bits) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.stop_bits = bits;
}

SHIM_API int shim_rig_get_serial_stop_bits(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.parm.serial.stop_bits;
}

SHIM_API void shim_rig_set_serial_parity(hamlib_shim_handle_t h, int parity) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.parity = (enum serial_parity_e)parity;
}

SHIM_API int shim_rig_get_serial_parity(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.rigport.parm.serial.parity;
}

SHIM_API void shim_rig_set_serial_handshake(hamlib_shim_handle_t h, int handshake) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.handshake = (enum serial_handshake_e)handshake;
}

SHIM_API int shim_rig_get_serial_handshake(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.rigport.parm.serial.handshake;
}

SHIM_API void shim_rig_set_serial_rts_state(hamlib_shim_handle_t h, int state) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.rts_state = (enum serial_control_state_e)state;
}

SHIM_API int shim_rig_get_serial_rts_state(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.rigport.parm.serial.rts_state;
}

SHIM_API void shim_rig_set_serial_dtr_state(hamlib_shim_handle_t h, int state) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.parm.serial.dtr_state = (enum serial_control_state_e)state;
}

SHIM_API int shim_rig_get_serial_dtr_state(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.rigport.parm.serial.dtr_state;
}

/* Port timing */
SHIM_API void shim_rig_set_port_timeout(hamlib_shim_handle_t h, int ms) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.timeout = ms;
}

SHIM_API int shim_rig_get_port_timeout(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.timeout;
}

SHIM_API void shim_rig_set_port_retry(hamlib_shim_handle_t h, int count) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.retry = (short)count;
}

SHIM_API int shim_rig_get_port_retry(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.rigport.retry;
}

SHIM_API void shim_rig_set_port_write_delay(hamlib_shim_handle_t h, int ms) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.write_delay = ms;
}

SHIM_API int shim_rig_get_port_write_delay(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.write_delay;
}

SHIM_API void shim_rig_set_port_post_write_delay(hamlib_shim_handle_t h, int ms) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.post_write_delay = ms;
}

SHIM_API int shim_rig_get_port_post_write_delay(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.post_write_delay;
}

SHIM_API void shim_rig_set_port_flushx(hamlib_shim_handle_t h, int enable) {
    RIG* rig = (RIG*)h;
    rig->state.rigport.flushx = enable;
}

SHIM_API int shim_rig_get_port_flushx(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return rig->state.rigport.flushx;
}

/* PTT/DCD port type */
SHIM_API void shim_rig_set_ptt_type(hamlib_shim_handle_t h, int type) {
    RIG* rig = (RIG*)h;
    rig->state.pttport.type.ptt = (ptt_type_t)type;
}

SHIM_API int shim_rig_get_ptt_type(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.pttport.type.ptt;
}

SHIM_API void shim_rig_set_dcd_type(hamlib_shim_handle_t h, int type) {
    RIG* rig = (RIG*)h;
    rig->state.dcdport.type.dcd = (dcd_type_t)type;
}

SHIM_API int shim_rig_get_dcd_type(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (int)rig->state.dcdport.type.dcd;
}

/* ===== Frequency control ===== */

SHIM_API int shim_rig_set_freq(hamlib_shim_handle_t h, int vfo, double freq) {
    return rig_set_freq((RIG*)h, (vfo_t)vfo, (freq_t)freq);
}

SHIM_API int shim_rig_get_freq(hamlib_shim_handle_t h, int vfo, double* freq) {
    freq_t f = 0;
    int ret = rig_get_freq((RIG*)h, (vfo_t)vfo, &f);
    if (freq) *freq = (double)f;
    return ret;
}

/* ===== VFO control ===== */

SHIM_API int shim_rig_set_vfo(hamlib_shim_handle_t h, int vfo) {
    return rig_set_vfo((RIG*)h, (vfo_t)vfo);
}

SHIM_API int shim_rig_get_vfo(hamlib_shim_handle_t h, int* vfo) {
    vfo_t v = 0;
    int ret = rig_get_vfo((RIG*)h, &v);
    if (vfo) *vfo = (int)v;
    return ret;
}

SHIM_API int shim_rig_parse_vfo(const char* vfo_str) {
    return (int)rig_parse_vfo(vfo_str);
}

SHIM_API const char* shim_rig_strvfo(int vfo) {
    return rig_strvfo((vfo_t)vfo);
}

/* ===== Mode control ===== */

SHIM_API int shim_rig_set_mode(hamlib_shim_handle_t h, int vfo, int mode, int width) {
    return rig_set_mode((RIG*)h, (vfo_t)vfo, (rmode_t)mode, (pbwidth_t)width);
}

SHIM_API int shim_rig_get_mode(hamlib_shim_handle_t h, int vfo, int* mode, int* width) {
    rmode_t m = 0;
    pbwidth_t w = 0;
    int ret = rig_get_mode((RIG*)h, (vfo_t)vfo, &m, &w);
    if (mode) *mode = (int)m;
    if (width) *width = (int)w;
    return ret;
}

SHIM_API int shim_rig_parse_mode(const char* mode_str) {
    return (int)rig_parse_mode(mode_str);
}

SHIM_API const char* shim_rig_strrmode(int mode) {
    return rig_strrmode((rmode_t)mode);
}

SHIM_API int shim_rig_passband_narrow(hamlib_shim_handle_t h, int mode) {
    return (int)rig_passband_narrow((RIG*)h, (rmode_t)mode);
}

SHIM_API int shim_rig_passband_wide(hamlib_shim_handle_t h, int mode) {
    return (int)rig_passband_wide((RIG*)h, (rmode_t)mode);
}

/* ===== PTT control ===== */

SHIM_API int shim_rig_set_ptt(hamlib_shim_handle_t h, int vfo, int ptt) {
    return rig_set_ptt((RIG*)h, (vfo_t)vfo, (ptt_t)ptt);
}

SHIM_API int shim_rig_get_ptt(hamlib_shim_handle_t h, int vfo, int* ptt) {
    ptt_t p = 0;
    int ret = rig_get_ptt((RIG*)h, (vfo_t)vfo, &p);
    if (ptt) *ptt = (int)p;
    return ret;
}

SHIM_API int shim_rig_get_dcd(hamlib_shim_handle_t h, int vfo, int* dcd) {
    dcd_t d = 0;
    int ret = rig_get_dcd((RIG*)h, (vfo_t)vfo, &d);
    if (dcd) *dcd = (int)d;
    return ret;
}

/* ===== Signal strength ===== */

SHIM_API int shim_rig_get_strength(hamlib_shim_handle_t h, int vfo, int* strength) {
    int s = 0;
    int ret = rig_get_strength((RIG*)h, (vfo_t)vfo, &s);
    if (strength) *strength = s;
    return ret;
}

/* ===== Level control ===== */

static void shim_ensure_targetable_level(hamlib_shim_handle_t h);

SHIM_API int shim_rig_set_level_f(hamlib_shim_handle_t h, int vfo, uint64_t level, float value) {
    shim_ensure_targetable_level(h);
    value_t val;
    memset(&val, 0, sizeof(val));
    val.f = value;
    return rig_set_level((RIG*)h, (vfo_t)vfo, (setting_t)level, val);
}

SHIM_API int shim_rig_set_level_i(hamlib_shim_handle_t h, int vfo, uint64_t level, int value) {
    shim_ensure_targetable_level(h);
    value_t val;
    memset(&val, 0, sizeof(val));
    val.i = value;
    return rig_set_level((RIG*)h, (vfo_t)vfo, (setting_t)level, val);
}

/*
 * Ensure RIG_TARGETABLE_LEVEL is set so rig_get_level() skips VFO
 * switching but still performs calibration and type conversion.
 * This fixes ICOM serial rigs where icom_set_vfo fails with
 * "unsupported VFO" during get_level calls.
 *
 * RIG_TARGETABLE_LEVEL = (1<<5) tells Hamlib: "this rig can read
 * levels without needing to switch VFO first".
 */
#ifndef RIG_TARGETABLE_LEVEL
#define RIG_TARGETABLE_LEVEL (1<<5)
#endif

static void shim_ensure_targetable_level(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    if (rig && rig->caps && !(rig->caps->targetable_vfo & RIG_TARGETABLE_LEVEL)) {
        rig->caps->targetable_vfo |= RIG_TARGETABLE_LEVEL;
    }
}

SHIM_API int shim_rig_get_level_f(hamlib_shim_handle_t h, int vfo, uint64_t level, float* value) {
    shim_ensure_targetable_level(h);
    value_t val;
    val.f = 0.0f;
    int ret = rig_get_level((RIG*)h, (vfo_t)vfo, (setting_t)level, &val);
    if (value) *value = val.f;
    return ret;
}

SHIM_API int shim_rig_get_level_i(hamlib_shim_handle_t h, int vfo, uint64_t level, int* value) {
    shim_ensure_targetable_level(h);
    value_t val;
    val.i = 0;
    int ret = rig_get_level((RIG*)h, (vfo_t)vfo, (setting_t)level, &val);
    if (value) *value = val.i;
    return ret;
}

SHIM_API int shim_rig_level_is_float(uint64_t level) {
    return RIG_LEVEL_IS_FLOAT((setting_t)level) ? 1 : 0;
}

/*
 * Auto-detect int/float level type using RIG_LEVEL_IS_FLOAT macro.
 * Returns the value as double regardless of the underlying type.
 * This avoids the common bug where STRENGTH (int) is read as float.
 */
SHIM_API int shim_rig_get_level_auto(hamlib_shim_handle_t h, int vfo, uint64_t level, double* value) {
    shim_ensure_targetable_level(h);
    value_t val;
    memset(&val, 0, sizeof(val));
    int ret = rig_get_level((RIG*)h, (vfo_t)vfo, (setting_t)level, &val);
    if (value) {
        if (RIG_LEVEL_IS_FLOAT((setting_t)level)) {
            *value = (double)val.f;
        } else {
            *value = (double)val.i;
        }
    }
    return ret;
}

/* ===== Function control ===== */

/*
 * Ensure RIG_TARGETABLE_FUNC is set so rig_get_func()/rig_set_func()
 * skip VFO switching. Same fix as RIG_TARGETABLE_LEVEL for ICOM serial
 * rigs where icom_set_vfo fails with "unsupported VFO".
 *
 * RIG_TARGETABLE_FUNC = (1<<4) tells Hamlib: "this rig can get/set
 * functions without needing to switch VFO first".
 */
#ifndef RIG_TARGETABLE_FUNC
#define RIG_TARGETABLE_FUNC (1<<4)
#endif

static void shim_ensure_targetable_func(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    if (rig && rig->caps && !(rig->caps->targetable_vfo & RIG_TARGETABLE_FUNC)) {
        rig->caps->targetable_vfo |= RIG_TARGETABLE_FUNC;
    }
}

SHIM_API int shim_rig_set_func(hamlib_shim_handle_t h, int vfo, uint64_t func, int enable) {
    shim_ensure_targetable_func(h);
    return rig_set_func((RIG*)h, (vfo_t)vfo, (setting_t)func, enable);
}

SHIM_API int shim_rig_get_func(hamlib_shim_handle_t h, int vfo, uint64_t func, int* state) {
    shim_ensure_targetable_func(h);
    int s = 0;
    int ret = rig_get_func((RIG*)h, (vfo_t)vfo, (setting_t)func, &s);
    if (state) *state = s;
    return ret;
}

/* ===== Parameter control ===== */

SHIM_API int shim_rig_set_parm_f(hamlib_shim_handle_t h, uint64_t parm, float value) {
    value_t val;
    val.f = value;
    return rig_set_parm((RIG*)h, (setting_t)parm, val);
}

SHIM_API int shim_rig_set_parm_i(hamlib_shim_handle_t h, uint64_t parm, int value) {
    value_t val;
    val.i = value;
    return rig_set_parm((RIG*)h, (setting_t)parm, val);
}

SHIM_API int shim_rig_get_parm_f(hamlib_shim_handle_t h, uint64_t parm, float* value) {
    value_t val;
    val.f = 0.0f;
    int ret = rig_get_parm((RIG*)h, (setting_t)parm, &val);
    if (value) *value = val.f;
    return ret;
}

SHIM_API int shim_rig_get_parm_i(hamlib_shim_handle_t h, uint64_t parm, int* value) {
    value_t val;
    val.i = 0;
    int ret = rig_get_parm((RIG*)h, (setting_t)parm, &val);
    if (value) *value = val.i;
    return ret;
}

SHIM_API uint64_t shim_rig_parse_parm(const char* parm_str) {
    return (uint64_t)rig_parse_parm(parm_str);
}

/* ===== Split operations ===== */

SHIM_API int shim_rig_set_split_freq(hamlib_shim_handle_t h, int vfo, double freq) {
    return rig_set_split_freq((RIG*)h, (vfo_t)vfo, (freq_t)freq);
}

SHIM_API int shim_rig_get_split_freq(hamlib_shim_handle_t h, int vfo, double* freq) {
    freq_t f = 0;
    int ret = rig_get_split_freq((RIG*)h, (vfo_t)vfo, &f);
    if (freq) *freq = (double)f;
    return ret;
}

SHIM_API int shim_rig_set_split_vfo(hamlib_shim_handle_t h, int rx_vfo, int split, int tx_vfo) {
    return rig_set_split_vfo((RIG*)h, (vfo_t)rx_vfo, (split_t)split, (vfo_t)tx_vfo);
}

SHIM_API int shim_rig_get_split_vfo(hamlib_shim_handle_t h, int vfo, int* split, int* tx_vfo) {
    split_t s = 0;
    vfo_t tv = 0;
    int ret = rig_get_split_vfo((RIG*)h, (vfo_t)vfo, &s, &tv);
    if (split) *split = (int)s;
    if (tx_vfo) *tx_vfo = (int)tv;
    return ret;
}

SHIM_API int shim_rig_set_split_mode(hamlib_shim_handle_t h, int vfo, int mode, int width) {
    return rig_set_split_mode((RIG*)h, (vfo_t)vfo, (rmode_t)mode, (pbwidth_t)width);
}

SHIM_API int shim_rig_get_split_mode(hamlib_shim_handle_t h, int vfo, int* mode, int* width) {
    rmode_t m = 0;
    pbwidth_t w = 0;
    int ret = rig_get_split_mode((RIG*)h, (vfo_t)vfo, &m, &w);
    if (mode) *mode = (int)m;
    if (width) *width = (int)w;
    return ret;
}

SHIM_API int shim_rig_set_split_freq_mode(hamlib_shim_handle_t h, int vfo, double freq, int mode, int width) {
#ifdef SHIM_HAS_SPLIT_FREQ_MODE
    return rig_set_split_freq_mode((RIG*)h, (vfo_t)vfo, (freq_t)freq, (rmode_t)mode, (pbwidth_t)width);
#else
    /* Fallback: set freq and mode separately */
    int ret = rig_set_split_freq((RIG*)h, (vfo_t)vfo, (freq_t)freq);
    if (ret != RIG_OK) return ret;
    return rig_set_split_mode((RIG*)h, (vfo_t)vfo, (rmode_t)mode, (pbwidth_t)width);
#endif
}

SHIM_API int shim_rig_get_split_freq_mode(hamlib_shim_handle_t h, int vfo, double* freq, int* mode, int* width) {
#ifdef SHIM_HAS_SPLIT_FREQ_MODE
    freq_t f = 0;
    rmode_t m = 0;
    pbwidth_t w = 0;
    int ret = rig_get_split_freq_mode((RIG*)h, (vfo_t)vfo, &f, &m, &w);
    if (freq) *freq = (double)f;
    if (mode) *mode = (int)m;
    if (width) *width = (int)w;
    return ret;
#else
    /* Fallback: get freq and mode separately */
    freq_t f = 0;
    int ret = rig_get_split_freq((RIG*)h, (vfo_t)vfo, &f);
    if (freq) *freq = (double)f;
    if (ret != RIG_OK) return ret;
    rmode_t m = 0;
    pbwidth_t w = 0;
    ret = rig_get_split_mode((RIG*)h, (vfo_t)vfo, &m, &w);
    if (mode) *mode = (int)m;
    if (width) *width = (int)w;
    return ret;
#endif
}

/* ===== RIT/XIT control ===== */

SHIM_API int shim_rig_set_rit(hamlib_shim_handle_t h, int vfo, int offset) {
    return rig_set_rit((RIG*)h, (vfo_t)vfo, (shortfreq_t)offset);
}

SHIM_API int shim_rig_get_rit(hamlib_shim_handle_t h, int vfo, int* offset) {
    shortfreq_t o = 0;
    int ret = rig_get_rit((RIG*)h, (vfo_t)vfo, &o);
    if (offset) *offset = (int)o;
    return ret;
}

SHIM_API int shim_rig_set_xit(hamlib_shim_handle_t h, int vfo, int offset) {
    return rig_set_xit((RIG*)h, (vfo_t)vfo, (shortfreq_t)offset);
}

SHIM_API int shim_rig_get_xit(hamlib_shim_handle_t h, int vfo, int* offset) {
    shortfreq_t o = 0;
    int ret = rig_get_xit((RIG*)h, (vfo_t)vfo, &o);
    if (offset) *offset = (int)o;
    return ret;
}

/* ===== Memory channel operations ===== */

static void shim_copy_channel_caps(shim_channel_caps_t* out, const channel_cap_t* caps) {
    if (!out || !caps) return;
    memset(out, 0, sizeof(*out));
    out->bank_num = caps->bank_num;
    out->vfo = caps->vfo;
    out->ant = caps->ant;
    out->freq = caps->freq;
    out->mode = caps->mode;
    out->width = caps->width;
    out->tx_freq = caps->tx_freq;
    out->tx_mode = caps->tx_mode;
    out->tx_width = caps->tx_width;
    out->split = caps->split;
    out->tx_vfo = caps->tx_vfo;
    out->rptr_shift = caps->rptr_shift;
    out->rptr_offs = caps->rptr_offs;
    out->tuning_step = caps->tuning_step;
    out->rit = caps->rit;
    out->xit = caps->xit;
    out->funcs = (uint64_t)caps->funcs;
    out->levels = (uint64_t)caps->levels;
    out->ctcss_tone = caps->ctcss_tone;
    out->ctcss_sql = caps->ctcss_sql;
    out->dcs_code = caps->dcs_code;
    out->dcs_sql = caps->dcs_sql;
    out->scan_group = caps->scan_group;
    out->flags = caps->flags;
    out->channel_desc = caps->channel_desc;
    out->tag = caps->tag;
}

static void shim_copy_range(shim_memory_range_t* out, const chan_t* range) {
    if (!out || !range) return;
    memset(out, 0, sizeof(*out));
    out->start = range->startc;
    out->end = range->endc;
    out->type = (int)range->type;
    shim_copy_channel_caps(&out->caps, &range->mem_caps);
}

static void shim_copy_to_hamlib_channel(channel_t* chan, const shim_channel_t* schan) {
    int i;
    if (!chan || !schan) return;
    memset(chan, 0, sizeof(*chan));
    chan->channel_num = schan->channel_num;
    chan->bank_num = schan->bank_num;
    chan->vfo = (vfo_t)schan->vfo;
    chan->ant = (ant_t)schan->ant;
    chan->freq = (freq_t)schan->freq;
    chan->mode = (rmode_t)schan->mode;
    chan->width = (pbwidth_t)schan->width;
    chan->tx_freq = (freq_t)schan->tx_freq;
    chan->tx_mode = (rmode_t)schan->tx_mode;
    chan->tx_width = (pbwidth_t)schan->tx_width;
    chan->split = (split_t)schan->split;
    chan->tx_vfo = (vfo_t)schan->tx_vfo;
    chan->rptr_shift = (rptr_shift_t)schan->rptr_shift;
    chan->rptr_offs = (shortfreq_t)schan->rptr_offs;
    chan->tuning_step = (shortfreq_t)schan->tuning_step;
    chan->rit = (shortfreq_t)schan->rit;
    chan->xit = (shortfreq_t)schan->xit;
    chan->funcs = (setting_t)schan->funcs;
    for (i = 0; i < schan->level_count && i < SHIM_HAMLIB_MAX_CHANNEL_LEVELS; i++) {
        int idx = shim_setting_to_index((setting_t)schan->level_tokens[i]);
        if (idx >= 0 && idx < RIG_SETTING_MAX) {
            if (RIG_LEVEL_IS_FLOAT((setting_t)schan->level_tokens[i])) {
                chan->levels[idx].f = (float)schan->level_values[i];
            } else {
                chan->levels[idx].i = (int)schan->level_values[i];
            }
        }
    }
    chan->ctcss_tone = (tone_t)schan->ctcss_tone;
    chan->ctcss_sql = (tone_t)schan->ctcss_sql;
    chan->dcs_code = (tone_t)schan->dcs_code;
    chan->dcs_sql = (tone_t)schan->dcs_sql;
    chan->scan_group = schan->scan_group;
    chan->flags = schan->flags;
    strncpy(chan->channel_desc, schan->channel_desc, sizeof(chan->channel_desc) - 1);
    chan->channel_desc[sizeof(chan->channel_desc) - 1] = '\0';
    strncpy(chan->tag, schan->tag, sizeof(chan->tag) - 1);
    chan->tag[sizeof(chan->tag) - 1] = '\0';
}

static void shim_copy_from_hamlib_channel(shim_channel_t* schan, const channel_t* chan, const chan_t* range) {
    int i;
    if (!schan || !chan) return;
    memset(schan, 0, sizeof(*schan));
    schan->channel_num = chan->channel_num;
    schan->bank_num = chan->bank_num;
    schan->channel_type = range ? (int)range->type : 0;
    schan->vfo = (int)chan->vfo;
    schan->ant = (int)chan->ant;
    schan->freq = (double)chan->freq;
    schan->mode = (int)chan->mode;
    schan->width = (int)chan->width;
    schan->tx_freq = (double)chan->tx_freq;
    schan->tx_mode = (int)chan->tx_mode;
    schan->tx_width = (int)chan->tx_width;
    schan->split = (int)chan->split;
    schan->tx_vfo = (int)chan->tx_vfo;
    schan->rptr_shift = (int)chan->rptr_shift;
    schan->rptr_offs = (int)chan->rptr_offs;
    schan->tuning_step = (int)chan->tuning_step;
    schan->rit = (int)chan->rit;
    schan->xit = (int)chan->xit;
    schan->funcs = (uint64_t)chan->funcs;
    schan->level_count = 0;
    for (i = 0; i < RIG_SETTING_MAX && schan->level_count < SHIM_HAMLIB_MAX_CHANNEL_LEVELS; i++) {
        setting_t setting = ((setting_t)1) << i;
        double value = RIG_LEVEL_IS_FLOAT(setting) ? (double)chan->levels[i].f : (double)chan->levels[i].i;
        if (value != 0) {
            int pos = schan->level_count++;
            schan->level_tokens[pos] = (uint64_t)setting;
            schan->level_values[pos] = value;
        }
    }
    schan->ctcss_tone = (int)chan->ctcss_tone;
    schan->ctcss_sql = (int)chan->ctcss_sql;
    schan->dcs_code = (int)chan->dcs_code;
    schan->dcs_sql = (int)chan->dcs_sql;
    schan->scan_group = chan->scan_group;
    schan->flags = chan->flags;
    strncpy(schan->channel_desc, chan->channel_desc, sizeof(schan->channel_desc) - 1);
    schan->channel_desc[sizeof(schan->channel_desc) - 1] = '\0';
    strncpy(schan->tag, chan->tag, sizeof(schan->tag) - 1);
    schan->tag[sizeof(schan->tag) - 1] = '\0';
}

SHIM_API int shim_rig_set_channel(hamlib_shim_handle_t h, int vfo, const shim_channel_t* schan) {
    channel_t chan;
    if (!h || !schan) return -RIG_EINVAL;
    shim_copy_to_hamlib_channel(&chan, schan);
    return rig_set_channel((RIG*)h, (vfo_t)vfo, &chan);
}

SHIM_API int shim_rig_get_channel(hamlib_shim_handle_t h, int vfo, shim_channel_t* schan, int read_only) {
    channel_t chan;
    const chan_t* range;
    int ret;
    if (!h || !schan) return -RIG_EINVAL;
    memset(&chan, 0, sizeof(chan));
    chan.channel_num = schan->channel_num;
    chan.vfo = (vfo_t)schan->vfo;
    ret = rig_get_channel((RIG*)h, (vfo_t)vfo, &chan, read_only);
    if (ret == RIG_OK) {
        range = rig_lookup_mem_caps((RIG*)h, chan.channel_num);
        shim_copy_from_hamlib_channel(schan, &chan, range);
    }
    free(chan.ext_levels);
    return ret;
}

SHIM_API int shim_rig_get_memory_range_count(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    int count = 0;
    if (!rig) return -RIG_EINVAL;
    while (count < HAMLIB_CHANLSTSIZ && !RIG_IS_CHAN_END(rig->state.chan_list[count])) {
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_memory_range(hamlib_shim_handle_t h, int index, shim_memory_range_t* out) {
    RIG* rig = (RIG*)h;
    int count;
    if (!rig || !out || index < 0) return -RIG_EINVAL;
    count = shim_rig_get_memory_range_count(h);
    if (count < 0) return count;
    if (index >= count) return -RIG_EINVAL;
    shim_copy_range(out, &rig->state.chan_list[index]);
    return RIG_OK;
}

SHIM_API int shim_rig_lookup_memory_caps(hamlib_shim_handle_t h, int channel_num, shim_memory_range_t* out) {
    const chan_t* range;
    if (!h || !out) return -RIG_EINVAL;
    range = rig_lookup_mem_caps((RIG*)h, channel_num);
    if (!range) return -RIG_ENAVAIL;
    shim_copy_range(out, range);
    return RIG_OK;
}

SHIM_API int shim_rig_get_chan_all(hamlib_shim_handle_t h, int vfo, shim_channel_t* chans, int max_count) {
    RIG* rig = (RIG*)h;
    int i, j, out_count = 0;
    if (!rig || !chans || max_count < 0) return -RIG_EINVAL;

    for (i = 0; i < HAMLIB_CHANLSTSIZ && !RIG_IS_CHAN_END(rig->state.chan_list[i]); i++) {
        const chan_t* range = &rig->state.chan_list[i];
        for (j = range->startc; j <= range->endc; j++) {
            channel_t chan;
            int ret;
            if (out_count >= max_count) return -RIG_EINVAL;
            memset(&chan, 0, sizeof(chan));
            chan.vfo = RIG_VFO_MEM;
            chan.channel_num = j;
            ret = rig_get_channel(rig, (vfo_t)vfo, &chan, 1);
            if (ret == -RIG_ENAVAIL) {
                memset(&chans[out_count], 0, sizeof(chans[out_count]));
                chans[out_count].channel_num = j;
                chans[out_count].channel_type = (int)range->type;
                out_count++;
                continue;
            }
            if (ret != RIG_OK) {
                free(chan.ext_levels);
                return ret;
            }
            shim_copy_from_hamlib_channel(&chans[out_count], &chan, range);
            free(chan.ext_levels);
            out_count++;
        }
    }
    return out_count;
}

SHIM_API int shim_rig_set_chan_all(hamlib_shim_handle_t h, int vfo, const shim_channel_t* schans, int count) {
    channel_t* chans;
    int i, max_channel = 0, ret;
    if (!h || !schans || count < 0) return -RIG_EINVAL;
    for (i = 0; i < count; i++) {
        if (schans[i].channel_num > max_channel) max_channel = schans[i].channel_num;
    }
    chans = (channel_t*)calloc((size_t)max_channel + 1, sizeof(channel_t));
    if (!chans) return -RIG_ENOMEM;
    for (i = 0; i < count; i++) {
        shim_copy_to_hamlib_channel(&chans[schans[i].channel_num], &schans[i]);
    }
    ret = rig_set_chan_all((RIG*)h, (vfo_t)vfo, chans);
    free(chans);
    return ret;
}

SHIM_API int shim_rig_set_mem(hamlib_shim_handle_t h, int vfo, int ch) {
    return rig_set_mem((RIG*)h, (vfo_t)vfo, ch);
}

SHIM_API int shim_rig_get_mem(hamlib_shim_handle_t h, int vfo, int* ch) {
    return rig_get_mem((RIG*)h, (vfo_t)vfo, ch);
}

SHIM_API int shim_rig_set_bank(hamlib_shim_handle_t h, int vfo, int bank) {
    return rig_set_bank((RIG*)h, (vfo_t)vfo, bank);
}

SHIM_API int shim_rig_mem_count(hamlib_shim_handle_t h) {
    return rig_mem_count((RIG*)h);
}

/* ===== Scanning ===== */

SHIM_API int shim_rig_scan(hamlib_shim_handle_t h, int vfo, int scan_type, int channel) {
    return rig_scan((RIG*)h, (vfo_t)vfo, (scan_t)scan_type, channel);
}

/* ===== VFO operations ===== */

/*
 * vfo_op with fallback for rigs with VFO switching issues.
 * There is no RIG_TARGETABLE for vfo_op, so we use the same
 * fallback pattern as the old getLevel fix: try standard call
 * first, then direct backend call if RIG_EINVAL (-1).
 */
SHIM_API int shim_rig_vfo_op(hamlib_shim_handle_t h, int vfo, int op) {
    RIG* rig = (RIG*)h;
    int ret = rig_vfo_op(rig, (vfo_t)vfo, (vfo_op_t)op);
    if (ret == -1 && rig->caps->vfo_op) {
        /* RIG_EINVAL: VFO switching failed, try direct backend call */
        ret = rig->caps->vfo_op(rig, (vfo_t)vfo, (vfo_op_t)op);
    }
    return ret;
}

/* ===== Antenna control ===== */

SHIM_API int shim_rig_set_ant(hamlib_shim_handle_t h, int vfo, int ant, float option) {
    value_t opt;
    opt.f = option;
    return rig_set_ant((RIG*)h, (vfo_t)vfo, (ant_t)ant, opt);
}

SHIM_API int shim_rig_get_ant(hamlib_shim_handle_t h, int vfo, int ant, float* option,
                               int* ant_curr, int* ant_tx, int* ant_rx) {
    value_t opt = {0};
    ant_t ac = 0, atx = 0, arx = 0;
    int ret = rig_get_ant((RIG*)h, (vfo_t)vfo, (ant_t)ant, &opt, &ac, &atx, &arx);
    if (option) *option = opt.f;
    if (ant_curr) *ant_curr = (int)ac;
    if (ant_tx) *ant_tx = (int)atx;
    if (ant_rx) *ant_rx = (int)arx;
    return ret;
}

/* ===== Tuning step ===== */

SHIM_API int shim_rig_set_ts(hamlib_shim_handle_t h, int vfo, int ts) {
    return rig_set_ts((RIG*)h, (vfo_t)vfo, (shortfreq_t)ts);
}

SHIM_API int shim_rig_get_ts(hamlib_shim_handle_t h, int vfo, int* ts) {
    shortfreq_t t = 0;
    int ret = rig_get_ts((RIG*)h, (vfo_t)vfo, &t);
    if (ts) *ts = (int)t;
    return ret;
}

/* ===== Repeater control ===== */

SHIM_API int shim_rig_set_rptr_shift(hamlib_shim_handle_t h, int vfo, int shift) {
    return rig_set_rptr_shift((RIG*)h, (vfo_t)vfo, (rptr_shift_t)shift);
}

SHIM_API int shim_rig_get_rptr_shift(hamlib_shim_handle_t h, int vfo, int* shift) {
    rptr_shift_t s = 0;
    int ret = rig_get_rptr_shift((RIG*)h, (vfo_t)vfo, &s);
    if (shift) *shift = (int)s;
    return ret;
}

SHIM_API const char* shim_rig_strptrshift(int shift) {
    return rig_strptrshift((rptr_shift_t)shift);
}

SHIM_API int shim_rig_set_rptr_offs(hamlib_shim_handle_t h, int vfo, int offset) {
    return rig_set_rptr_offs((RIG*)h, (vfo_t)vfo, (shortfreq_t)offset);
}

SHIM_API int shim_rig_get_rptr_offs(hamlib_shim_handle_t h, int vfo, int* offset) {
    shortfreq_t o = 0;
    int ret = rig_get_rptr_offs((RIG*)h, (vfo_t)vfo, &o);
    if (offset) *offset = (int)o;
    return ret;
}

/* ===== CTCSS/DCS tone control ===== */

SHIM_API int shim_rig_set_ctcss_tone(hamlib_shim_handle_t h, int vfo, unsigned int tone) {
    return rig_set_ctcss_tone((RIG*)h, (vfo_t)vfo, (tone_t)tone);
}

SHIM_API int shim_rig_get_ctcss_tone(hamlib_shim_handle_t h, int vfo, unsigned int* tone) {
    tone_t t = 0;
    int ret = rig_get_ctcss_tone((RIG*)h, (vfo_t)vfo, &t);
    if (tone) *tone = (unsigned int)t;
    return ret;
}

SHIM_API int shim_rig_set_dcs_code(hamlib_shim_handle_t h, int vfo, unsigned int code) {
    return rig_set_dcs_code((RIG*)h, (vfo_t)vfo, (tone_t)code);
}

SHIM_API int shim_rig_get_dcs_code(hamlib_shim_handle_t h, int vfo, unsigned int* code) {
    tone_t c = 0;
    int ret = rig_get_dcs_code((RIG*)h, (vfo_t)vfo, &c);
    if (code) *code = (unsigned int)c;
    return ret;
}

SHIM_API int shim_rig_set_ctcss_sql(hamlib_shim_handle_t h, int vfo, unsigned int tone) {
    return rig_set_ctcss_sql((RIG*)h, (vfo_t)vfo, (tone_t)tone);
}

SHIM_API int shim_rig_get_ctcss_sql(hamlib_shim_handle_t h, int vfo, unsigned int* tone) {
    tone_t t = 0;
    int ret = rig_get_ctcss_sql((RIG*)h, (vfo_t)vfo, &t);
    if (tone) *tone = (unsigned int)t;
    return ret;
}

SHIM_API int shim_rig_set_dcs_sql(hamlib_shim_handle_t h, int vfo, unsigned int code) {
    return rig_set_dcs_sql((RIG*)h, (vfo_t)vfo, (tone_t)code);
}

SHIM_API int shim_rig_get_dcs_sql(hamlib_shim_handle_t h, int vfo, unsigned int* code) {
    tone_t c = 0;
    int ret = rig_get_dcs_sql((RIG*)h, (vfo_t)vfo, &c);
    if (code) *code = (unsigned int)c;
    return ret;
}

/* ===== DTMF ===== */

SHIM_API int shim_rig_send_dtmf(hamlib_shim_handle_t h, int vfo, const char* digits) {
    return rig_send_dtmf((RIG*)h, (vfo_t)vfo, digits);
}

SHIM_API int shim_rig_recv_dtmf(hamlib_shim_handle_t h, int vfo, char* digits, int* length) {
    return rig_recv_dtmf((RIG*)h, (vfo_t)vfo, digits, length);
}

/* ===== Morse code ===== */

SHIM_API int shim_rig_send_morse(hamlib_shim_handle_t h, int vfo, const char* msg) {
    return rig_send_morse((RIG*)h, (vfo_t)vfo, msg);
}

SHIM_API int shim_rig_stop_morse(hamlib_shim_handle_t h, int vfo) {
    return rig_stop_morse((RIG*)h, (vfo_t)vfo);
}

SHIM_API int shim_rig_wait_morse(hamlib_shim_handle_t h, int vfo) {
    return rig_wait_morse((RIG*)h, (vfo_t)vfo);
}

/* ===== Voice memory ===== */

SHIM_API int shim_rig_send_voice_mem(hamlib_shim_handle_t h, int vfo, int ch) {
    return rig_send_voice_mem((RIG*)h, (vfo_t)vfo, ch);
}

SHIM_API int shim_rig_stop_voice_mem(hamlib_shim_handle_t h, int vfo) {
#ifdef SHIM_HAS_STOP_VOICE_MEM
    return rig_stop_voice_mem((RIG*)h, (vfo_t)vfo);
#else
    (void)h; (void)vfo;
    return -RIG_ENIMPL;
#endif
}

/* ===== Power control ===== */

SHIM_API int shim_rig_set_powerstat(hamlib_shim_handle_t h, int status) {
    return rig_set_powerstat((RIG*)h, (powerstat_t)status);
}

SHIM_API int shim_rig_get_powerstat(hamlib_shim_handle_t h, int* status) {
    powerstat_t s = 0;
    int ret = rig_get_powerstat((RIG*)h, &s);
    if (status) *status = (int)s;
    return ret;
}

/* ===== Power conversion ===== */

SHIM_API int shim_rig_power2mW(hamlib_shim_handle_t h, unsigned int* mwpower, float power, double freq, int mode) {
    return rig_power2mW((RIG*)h, mwpower, power, (freq_t)freq, (rmode_t)mode);
}

SHIM_API int shim_rig_mW2power(hamlib_shim_handle_t h, float* power, unsigned int mwpower, double freq, int mode) {
    return rig_mW2power((RIG*)h, power, mwpower, (freq_t)freq, (rmode_t)mode);
}

/* ===== Reset ===== */

SHIM_API int shim_rig_reset(hamlib_shim_handle_t h, int reset_type) {
    return rig_reset((RIG*)h, (reset_t)reset_type);
}

/* ===== Rotator control ===== */

SHIM_API int shim_rot_set_position(hamlib_shim_handle_t h, double azimuth, double elevation) {
    return rot_set_position((ROT*)h, (azimuth_t)azimuth, (elevation_t)elevation);
}

SHIM_API int shim_rot_get_position(hamlib_shim_handle_t h, double* azimuth, double* elevation) {
    azimuth_t az = 0;
    elevation_t el = 0;
    int ret = rot_get_position((ROT*)h, &az, &el);
    if (azimuth) *azimuth = (double)az;
    if (elevation) *elevation = (double)el;
    return ret;
}

SHIM_API int shim_rot_stop(hamlib_shim_handle_t h) {
    return rot_stop((ROT*)h);
}

SHIM_API int shim_rot_park(hamlib_shim_handle_t h) {
    return rot_park((ROT*)h);
}

SHIM_API int shim_rot_reset(hamlib_shim_handle_t h, int reset_type) {
    return rot_reset((ROT*)h, (rot_reset_t)reset_type);
}

SHIM_API int shim_rot_move(hamlib_shim_handle_t h, int direction, int speed) {
    return rot_move((ROT*)h, direction, speed);
}

SHIM_API const char* shim_rot_get_info(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    if (!rot) return "";
    {
        const char *info = rot_get_info(rot);
        return info ? info : "";
    }
}

SHIM_API int shim_rot_get_status(hamlib_shim_handle_t h, int* status) {
    rot_status_t s = 0;
    int ret = rot_get_status((ROT*)h, &s);
    if (status) *status = (int)s;
    return ret;
}

SHIM_API int shim_rot_set_conf(hamlib_shim_handle_t h, const char* name, const char* val) {
    ROT *rot = (ROT *)h;
    hamlib_token_t token;
    if (!rot || !name || !val) return -RIG_EINVAL;
    token = rot_token_lookup(rot, name);
    if (token == 0) return -RIG_EINVAL;
    return rot_set_conf(rot, token, val);
}

SHIM_API int shim_rot_get_conf(hamlib_shim_handle_t h, const char* name, char* buf, int buflen) {
    ROT *rot = (ROT *)h;
    hamlib_token_t token;
    if (!rot || !name || !buf || buflen <= 0) return -RIG_EINVAL;
    token = rot_token_lookup(rot, name);
    if (token == 0) return -RIG_EINVAL;
    return rot_get_conf2(rot, token, buf, buflen);
}

SHIM_API uint64_t shim_rot_get_caps_has_get_level(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_get_level : 0;
}

SHIM_API uint64_t shim_rot_get_caps_has_set_level(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_set_level : 0;
}

SHIM_API uint64_t shim_rot_get_caps_has_get_func(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_get_func : 0;
}

SHIM_API uint64_t shim_rot_get_caps_has_set_func(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_set_func : 0;
}

SHIM_API uint64_t shim_rot_get_caps_has_get_parm(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_get_parm : 0;
}

SHIM_API uint64_t shim_rot_get_caps_has_set_parm(hamlib_shim_handle_t h) {
    ROT *rot = (ROT *)h;
    return rot && rot->caps ? (uint64_t)rot->caps->has_set_parm : 0;
}

SHIM_API int shim_rot_set_level_f(hamlib_shim_handle_t h, uint64_t level, float value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    val.f = value;
    return rot_set_level((ROT*)h, (setting_t)level, val);
}

SHIM_API int shim_rot_set_level_i(hamlib_shim_handle_t h, uint64_t level, int value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    val.i = value;
    return rot_set_level((ROT*)h, (setting_t)level, val);
}

SHIM_API int shim_rot_get_level_f(hamlib_shim_handle_t h, uint64_t level, float* value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    {
        int ret = rot_get_level((ROT*)h, (setting_t)level, &val);
        if (value) *value = val.f;
        return ret;
    }
}

SHIM_API int shim_rot_get_level_i(hamlib_shim_handle_t h, uint64_t level, int* value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    {
        int ret = rot_get_level((ROT*)h, (setting_t)level, &val);
        if (value) *value = val.i;
        return ret;
    }
}

SHIM_API int shim_rot_get_level_auto(hamlib_shim_handle_t h, uint64_t level, double* value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    {
        int ret = rot_get_level((ROT*)h, (setting_t)level, &val);
        if (ret != RIG_OK) return ret;
        if (value) {
            if (ROT_LEVEL_IS_FLOAT((setting_t)level)) *value = (double)val.f;
            else *value = (double)val.i;
        }
        return ret;
    }
}

SHIM_API int shim_rot_set_func(hamlib_shim_handle_t h, uint64_t func, int enable) {
    return rot_set_func((ROT*)h, (setting_t)func, enable);
}

SHIM_API int shim_rot_get_func(hamlib_shim_handle_t h, uint64_t func, int* state) {
    return rot_get_func((ROT*)h, (setting_t)func, state);
}

SHIM_API int shim_rot_set_parm_f(hamlib_shim_handle_t h, uint64_t parm, float value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    val.f = value;
    return rot_set_parm((ROT*)h, (setting_t)parm, val);
}

SHIM_API int shim_rot_set_parm_i(hamlib_shim_handle_t h, uint64_t parm, int value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    val.i = value;
    return rot_set_parm((ROT*)h, (setting_t)parm, val);
}

SHIM_API int shim_rot_get_parm_f(hamlib_shim_handle_t h, uint64_t parm, float* value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    {
        int ret = rot_get_parm((ROT*)h, (setting_t)parm, &val);
        if (value) *value = val.f;
        return ret;
    }
}

SHIM_API int shim_rot_get_parm_i(hamlib_shim_handle_t h, uint64_t parm, int* value) {
    value_t val;
    memset(&val, 0, sizeof(val));
    {
        int ret = rot_get_parm((ROT*)h, (setting_t)parm, &val);
        if (value) *value = val.i;
        return ret;
    }
}

SHIM_API uint64_t shim_rot_parse_level(const char* level_str) {
    return (uint64_t)rot_parse_level(level_str);
}

SHIM_API uint64_t shim_rot_parse_func(const char* func_str) {
    return (uint64_t)rot_parse_func(func_str);
}

SHIM_API uint64_t shim_rot_parse_parm(const char* parm_str) {
    return (uint64_t)rot_parse_parm(parm_str);
}

SHIM_API const char* shim_rot_strlevel(uint64_t level) {
    return rot_strlevel((setting_t)level);
}

SHIM_API const char* shim_rot_strfunc(uint64_t func) {
    return rot_strfunc((setting_t)func);
}

SHIM_API const char* shim_rot_strparm(uint64_t parm) {
    return rot_strparm((setting_t)parm);
}

/* ===== Callbacks ===== */

/* Internal adapter structs for callback forwarding */
struct shim_freq_cb_adapter {
    shim_freq_cb_t user_cb;
    void* user_arg;
};

struct shim_ptt_cb_adapter {
    shim_ptt_cb_t user_cb;
    void* user_arg;
};

struct shim_spectrum_cb_adapter {
    shim_spectrum_cb_t user_cb;
    void* user_arg;
};

/* We store adapters statically (one per rig handle - simplified) */
static struct shim_freq_cb_adapter freq_cb_adapter = {NULL, NULL};
static struct shim_ptt_cb_adapter ptt_cb_adapter = {NULL, NULL};
static struct shim_spectrum_cb_adapter spectrum_cb_adapter = {NULL, NULL};

static int shim_freq_cb_thunk(RIG *rig, vfo_t vfo, freq_t freq, rig_ptr_t arg) {
    struct shim_freq_cb_adapter *adapter = (struct shim_freq_cb_adapter*)arg;
    if (adapter && adapter->user_cb) {
        return adapter->user_cb((void*)rig, (int)vfo, (double)freq, adapter->user_arg);
    }
    return 0;
}

static int shim_ptt_cb_thunk(RIG *rig, vfo_t vfo, ptt_t ptt, rig_ptr_t arg) {
    struct shim_ptt_cb_adapter *adapter = (struct shim_ptt_cb_adapter*)arg;
    if (adapter && adapter->user_cb) {
        return adapter->user_cb((void*)rig, (int)vfo, (int)ptt, adapter->user_arg);
    }
    return 0;
}

static int shim_spectrum_cb_thunk(RIG *rig, struct rig_spectrum_line *line, rig_ptr_t arg) {
    struct shim_spectrum_cb_adapter *adapter = (struct shim_spectrum_cb_adapter*)arg;
    shim_spectrum_line_t safe_line;

    if (!adapter || !adapter->user_cb || !line) {
        return 0;
    }

    memset(&safe_line, 0, sizeof(safe_line));
    safe_line.id = line->id;
    safe_line.data_level_min = line->data_level_min;
    safe_line.data_level_max = line->data_level_max;
    safe_line.signal_strength_min = line->signal_strength_min;
    safe_line.signal_strength_max = line->signal_strength_max;
    safe_line.spectrum_mode = (int)line->spectrum_mode;
    safe_line.center_freq = (double)line->center_freq;
    safe_line.span_freq = (double)line->span_freq;
    safe_line.low_edge_freq = (double)line->low_edge_freq;
    safe_line.high_edge_freq = (double)line->high_edge_freq;
    safe_line.data_length = (int)((line->spectrum_data_length > sizeof(safe_line.data)) ? sizeof(safe_line.data) : line->spectrum_data_length);
    if (safe_line.data_length > 0 && line->spectrum_data) {
        memcpy(safe_line.data, line->spectrum_data, (size_t)safe_line.data_length);
    }

    return adapter->user_cb((void*)rig, &safe_line, adapter->user_arg);
}

SHIM_API int shim_rig_set_freq_callback(hamlib_shim_handle_t h, shim_freq_cb_t cb, void* arg) {
    freq_cb_adapter.user_cb = cb;
    freq_cb_adapter.user_arg = arg;
    return rig_set_freq_callback((RIG*)h, shim_freq_cb_thunk, &freq_cb_adapter);
}

SHIM_API int shim_rig_set_ptt_callback(hamlib_shim_handle_t h, shim_ptt_cb_t cb, void* arg) {
    ptt_cb_adapter.user_cb = cb;
    ptt_cb_adapter.user_arg = arg;
    return rig_set_ptt_callback((RIG*)h, shim_ptt_cb_thunk, &ptt_cb_adapter);
}

SHIM_API int shim_rig_set_spectrum_callback(hamlib_shim_handle_t h, shim_spectrum_cb_t cb, void* arg) {
    spectrum_cb_adapter.user_cb = cb;
    spectrum_cb_adapter.user_arg = arg;
    return rig_set_spectrum_callback((RIG*)h, cb ? shim_spectrum_cb_thunk : NULL, cb ? &spectrum_cb_adapter : NULL);
}

/* rig_set_trn: deprecated in Hamlib 4.7.0, may be removed in future versions. */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
SHIM_API int shim_rig_set_trn(hamlib_shim_handle_t h, int trn) {
    return rig_set_trn((RIG*)h, trn);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/* ===== Capability queries ===== */

SHIM_API uint64_t shim_rig_get_mode_list(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (uint64_t)(rig->state.mode_list);
}

SHIM_API uint64_t shim_rig_get_caps_has_get_level(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (uint64_t)(rig->caps->has_get_level);
}

SHIM_API uint64_t shim_rig_get_caps_has_set_level(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (uint64_t)(rig->caps->has_set_level);
}

SHIM_API uint64_t shim_rig_get_caps_has_get_func(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (uint64_t)(rig->caps->has_get_func);
}

SHIM_API uint64_t shim_rig_get_caps_has_set_func(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    return (uint64_t)(rig->caps->has_set_func);
}

SHIM_API int shim_rig_is_async_data_supported(hamlib_shim_handle_t h) {
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps) return 0;
    return rig->caps->async_data_supported ? 1 : 0;
}

SHIM_API int shim_rig_get_caps_spectrum_scope_count(hamlib_shim_handle_t h) {
    int count = 0;
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps) return 0;
    while (count < HAMLIB_MAX_SPECTRUM_SCOPES && rig->caps->spectrum_scopes[count].name) {
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_caps_spectrum_scope(hamlib_shim_handle_t h, int index, shim_spectrum_scope_t* out) {
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps || !out || index < 0 || index >= HAMLIB_MAX_SPECTRUM_SCOPES) return -RIG_EINVAL;
    if (!rig->caps->spectrum_scopes[index].name) return -RIG_ENAVAIL;
    memset(out, 0, sizeof(*out));
    out->id = rig->caps->spectrum_scopes[index].id;
    strncpy(out->name, rig->caps->spectrum_scopes[index].name, sizeof(out->name) - 1);
    return RIG_OK;
}

SHIM_API int shim_rig_get_caps_spectrum_mode_count(hamlib_shim_handle_t h) {
    int count = 0;
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps) return 0;
    while (count < HAMLIB_MAX_SPECTRUM_MODES && rig->caps->spectrum_modes[count] != RIG_SPECTRUM_MODE_NONE) {
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_caps_spectrum_mode(hamlib_shim_handle_t h, int index, int* out) {
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps || !out || index < 0 || index >= HAMLIB_MAX_SPECTRUM_MODES) return -RIG_EINVAL;
    if (rig->caps->spectrum_modes[index] == RIG_SPECTRUM_MODE_NONE) return -RIG_ENAVAIL;
    *out = (int)rig->caps->spectrum_modes[index];
    return RIG_OK;
}

SHIM_API int shim_rig_get_caps_spectrum_span_count(hamlib_shim_handle_t h) {
    int count = 0;
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps) return 0;
    while (count < HAMLIB_MAX_SPECTRUM_SPANS && rig->caps->spectrum_spans[count] != 0) {
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_caps_spectrum_span(hamlib_shim_handle_t h, int index, double* out) {
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps || !out || index < 0 || index >= HAMLIB_MAX_SPECTRUM_SPANS) return -RIG_EINVAL;
    if (rig->caps->spectrum_spans[index] == 0) return -RIG_ENAVAIL;
    *out = (double)rig->caps->spectrum_spans[index];
    return RIG_OK;
}

SHIM_API int shim_rig_get_caps_spectrum_avg_mode_count(hamlib_shim_handle_t h) {
    int count = 0;
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps) return 0;
    while (count < HAMLIB_MAX_SPECTRUM_AVG_MODES && rig->caps->spectrum_avg_modes[count].name) {
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_caps_spectrum_avg_mode(hamlib_shim_handle_t h, int index, shim_spectrum_avg_mode_t* out) {
    RIG* rig = (RIG*)h;
    if (!rig || !rig->caps || !out || index < 0 || index >= HAMLIB_MAX_SPECTRUM_AVG_MODES) return -RIG_EINVAL;
    if (!rig->caps->spectrum_avg_modes[index].name) return -RIG_ENAVAIL;
    memset(out, 0, sizeof(*out));
    out->id = rig->caps->spectrum_avg_modes[index].id;
    strncpy(out->name, rig->caps->spectrum_avg_modes[index].name, sizeof(out->name) - 1);
    return RIG_OK;
}

SHIM_API const char* shim_rig_str_spectrum_mode(int mode) {
    return rig_strspectrummode((enum rig_spectrum_mode_e)mode);
}

SHIM_API int shim_rig_sprintf_mode(uint64_t modes, char* buf, int buflen) {
    if (!buf || buflen <= 0) return 0;
    buf[0] = '\0';
    int pos = 0;
    uint64_t mode;
    for (mode = 1; mode != 0 && pos < buflen - 1; mode <<= 1) {
        if (modes & mode) {
            const char* name = rig_strrmode((rmode_t)mode);
            if (name && name[0] != '?') {
                int len = (int)strlen(name);
                if (pos + len + 1 < buflen) {
                    if (pos > 0) buf[pos++] = ' ';
                    memcpy(buf + pos, name, len);
                    pos += len;
                }
            }
        }
    }
    buf[pos] = '\0';
    return pos;
}

/* ===== Rig type to string ===== */

SHIM_API const char* shim_rig_type_str(int rig_type) {
    switch (rig_type & RIG_TYPE_MASK) {
        case RIG_TYPE_TRANSCEIVER: return "Transceiver";
        case RIG_TYPE_HANDHELD:    return "Handheld";
        case RIG_TYPE_MOBILE:      return "Mobile";
        case RIG_TYPE_RECEIVER:    return "Receiver";
        case RIG_TYPE_PCRECEIVER:  return "PC Receiver";
        case RIG_TYPE_SCANNER:     return "Scanner";
        case RIG_TYPE_TRUNKSCANNER: return "Trunk Scanner";
        case RIG_TYPE_COMPUTER:    return "Computer";
        case RIG_TYPE_OTHER:       return "Other";
        default:                   return "Unknown";
    }
}

/* ===== Lock Mode (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_set_lock_mode(hamlib_shim_handle_t h, int lock) {
#ifdef SHIM_HAS_LOCK_MODE
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    return rig_set_lock_mode(rig, lock);
#else
    (void)h; (void)lock;
    return -RIG_ENIMPL;
#endif
}

SHIM_API int shim_rig_get_lock_mode(hamlib_shim_handle_t h, int *lock) {
#ifdef SHIM_HAS_LOCK_MODE
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    return rig_get_lock_mode(rig, lock);
#else
    (void)h; (void)lock;
    return -RIG_ENIMPL;
#endif
}

/* ===== Clock (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_set_clock(hamlib_shim_handle_t h, int year, int month, int day,
                                 int hour, int min, int sec, double msec, int utc_offset) {
#ifdef SHIM_HAS_CLOCK
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    return rig_set_clock(rig, year, month, day, hour, min, sec, msec, utc_offset);
#else
    (void)h; (void)year; (void)month; (void)day;
    (void)hour; (void)min; (void)sec; (void)msec; (void)utc_offset;
    return -RIG_ENIMPL;
#endif
}

SHIM_API int shim_rig_get_clock(hamlib_shim_handle_t h, int *year, int *month, int *day,
                                 int *hour, int *min, int *sec, double *msec, int *utc_offset) {
#ifdef SHIM_HAS_CLOCK
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    return rig_get_clock(rig, year, month, day, hour, min, sec, msec, utc_offset);
#else
    (void)h; (void)year; (void)month; (void)day;
    (void)hour; (void)min; (void)sec; (void)msec; (void)utc_offset;
    return -RIG_ENIMPL;
#endif
}

/* ===== VFO Info (Hamlib >= 4.7.0) ===== */

SHIM_API int shim_rig_get_vfo_info(hamlib_shim_handle_t h, int vfo,
                                    double *freq, uint64_t *mode,
                                    long *width, int *split, int *satmode) {
#ifdef SHIM_HAS_VFO_INFO
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    freq_t f = 0;
    rmode_t m = 0;
    pbwidth_t w = 0;
    split_t s = RIG_SPLIT_OFF;
    int sm = 0;
    int ret = rig_get_vfo_info(rig, (vfo_t)vfo, &f, &m, &w, &s, &sm);
    if (freq) *freq = (double)f;
    if (mode) *mode = (uint64_t)m;
    if (width) *width = (long)w;
    if (split) *split = (int)s;
    if (satmode) *satmode = sm;
    return ret;
#else
    (void)h; (void)vfo; (void)freq; (void)mode; (void)width; (void)split; (void)satmode;
    return -RIG_ENIMPL;
#endif
}

/* ===== Rig info / Raw / Conf ===== */

SHIM_API const char* shim_rig_get_info(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    if (!rig) return "";
    const char *info = rig_get_info(rig);
    return info ? info : "";
}

SHIM_API int shim_rig_send_raw(hamlib_shim_handle_t h,
    const unsigned char* send, int send_len,
    unsigned char* reply, int reply_len, const unsigned char* term) {
#ifdef SHIM_HAS_SEND_RAW
    RIG *rig = (RIG *)h;
    if (!rig) return -RIG_EINVAL;
    return rig_send_raw(rig, send, send_len, reply, reply_len, (unsigned char*)term);
#else
    (void)h; (void)send; (void)send_len; (void)reply; (void)reply_len; (void)term;
    return -RIG_ENIMPL;
#endif
}

SHIM_API int shim_rig_set_conf(hamlib_shim_handle_t h, const char* name, const char* val) {
    RIG *rig = (RIG *)h;
    if (!rig || !name || !val) return -RIG_EINVAL;
    token_t token = rig_token_lookup(rig, name);
    if (token == 0) return -RIG_EINVAL;
    return rig_set_conf(rig, token, val);
}

SHIM_API int shim_rig_get_conf(hamlib_shim_handle_t h, const char* name, char* buf, int buflen) {
    RIG *rig = (RIG *)h;
    if (!rig || !name || !buf || buflen <= 0) return -RIG_EINVAL;
    token_t token = rig_token_lookup(rig, name);
    if (token == 0) return -RIG_EINVAL;
#ifdef SHIM_HAS_CONF2
    return rig_get_conf2(rig, token, buf, buflen);
#else
    return rig_get_conf(rig, token, buf);
#endif
}

/* ===== Passband / Resolution ===== */

SHIM_API int shim_rig_passband_normal(hamlib_shim_handle_t h, int mode) {
    return (int)rig_passband_normal((RIG*)h, (rmode_t)mode);
}

SHIM_API int shim_rig_get_resolution(hamlib_shim_handle_t h, int mode) {
    RIG *rig = (RIG *)h;
    if (!rig) return 0;
    return (int)rig_get_resolution(rig, (rmode_t)mode);
}

/* ===== Capability queries (parm / vfo_ops / scan) ===== */

SHIM_API uint64_t shim_rig_get_caps_has_get_parm(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    return (uint64_t)(rig->caps->has_get_parm);
}

SHIM_API uint64_t shim_rig_get_caps_has_set_parm(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    return (uint64_t)(rig->caps->has_set_parm);
}

SHIM_API int shim_rig_get_caps_vfo_ops(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    return (int)(rig->caps->vfo_ops);
}

SHIM_API int shim_rig_get_caps_has_scan(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    return (int)(rig->caps->scan_ops);
}

/* ===== Capability Query: Preamp/Attenuator/Max values ===== */

SHIM_API int shim_rig_get_caps_preamp(hamlib_shim_handle_t h, int* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_MAXDBLSTSIZ && rig->caps->preamp[i] != 0 && count < max_count; i++) {
        out[count++] = rig->caps->preamp[i];
    }
    return count;
}

SHIM_API int shim_rig_get_caps_attenuator(hamlib_shim_handle_t h, int* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_MAXDBLSTSIZ && rig->caps->attenuator[i] != 0 && count < max_count; i++) {
        out[count++] = rig->caps->attenuator[i];
    }
    return count;
}

SHIM_API int shim_rig_get_caps_agc_levels(hamlib_shim_handle_t h, int* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;

    int count = 0;
    if (rig->caps->agc_level_count > 0) {
        for (int i = 0; i < rig->caps->agc_level_count && count < max_count; i++) {
            out[count++] = rig->caps->agc_levels[i];
        }
        return count;
    }

    /* Backwards-compatibility: Hamlib documents agc_level_count=0 as "all standard modes". */
    static const int default_agc_levels[] = {
        RIG_AGC_OFF,
        RIG_AGC_SUPERFAST,
        RIG_AGC_FAST,
        RIG_AGC_SLOW,
        RIG_AGC_USER,
        RIG_AGC_MEDIUM,
        RIG_AGC_AUTO,
        RIG_AGC_LONG,
        RIG_AGC_ON,
    };
    const int default_count = (int)(sizeof(default_agc_levels) / sizeof(default_agc_levels[0]));
    for (int i = 0; i < default_count && count < max_count; i++) {
        out[count++] = default_agc_levels[i];
    }
    return count;
}

SHIM_API long shim_rig_get_caps_max_rit(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    if (!rig) return 0;
    return (long)rig->caps->max_rit;
}

SHIM_API long shim_rig_get_caps_max_xit(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    if (!rig) return 0;
    return (long)rig->caps->max_xit;
}

SHIM_API long shim_rig_get_caps_max_ifshift(hamlib_shim_handle_t h) {
    RIG *rig = (RIG *)h;
    if (!rig) return 0;
    return (long)rig->caps->max_ifshift;
}

/* ===== Capability Query: CTCSS/DCS lists ===== */

SHIM_API int shim_rig_get_caps_ctcss_list(hamlib_shim_handle_t h, unsigned int* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    if (!rig->caps->ctcss_list) return 0;
    int count = 0;
    for (int i = 0; rig->caps->ctcss_list[i] != 0 && count < max_count; i++) {
        out[count++] = rig->caps->ctcss_list[i];
    }
    return count;
}

SHIM_API int shim_rig_get_caps_dcs_list(hamlib_shim_handle_t h, unsigned int* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    if (!rig->caps->dcs_list) return 0;
    int count = 0;
    for (int i = 0; rig->caps->dcs_list[i] != 0 && count < max_count; i++) {
        out[count++] = rig->caps->dcs_list[i];
    }
    return count;
}

/* ===== Capability Query: Frequency ranges ===== */

SHIM_API int shim_rig_get_rx_range(hamlib_shim_handle_t h, shim_freq_range_t* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_FRQRANGESIZ && count < max_count; i++) {
        const freq_range_t *r = &rig->state.rx_range_list[i];
        if (r->startf == 0 && r->endf == 0) break;
        out[count].start_freq = (double)r->startf;
        out[count].end_freq = (double)r->endf;
        out[count].modes = (uint64_t)r->modes;
        out[count].low_power = (int)r->low_power;
        out[count].high_power = (int)r->high_power;
        out[count].vfo = (int)r->vfo;
        out[count].ant = (int)r->ant;
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_tx_range(hamlib_shim_handle_t h, shim_freq_range_t* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_FRQRANGESIZ && count < max_count; i++) {
        const freq_range_t *r = &rig->state.tx_range_list[i];
        if (r->startf == 0 && r->endf == 0) break;
        out[count].start_freq = (double)r->startf;
        out[count].end_freq = (double)r->endf;
        out[count].modes = (uint64_t)r->modes;
        out[count].low_power = (int)r->low_power;
        out[count].high_power = (int)r->high_power;
        out[count].vfo = (int)r->vfo;
        out[count].ant = (int)r->ant;
        count++;
    }
    return count;
}

/* ===== Capability Query: Tuning steps / Filters ===== */

SHIM_API int shim_rig_get_caps_tuning_steps(hamlib_shim_handle_t h, shim_mode_value_t* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_TSLSTSIZ && count < max_count; i++) {
        if (rig->caps->tuning_steps[i].modes == 0) break;
        out[count].modes = (uint64_t)rig->caps->tuning_steps[i].modes;
        out[count].value = (int)rig->caps->tuning_steps[i].ts;
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_caps_filters(hamlib_shim_handle_t h, shim_mode_value_t* out, int max_count) {
    RIG *rig = (RIG *)h;
    if (!rig || !out || max_count <= 0) return 0;
    int count = 0;
    for (int i = 0; i < HAMLIB_FLTLSTSIZ && count < max_count; i++) {
        if (rig->caps->filters[i].modes == 0) break;
        out[count].modes = (uint64_t)rig->caps->filters[i].modes;
        out[count].value = (int)rig->caps->filters[i].width;
        count++;
    }
    return count;
}

SHIM_API int shim_rig_get_level_granularity(hamlib_shim_handle_t h, uint64_t level, shim_granularity_t* out) {
    RIG *rig = (RIG *)h;
    struct rig_state *state = NULL;
    setting_t setting = (setting_t)level;
    int index;
    gran_t gran;

    if (!rig || !out) return -RIG_EINVAL;
    state = HAMLIB_STATE(rig);
    if (!state) return -RIG_EINVAL;

    memset(out, 0, sizeof(*out));
    index = shim_setting_to_index(setting);
    if (index < 0 || index >= RIG_SETTING_MAX) return -RIG_EINVAL;

    gran = state->level_gran[index];
    out->is_float = RIG_LEVEL_IS_FLOAT(setting) ? 1 : 0;

    if (out->is_float) {
        out->min_value = (double)gran.min.f;
        out->max_value = (double)gran.max.f;
        out->step_value = (double)gran.step.f;
        out->is_defined = (gran.min.f != 0.0f || gran.max.f != 0.0f || gran.step.f != 0.0f) ? 1 : 0;
        if (out->is_defined && gran.min.f == 0.0f && gran.max.f == 0.0f && gran.step.f != 0.0f) {
            out->max_value = 1.0;
        }
    } else {
        out->min_value = (double)gran.min.i;
        out->max_value = (double)gran.max.i;
        out->step_value = (double)gran.step.i;
        out->is_defined = (gran.min.i != 0 || gran.max.i != 0 || gran.step.i != 0) ? 1 : 0;
    }

    return SHIM_RIG_OK;
}

SHIM_API int shim_rig_get_rfpower_metadata(hamlib_shim_handle_t h, int* out_current, int* out_min, int* out_max) {
    RIG *rig = (RIG *)h;
    struct rig_state *state = NULL;
    if (!rig) return -RIG_EINVAL;
    state = HAMLIB_STATE(rig);
    if (!state) return -RIG_EINVAL;
    if (out_current) *out_current = state->power_now;
    if (out_min) *out_min = state->power_min;
    if (out_max) *out_max = state->power_max;
    return SHIM_RIG_OK;
}

/* ===== Static info ===== */

SHIM_API const char* shim_rig_copyright(void) {
    return rig_copyright();
}

SHIM_API const char* shim_rig_license(void) {
    return rig_license();
}
