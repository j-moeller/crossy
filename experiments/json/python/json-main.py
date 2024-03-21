def foo(data: bytes, size: int):
    return json.dumps(json.loads(data.decode("utf-8"))).encode("utf-8")


if __name__ == "__main__":
    from fuzzer_bridge import fuzz, atheris

    with atheris.instrument_imports(include=["json"]):
        import json

    fuzz(foo)
