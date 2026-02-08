#ifndef AUTO_COMPLETION_H
#define AUTO_COMPLETION_H

#include <string>
#include <vector>

std::vector<std::string> CompleteCommandNames(std::string_view prefix);

#endif //AUTO_COMPLETION_H
