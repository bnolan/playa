#pragma once

#include <string>

class User{
public:
  int32_t id_;
  std::string name_;
  User(int id);
  std::string inspect();
};
