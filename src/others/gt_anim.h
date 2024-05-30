/**
 * @file gt_anim.h
 * @author Feyoung
 * @brief Animation-related logic implementation
 * @version 0.1
 * @date 2022-06-21 17:59:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ANIM_H_
#define _GT_ANIM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdbool.h"
#include "./gt_list.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_types.h"
#include "../core/gt_mem.h"


/* define ---------------------------------------------------------------*/
#define GT_ANIM_REPEAT_INFINITE     (0xFFFF)
#define GT_ANIM_PLAYTIME_INFINITE   (0xFFFFFFFF)


/* typedef --------------------------------------------------------------*/
/**
 * @brief anim path type enum
 */
typedef enum {
    GT_ANIM_PATH_TYPE_LINEAR = 0,   /* default type/mode */
    GT_ANIM_PATH_TYPE_EASE_IN,
    GT_ANIM_PATH_TYPE_EASE_OUT,
    GT_ANIM_PATH_TYPE_EASE_IN_OUT,
    GT_ANIM_PATH_TYPE_OVERSHOOT,
    GT_ANIM_PATH_TYPE_BOUNCE,
    GT_ANIM_PATH_TYPE_STEP,

    /* get path type max count */
    GT_ANIM_PATH_TYPE_COUNT,
}gt_anim_path_type_em;


struct gt_anim_s;

typedef int32_t ( * gt_anim_path_cb_t)(const struct gt_anim_s *);


/**
 * @brief animation trace
 * @param value1 the object of animate
 * @param value2 the value to set
 */
typedef void ( * gt_anim_exec_cb_t)(void *, int32_t);

/**
 * @brief The event is fired when the animation is complete
 */
typedef void ( * gt_anim_ready_cb_t)(struct gt_anim_s *);

/**
 * @brief The anim object is before deleted handler
 */
typedef void ( * gt_anim_deleted_cb_t)(struct gt_anim_s *);

/**
 * @brief The event is fired when the animation is start
 */
typedef void ( * gt_anim_start_cb_t)(struct gt_anim_s *);

/**
 * @brief Get anim current timestamp value
 */
typedef int32_t ( * gt_anim_get_value_cb_t)(struct gt_anim_s *);

/**
 * @brief animation param data
 * User defined parameters @ref gt_anim_pos_move()
 */
typedef struct gt_anim_param_s {
    gt_point_st dst;            // destination position
    int32_t time_delay_start;   // The animation is executed after this delay [ms]
    int32_t time_anim_exec;     // Animation execution time [ms]
    gt_anim_path_type_em type;  // path type
}gt_anim_param_st;

/**
 * @brief anim control block
 * [Core using: User do not modified]
 */
typedef struct gt_anim_s {
    struct _gt_list_head list;          /* Do not modified it, Core need to use it */
    void * tar;                         // Target object
    gt_anim_exec_cb_t exec_cb;          // The path execution callback which object run
    gt_anim_start_cb_t start_cb;        // When the animation starts need to be executed
    gt_anim_ready_cb_t ready_cb;        // When the animation done need to be executed
    gt_anim_deleted_cb_t deleted_cb;    // When the animation object is deleted
    gt_anim_path_cb_t _path_cb;         /* Warn: animation system used - user should not modified it */

    void * data;

    int32_t tick_create;        // [Tick Timestamp] create this animation tick timer
    int32_t time_delay_start;   // The animation is executed after this delay [ms]
    int32_t time;               // remark animation running total time [ms]
    int32_t time_act;           // remark active animation time [0 -> time ms]

    int32_t value_start;        // The status value from the beginning
    int32_t value_current;      // The current value between start and end
    int32_t value_end;          // The status value to the end

    int32_t playback_time;      // Playback time [ms]
    uint32_t playback_delay;    // Delay ms to playback animation [ms]
    uint32_t repeat_delay;      // The time to repeat playback animation [ms]
    uint16_t repeat_count;      // The number of times to repeat playback animation, infinite: GT_ANIM_REPEAT_INFINITE
    gt_anim_path_type_em type;  // The path which is animation object to run

    /* Warn: anim core system used these - user should not modified it */
    uint8_t run_already : 1;    // Determines whether the current moment has been executed
    uint8_t invert      : 1;    // remark playback current status [0: forward, 1: backward]
    uint8_t playback    : 1;    // TODO enabled playback
    uint8_t paused      : 1;    // paused status
    uint8_t reserved    : 4;
}gt_anim_st;


/* macros ---------------------------------------------------------------*/

/**
 * @brief Set animation which widget wants to animate.
 *
 * @param anim animation description
 * @param target widget object
 */
static inline void gt_anim_set_target(gt_anim_st * anim, void * const target) {
    anim->tar = target;
}

static inline void gt_anim_set_time_delay_start(gt_anim_st * anim, int32_t time_ms) {
    anim->time_delay_start = time_ms;
}

static inline void gt_anim_set_time(gt_anim_st * anim, int32_t time_ms) {
    anim->time = time_ms;
}

static inline void gt_anim_set_value(gt_anim_st * anim, int32_t start, int32_t end) {
    anim->value_start = start;
    anim->value_end   = end;
}

static inline void gt_anim_set_playback(gt_anim_st * anim, bool enabled) {
    anim->playback = enabled ? 1 : 0;
}

