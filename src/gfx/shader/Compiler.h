#pragma once

#include <vector>
#include <shaderc/shaderc.hpp>

namespace gfx::shader {
class Compiler {
 public:
  struct MacroDefinition {
    std::string name;
    std::string value;
  };


 private:
  shaderc::Compiler compiler_;
  std::vector<MacroDefinition> globalDefinitions_;
};
}  // namespace gfx::shader
