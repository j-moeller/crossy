import dis

from fuzzer_bridge.atheris.counter import inc_counter

user_callback_running = False


def _trace_cmp(cmp1, cmp2, opid, counter, is_const):
    # TODO: Fill bitmap with values
    if dis.cmp_op[opid] == "<":
        return cmp1 < cmp2
    elif dis.cmp_op[opid] == "<=":
        return cmp1 <= cmp2
    elif dis.cmp_op[opid] == "==":
        return cmp1 == cmp2
    elif dis.cmp_op[opid] == ">=":
        return cmp1 >= cmp2
    elif dis.cmp_op[opid] == ">":
        return cmp1 > cmp2
    elif dis.cmp_op[opid] == "!=":
        return cmp1 != cmp2

    assert False


def _trace_branch(idx):
    if not user_callback_running:
        return

    inc_counter(idx)


def set_user_callback_running(value):
    global user_callback_running
    user_callback_running = value
