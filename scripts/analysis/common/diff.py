import json


class NormalizerException:
    pass


class Reason:
    def __init__(self, obj1, obj2):
        self.obj1_typename = type(obj1).__name__
        self.obj2_typename = type(obj2).__name__

    def get_name(self):
        return type(self).__name__


class InvalidControlCharacter(Reason, NormalizerException):
    pass


class InvalidOutput(Reason):
    pass


class ParserErrorOne(Reason):
    def __init__(self, exc):
        super().__init__(None, None)
        self.msg = exc.msg

    def get_name(self):
        return type(self).__name__ + "(" + self.msg + ")"


class ParserErrorBoth(Reason):
    def __init__(self, obj1, obj2):
        super().__init__(None, None)

        if isinstance(obj1, json.decoder.JSONDecodeError):
            self.msg1 = obj1.msg
        else:
            self.msg1 = ""

        if isinstance(obj2, json.decoder.JSONDecodeError):
            self.msg2 = obj2.msg
        else:
            self.msg2 = ""

        self.msg1, self.msg2 = sorted([self.msg1, self.msg2])

    def get_name(self):
        return type(self).__name__ + "(" + self.msg1 + "," + self.msg2 + ")"


class UnexpectedNone(Reason):
    pass


class TypeMismatch(Reason):
    def __init__(self, obj1, obj2):
        self.obj1_typename, self.obj2_typename = sorted(
            [type(obj1).__name__, type(obj2).__name__]
        )

    def get_name(self):
        return (
            type(self).__name__
            + "("
            + self.obj1_typename
            + ","
            + self.obj2_typename
            + ")"
        )


class DictLengthMismatch(Reason):
    def __init__(self, reasons):
        super().__init__(None, None)
        self.reasons = sorted([type(reason).__name__ for reason in reasons])

    def get_name(self):
        return type(self).__name__ + "(" + ",".join(self.reasons) + ")"


class DictKeysMismatch(Reason):
    def __init__(self, reasons):
        super().__init__(None, None)
        self.reasons = sorted([type(reason).__name__ for reason in reasons])

    def get_name(self):
        return type(self).__name__ + "(" + ",".join(self.reasons) + ")"


class ListSortMismatch(Reason):
    pass


class DictTrailingDotInKeyError(Reason):
    pass


class DictTrailingParanthesesKeyError(Reason):
    pass


class DictEmptyKeyError(Reason):
    pass


class DictZeroCharacter(Reason):
    pass


class DictValueNull(Reason):
    pass


class DictUnicodeKeyError(Reason):
    pass


class ListLengthMismatch(Reason):
    pass


class DictDoubleBackslashKey(Reason):
    pass


class Unequal(Reason):
    pass


class IntError(Reason):
    pass


class FloatError(Reason):
    pass


class StringError(Reason):
    pass


class StringU0000Error(Reason):
    pass


class IEEE754LargeNumberError(Reason):
    pass


class FloatRoundingError(Reason):
    pass


class StringUnicodeError(Reason):
    pass


class DoubleBackslashString(Reason):
    pass


class DictDuplicateKeyError(Reason):
    pass


def has_trailing_dot_key(strings):
    return any((s.endswith(".") for s in strings))


def has_trailing_para_key(strings):
    return any((s.endswith("]") for s in strings))


def has_empty_key(strings):
    return any((len(s) == 0 for s in strings))


def has_unicode_key(strings):
    return any(not s.isascii() for s in strings)


def has_double_backslash(strings):
    return any("\\\\" in s for s in strings)


def has_null_value(values):
    return any(v is None for v in values)


def hash_mutable(e):
    if isinstance(e, list) or isinstance(e, dict):
        return hash(str(e))
    else:
        return hash(hash(e) + hash(type(e)))


def has_u0000(strings):
    return any("\x00" in s for s in strings)


