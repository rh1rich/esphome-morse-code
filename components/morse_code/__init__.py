import logging
import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import automation
from esphome.components.output import BinaryOutput

from esphome.const import (
    CONF_ID,
    CONF_OUTPUT,
    CONF_TRIGGER_ID,
    CONF_TEXT,
)

CODEOWNERS = ["@rh1rich"]

MULTI_CONF = True

CONF_MORSE_CODE = "morse_code"
CONF_DIT_DURATION = "dit_duration"
CONF_ON_FINISHED = "on_finished"

morse_code_ns = cg.esphome_ns.namespace("morse_code")

MorseCodeComponent = morse_code_ns.class_("MorseCode", cg.Component)

StartAction = morse_code_ns.class_("StartAction", automation.Action)
StopAction = morse_code_ns.class_("StopAction", automation.Action)

FinishedTrigger = morse_code_ns.class_(
    "FinishedTrigger", automation.Trigger.template()
)

IsRunningCondition = morse_code_ns.class_("IsRunningCondition", automation.Condition)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(MorseCodeComponent),
            cv.Optional(CONF_OUTPUT): cv.use_id(BinaryOutput),
            cv.Optional(CONF_DIT_DURATION, default="100"): cv.positive_int,
            cv.Optional(CONF_ON_FINISHED): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        FinishedTrigger
                    ),
                }
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_OUTPUT in config:
        out = await cg.get_variable(config[CONF_OUTPUT])
        cg.add(var.set_output(out))

    cg.add(var.set_dit_duration(config[CONF_DIT_DURATION]))

    for conf in config.get(CONF_ON_FINISHED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)


@automation.register_action(
    "morse_code.start",
    StartAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(CONF_ID): cv.use_id(MorseCodeComponent),
            cv.Required(CONF_TEXT): cv.templatable(cv.string),
        },
        key=CONF_TEXT,
    ),
)
async def morse_code_start_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_value(template_))
    return var


@automation.register_action(
    "morse_code.stop",
    StopAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(MorseCodeComponent),
        }
    ),
)
async def morse_code_stop_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_condition(
    "morse_code.is_running",
    IsRunningCondition,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(MorseCodeComponent),
        }
    ),
)
async def morse_code_is_running_to_code(config, condition_id, template_arg, args):
    var = cg.new_Pvariable(condition_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
