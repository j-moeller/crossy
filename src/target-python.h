#ifndef TARGET_PYTHON
#define TARGET_PYTHON

#include <memory>
#include <vector>

#include "targets.h"

class PythonTarget : public Target
{
  public:
    PythonTarget(std::string id, std::string cmd, std::vector<std::string> args,
                 std::vector<std::string> env);
    ~PythonTarget();

    std::string getId() const;
    void run(const Package& package);
    Output waitForTarget();
    void reset();

  private:
    std::string id;

    class PythonTargetImpl;
    std::unique_ptr<PythonTargetImpl> impl;
};

#endif