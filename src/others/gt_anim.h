/**
 * @file gt_anim.h
 * @author Feyoung
 * @brief Animation-related logic implementation
 * @version 0.1
 * @date 2022-06-21 17:59:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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
 * param 1: the object of animate
 * param 2: the value to set
 */
typedef void ( * gt_anim_exec_cb_t)(gt_obj_st *, int32_t);
typedef void ( * gt_anim_ready_cb_t)(struct gt_anim_s *);
typedef void ( * gt_anim_start_cb_t)(struct gt_anim_s *);
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
    struct gt_list_head list;       /* Do not modified it, Core need to use it */
    gt_obj_st * target;             // Target object
    gt_anim_exec_cb_t exec_cb;      // The path execution callback which object run
    gt_anim_ready_cb_t ready_cb;    // When the animation done need to be executed
    gt_anim_start_cb_t start_cb;    // When the animation starts need to be executed
    gt_anim_path_cb_t _path_cb; /* Warn: animation system used - user should not modified it */
    void * data;

    int32_t tick_create;        // create this animation tick timer
    int32_t time_delay_start;   // The animation is executed after this delay [ms]
    int32_t time;               // remark animation running total time [ms]
    int32_t time_act;           // remark active animation time [0 -> time ms]

    int32_t value_start;        // The status value from the beginning
    int32_t value_current;      // The current value between start and end
    int32_t value_end;          // The status value to the end

    int32_t playback_time;      // Playback time [ms]
    uint32_t playback_delay;    // Delay ms to playback animation [ms]
    uint32_t repeat_delay;      // The time to repeat playback animation [ms]
    uint16_t repeat_count;      // The number of times to repeat playback animation
    gt_anim_path_type_em type;  // The path which is animation object to run

    /* Warn: anim core system used these - user should not modified it */
    uint8_t run_already     : 1;
    uint8_t playback_status : 1;
}gt_anim_st;


/* macros ---------------------------------------------------------------*/

/**
 * @brief Set animation which widget wants to animate.
 *
 * @param anim animation description
 * @param target widget object
 */
static inline void gt_anim_set_target(gt_anim_st * anim, gt_obj_st * const target) {
    anim->target = target;
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
    anim->time = enabled ? 1 : 0;
}

static inline void gt_anim_set_start_cb(gt_anim_st * anim, gt_anim_start_cb_t start_cb) {
    anim->start_cb = start_cb;
}

static inline void gt_anim_set_ready_cb(gt_anim_st * anim, gt_anim_ready_cb_t ready_cb) {
    anim->ready_cb = ready_cb;
}

static inline void gt_anim_set_exec_cb(gt_anim_st * anim, gt_anim_exec_cb_t exec_cb) {
    anim->exec_cb = exec_cb;
}

/**
 * @brief Set User Animation data
 *
 * @param anim
 * @param data
 */
static inline void gt_anim_set_data(gt_anim_st * anim, void * data) {
    anim->data = data;
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
 */
void gt_anim_start(const gt_anim_st * anim);

/**
 * @brief delete animation object in core
 *
 * @param target Which animation object want to be deleted
 * @param exec_cb Which animation object exec callback function want to be deleted
 * @return true Delete animation object successfully
 * @return False Delete animation object failed
 */
bool gt_anim_del(gt_obj_st * target, gt_anim_exec_cb_t exec_cb);

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
