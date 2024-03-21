#include "target-python.h"

#include <iostream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern "C" {
void __sanitizer_cov_8bit_counters_init(uint8_t* start, uint8_t* stop);
void __sanitizer_cov_pcs_init(uint8_t* pcs_beg, uint8_t* pcs_end);
}

/*
 * TODO: If we use an in-process approach, we share state between multiple
 * Python projects. We therefore have to be sure that there is no shared global
 * state between the projects... Implement anyway and compare runtime to a
 * 'normal' run. Maybe this is vastly faster.
 */

std::string GetCoverageSymbolsLocation()
{
    Dl_info dl_info;
    if (!dladdr((void*)&__sanitizer_cov_8bit_counters_init, &dl_info)) {
        return "<Not a shared object>";
    }
    return (dl_info.dli_fname);
}

class PythonTarget::PythonTargetImpl
{
  public:
    PythonTargetImpl(std::string id, std::string cmd,
                     std::vector<std::string> args,
                     std::vector<std::string> env);
    ~PythonTargetImpl();

    void run(const Package& package);
    Output waitForTarget();

  private:
    PyObject* targetFunc;
    PyObject* targetModule;

    PyObject* atherisCore;
    PyObject* allocateCountersAndPcsFunc;
    PyObject* testOneInputFunc;

    PyObject* atheris;
    PyObject* setupFunc;
};

PyObject* loadModule(const char* name)
{
    PyObject* module_name = PyUnicode_DecodeFSDefault(name);
    if (module_name == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to decode \"%s\"\n", name);
        throw "up";
    }

    auto py_module = PyImport_Import(module_name);
    Py_DECREF(module_name);

    if (py_module == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", name);
        throw "up";
    }

    return py_module;
}

PyObject* getAttrString(PyObject* module, const char* funcName)
{
    PyObject* func = PyObject_GetAttrString(module, funcName);

    if (func == NULL) {
        if (PyErr_Occurred()) {
            PyErr_Print();
        }
        fprintf(stderr, "Cannot find function \"%s\"\n", funcName);
        throw "up";
    }

    return func;
}

PyObject* getAttrStringFunc(PyObject* module, const char* funcName)
{
    PyObject* func = PyObject_GetAttrString(module, funcName);

    if (func == NULL || !PyCallable_Check(func)) {
        if (PyErr_Occurred()) {
            PyErr_Print();
        }
        fprintf(stderr, "Cannot find function \"%s\"\n", funcName);
        throw "up";
    }

    return func;
}

void setAttrString(PyObject* obj, const char* attr, PyObject* val)
{
    if (PyObject_SetAttrString(obj, attr, val) < 0) {
        if (PyErr_Occurred()) {
            PyErr_Print();
        }
        fprintf(stderr, "Cannot set function \"%s\"\n", attr);
        throw "up";
    }
}