def are_equal(obj1, obj2, in_file):
    if obj1 is None and obj2 is None:
        return True, None

    elif obj1 is None or obj2 is None:
        return False, UnexpectedNone(obj1, obj2)

    elif type(obj1) != type(obj2):
        return False, TypeMismatch(obj1, obj2)

    elif isinstance(obj1, dict):
        if len(obj1.keys()) != len(obj2.keys()) or set(obj1.keys()) != set(obj2.keys()):
            reasons = []
            if has_trailing_dot_key(obj1.keys()) or has_trailing_dot_key(obj2.keys()):
                reasons.append(DictTrailingDotInKeyError(obj1, obj2))
            if has_trailing_para_key(obj1.keys()) or has_trailing_para_key(obj2.keys()):
                reasons.append(DictTrailingParanthesesKeyError(obj1, obj2))
            if has_empty_key(obj1.keys()) or has_empty_key(obj2.keys()):
                reasons.append(DictEmptyKeyError(obj1, obj2))
            if has_null_value(obj1.values()) or has_null_value(obj2.values()):
                reasons.append(DictValueNull(obj1, obj2))
            if has_unicode_key(obj1.keys()) or has_unicode_key(obj2.keys()):
                reasons.append(DictUnicodeKeyError(obj1, obj2))
            if has_double_backslash(obj1.keys()) or has_double_backslash(obj2.keys()):
                reasons.append(DictDoubleBackslashKey(obj1, obj2))
            if has_u0000(obj1.keys()) or has_u0000(obj2.keys()):
                reasons.append(DictZeroCharacter(obj1, obj2))

            if len(obj1.keys()) != len(obj2.keys()):
                return False, DictLengthMismatch(reasons)

            elif set(obj1.keys()) != set(obj2.keys()):
                return False, DictKeysMismatch(reasons)

            else:
                assert False

        else:
            for key in obj1:
                ok, error = are_equal(obj1[key], obj2[key], in_file)

                if ok:
                    continue

                # with open(in_file) as f:
                #    content = f.read()

                # regex = '"' + re.escape(key) + '"\\s*:\\s*'
                # m = re.findall(regex, content)
                # if m is not None and len(m) >= 2:
                #    return False, DictDuplicateKeyError(obj1, obj2)
                # else:
                #    return ok, error

            return True, None

    elif isinstance(obj1, list):
        if len(obj1) != len(obj2):
            return False, ListLengthMismatch(obj1, obj2)

        else:
            # (1) If the list is equal (without sorting), return equal
            # ( ) If the list is unequal, sort it:
            #   (2) If the list is still unequal, return original error
            #   (3) If the list is now equal, return sort error
            for o1, o2 in zip(obj1, obj2):
                ok, first_error = are_equal(o1, o2, in_file)
                if not ok:
                    break
            else:
                # (1)
                return True, None

            o1_hashed = [hash_mutable(e) for e in obj1]
            o2_hashed = [hash_mutable(e) for e in obj2]

            for el1, el2 in zip(sorted(o1_hashed), sorted(o2_hashed)):
                if el1 != el2:
                    # (2)
                    return False, first_error

            # (3)
            return False, ListSortMismatch(obj1, obj2)

    elif obj1 != obj2:
        if isinstance(obj1, float) and isinstance(obj2, float):
            if abs(obj1 - obj2) < 0.0001:
                return False, FloatRoundingError(obj1, obj2)
            else:
                return False, FloatError(obj1, obj2)

        elif isinstance(obj1, int) and isinstance(obj2, int):
            if max(abs(obj1), abs(obj2)) >= 2**53:
                return False, IEEE754LargeNumberError(obj1, obj2)
            else:
                return False, IntError(obj1, obj2)

        elif isinstance(obj1, str) and isinstance(obj2, str):
            if has_double_backslash([obj1, obj2]):
                return False, DoubleBackslashString(obj1, obj2)
            if has_u0000([obj1, obj2]):
                return False, StringU0000Error(obj1, obj2)
            elif obj1.isascii() and obj2.isascii():
                return False, StringError(obj1, obj2)
            else:
                # TODO: Not an error. \u000a == "\n"
                return False, StringUnicodeError(obj1, obj2)

        return False, Unequal(obj1, obj2)

    return True, None
