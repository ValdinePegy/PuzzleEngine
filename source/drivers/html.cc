#include "web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

extern "C" int main()
{
  UI::Initialize();

  doc << "Testing!";
  
  return 0;
}
