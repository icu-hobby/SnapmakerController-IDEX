#include "event_system.h"
#include "event_base.h"
#include "subscribe.h"
#include "../module/fdm.h"
#include "../module/bed_control.h"
#include "../module/system.h"
#include "../module/motion_control.h"

static ErrCode subscribe_event(event_param_t& event) {
  event.data[0] = subscribe.enable(event);
  event.length = 1;
  return send_event(event);
}

static ErrCode unsubscribe_event(event_param_t& event) {
  event.data[0] = subscribe.disable(event);
  event.length = 1;
  return send_event(event);
}

static ErrCode set_log_grade(event_param_t& event) {
  return E_SUCCESS;
}

static ErrCode req_protocol_ver(event_param_t& event) {
  return E_SUCCESS;
}

static ErrCode set_debug_mode(event_param_t& event) {
  return E_SUCCESS;
}

static ErrCode heart_event(event_param_t& event) {
  event.data[0] = E_SUCCESS;
  event.data[1] = system_service.get_status();
  event.length = 2;
  send_event(event);
  return E_SUCCESS;
}

static ErrCode retport_log(event_param_t& event) {
  return E_SUCCESS;
}


static ErrCode req_module_info(event_param_t& event) {
  uint8_t *array_count = &event.data[1];
  module_info_t *module_info = (module_info_t *)(event.data + 2);
  event.data[0] = E_SUCCESS;

  uint8_t index = 0;
  fdm_head.get_module_info(0, module_info[index++]);
  fdm_head.get_module_info(1, module_info[index++]);
  bed_control.get_module_info(module_info[index++]);
  *array_count = index;
  event.length = index * sizeof(module_info_t) + 2;
  send_event(event);
  return E_SUCCESS;
}

static ErrCode req_machine_info(event_param_t& event) {
  machine_info_t *machine_info = (machine_info_t *)(event.data + 1);
  event.data[0] = E_SUCCESS;
  system_service.get_machine_info(machine_info);
  event.length = sizeof(machine_info_t) + 1;
  send_event(event);
  return E_SUCCESS;
}

static ErrCode req_coordinate_system(event_param_t& event) {
  coordinate_system_t * info = (coordinate_system_t *)(event.data + 1);
  event.data[0] = E_SUCCESS;
  system_service.get_coordinate_system_info(info);
  event.length = sizeof(coordinate_system_t);
  send_event(event);
  return E_SUCCESS;
}

static ErrCode set_coordinate_system(event_param_t& event) {
  return E_SUCCESS;
}

static ErrCode set_origin(event_param_t& event) {
  return E_SUCCESS;
}

static ErrCode move_relative(event_param_t& event) {
  mobile_instruction_t *move = (mobile_instruction_t *)(event.data + 1);
  motion_control.move_axis(*move);
  event.data[0] = E_SUCCESS;
  event.length = 1;
  return send_event(event);
}

static ErrCode move(event_param_t& event) {
  mobile_instruction_t *move = (mobile_instruction_t *)(event.data + 1);
  motion_control.move_axis_to(*move);
  event.data[0] = E_SUCCESS;
  event.length = 1;
  return send_event(event);
}

static ErrCode home(event_param_t& event) {
  motion_control.home();
  event.data[0] = E_SUCCESS;
  event.length = 1;
  return send_event(event);
}

event_cb_info_t system_cb_info[SYS_ID_CB_COUNT] = {
  {SYS_ID_SUBSCRIBE             , EVENT_CB_DIRECT_RUN, subscribe_event},
  {SYS_ID_UNSUBSCRIBE           , EVENT_CB_DIRECT_RUN, unsubscribe_event},
  {SYS_ID_SET_LOG_GRADE         , EVENT_CB_DIRECT_RUN, set_log_grade},
  {SYS_ID_REQ_PROTOCOL_VER      , EVENT_CB_DIRECT_RUN, req_protocol_ver},
  {SYS_ID_SET_DEBUG_MODE        , EVENT_CB_DIRECT_RUN, set_debug_mode},
  {SYS_ID_HEARTBEAT             , EVENT_CB_DIRECT_RUN, heart_event},
  {SYS_ID_REPORT_LOG            , EVENT_CB_DIRECT_RUN, retport_log},
  {SYS_ID_REQ_MODULE_INFO       , EVENT_CB_DIRECT_RUN, req_module_info},
  {SYS_ID_REQ_MACHINE_INFO      , EVENT_CB_DIRECT_RUN, req_machine_info},
  {SYS_ID_REQ_COORDINATE_SYSTEM , EVENT_CB_DIRECT_RUN, req_coordinate_system},
  {SYS_ID_SET_COORDINATE_SYSTEM , EVENT_CB_DIRECT_RUN, set_coordinate_system},
  {SYS_ID_SET_ORIGIN            , EVENT_CB_DIRECT_RUN, set_origin},
  {SYS_ID_MOVE_RELATIVE         , EVENT_CB_TASK_RUN  , move_relative},
  {SYS_ID_MOVE                  , EVENT_CB_TASK_RUN  , move},
  {SYS_ID_HOME                  , EVENT_CB_TASK_RUN  , home},
};