PythonTarget::PythonTargetImpl::PythonTargetImpl(std::string id,
                                                 std::string cmd,
                                                 std::vector<std::string> args,
                                                 std::vector<std::string> env)
{
    const char* moduleName = args[1].c_str();
    const char* funcName = args[2].c_str();

    std::cout << "************\nINIT PYTHON INTERPRETER\n************"
              << std::endl;
    std::cout << "ModuleName: " << moduleName << std::endl;
    std::cout << "FuncName: " << funcName << std::endl;

    PyPreConfig preConfig;
    PyPreConfig_InitPythonConfig(&preConfig);

    Py_PreInitialize(&preConfig);
    size_t size;
    wchar_t* programName = Py_DecodeLocale(moduleName, &size);
    if (programName == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to decode \"%s\"\n", moduleName);
        throw "up";
    }

    Py_SetProgramName(programName);
    PyMem_RawFree(programName);

    std::cout << "IS INITIALIZED: " << Py_IsInitialized() << std::endl;

    Py_Initialize();
    PySys_SetArgvEx(0, NULL, 1);

    {
        this->atheris = loadModule("atheris");
        this->atherisCore = loadModule("atheris.core_with_libfuzzer");

        /*
         * After loading atheris / atheris-core we immediately initialize the
         * trace functions so that we do not miss the registration of any edge.
         * In atheris, this is done after the Setup() call in the Fuzz()
         * function, because (I think) they have to make sure that the libFuzzer
         * functions are available. Since our program provides the libFuzzer
         * callbacks we should be able to safely skip this.
         */
        // clang-format off
        setAttrString(this->atheris, "Mutate", getAttrStringFunc(this->atherisCore, "Mutate"));
        setAttrString(this->atheris, "_trace_cmp", getAttrStringFunc(this->atherisCore, "_trace_cmp"));
        setAttrString(this->atheris, "_trace_regex_match", getAttrStringFunc(this->atherisCore, "_trace_regex_match"));
        setAttrString(this->atheris, "_trace_branch", getAttrStringFunc(this->atherisCore, "_trace_branch"));
        setAttrString(this->atheris, "_reserve_counter", getAttrStringFunc(this->atherisCore, "_reserve_counter"));
        // clang-format on

        /*
         * Now, we can load the user's target function and the instrumentation
         * should be added correctly
         */
        this->targetModule = loadModule(moduleName);
        this->targetFunc = getAttrStringFunc(targetModule, funcName);

        this->allocateCountersAndPcsFunc =
            getAttrStringFunc(this->atherisCore, "AllocateCountersAndPcs");
        this->testOneInputFunc =
            getAttrStringFunc(this->atherisCore, "TestOneInput");

        this->setupFunc = getAttrStringFunc(this->atheris, "Setup");

        PyObject* tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, PyList_New(0));
        PyTuple_SET_ITEM(tuple, 1, this->setupFunc);
        PyObject_Call(this->setupFunc, tuple, NULL);
        Py_DECREF(tuple);
    }
}

PythonTarget::PythonTargetImpl::~PythonTargetImpl()
{
    Py_XDECREF(this->targetFunc);
    Py_DECREF(this->targetModule);

    Py_XDECREF(this->allocateCountersAndPcsFunc);
    Py_XDECREF(this->testOneInputFunc);
    Py_DECREF(this->atherisCore);

    Py_XDECREF(this->setupFunc);
    Py_DECREF(this->atheris);

    Py_FinalizeEx();
}

void PythonTarget::PythonTargetImpl::run(const Package& package)
{
    // PyObject_CallNoArgs(this->allocateCountersAndPcsFunc);

    /*
    const auto alloc = AllocateCountersAndPcs();
    if (alloc.counters_start && alloc.counters_end) {
        __sanitizer_cov_8bit_counters_init(alloc.counters_start,
                                           alloc.counters_end);
    }
    if (alloc.pctable_start && alloc.pctable_end) {
        __sanitizer_cov_pcs_init(alloc.pctable_start, alloc.pctable_end);
    }
    */

    PyObject* tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(
        tuple, 0,
        PyBytes_FromStringAndSize((char*)package.payload, package.payloadSize));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromUnsignedLong(package.payloadSize));
    PyObject_Call(this->targetFunc, tuple, NULL);
    Py_DECREF(tuple);

    /*

    pArgs = PyTuple_New(argc - 3);
    for (i = 0; i < argc - 3; ++i) {
        pValue = PyLong_FromLong(atoi(argv[i + 3]));
        if (!pValue) {
            Py_DECREF(pArgs);
            Py_DECREF(this->pModule);
            fprintf(stderr, "Cannot convert argument\n");
            return 1;
        }
        // pValue reference stolen here
        PyTuple_SetItem(pArgs, i, pValue);
    }
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    if (pValue != NULL) {
        printf("Result of call: %ld\n", PyLong_AsLong(pValue));
        Py_DECREF(pValue);
    } else {
        Py_DECREF(pFunc);
        Py_DECREF(this->pModule);
        PyErr_Print();
        fprintf(stderr, "Call failed\n");
        return 1;
    }
    */
}

Target::Output PythonTarget::PythonTargetImpl::waitForTarget()
{
    return Target::Output();
}

PythonTarget::PythonTarget(std::string id, std::string cmd,
                           std::vector<std::string> args,
                           std::vector<std::string> env)
    : id(id), impl(std::make_unique<PythonTargetImpl>(id, cmd, args, env))
{
}

PythonTarget::~PythonTarget() {}

std::string PythonTarget::getId() const { return this->id; }

void PythonTarget::run(const Package& package) { this->impl->run(package); }

Target::Output PythonTarget::waitForTarget()
{
    return this->impl->waitForTarget();
}

void PythonTarget::reset() {}