/**
 * @brief
 *
 * @param anim
 * @param count Default: 0, infinite: GT_ANIM_REPEAT_INFINITE
 */
static inline void gt_anim_set_repeat_count(gt_anim_st * anim, uint16_t count) {
    anim->repeat_count = count;
}

/**
 * @brief
 *
 * @param anim
 * @param delay Set delay time to playback animation [ms]
 */
static inline void gt_anim_set_repeat_delay(gt_anim_st * anim, uint32_t delay) {
    anim->repeat_delay = delay;
}

static inline void gt_anim_set_paused(gt_anim_st * anim, bool paused) {
    anim->paused = paused ? 1 : 0;
}

static inline bool gt_anim_is_paused(const gt_anim_st * anim) {
    return anim->paused;
}

static inline void gt_anim_set_start_cb(gt_anim_st * anim, gt_anim_start_cb_t start_cb) {
    anim->start_cb = start_cb;
}

static inline void gt_anim_set_exec_cb(gt_anim_st * anim, gt_anim_exec_cb_t exec_cb) {
    anim->exec_cb = exec_cb;
}

static inline void gt_anim_set_ready_cb(gt_anim_st * anim, gt_anim_ready_cb_t ready_cb) {
    anim->ready_cb = ready_cb;
}

static inline void gt_anim_set_deleted_cb(gt_anim_st * anim, gt_anim_deleted_cb_t deleted_cb) {
    anim->deleted_cb = deleted_cb;
}

/**
 * @brief Set User Animation data
 *
 * @param anim
 * @param data
 */
static inline void gt_anim_set_data(gt_anim_st * anim, void * data, gt_size_t size) {
    if (NULL == data || 0 == size) {
        return ;
    }
    anim->data = anim->data ? gt_mem_realloc(anim->data, size) : gt_mem_malloc(size);
    if (NULL == anim->data) {
        return ;
    }
    gt_memcpy(anim->data, data, size);
}

/* =================== Movement control of Widgets =================== */

/**
 * @brief Set Object animation param time
 *
 * @param param
 * @param time_delay_start The time which delay to start animation [ms]
 * @param time_anim  The time which animation playing the whole time
 */
static inline void gt_anim_param_set_time(gt_anim_param_st * param, int32_t time_delay_start_ms, int32_t time_anim_ms) {
    param->time_delay_start = time_delay_start_ms;
    param->time_anim_exec = time_anim_ms;
}

/**
 * @brief Set Object animation param, which position the object want to move.
 *
 * @param param
 * @param x The destination x position
 * @param y The destination y position
 */
static inline void gt_anim_param_set_dst_point(gt_anim_param_st * param, gt_size_t x, gt_size_t y) {
    param->dst.x = x;
    param->dst.y = y;
}

/**
 * @brief Set object path type animation param
 *
 * @param param
 * @param type @ref gt_anim_path_type_em
 */
static inline void gt_anim_param_set_path_type(gt_anim_param_st * param, gt_anim_path_type_em type) {
    param->type = type;
}
/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief
 *
 * @param speed
 * @param start
 * @param end
 * @return uint32_t [ms] The time which animation playing the whole time
 */
uint32_t gt_anim_speed_to_time(uint32_t speed, int32_t start, int32_t end);

/**
 * @brief Get User animation Data
 *
 * @return void* The data
 */
void * gt_anim_get_data(void);

/**
 * @brief The animation base value init
 *
 * @param param The pointer to Param
 */
void gt_anim_param_init(gt_anim_param_st * param);

/**
 * @brief Set the widget want to move a new position animation
 *
 * @param obj
 * @param param The param which describe object animation
 */
void gt_anim_pos_move(gt_obj_st * obj, gt_anim_param_st * param);

/**
 * @brief [only kernel calling] initialize the animation timer by core
 */
void _gt_anim_core_init(void);

/**
 * @brief initialize the animation object
 *
 * @param anim
 */
void gt_anim_init(gt_anim_st * anim);

/**
 * @brief Which animation object start to run
 *
 * @param anim The animation object
 * @return gt_anim_st* The animation object which is insert to core
 */
gt_anim_st * gt_anim_start(const gt_anim_st * anim);

/**
 * @brief Reset the animation object create tick to restart to run
 *
 * @param anim The animation object
 */
void gt_anim_restart(gt_anim_st * anim);

/**
 * @brief delete animation object in core
 *
 * @param target Which animation object want to be deleted
 * @param exec_cb Which animation object exec callback function want to be deleted
 * @return true Delete animation object successfully
 * @return False Delete animation object failed
 */
bool gt_anim_del(void const * const target, gt_anim_exec_cb_t exec_cb);

/**
 * @brief delete animation object in core
 *
 * @param anim Which animation object want to be deleted
 * @return true Delete animation object successfully
 * @return False Delete animation object failed
 */
bool gt_anim_del_by(gt_anim_st * anim);

/**
 * @brief Delete all animation object
 */
void gt_anim_del_all(void);

/**
 * @brief Set the animation object path type
 *
 * @param anim Which animation object
 * @param type @ref gt_anim_path_type_em
 */
void gt_anim_set_path_type(gt_anim_st * anim, gt_anim_path_type_em type);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ANIM_H_
