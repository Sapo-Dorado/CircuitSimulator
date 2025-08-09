#pragma once

#include <map>
#include <string>
#include <vector>

struct Wire;

std::map<std::string, std::vector<long long>> simulate(const std::vector<const Wire*>& targets, int cycles, bool restore_state = true);
std::map<std::string, std::vector<long long>> simulate(const Wire& target, int cycles, bool restore_state = true);


